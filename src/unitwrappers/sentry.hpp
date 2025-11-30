#pragma once
#include <sc2api/sc2_api.h>
#include <map>
#include "armyunit.hpp"
#include "../auxiliary/helpers.hpp"
#include "../auxiliary/debugging.hpp"
#include "unitmanager.hpp"

class Sentry : public ArmyUnit {
private:
public:

    Sentry(const Unit* unit, SquadManager::Squad* squad_) : ArmyUnit(unit, UNIT_TYPEID::PROTOSS_SENTRY, squad_) {

    }

    virtual void execute(Agent* const agent) {
        ArmyUnit::execute(agent);
    }

    virtual void executeAttack(Agent* const agent) {
        ArmyUnit::executeAttack(agent);
    }
};

typedef std::shared_ptr<Sentry> SentryPtr;