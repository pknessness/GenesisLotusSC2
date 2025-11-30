#pragma once
#include <sc2api/sc2_api.h>
#include <map>
#include "armyunit.hpp"
#include "../auxiliary/helpers.hpp"
#include "../auxiliary/debugging.hpp"
#include "unitmanager.hpp"

class Zealot : public ArmyUnit {
private:
public:

    Zealot(const Unit* unit, SquadManager::Squad* squad_) : ArmyUnit(unit, UNIT_TYPEID::PROTOSS_ZEALOT, squad_) {

    }

    //virtual void execute(Agent* const agent) {
    //    ArmyUnit::execute(agent);
    //}

    //virtual void executeAttack(Agent* const agent) {
    //    ArmyUnit::executeAttack(agent);
    //}
};

typedef std::shared_ptr<Zealot> ZealotPtr;