#pragma once
#include <sc2api/sc2_api.h>
#include "../auxiliary/helpers.hpp"
#include "unitwrapper.hpp"
#include "../auxiliary/squadmanager.hpp"
#include "../auxiliary/visiblemap.hpp"
#include "../auxiliary/weapongrid.hpp"
#include "../auxiliary/debugging.hpp"

//#define MOVSAFELY_DEBUG
#define TARGET_DEBUG

constexpr float DIVISION_MOVSAFELY_CELLS = 1; //how far apart getPathDamage checks in cells
constexpr int COOLDOWN_FRAMES = 5;
constexpr int ARMYUNIT_KITE_TOLERANCE_EPSILON_FFRAMES = 5;
constexpr int MOVSAFELY_MAINCHECK_BISECTIONS = 4;

#ifdef BUILD_FOR_LADDER
constexpr int POINT_CHECKS_DEFAULT = 10;
#else
constexpr int POINT_CHECKS_DEFAULT = 5;
#endif

class ArmyUnit : public UnitWrapper{
private:
public:

    enum ActiveActionType {
        NOTHING,
        MOV,
        ATK_MOV,
        ATK,
    };

    ActiveActionType activeActionType;

    SquadManager::Squad* squad;

    float prevMoveLocationDPS = 0.0F;
    Point2D moveLocation;
    Point2D targetLocation;

    float localDPS = 0.0F;

    UnitWrapperPtr target;

    int8_t cooldownFrames = 0;

    bool safetyMode = false;

    ArmyUnit(const Unit* unit, UnitTypeID sType, SquadManager::Squad* squad_) : UnitWrapper(unit, sType), squad(squad_), activeActionType(NOTHING){
        squad->squadMainStates[unit->tag] = 'u';
        squad->unitStates[unit->tag] = ' ';
    }

    virtual void atk(Agent* const agent, Point2D point) {
        const Unit* unit = getReturn(agent);
        if (unit->orders.size() == 0 || unit->orders[0].ability_id != ABILITY_ID::ATTACK || unit->orders[0].target_pos != point) {
            agent->Actions()->UnitCommand(self, ABILITY_ID::ATTACK, point);
            moveLocation = point;
            activeActionType = ATK_MOV;
        }
    }

    virtual void atk(Agent* const agent, UnitWrapperPtr target) {
        const Unit* unit = getReturn(agent);
        if (unit->orders.size() == 0 || unit->orders[0].ability_id != ABILITY_ID::ATTACK || unit->orders[0].target_unit_tag != target->self) {
            agent->Actions()->UnitCommand(self, ABILITY_ID::ATTACK, target->self);
            moveLocation = Point2D();
            activeActionType = ATK;
        }
    }

    virtual void atkmov(Agent* const agent, Point2D point) {
        if (getReturn(agent)->weapon_cooldown > 0) {
            mov(agent, point);
        }
        else {
            atk(agent, point);
        }
    }

    virtual void mov(Agent* const agent, Point2D point, bool queue = false) {
        const Unit* unit = getReturn(agent);
        if (unit->orders.size() == 0 || unit->orders[0].ability_id != ABILITY_ID::GENERAL_MOVE || unit->orders[0].target_pos != point) {
            agent->Actions()->UnitCommand(self, ABILITY_ID::GENERAL_MOVE, point, queue);
            moveLocation = point;
            activeActionType = MOV;
        }
    }

    inline float getEnemyDPS(Point2D point, float extraRadius, WeaponGrid::relevantTargetDamageInfo selfTargetInfo, Agent* const agent) {
        return WeaponGrid::getRadiusAvgDPS(point, radius(agent) + extraRadius, selfTargetInfo, agent);
    }

    inline float getEnemyDPS(Point2D point, float extraRadius, Agent* const agent) {
        WeaponGrid::relevantTargetDamageInfo selfTargetInfo = WeaponGrid::wrapToTargetInfo(shared_from_this(), agent);
        return getEnemyDPS(point, extraRadius, selfTargetInfo, agent);
    }

