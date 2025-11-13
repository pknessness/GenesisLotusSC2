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

constexpr int SECOND_DIVISION_MOVSAFELY = 4;
constexpr int COOLDOWN_FRAMES = 10;

#ifdef BUILD_FOR_LADDER
constexpr int POINT_CHECKS_DEFAULT = 10;
#else
constexpr int POINT_CHECKS_DEFAULT = 5;
#endif

class ArmyUnit : public UnitWrapper{
private:
public:

    SquadManager::Squad* squad;

    float prevMoveLocationDPS = 0.0F;
    Point2D moveLocation;

    float localDPS = 0.0F;

    UnitWrapperPtr target;

    int8_t cooldownFrames = 0;

    bool safetyMode = false;

    ArmyUnit(const Unit* unit, UnitTypeID sType, SquadManager::Squad* squad_) : UnitWrapper(unit, sType), squad(squad_){
        squad->squadMainStates[unit->tag] = 'u';
        squad->unitStates[unit->tag] = ' ';
    }

    virtual void atk(Agent* const agent, Point2D point) {
        const Unit* unit = getReturn(agent);
        if (unit->orders.size() == 0 || unit->orders[0].ability_id != ABILITY_ID::ATTACK || unit->orders[0].target_pos != point) {
            agent->Actions()->UnitCommand(self, ABILITY_ID::ATTACK, point);
            moveLocation = point;
        }
    }

