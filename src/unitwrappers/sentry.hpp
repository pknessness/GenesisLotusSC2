#pragma once
#include <sc2api/sc2_api.h>
#include <map>
#include "armyunit.hpp"
#include "../auxiliary/helpers.hpp"
#include "../auxiliary/debugging.hpp"
#include "unitmanager.hpp"

//wall: 50
//guardian shield: 75
//hallucination: 50
class Sentry : public ArmyUnit {
private:
public:

    Sentry(const Unit* unit, SquadManager::Squad* squad_) : ArmyUnit(unit, UNIT_TYPEID::PROTOSS_SENTRY, squad_) {

    }

    //virtual void execute(Agent* const agent) {
    //    ArmyUnit::execute(agent);
    //}
    
    virtual void updateCooldownWithWeapon(Agent* const agent) {
    }

    //needs to update cooldownFrames
    virtual void action_Attack(Agent* const agent, const Aux::ExtraWeapon& weapon, bool radiusOfSafety) {
        if (target != nullptr && radiusOfSafety) {
            hasTargetAction_Attack(agent, weapon, radiusOfSafety);
        }
        else {
            hasNoTargetAction_Attack(agent);
        }
        if(squad->squadTargets.size() > 5 && getEnergy(agent) > 75){
            agent->Actions()->UnitCommand(self, ABILITY_ID::EFFECT_GUARDIANSHIELD);
        }
    }
};

typedef std::shared_ptr<Sentry> SentryPtr;