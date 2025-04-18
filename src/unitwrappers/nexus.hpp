#pragma once
#include <sc2api/sc2_api.h>
#include <map>
#include "probe.hpp"
#include "vespene.hpp"
#include "../auxiliary/helpers.hpp"

class Nexus : public UnitWrapper {
private:
public:

    Nexus(const Unit* unit) : UnitWrapper(unit, UNIT_TYPEID::PROTOSS_NEXUS) {

    }

    void init(Agent* const agent) {
        UnitWrappers mineralWraps = UnitManager::getMinerals();
        UnitWrappers assimilatorWraps = UnitManager::getSelf(UNIT_TYPEID::PROTOSS_ASSIMILATOR);
        mineralWraps.insert(assimilatorWraps.begin(), assimilatorWraps.end());

        bool hasN = false;
        for (UnitWrapperPtr targetWrap : mineralWraps) {
            if (DistanceSquared2D(targetWrap->pos(agent), pos(agent)) < 100) {
                nexusNearby[targetWrap->self] = true;
                break;
            }
        }

        UnitWrappers vespenes = UnitManager::getVespene();
        for (UnitWrapperPtr vespeneW : vespenes) {
            VespenePtr vespene = std::static_pointer_cast<Vespene>(vespeneW);
            if (DistanceSquared2D(vespene->pos(agent), pos(agent)) < 100) {
                vespene->nearNexus = true;
                break;
            }
        }
    }
};

typedef std::shared_ptr<Nexus> NexusPtr;