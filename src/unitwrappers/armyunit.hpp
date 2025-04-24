#pragma once
#include <sc2api/sc2_api.h>
#include "../auxiliary/helpers.hpp"
#include "unitwrapper.hpp"
#include "../auxiliary/squadmanager.hpp"

class ArmyUnit : public UnitWrapper {
private:
public:

    SquadManager::Squad* squad;

    ArmyUnit(const Unit* unit, UnitTypeID sType) : UnitWrapper(unit, sType){

    }

    virtual void atk(Agent* const agent, Point2D point) {
        agent->Actions()->UnitCommand(self, ABILITY_ID::ATTACK, point);
    }

    virtual void atk(Agent* const agent, UnitWrapper* target) {
        agent->Actions()->UnitCommand(self, ABILITY_ID::ATTACK, target->self);
    }

    virtual void mov(Agent* const agent, Point2D point) {
        agent->Actions()->UnitCommand(self, ABILITY_ID::GENERAL_MOVE, point);
    }

    virtual void executeAttack(Agent* const agent) {
        atk(agent, squad->targetPosition);
    }

    virtual void executeHarass(Agent* const agent) {
        atk(agent, squad->targetPosition);
    }

    virtual void executeDefend(Agent* const agent) {
        atk(agent, squad->targetPosition);
    }

    virtual void executeSearch(Agent* const agent) {
        atk(agent, squad->targetPosition);
    }

    virtual void execute(Agent* const agent) {
        if (get(agent) == nullptr) {
            return;
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
    }
};

typedef std::shared_ptr<ArmyUnit> ArmyUnitPtr;