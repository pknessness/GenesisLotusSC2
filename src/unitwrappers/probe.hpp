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

std::vector<Building> failedBuildings;

class Probe : public UnitWrapper {
private:

    UnitWrapperPtr target;
    float ignoreFrames;
    Point2D patrolCenter;

public:
    std::vector<Building> buildings;

    Probe(const Unit* unit) : UnitWrapper(unit, UNIT_TYPEID::PROTOSS_PROBE), ignoreFrames(0), target(nullptr) {
    }

    ~Probe() {
        setTarget(nullptr);
    }

    void setTarget(UnitWrapperPtr newTarget) {
        FUNCTION_LOG();
        if (target != nullptr) {
            probeTargetting[target->self] -= 1;
        }
        if (newTarget != nullptr) {
            probeTargetting[newTarget->self] += 1;
        }
        target = newTarget;
    }

    UnitWrapperPtr rawTargetTag() {
        return target;
    }

    UnitWrapperPtr getTargetTag(Agent* agent) { //TODO: IF ASSIMILATOR EMPTY RETARGET
        FUNCTION_LOG();
        if (target != nullptr && (target->getReturn(agent) == nullptr || (target->getStorageType() == UNIT_TYPEID::PROTOSS_ASSIMILATOR && target->getReturn(agent)->vespene_contents == 0))) {
            UnitWrapperPtr oldTarget = target;
            setTarget(nullptr);
            if (oldTarget->getReturn(agent) == nullptr) {
                probeTargetting.erase(oldTarget->self);
            }
        }
        if (target == nullptr) {
            //TODO: CHOOSE NEW TARGET
            UnitWrappers mineralWraps = UnitManager::getMinerals();
            UnitWrappers assimilatorWraps = UnitManager::getSelf(UNIT_TYPEID::PROTOSS_ASSIMILATOR);
            mineralWraps.insert(assimilatorWraps.begin(), assimilatorWraps.end());

            float minDist = -1;
            int capacity = 3;
            UnitWrapperPtr nextTarget;
            bool hasNexus = false;

            for (UnitWrapperPtr targetWrap : mineralWraps) {
                if (targetWrap->getStorageType() == UNIT_TYPEID::PROTOSS_ASSIMILATOR && targetWrap->getReturn(agent)->vespene_contents == 0) {
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

                //if (capacity < probeTargetting[targetWrap->self]) {
                //    continue;
                //}

                //if viable minerals exist, don't do assimilators
                //should only do assimilators over minerals if minerals are over 100 dist2 away
                if (nextTarget != nullptr && Aux::isMineralType(nextTarget->getActualType(agent)) && DistanceSquared2D(nextTarget->pos(agent), pos(agent)) < 100 && targetWrap->getActualType(agent) == UNIT_TYPEID::PROTOSS_ASSIMILATOR) {
                    continue;
                }


                float dist = DistanceSquared2D(pos(agent), targetWrap->pos(agent));

                //if assimilator is chosen and a mineral closer than 100 apears, swap to it for sure
                //or if no item is chosen yet (minDist == -1)
                //or if dist < minDist
                if (minDist == -1 || (nextTarget != nullptr && nextTarget->getActualType(agent) == UNIT_TYPEID::PROTOSS_ASSIMILATOR && Aux::isMineralType(targetWrap->getActualType(agent)) && DistanceSquared2D(targetWrap->pos(agent), pos(agent)) < 100) || (!hasNexus && nexusNearby[targetWrap->self])/* || (capacity > probeTargetting[targetWrap->self])*/ || dist < minDist) {
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
        FUNCTION_LOG();
        buildings.push_back(b);
    }

    bool isPatrolling() {
        return patrolCenter != Point2D();
    }

    void conditionalDisengage(Agent* const agent, Point2D location) {
        if (DistanceSquared2D(agent->Observation()->GetUnit(self)->pos, location) > 900) {
            setTarget(nullptr);
        }
    }

    //TODO: MAKE SURE THAT PROBES HAVE TO BE WITHIN 10 TO CLAIM, AND OTHERWISE THEY WILL JUST MOVE TOWARD THE ONE THEY WANT TO CLAIM
    void execute(Agent* const agent) {
        FUNCTION_LOG();
        const Unit* unit = getReturn(agent);
        if (unit == nullptr) {
            return;
        }
        if (buildings.size() != 0 && (unit->orders.size() == 0 || unit->orders[0].ability_id == ABILITY_ID::HARVEST_GATHER || unit->orders[0].ability_id == ABILITY_ID::HARVEST_RETURN || unit->orders[0].ability_id == ABILITY_ID::GENERAL_PATROL)) {
            Building top = buildings[0];
            if (top.build == ABILITY_ID::GENERAL_MOVE) {
                agent->Actions()->UnitCommand(self, ABILITY_ID::GENERAL_MOVE, top.pos);
                conditionalDisengage(agent, top.pos);
                buildings.erase(buildings.begin());
            }
            else if (top.build == ABILITY_ID::GENERAL_PATROL) {
                agent->Actions()->UnitCommand(self, ABILITY_ID::GENERAL_MOVE, top.pos + Point2D{-1,0});
                agent->Actions()->UnitCommand(self, ABILITY_ID::GENERAL_PATROL, top.pos + Point2D{ 1,0 }, true);
                patrolCenter = top.pos;
                conditionalDisengage(agent, top.pos);
                buildings.erase(buildings.begin());
            }
            else if (DistanceSquared2D(agent->Observation()->GetUnit(self)->pos, top.pos) < 4) {
                DebugBox(agent, AP3D(top.pos) + Point3D{ -1.5,-1.5,0 }, AP3D(top.pos) + Point3D{ 1.5,1.5,3 }, Colors::Blue);
                Aux::Cost buildingCost = Aux::buildAbilityToCost(top.build, agent);
                if (buildingCost.minerals > agent->Observation()->GetMinerals() || buildingCost.vespene > agent->Observation()->GetVespene()) {
                    return;
                }
                UnitTypeData* ability_stats = Aux::getStats(Aux::buildAbilityToUnit(top.build), agent);
                UnitTypeID prerequisite = ability_stats->tech_requirement;
                if (prerequisite != UNIT_TYPEID::INVALID) {
                    UnitWrappers prereqs = UnitManager::getSelf(prerequisite);

                    bool built = false;

                    for (auto it = prereqs.begin(); it != prereqs.end(); it++) {
                        const Unit* prereq = (*it)->getReturn(agent);
                        if (prereq != nullptr && prereq->build_progress == 1.0F) {
                            built = true;
                        }
                    }
                    if (!built) {
                        return;
                    }
                }

                if (top.build == ABILITY_ID::BUILD_ASSIMILATOR) {
                    UnitWrappers vespenes = UnitManager::getVespene();
                    for (auto it = vespenes.begin(); it != vespenes.end(); it++) {
                        //printf("TRY: %.1f,%.1f %.1f,%.1f\n", (*it)->pos(agent).x, (*it)->pos(agent).y,
                        //    pos(agent).x, pos(agent).y);
                        if (DistanceSquared2D((*it)->pos(agent), top.pos) < 4) {
                            //printf("%Ix %s %Ix\n", self, AbilityTypeToName(top.build), (*it)->self);
                            agent->Actions()->UnitCommand(self, top.build, (*it)->self);
                            std::static_pointer_cast<Vespene>((*it))->taken = true;
                            break;
                        }
                    }
                }
                else {
                    if (agent->Query()->Placement(top.build, top.pos)) {
                        //printf("CAN PLACE %s %.1f,%.1f\n", AbilityTypeToName(top.build), top.pos.x, top.pos.y);
                        agent->Actions()->UnitCommand(self, top.build, top.pos);
                        if (patrolCenter != Point2D()) {
                            agent->Actions()->UnitCommand(self, ABILITY_ID::GENERAL_MOVE, patrolCenter + Point2D{ -1,0 }, true);
                            agent->Actions()->UnitCommand(self, ABILITY_ID::GENERAL_PATROL, patrolCenter + Point2D{ 1,0 }, true);
                        }
                    }
                    else {
                        failedBuildings.push_back(*buildings.begin());
                        buildings.erase(buildings.begin());
                        return;
                    }
                }
                buildings.erase(buildings.begin());
                Aux::Cost g = Aux::buildAbilityToCost(top.build, agent);
                //because the building is being begun this frame, the minerals/vesp are not yet updated to take into account that this is being built.
                //TODO: TAKE INTO ACCOUNT SUPPLY ALSO
                Aux::effectiveMinerals -= g.minerals; 
                Aux::effectiveVespene -= g.vespene;
            }
            else {
                DebugBox(agent, AP3D(top.pos) + Point3D{ -1.5,-1.5,0 }, AP3D(top.pos) + Point3D{ 1.5,1.5,3 }, Colors::Teal);
                const Unit* prob = getReturn(agent);
                if (prob->orders.size() == 0 || prob->orders.front().target_pos != top.pos) {
                    agent->Actions()->UnitCommand(self, ABILITY_ID::MOVE_MOVE, top.pos);
                    conditionalDisengage(agent, top.pos);
                }
            }
        }
        //if unit has no buildings
        //and if unit has either no orders, or the order is harvest
        else if((unit->orders.size() == 0 || unit->orders[0].ability_id == ABILITY_ID::HARVEST_GATHER) && buildings.size() == 0) {
            UnitWrapperPtr targ = getTargetTag(agent);
            //if unit has no target
            //and if unit has either no orders, or the order is harvest to the wrong target
            if (targ != nullptr && (unit->orders.size() == 0 || (unit->orders[0].ability_id == ABILITY_ID::HARVEST_GATHER &&
                unit->orders[0].target_unit_tag != targ->self))) {
                /*printf("REASING\n");*/
                agent->Actions()->UnitCommand(self, ABILITY_ID::HARVEST_GATHER, targ->self);
            }
        }
    }

    float getPathLengthFromLastAction(Point2D end, Agent* const agent) {
        if (buildings.size() > 0) {
            return getPathLength(buildings[buildings.size() - 1].pos, end, agent);
        }
        else {
            return getPathLength(end, agent);
        }
    }
};

typedef std::shared_ptr<Probe> ProbePtr;