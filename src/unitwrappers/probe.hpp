#pragma once
#include <sc2api/sc2_api.h>
#include <map>
#include "unitwrapper.hpp"
#include "../auxiliary/helpers.hpp"
#include "unitmanager.hpp"
#include "../auxiliary/debugging.hpp"
#include "vespene.hpp"

struct Building {
    AbilityID build;
    Point2D pos;

    Aux::Cost cost(Agent* agent) {
        return Aux::buildAbilityToCost(build, agent);
    }
};

std::map<Tag, int8_t> probeTargetting;
//std::map<Tag, float> mineralDistance;
std::map<Tag, bool> nexusNearby;

class Probe : public UnitWrapper {
private:

    UnitWrapperPtr target;
    float ignoreFrames;

public:
    std::vector<Building> buildings;

    Probe(const Unit* unit) : UnitWrapper(unit, UNIT_TYPEID::PROTOSS_PROBE), ignoreFrames(0), target(nullptr) {
    }

    ~Probe() {
        setTarget(nullptr);
    }

    void setTarget(UnitWrapperPtr newTarget) {
        if (target != nullptr) {
            probeTargetting[target->self] -= 1;
        }
        if (newTarget != nullptr) {
            probeTargetting[newTarget->self] += 1;
        }
        target = newTarget;
    }

    UnitWrapperPtr getTargetTag(Agent* agent) { //TODO: IF ASSIMILATOR EMPTY RETARGET
        if (target != nullptr && (target->get(agent) == nullptr || (target->getStorageType() == UNIT_TYPEID::PROTOSS_ASSIMILATOR && target->get(agent)->vespene_contents == 0))) {
            setTarget(nullptr);
        }
        if (target == nullptr) {
            //probeTargetting.clear();
            //for (UnitWrapperPtr probesWr : UnitManager::getSelf(UNIT_TYPEID::PROTOSS_PROBE)) {
            //    std::shared_ptr<Probe> probe = std::static_pointer_cast<Probe>(probesWr);
            //    if (probe->target == nullptr) {
            //        continue;
            //    }

            //    if (probe->target != nullptr && probeTargetting.find(probe->target->self) == probeTargetting.end()) {
            //        probeTargetting[probe->target->self] = 0;
            //    }
            //    probeTargetting[probe->target->self] += 1;
            //}

            //TODO: CHOOSE NEW TARGET
            UnitWrappers mineralWraps = UnitManager::getMinerals();
            UnitWrappers assimilatorWraps = UnitManager::getSelf(UNIT_TYPEID::PROTOSS_ASSIMILATOR);
            mineralWraps.insert(assimilatorWraps.begin(), assimilatorWraps.end());

            float minDist = -1;
            int capacity = 3;
            UnitWrapperPtr nextTarget;
            bool hasNexus = false;

            for (UnitWrapperPtr targetWrap : mineralWraps) {
                if (targetWrap->getStorageType() == UNIT_TYPEID::PROTOSS_ASSIMILATOR && targetWrap->get(agent)->vespene_contents == 0) {
                    continue;
                }

                if (nexusNearby.find(targetWrap->self) == nexusNearby.end()) {
                    UnitWrappers nexuses = UnitManager::getSelf(UNIT_TYPEID::PROTOSS_NEXUS);
                    bool hasN = false;
                    for (UnitWrapperPtr nexus : nexuses) {
                        if (DistanceSquared2D(targetWrap->pos(agent), nexus->pos(agent)) < 100) {
                            hasN = true;
                            break;
                        }
                    }
                    nexusNearby[targetWrap->self] = hasN;
                }

                int limit = 3;
                if (targetWrap->getStorageType() == UNIT_TYPEID::NEUTRAL_MINERALFIELD) {
                    limit = 2;
                }
                if (probeTargetting[targetWrap->self] >= limit) {
                    continue;
                }

                if (hasNexus && !nexusNearby[targetWrap->self]) {
                    continue;
                }

                if (capacity < probeTargetting[targetWrap->self]) {
                    continue;
                }

                float dist = DistanceSquared2D(pos(agent), targetWrap->pos(agent));

                if (minDist == -1 || (!hasNexus && nexusNearby[targetWrap->self]) || (capacity > probeTargetting[targetWrap->self]) || dist < minDist) {
                    if (nexusNearby[targetWrap->self]) {
                        hasNexus = true;
                    }
                    minDist = dist;
                    nextTarget = targetWrap;
                    capacity = probeTargetting[targetWrap->self];
                    /*probeTargetting[targetWrap->self] += 1;*/
                }
            }
            setTarget(nextTarget);
        }
        return target;
    }