    //avg damage of path plus a 2x damage spike at the endpoint
    float getPathDamage(std::vector<Point2D> path, Agent* const agent, float extraRadiusAlong = 0.5, float extraRadiusEnd = 3.5) {
        float len = PrimordialStar::getPathLength(path);
        std::vector<Point2D> checks = PrimordialStar::stepPointsAlongPath(path, DIVISION_MOVSAFELY_CELLS);
        float dps = 0;
        WeaponGrid::relevantTargetDamageInfo selfTargetInfo = WeaponGrid::wrapToTargetInfo(shared_from_this(), agent);
        for (int i = 0; i < checks.size(); i++) {
            float addDps = getEnemyDPS(checks[i], extraRadiusAlong, agent);
            dps += addDps;
#ifdef MOVSAFELY_DEBUG
            Color c;

            constexpr int maxDam = 32;
            constexpr int mult = 255 / maxDam;

            if (addDps < maxDam) {
                c = { 255, (uint8_t)(addDps * mult), 255 };
            }
            else {
                c = { 255, 255, 255 };
            }
            DebugSphere(agent, AP3D(checks[i]), radius(agent) + 0.5, c);
#endif
        }
        dps /= checks.size(); //PrimordialStar::getPathLength(path);
        dps += getEnemyDPS(path[path.size() - 1], extraRadiusEnd, agent) * 2;
#ifdef MOVSAFELY_DEBUG
        if (path.size() > 0) {
            for (int i = 0; i < path.size() - 1; i++) {
                DebugLine(agent, AP3D(path[i]) + Point3D{ 0, 0, 1 }, AP3D(path[i + 1]) + Point3D{ 0, 0, 1 });
            }
        }
        else {
            printf("");
            DebugSphere(agent, AP3D(p), 1, Colors::Red);
        }
#endif
        return dps;
    }

    float getPathDamage(Point2D p, Agent* const agent, float extraRadiusAlong = 0.5, float extraRadiusEnd = 3.5) {
        std::vector<Point2D> path = getPathUniversal(p, agent);
        return getPathDamage(path, agent, extraRadiusAlong, extraRadiusEnd);
    }

    virtual void movSafely(Agent* const agent, Point2D point, int attempts, float searchRadius, float toleranceRadius = 1.5, bool queue = false) {
        //agent->Actions()->UnitCommand(self, ABILITY_ID::GENERAL_MOVE, point);
        Profiler movProfiler("movSafely");
        Point2D solution;
        float damage = FLT_MAX;
        float distance2 = FLT_MAX;
        const Unit* unit = getReturn(agent);
        if (unit->orders.size() > 0 && unit->orders[0].target_pos != Point2D()) {
            //solution = get(agent)->orders[0].target_pos;
            std::vector<Point2D> path = getPathUniversal(unit->orders[0].target_pos, agent);
            damage = getPathDamage(path, agent, 0.5, toleranceRadius);//WeaponGrid::getRadiusAvgDPS(get(agent)->orders[0].target_pos, radius(agent)+3.5, shared_from_this(), agent);
            distance2 = PrimordialStar::getPathLength(path);
            if (unit->is_selected) {
                DebugPath(agent, path, Colors::Yellow);
                DebugText(agent, strprintf("%.2f", damage), AP3D(unit->orders[0].target_pos), Colors::Yellow);
                DebugSphere(agent, AP3D(unit->orders[0].target_pos), 0.5, Colors::Yellow);
            }

        }
        movProfiler.midLog("movSafely.setup");
        std::vector<Point2D> path = getPathUniversal(point, agent);
        float l = PrimordialStar::getPathLength(path);
        std::vector<Point2D> ptsToCheck;
        bool quickEnd = true;
        if (l != 0) {
            ptsToCheck = PrimordialStar::stepPointsAlongPath(path, DIVISION_MOVSAFELY_CELLS);
            for (int i = 0; i < ptsToCheck.size(); i++) {
                float dmag = getEnemyDPS(ptsToCheck[i], 0.5, agent);
                if (dmag != 0) {
                    if (i == 0) {
                        quickEnd = false;
                    }
                    else {
                        damage = dmag;
                        distance2 = getPathLength(ptsToCheck[i - 1], point, agent);;
                        solution = ptsToCheck[i - 1];
                        break;
                    }
                }
            }
            if (quickEnd) {
                solution = point;
                distance2 = 0;
            }
        }
        if (quickEnd) {
            if (unit->is_selected) {
                DebugPath(agent, path, Colors::Yellow);
                DebugText(agent, "0", AP3D(solution), Colors::Yellow);
                DebugSphere(agent, AP3D(solution), 0.5, Colors::Yellow);
                DEBUG_DETAILTRIGGER = true;
            }
        }
        else {
            for (int i = 0; i < attempts; i++) {
                Point2D p = Aux::getRandomPointRadius(pos(agent), searchRadius, 1.0F);
                if (!Aux::isPathable(p) && !isFlying(agent)) {
                    i--;
                    continue;
                }
                float endDMG = getEnemyDPS(p, toleranceRadius, agent) * 2;
                if (endDMG > damage) {
                    //i--;
                    continue;
                }
                std::vector<Point2D> prospath = getPathUniversal(p, agent);
                float dmg = getPathDamage(prospath, agent, 0.5, toleranceRadius);
                if (unit->is_selected) {
                    DebugPath(agent, prospath, Colors::Green);
                    DebugText(agent, strprintf("%.2f", dmg), AP3D(p), Colors::Green);
                    DebugSphere(agent, AP3D(p), 0.5, Colors::Green);
                    DEBUG_DETAILTRIGGER = true;
                }
                //WeaponGrid::getRadiusAvgDPS(p, radius(agent) + 3.5, shared_from_this(), agent);
                if (damage > dmg) {
                    damage = dmg;
                    distance2 = getPathLength(p, point, agent);
                    solution = p;
                }
                else if (damage == dmg) {
                    float dist2 = getPathLength(p, point, agent);
                    if (dist2 < distance2) {
                        damage = dmg;
                        distance2 = dist2;
                        solution = p;
                    }
                }
            }
        }
        
        movProfiler.midLog("movSafely.findSpot");
        if (solution != Point2D()) {
            mov(agent, solution, queue);
        }
        movProfiler.midLog("movSafely.mov");
        //TODO: add path itself as a weight (what does this mean?) (does it mean to include the length of the path in the calculations? because that is done)
    }

