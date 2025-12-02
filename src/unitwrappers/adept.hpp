#pragma once
#include <sc2api/sc2_api.h>
#include <map>
#include "armyunit.hpp"
#include "../auxiliary/helpers.hpp"
#include "../auxiliary/debugging.hpp"
#include "unitmanager.hpp"

#define ADEPT_CHECK_DO_SHADE 30
#define ADEPT_SHADE_LIFETIME_FRAMES 156

#define ADEPT_SHADE_COOLDOWN_FRAMES 20


float adeptAttainmentPriority(Point2D pos, Agent* const agent) {
    return 0;
}

class Adept : public ArmyUnit {
private:
public:

    UnitWrapperPtr linkedShade;

    Adept(const Unit* unit, SquadManager::Squad* squad_) : ArmyUnit(unit, UNIT_TYPEID::PROTOSS_ADEPT, squad_) {

    }

    virtual void execute(Agent* const agent) {
        if (linkedShade != nullptr) {
            DebugLine(agent, pos3D(agent) + Point3D{ 0,1,0 }, linkedShade->pos3D(agent) + Point3D{ 0,1,0 }, Colors::Purple);
        }
        ArmyUnit::execute(agent);
    }

    virtual void executeAttack(Agent* const agent) {
        bool found = false;
        const Unit* unit = getReturn(agent);
        if (agent->Observation()->GetGameLoop() % ADEPT_CHECK_DO_SHADE == 0 && unit != nullptr) {
            AvailableAbilities unitAbilities = agent->Query()->GetAbilitiesForUnit(unit);
            if (unitAbilities.abilities.size() > 0) {
                for (AvailableAbility abil : unitAbilities.abilities) {
                    if (abil.ability_id == ABILITY_ID::EFFECT_ADEPTPHASESHIFT) {
                        found = true;
                        break;
                    }
                }
            }
        }
        if(found){
            auto orders = unit->orders;
            if (moveLocation != Point2D()) {
                agent->Actions()->UnitCommand(self, ABILITY_ID::EFFECT_ADEPTPHASESHIFT, moveLocation);
            }
            else {
                agent->Actions()->UnitCommand(self, ABILITY_ID::EFFECT_ADEPTPHASESHIFT, pos(agent));
            }
        }
        else {
            ArmyUnit::executeAttack(agent);
        }
    }
};

typedef std::shared_ptr<Adept> AdeptPtr;

class AdeptShade : public ArmyUnit {
private:
public:

    AdeptPtr linkedAdept;
    long creationTime = 0;
    long lastTime = 0;

    AdeptShade(const Unit* unit, SquadManager::Squad* squad_) : ArmyUnit(unit, UNIT_TYPEID::PROTOSS_ADEPT, squad_) {

    }

    void init(Agent* const agent) {
        FUNCTION_LOG();
        creationTime = agent->Observation()->GetGameLoop();

        UnitWrappers adepts = UnitManager::getSelf(UNIT_TYPEID::PROTOSS_ADEPT);
        //UnitWrapperPtr nearestAdept = nullptr;
        float nearestAdeptDistance2 = -1;
        for (UnitWrapperPtr adept : adepts) {
            float dist2 = DistanceSquared2D(pos(agent), adept->pos(agent));
            if (linkedAdept == nullptr || nearestAdeptDistance2 > dist2) {
                linkedAdept = std::static_pointer_cast<Adept>(adept);
                nearestAdeptDistance2 = dist2;
            }
        }
        linkedAdept->linkedShade = shared_from_this();
    }

    //shade micro:
    //if walking a distance, go to that point (shade is faster)
    //if in battle, either go to a position of safety/opportunity or scout visibility (if path forward is fog)
    
    //shade swap micro:
    //switch only if less dps and closer to target
    virtual void execute(Agent* const agent) {
        FUNCTION_LOG();

        if (cooldownCheckUpdate(agent)) {
            return;
        }

        if (linkedAdept->target != nullptr && VisibleMap2D::getVisibilityRecency(linkedAdept->target->pos(agent)) < (VISIBILITY_MAX - 15)) {
            mov(agent, linkedAdept->target->pos(agent));
        }
        else {
            if (lastTime - creationTime > 30) { 
                float distToTravel = Aux::getStats(UNIT_TYPEID::PROTOSS_ADEPTPHASESHIFT, agent)->movement_speed / native_fps;
                movSafely(agent, linkedAdept->targetLocation, 1, distToTravel);
            }
            else {
                mov(agent, linkedAdept->targetLocation);
            }
        }
        cooldownFrames = ADEPT_SHADE_COOLDOWN_FRAMES;

        lastTime = agent->Observation()->GetGameLoop();

        if (lastTime - creationTime > ADEPT_SHADE_LIFETIME_FRAMES) {
            float adeptDPS = linkedAdept->getEnemyDPS(linkedAdept->pos(agent), linkedAdept->safetyMode ? 4.0F : 1.0F, agent);
            float shadeDPS = getEnemyDPS(pos(agent), linkedAdept->safetyMode ? 4.0F : 1.0F, agent);

            if (adeptDPS >= shadeDPS) {
                float adeptDist = linkedAdept->getPathLength(squad->targetPosition, agent);
                float shadeDist = getPathLength(squad->targetPosition, agent);
                if (adeptDist >= shadeDist) {
                    return; //tp to shade
                }
            }
            agent->Actions()->UnitCommand(linkedAdept->self, ABILITY_ID::CANCEL_ADEPTPHASESHIFT);
        }
    }

    ~AdeptShade() {
        linkedAdept->linkedShade = nullptr;
    }
};

typedef std::shared_ptr<AdeptShade> AdeptShadePtr;