    void addBuilding(Building b) {
        buildings.push_back(b);
    }

    void execute(Agent* const agent) {
        const Unit* unit = get(agent);
        if (unit == nullptr) {
            return;
        }
        if (buildings.size() != 0 && (unit->orders.size() == 0 || unit->orders[0].ability_id == ABILITY_ID::HARVEST_GATHER || unit->orders[0].ability_id == ABILITY_ID::HARVEST_RETURN)) {
            Building top = buildings[0];
            if (top.build == ABILITY_ID::GENERAL_MOVE) {
                agent->Actions()->UnitCommand(self, ABILITY_ID::GENERAL_MOVE, top.pos);
                buildings.erase(buildings.begin());
            }
            else if (DistanceSquared2D(agent->Observation()->GetUnit(self)->pos, top.pos) < 4) {
                DebugBox(agent, AP3D(top.pos) + Point3D{ -1.5,-1.5,0 }, AP3D(top.pos) + Point3D{ 1.5,1.5,3 }, Colors::Blue);
                Aux::Cost buildingCost = Aux::buildAbilityToCost(top.build, agent);
                if (buildingCost.minerals > agent->Observation()->GetMinerals() || buildingCost.vespene > agent->Observation()->GetVespene())
                    return;

                UnitTypeData ability_stats = Aux::getStats(Aux::buildAbilityToUnit(top.build), agent);
                UnitTypeID prerequisite = ability_stats.tech_requirement;
                if (prerequisite != UNIT_TYPEID::INVALID) {
                    UnitWrappers prereqs = UnitManager::getSelf(prerequisite);

                    bool built = false;

                    for (auto it = prereqs.begin(); it != prereqs.end(); it++) {
                        const Unit* prereq = (*it)->get(agent);
                        if (prereq != nullptr && prereq->build_progress == 1.0F) {
                            built = true;
                        }
                        //if ((*it)->construction_finished) {
                        //    built = true;
                        //    break;
                        //}
                    }
                    if (!built) {
                        return;
                    }
                }

                if (top.build == ABILITY_ID::BUILD_ASSIMILATOR) {
                    UnitWrappers vespenes = UnitManager::getVespene();
                    for (auto it = vespenes.begin(); it != vespenes.end(); it++) {
                        printf("TRY: %.1f,%.1f %.1f,%.1f\n", (*it)->pos(agent).x, (*it)->pos(agent).y,
                            pos(agent).x, pos(agent).y);
                        if (DistanceSquared2D((*it)->pos(agent), pos(agent)) < 4) {
                            printf("%Ix %s %Ix\n", self, AbilityTypeToName(top.build), (*it)->self);
                            agent->Actions()->UnitCommand(self, top.build, (*it)->self);
                            std::static_pointer_cast<Vespene>((*it))->taken = true;
                            break;
                        }
                    }
                }
                else {
                    if (agent->Query()->Placement(top.build, top.pos)) {
                        printf("CAN PLACE %s %.1f,%.1f\n", AbilityTypeToName(top.build), top.pos.x, top.pos.y);
                        agent->Actions()->UnitCommand(self, top.build, top.pos);
                    }
                    else {
                        return;
                    }
                }
                buildings.erase(buildings.begin());
            }
            else {
                DebugBox(agent, AP3D(top.pos) + Point3D{ -1.5,-1.5,0 }, AP3D(top.pos) + Point3D{ 1.5,1.5,3 }, Colors::Teal);
                const Unit* prob = get(agent);
                if (prob->orders.size() == 0 || prob->orders.front().target_pos != top.pos) {
                    agent->Actions()->UnitCommand(self, ABILITY_ID::MOVE_MOVE, top.pos);
                }
            }
        }
        else {
            UnitWrapperPtr targ = getTargetTag(agent);
            if (targ != nullptr && (unit->orders.size() == 0 || (unit->orders[0].ability_id == ABILITY_ID::HARVEST_GATHER &&
                unit->orders[0].target_unit_tag != targ->self))) {
                /*printf("REASING\n");*/
                agent->Actions()->UnitCommand(self, ABILITY_ID::HARVEST_GATHER, targ->self);
            }
        }
    }
};

typedef std::shared_ptr<Probe> ProbePtr;