    virtual void findTarget(Agent* const agent, Aux::ExtraWeapon& weapon, float& dmagToTarget) {
        float priority = 0; //fine because condition is also target == nullptr
        for (auto it = squad->squadTargets.begin(); it != squad->squadTargets.end(); it++) {
            if (squad->squadTargetDamage[(*it)->self] < (*it)->getHealth(agent)) {

                if (getReturn(agent)->is_selected) {
                    printf("");
                }

                float p = squad->getEnemyUnitPriority(*it, agent);
                float damagePerHit = 0;
                UnitTypeID selfType = getActualType(agent);

                bool hittable = false;
                for (int i = 0; i < WeaponGrid::unitDamageSources[selfType].size(); i++) {
                    int index = WeaponGrid::unitDamageSources[selfType][i].weaponIndex;
                    const Aux::ExtraWeapon w = WeaponGrid::getSelfWeaponFromIndex(index, getAttackUpgradeLevel(agent));
                    float damage = WeaponGrid::DamageCalculation(w, *it, agent);
                    if (damage > damagePerHit) {
                        damagePerHit = damage;
                        weapon = w;
                        hittable = true;
                    }
                }
                if (!hittable) { //making sure old weapon isn' applied to another enemy that is not hittable
                    DebugText(agent, "NA", (*it)->pos3D(agent) + Point3D{ 0,0,1 });
                    continue;
                }
                float distanceToEnemy = Distance2D((*it)->pos(agent), pos(agent));
                //float damageLost = (distanceToEnemy > weapon.range) ? (((distanceToEnemy - weapon.range) / Aux::getStats(getActualType(agent), agent).movement_speed) * (damagePerHit / weapon.speed)) : 0.0F;
                //p -= damageLost / 50; //every 50 damage lost takes a unit down one priority peg;
                float range = (weapon.range + (*it)->radius(agent) + radius(agent));

                if (distanceToEnemy > range) {
                    p -= 4 * (distanceToEnemy - range); //every one unit outside of range a unit is, it gets taken down four priority pegs
                }
                //p += damagePerHit / weapon->speed; //each DPS, up one peg of priority

                float enemyHealth = (*it)->getHealth(agent);
                if (squad->squadTargetDamage.find((*it)->self) != squad->squadTargetDamage.end()) {
                    enemyHealth -= squad->squadTargetDamage[(*it)->self];
                }
                int hitsToEnemyDeath = std::ceil(enemyHealth / damagePerHit);
                float dtToEnemyDeath_natsec = hitsToEnemyDeath * weapon.speed;
                p /= dtToEnemyDeath_natsec;

                if (enemyHealth < damagePerHit) {
                    p *= 2;
                }
                if (target == nullptr || p > priority) {
                    priority = p;
                    target = *it;
                    dmagToTarget = damagePerHit;
                }

                if (getReturn(agent)->is_selected) {
                    //DebugText(agent, strprintf("%f", p), (*it)->pos3D(agent) + Point3D{ 0,0,1 }, Colors::Blue);
                    DebugText(agent, strprintf("%f", p), (*it)->pos3D(agent) + Point3D{ 0,0,1 });
                }
            }
        }
    }


