#pragma once
#include <sc2api/sc2_api.h>
#include <map>
#include "probe.hpp"
#include "vespene.hpp"
#include "../auxiliary/helpers.hpp"

static std::vector<UnitWrapperPtr> chronoBoosts;

class Nexus : public UnitWrapper {
private:
public:
    UnitWrapperPtr assimilator1;
    UnitWrapperPtr assimilator2;

    UnitWrapperPtr minerals[8];
    int8_t minsFound;

    Nexus(const Unit* unit) : UnitWrapper(unit, UNIT_TYPEID::PROTOSS_NEXUS), assimilator1(nullptr), assimilator2(nullptr), minsFound(0) {
        for (int i = 0; i < 8; i++) {
            minerals[i] = nullptr;
        }
    }

    void addMineral(UnitWrapperPtr mineral) {
        if (minsFound == 8) {
            throw 8;
        }
        minerals[minsFound++] = mineral;
    }

    void addAssimilator(UnitWrapperPtr assim) {
        if (assimilator1 == nullptr) {
            assimilator1 = assim;
        }
        else if (assimilator2 == nullptr) {
            assimilator2 = assim;
        }
        else {
            throw 9;
        }
    }

    void init(Agent* const agent) {
        UnitWrappers mineralWraps = UnitManager::getMinerals();
        UnitWrappers assimilatorWraps = UnitManager::getSelf(UNIT_TYPEID::PROTOSS_ASSIMILATOR);
        mineralWraps.insert(assimilatorWraps.begin(), assimilatorWraps.end());

        bool hasN = false;
        for (UnitWrapperPtr targetWrap : mineralWraps) {
            if (DistanceSquared2D(targetWrap->pos(agent), pos(agent)) < 100) {
                if (targetWrap->getStorageType() == UNIT_TYPEID::NEUTRAL_MINERALFIELD) {
                    addMineral(targetWrap);
                } else if (targetWrap->getStorageType() == UNIT_TYPEID::PROTOSS_ASSIMILATOR) {
                    addAssimilator(targetWrap);
                } else {
                    throw 10;
                }
                nexusNearby[targetWrap->self] = true;
            }
        }

        UnitWrappers vespenes = UnitManager::getVespene();
        for (UnitWrapperPtr vespeneW : vespenes) {
            VespenePtr vespene = std::static_pointer_cast<Vespene>(vespeneW);
            if (DistanceSquared2D(vespene->pos(agent), pos(agent)) < 100) {
                vespene->nearNexus = true;
            }
        }
    }

    static void addChrono(UnitWrapperPtr toChrono) {
        chronoBoosts.push_back(toChrono);
    }

    void execute(Agent* const agent) {
        if (chronoBoosts.size() > 0) {
            if (get(agent)->energy >= Aux::unitAbilityToCost(ABILITY_ID::EFFECT_CHRONOBOOSTENERGYCOST, agent).energy) {
                std::vector<BuffID> buffs = chronoBoosts[0]->get(agent)->buffs;
                if (std::find(buffs.begin(), buffs.end(), BUFF_ID::CHRONOBOOSTENERGYCOST) == buffs.end()) {
                    agent->Actions()->UnitCommand(self, ABILITY_ID::EFFECT_CHRONOBOOSTENERGYCOST, chronoBoosts[0]->self);
                    chronoBoosts.erase(chronoBoosts.begin());
                }

            }
        }
    }
};

typedef std::shared_ptr<Nexus> NexusPtr;