    virtual void atk(Agent* const agent, UnitWrapperPtr target) {
        const Unit* unit = getReturn(agent);
        if (unit->orders.size() == 0 || unit->orders[0].ability_id != ABILITY_ID::ATTACK || unit->orders[0].target_unit_tag != target->self) {
            agent->Actions()->UnitCommand(self, ABILITY_ID::ATTACK, target->self);
            moveLocation = Point2D();
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

    virtual void mov(Agent* const agent, Point2D point) {
        const Unit* unit = getReturn(agent);
        if (unit->orders.size() == 0 || unit->orders[0].ability_id != ABILITY_ID::GENERAL_MOVE || unit->orders[0].target_pos != point) {
            agent->Actions()->UnitCommand(self, ABILITY_ID::GENERAL_MOVE, point);
            moveLocation = point;
        }
    }

    inline float getEnemyDPS(Point2D point, float extraRadius, WeaponGrid::relevantTargetDamageInfo selfTargetInfo, Agent* const agent) {
        return WeaponGrid::getRadiusAvgDPS(point, radius(agent) + extraRadius, selfTargetInfo, agent);
    }

    inline float getEnemyDPS(Point2D point, float extraRadius, Agent* const agent) {
        WeaponGrid::relevantTargetDamageInfo selfTargetInfo = WeaponGrid::wrapToTargetInfo(shared_from_this(), agent);
        return getEnemyDPS(point, extraRadius, selfTargetInfo, agent);
    }

    float getPathDamage(Point2D p, Agent* const agent) {
        std::vector<Point2D> path = getPathUniversal(p, agent);
        std::vector<Point2D> checks = PrimordialStar::stepPointsAlongPath(path, Aux::getStats(getActualType(agent), agent)->movement_speed / SECOND_DIVISION_MOVSAFELY);
        float dps = 0;
        WeaponGrid::relevantTargetDamageInfo selfTargetInfo = WeaponGrid::wrapToTargetInfo(shared_from_this(), agent);
        for (int i = 0; i < checks.size(); i++) {
            float addDps = getEnemyDPS(checks[i], 0.5, agent) * SECOND_DIVISION_MOVSAFELY;
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
        dps += getEnemyDPS(p, 0.5, agent) * 2 * SECOND_DIVISION_MOVSAFELY;
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

    virtual void movSafely(Agent* const agent, Point2D point, int attempts, float searchRadius) {
        //agent->Actions()->UnitCommand(self, ABILITY_ID::GENERAL_MOVE, point);
        Profiler movProfiler("movSafely");
        Point2D solution;
        float damage = FLT_MAX;
        float distance2 = FLT_MAX;
        const Unit* unit = getReturn(agent);
        if (unit->orders.size() > 0 && unit->orders[0].target_pos != Point2D()) {
            //solution = get(agent)->orders[0].target_pos;
            damage = getPathDamage(unit->orders[0].target_pos, agent);//WeaponGrid::getRadiusAvgDPS(get(agent)->orders[0].target_pos, radius(agent)+3.5, shared_from_this(), agent);
            distance2 = PrimordialStar::getPathLengthAStar(unit->orders[0].target_pos, point, radius(agent), agent);
        }
        movProfiler.midLog("movSafely.setup");
        std::vector<Point2D> path = PrimordialStar::getPathAStar(pos(agent), point, radius(agent), agent);
        float l = PrimordialStar::getPathLength(path);
        std::vector<Point2D> ptsToCheck;
        if (l != 0) {
            ptsToCheck = { PrimordialStar::distanceAlongPath(path, l / 2), point, pos(agent)};
        }
        else {
            ptsToCheck = { point, pos(agent) };
        }
        
        for (int i = 0; i < attempts + ptsToCheck.size(); i++) {
            Point2D p;
            if (i < ptsToCheck.size()) {
                p = ptsToCheck[i];
            }
            else {
                p = Aux::getRandomPointRadius(pos(agent), searchRadius);
                if (!Aux::isPathable(p) && !isFlying(agent)) {
                    i--;
                    continue;
                }
            }
            float dmg = getPathDamage(p, agent);
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
        movProfiler.midLog("movSafely.findSpot");
        if (solution != Point2D()) {
            mov(agent, solution);
        }
        movProfiler.midLog("movSafely.mov");
        //TODO: add path itself as a weight
    }

    virtual void executeAttack(Agent* const agent) {
        FUNCTION_LOG();
        Profiler armyUnitProfiler("armyu(atk)");

        float moveLocationDPS = WeaponGrid::getRadiusAvgDPS(moveLocation, radius(agent) + 3.5, WeaponGrid::wrapToTargetInfo(shared_from_this(), agent), agent);

        if (cooldownFrames > 0) {
            if ((target != nullptr && target->isDead()) || getReturn(agent)->orders.size() == 0) {
                cooldownFrames = 0;
            }
            else if (moveLocationDPS > prevMoveLocationDPS) {
                cooldownFrames /= 2;
            }
            else {
                cooldownFrames --;
                return;
            }
        }

        prevMoveLocationDPS = moveLocationDPS;

        moveLocation = Point2D{ -1, -1 };
        target = nullptr;
        //float damageToTarget = 0.0F;
        Aux::ExtraWeapon* weapon = WeaponGrid::emptyWeapon;

        bool radiusOfSafety = (WeaponGrid::getRadiusAvgDPS(pos(agent), radius(agent) + 3.5, WeaponGrid::wrapToTargetInfo(shared_from_this(), agent), agent) == 0.0F);
        if (!safetyMode && getShields(agent) < 20.0F) {
            safetyMode = true;
        }
        else if (safetyMode && getShields(agent) > (getShieldsMax(agent) - 20.0F)) {
            safetyMode = false;
        }

        float priority = 0; //fine because condition is also target == nullptr
        float dmagToTarget = 0;
        for (auto it = squad->squadTargets.begin(); it != squad->squadTargets.end(); it++) {
            if (squad->squadTargetDamage[(*it)->self] < (*it)->getHealth(agent)) {
                float p = squad->getEnemyUnitPriority(*it, agent);
                float damagePerHit = 0;
                UnitTypeID selfType = getActualType(agent);
                for (int i = 0; i < WeaponGrid::unitDamageSources[selfType].size(); i++) {
                    int index = WeaponGrid::unitDamageSources[selfType][i].weaponIndex;
                    float damage = WeaponGrid::DamageCalculation(WeaponGrid::getSelfWeaponFromIndex(index), *it, agent);
                    if (damage > damagePerHit) {
                        damagePerHit = damage;
                        weapon = WeaponGrid::getSelfWeaponFromIndex(index);
                    }
                }
                float distanceToEnemy = Distance2D((*it)->pos(agent), pos(agent));
                //float damageLost = (distanceToEnemy > weapon.range) ? (((distanceToEnemy - weapon.range) / Aux::getStats(getActualType(agent), agent).movement_speed) * (damagePerHit / weapon.speed)) : 0.0F;
                //p -= damageLost / 50; //every 50 damage lost takes a unit down one priority peg;
                float range = (weapon->range + (*it)->radius(agent));

                //if (distanceToEnemy > range) {
                //    p -= 2*(distanceToEnemy - range); //every one unit outside of range a unit is, it gets taken down two priority pegs
                //}
                //p += damagePerHit / weapon->speed; //each DPS, up one peg of priority

                float enemyHealth = (*it)->getHealth(agent);
                float dtToEnemyDeath = enemyHealth / (damagePerHit / weapon->speed);
                p /= dtToEnemyDeath;

                if (enemyHealth < damagePerHit) {
                    p *= 2;
                }
                if (target == nullptr || p > priority) {
                    priority = p;
                    target = *it;
                    dmagToTarget = damagePerHit;
                }
            }
        }
        armyUnitProfiler.midLog("armyu(atk).findTarget");

        bool attacking = false;

        if (squad->squadMainStates[self] == 'u') {
            if (getReturn(agent)->weapon_cooldown > 0) {
                squad->unitStates[self] = 'n';
            }
            else {
                squad->unitStates[self] = 'k';
            }
        //}
        //if (squad->squadMainStates[self] == 'u') {
            //moveLocation = squad->getCorePosition(agent);
            if (squad->unitStates[self] == 'n') {
                mov(agent, squad->getCorePosition(agent));
                cooldownFrames = COOLDOWN_FRAMES;
            }
            else if (squad->unitStates[self] == 'k') {
                atk(agent, squad->getCorePosition(agent));
                cooldownFrames = COOLDOWN_FRAMES;
            }
            else {
                atk(agent, squad->getCorePosition(agent));
                cooldownFrames = COOLDOWN_FRAMES;
            }
            
        }
        else if (squad->squadMainStates[self] == 'j') {
            //atk(agent, squad->targetPosition);
            if (target != nullptr) {
#ifdef TARGET_DEBUG
                DebugLine(agent, pos3D(agent) + Point3D{ 0,0,1 }, target->pos3D(agent) + Point3D{ 0,0,1 }, Colors::Teal);
#endif
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
                    float dTtoEnemy = abs(Distance2D(pos(agent), target->pos(agent)) - (weapon->range + target->radius(agent))) / Aux::getStats(getActualType(agent), agent)->movement_speed;
                    if (dTtoEnemy >= getReturn(agent)->weapon_cooldown) {
                        //if (get(agent)->is_selected) {
                        //    printf("");
                        //}
                        if (safetyMode && !radiusOfSafety) {
                            movSafely(agent, squad->getCorePosition(agent), 10, 7);
                        }
                        else {
                            atk(agent, target);
                        }
                        cooldownFrames = COOLDOWN_FRAMES;
                        if (getStorageType() == UNIT_TYPEID::PROTOSS_COLOSSUS) {
                            cooldownFrames = 0;
                        }
                        attacking = true;
                    }
                    else {
                        if (squad->isWithinRadius(pos(agent), agent)) {
                            movSafely(agent, target->pos(agent), 10, 7);
                        }
                        else {
                            movSafely(agent, squad->getCorePosition(agent), 10, 7);
                        }
                        
                        cooldownFrames = COOLDOWN_FRAMES;
                        if (COOLDOWN_FRAMES * fps > getReturn(agent)->weapon_cooldown) {
                            cooldownFrames = getReturn(agent)->weapon_cooldown / fps;
                        }
                    }
                }
            }
            else {
                if (squad->isWithinRadius(pos(agent), agent)) {
                    movSafely(agent, squad->targetPosition, POINT_CHECKS_DEFAULT, 7);
                }
                else {
                    movSafely(agent, squad->getCorePosition(agent), POINT_CHECKS_DEFAULT, 7);
                }
                cooldownFrames = COOLDOWN_FRAMES;
            }
            
        }
        if (target != nullptr && attacking) {
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