    virtual void hasNoTargetAction_Attack(Agent* const agent) {
        if (squad->isWithinRadius(pos(agent), agent)) {
            movSafely(agent, squad->targetPosition, POINT_CHECKS_DEFAULT, 7);
        }
        else {
            movSafely(agent, squad->getCorePosition(agent), POINT_CHECKS_DEFAULT, 7);
        }
        cooldownFrames = COOLDOWN_FRAMES;
    }

    virtual void hasTargetAction_Attack(Agent* const agent, const Aux::ExtraWeapon& weapon, bool radiusOfSafety) {
#ifdef TARGET_DEBUG
        DebugLine(agent, pos3D(agent) + Point3D{ 0,0,1 }, target->pos3D(agent) + Point3D{ 0,0,1 }, Colors::Teal);
        DebugLine(agent, pos3D(agent) + Point3D{ 0,0,1.1 }, target->pos3D(agent) + Point3D{ 0,0,1.1 }, Colors::Yellow);
#endif
        float distToAtkEnemy = std::max(0.0F, Distance2D(pos(agent), target->pos(agent)) - (weapon.range + target->radius(agent) + radius(agent)));
        float dTtoEnemy_frames = distToAtkEnemy / (Aux::getStats(getActualType(agent), agent)->movement_speed / native_fps);

        if (dTtoEnemy_frames + ARMYUNIT_KITE_TOLERANCE_EPSILON_FFRAMES >= getReturn(agent)->weapon_cooldown) {
            if (safetyMode && !radiusOfSafety) {
                movSafely(agent, squad->getCorePosition(agent), 10, 7, 3.5);
            }
            else {
                atk(agent, target);
            }
            cooldownFrames = COOLDOWN_FRAMES;
            if (getStorageType() == UNIT_TYPEID::PROTOSS_COLOSSUS) {
                cooldownFrames = 1;
                //movSafely(agent, squad->getCorePosition(agent), 10, 7, 3.5, true);
            }
        }
        else {
            if (squad->isWithinRadius(pos(agent), agent)) {
                movSafely(agent, target->pos(agent), 10, 7);
            }
            else {
                movSafely(agent, squad->getCorePosition(agent), 10, 7);
            }

            cooldownFrames = COOLDOWN_FRAMES;
            if (COOLDOWN_FRAMES > getReturn(agent)->weapon_cooldown) {
                cooldownFrames = (uint8_t)getReturn(agent)->weapon_cooldown;
            }
        }

        if (target->getReturn(agent) == nullptr) {
            if (safetyMode) {
                movSafely(agent, squad->getCorePosition(agent), 10, 7);
            }
            else {
                atk(agent, target->pos(agent));
            }
            cooldownFrames = COOLDOWN_FRAMES;
        }
        else {

        }
    }

    virtual void action_Attack(Agent* const agent, const Aux::ExtraWeapon& weapon, bool radiusOfSafety) {
        if (target != nullptr) {
            hasTargetAction_Attack(agent, weapon, radiusOfSafety);
        }
        else {
            hasNoTargetAction_Attack(agent);
        }
    }

    //returns true if cooldown is not met
    virtual bool cooldownCheckUpdate(Agent* const agent) {
        float moveLocationDPS = WeaponGrid::getRadiusAvgDPS(moveLocation, radius(agent) + 3.5, WeaponGrid::wrapToTargetInfo(shared_from_this(), agent), agent);

        if (cooldownFrames > 0) {
            if ((target != nullptr && target->isDead()) || getReturn(agent)->orders.size() == 0) {
                cooldownFrames = 0;
            }
            else if (moveLocationDPS > prevMoveLocationDPS) {
                cooldownFrames /= 2;
                return true;
            }
            else {
                cooldownFrames--;
                return true;
            }
        }

        prevMoveLocationDPS = moveLocationDPS;
        return false;
    }

