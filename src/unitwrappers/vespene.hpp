#pragma once
#include <sc2api/sc2_api.h>
#include <map>
#include "probe.hpp"
#include "../auxiliary/helpers.hpp"

class Vespene : public UnitWrapper {
private:
public:
    bool nearNexus;
    bool taken;

    Vespene(const Unit* unit, UnitTypeID sType) : UnitWrapper(unit, sType), taken(false), nearNexus(false) {

    }

    void init(Agent* const agent) {
        UnitWrappers vespenes = UnitManager::getSelf(UNIT_TYPEID::PROTOSS_NEXUS);
        for (UnitWrapperPtr vespeneW : vespenes) {
            std::shared_ptr<Vespene> vespene = std::static_pointer_cast<Vespene>(vespeneW);
            if (DistanceSquared2D(vespene->pos(agent), pos(agent)) < 100) {
                vespene->nearNexus = true;
                break;
            }
        }
    }
};

typedef std::shared_ptr<Vespene> VespenePtr;