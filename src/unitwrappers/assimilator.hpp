#pragma once
#include <sc2api/sc2_api.h>
#include <map>
#include "probe.hpp"
#include "vespene.hpp"
#include "nexus.hpp"
#include "../auxiliary/helpers.hpp"

class Assimilator : public UnitWrapper {
private:
public:
    UnitWrapperPtr nexus;
    VespenePtr vespene;

    Assimilator(const Unit* unit) : UnitWrapper(unit, UNIT_TYPEID::PROTOSS_ASSIMILATOR), vespene(nullptr), nexus(nullptr) {

    }

    //void init(Agent* const agent) {
    //    UnitWrappers nexuses = UnitManager::getSelf(UNIT_TYPEID::PROTOSS_NEXUS);
    //    for (UnitWrapperPtr nexus : nexuses) {
    //        if (DistanceSquared2D(pos(agent), nexus->pos(agent)) < 100) {
    //            std::static_pointer_cast<Nexus>(nexus)->addMineral(mineral);
    //            break;
    //        }
    //    }
    //    units[stype].insert(mineral);

    //    UnitWrappers vespenes = UnitManager::getVespene();
    //    for (UnitWrapperPtr vespeneW : vespenes) {
    //        VespenePtr vespene = std::static_pointer_cast<Vespene>(vespeneW);
    //        if (DistanceSquared2D(vespene->pos(agent), pos(agent)) < 1) {
    //            vespene = 
    //            break;
    //        }
    //    }
    //}
};

typedef std::shared_ptr<Nexus> NexusPtr;