    virtual void executeAttack(Agent* const agent) {
        FUNCTION_LOG();
        Profiler armyUnitProfiler("armyu(atk)");

        if (getReturn(agent)->is_selected) {
            WeaponGrid::showDamageGrid(shared_from_this(), agent);
            std::vector<WeaponGrid::DamageSourceID> weapons = WeaponGrid::getWeapons(getActualType(agent));
            if (weapons.size() > 0) {
                if (getReturn(agent)->weapon_cooldown != 0) {
                    printf("");
                }
                float cd_frames = WeaponGrid::getSelfWeaponFromIndex(weapons[0].weaponIndex, getAttackUpgradeLevel(agent)).speed * native_fps;
                DebugText(agent, Aux::barToString(1.0F - getReturn(agent)->weapon_cooldown / cd_frames), pos3D(agent) + Point3D{ 0,0,2 });
            }
        }

        if (cooldownCheckUpdate(agent)) {
            return;
        }

        moveLocation = Point2D{ -1, -1 };
        target = nullptr;
        Aux::ExtraWeapon weapon = WeaponGrid::emptyWeapon;
        float dmagToTarget = 0;

        bool radiusOfSafety = (WeaponGrid::getRadiusAvgDPS(pos(agent), radius(agent) + 3.5, WeaponGrid::wrapToTargetInfo(shared_from_this(), agent), agent) == 0.0F);
        //if (!safetyMode && getShields(agent) < 20.0F) {
        //    safetyMode = true;
        //}
        //else if (safetyMode && getShields(agent) > (getShieldsMax(agent) - 20.0F)) {
        //    safetyMode = false;
        //}

        findTarget(agent, weapon, dmagToTarget);

        if (getReturn(agent)->is_selected && radiusOfSafety) {
            DebugBox(agent, pos3D(agent) + Point3D{ -0.2,-0.2, 1 }, pos3D(agent) + Point3D{ 0.2,0.2,1.4 }, Colors::Green);
        }

        armyUnitProfiler.midLog("armyu(atk).findTarget");

        if (squad->squadMainStates[self] == 'u') {
            if (getReturn(agent)->weapon_cooldown > 0) {
                mov(agent, squad->getCorePosition(agent));
            }
            else {
                atk(agent, squad->getCorePosition(agent));
            }
            cooldownFrames = COOLDOWN_FRAMES;
            targetLocation = squad->getCorePosition(agent);
        }
        else if (squad->squadMainStates[self] == 'j') {
            targetLocation = squad->targetPosition;
            action_Attack(agent, weapon, radiusOfSafety);
        }
        if (target != nullptr && activeActionType == ATK) {
            if (squad->squadTargetDamage.find(target->self) != squad->squadTargetDamage.end()) {
                squad->squadTargetDamage[target->self] += dmagToTarget;
            }
            else {
                squad->squadTargetDamage[target->self] = dmagToTarget;
            }
        }
        armyUnitProfiler.midLog("armyu(atk).executeAction");
    }

    virtual void executeHarass(Agent* const agent) {
        FUNCTION_LOG();
        atk(agent, squad->targetPosition);
    }

    virtual void executeDefend(Agent* const agent) {
        FUNCTION_LOG();
        atk(agent, squad->targetPosition);
    }

    float searchCost(Point2D p) {
        FUNCTION_LOG();
        if (p.x == 0 && p.y == 0) {
            return -1;
        }
        return VisibleMap2D::getVisibilityRecency(p);
    }

    virtual void executeSearch(Agent* const agent) {
        FUNCTION_LOG();
        //atk(agent, squad->targetPosition);
        float cost = -1;
        if (Aux::withinBounds(moveLocation)) {
            cost = searchCost(moveLocation);
        }
        //posTarget = { 0,0 };
        for (int i = 0; i < 5; i++) {
            Point2D check;
            check = Aux::getRandomPathable();
            float cos = searchCost(check);
            if (cos < cost || cost == -1) {
                cost = cos;
                moveLocation = check;
            }
        }
        atkmov(agent, moveLocation);
    }

    virtual void execute(Agent* const agent) {
        FUNCTION_LOG();
        if (getReturn(agent) == nullptr) { //is this needed?
            return;
        }
        if (squad->squadMainStates[self] == 'u' && DistanceSquared2D(pos(agent), squad->getCorePosition(agent)) < squad->armyballSquaredRadius()) {
            squad->squadMainStates[self] = 'j';
        }
        if (squad->squadMode == SquadManager::ATTACK) {
            executeAttack(agent);
        }
        else if (squad->squadMode == SquadManager::HARASS) {
            executeHarass(agent);
        }
        else if (squad->squadMode == SquadManager::DEFEND) {
            executeDefend(agent);
        }
        else if (squad->squadMode == SquadManager::SEARCH) {
            executeSearch(agent);
        }
        if (moveLocation != Point2D()) {
            DebugSphere(agent, AP3D(moveLocation), 0.5, Colors::Teal);
        }
    }
};

typedef std::shared_ptr<ArmyUnit> ArmyUnitPtr;