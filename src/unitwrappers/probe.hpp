#pragma once
#include <sc2api/sc2_api.h>
#include <map>
#include "unitwrapper.hpp"
#include "../auxiliary/helpers.hpp"
#include "unitmanager.hpp"
#include "../auxiliary/debugging.hpp"
#include "vespene.hpp"
#include "../auxiliary/spatialhashgrid.hpp"
//#include "nexus.hpp"
#include "probetarget.hpp"

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
        //gatherPoint = Point2D();
        //nexus = nullptr;
    }

    //UnitWrapperPtr getNexus(Agent* const agent) {
    //    if (nexus == nullptr) {
    //        if (getTargetTag(agent) == nullptr || target->getReturn(agent) == nullptr) {
    //            return nullptr;
    //        }
    //        UnitWrapperPtr nearest = nullptr;
    //        float nearestDistance2 = -1;
    //        for (const auto& unit : UnitManager::getSelf(UNIT_TYPEID::PROTOSS_NEXUS)) {
    //            float dist2 = DistanceSquared2D(unit->pos(agent), target->pos(agent));
    //            if (nearest == nullptr || dist2 < nearestDistance2) {
    //                nearestDistance2 = dist2;
    //                nearest = unit;
    //            }
    //        }
    //        nexus = nearest;
    //        nexusDistance2 = nearestDistance2;
    //    }
    //    return nexus;
    //}

    //Point2D getGatherPoint(Agent* const agent) {
    //    UnitWrapperPtr nexus = getNexus(agent);
    //    if (gatherPoint == Point2D() && getTargetTag(agent) != nullptr && nexus != nullptr) {
    //        gatherPoint = SpeedMining::calculateGatherPoint(agent, target->pos(agent), nexus->pos(agent));
    //    }
    //    return gatherPoint;
    //}

    //Point2D getReturnPoint(Agent* const agent) {
    //    UnitWrapperPtr nexus = getNexus(agent);
    //    if (nexusReturnPoint == Point2D() && getTargetTag(agent) != nullptr && nexus != nullptr) {
    //        nexusReturnPoint = SpeedMining::calculateReturnPoint(agent, getGatherPoint(agent), nexus->pos(agent));
    //    }
    //    return nexusReturnPoint;
    //}

    UnitWrapperPtr rawTargetTag() {
        return target;
    }

    UnitWrapperPtr getTargetTag(Agent* agent) {
        FUNCTION_LOG();
        if (target != nullptr && (target->getReturn(agent) == nullptr || (target->getStorageType() == UNIT_TYPEID::PROTOSS_ASSIMILATOR && target->getReturn(agent)->vespene_contents == 0))) {
            UnitWrapperPtr oldTarget = target;
            setTarget(nullptr);
            if (oldTarget->getReturn(agent) == nullptr) {
                probeTargetting.erase(oldTarget->self);
            }
        }
        if (target == nullptr) {
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

                UnitTypeID unitToCreate = Aux::buildAbilityToUnit(top.build); //if a unit needs a pylon, check for pylon in range
                if (unitToCreate != UNIT_TYPEID::INVALID && unitToCreate != UNIT_TYPEID::PROTOSS_NEXUS && unitToCreate != UNIT_TYPEID::PROTOSS_PYLON && unitToCreate != UNIT_TYPEID::PROTOSS_ASSIMILATOR) {
                    UnitWrappers pylons = UnitManager::getSelf(UNIT_TYPEID::PROTOSS_PYLON);
                    bool pylonInRange = false;
                    for (auto it = pylons.begin(); it != pylons.end(); it++) {
                        if ((*it)->getReturn(agent)->build_progress == 1.0 && DistanceSquared2D((*it)->pos(agent), top.pos) < PYLON_RADIUS_SQUARED_REAL) {
                            pylonInRange = true;
                        }
                    }

                    if (!pylonInRange) {
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
                            MacroManager::dataEncoding[top.pos].stepBegin = agent->Observation()->GetGameLoop();
                            std::static_pointer_cast<Vespene>((*it))->taken = true;
                            break;
                        }
                    }
                }
                else { //TODO: ADD REQUIRED ENCODING CONFIRMATION FOR BUILDINGS TO KNOW THEYVE BEEN BUILT, COUNT # of failures and exceeding a certain number of retries, go to failed building
                    //if (agent->Query()->Placement(top.build, top.pos)) {
                    UnitTypeID type = Aux::buildAbilityToUnit(top.build);
                    if (type == UNIT_TYPEID::INVALID) {
                        printf("PROBE 'BUILDING' ERROR: %s [%d] @ {%.2f, %.2f}\n", AbilityTypeToName(top.build), top.build, top.pos.x, top.pos.y); //remove this once i figure out the 1518 bug
                        throw 81;
                    }
                    else {
                        if (Aux::checkStructurePlacement(top.pos, top.build, true)) {

                            agent->Actions()->UnitCommand(self, top.build, top.pos);
                            MacroManager::dataEncoding[top.pos].stepBegin = agent->Observation()->GetGameLoop();

                            if (patrolCenter != Point2D()) {
                                agent->Actions()->UnitCommand(self, ABILITY_ID::GENERAL_MOVE, patrolCenter + Point2D{ -1,0 }, true);
                                agent->Actions()->UnitCommand(self, ABILITY_ID::GENERAL_PATROL, patrolCenter + Point2D{ 1,0 }, true);
                            }
                        }
                        else {
                            Point3D p = AP3D(buildings.begin()->pos);
                            DebugBox(agent, p - Point3D{ 1.5,1.5,0 }, p + Point3D{ 1.5,1.5,3 }, Colors::Red);
                            SendDebug(agent);
                            failedBuildings.push_back(*buildings.begin());
                            buildings.erase(buildings.begin());
                            return;
                        }
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
#ifdef SPEEDMINING
        //no buildings, potentially speedmining time
        else if (buildings.size() == 0) {
            //if no orders: speedmine ||
            //if one order: and that order is not harvest or unit is not NEAR gatherPoint, speedmine || (if the order is harvest directly, just speedharvest)
            //if one order: and that order is not return or unit is not NEAR returnPoint, speedmine || (if the order is return directly, just speedreturn)
            //if two orders: and first order is not a move command to gatherPoint or speedPoint, speedmine (maybe i can ignore this)
            
            std::vector<UnitOrder> orders = unit->orders;

            
            UnitWrapperPtr targ = getTargetTag(agent);
            ProbeTargetPtr probetarg = std::static_pointer_cast<ProbeTarget>(targ);
            //UnitWrapperPtr nexus = getNexus(agent);

            if (getReturn(agent)->is_selected) {
                printf("");
            }

            char speedmode = 0;
            if (orders.size() == 0){
                AvailableAbilities unitAbilities = agent->Query()->GetAbilitiesForUnit(getReturn(agent));
                bool hasMinerals = false;
                for (int a = 0; a < unitAbilities.abilities.size(); a++) {
                    if (unitAbilities.abilities[a].ability_id == ABILITY_ID::HARVEST_RETURN) {
                        hasMinerals = true;
                        break;
                    }
                }
                if (hasMinerals) {
                    if (probetarg->nexus != nullptr && probetarg->nexus->getReturn(agent)->build_progress == 1.0F) {
                        agent->Actions()->UnitCommand(self, ABILITY_ID::GENERAL_MOVE, probetarg->returnPoint);
                        agent->Actions()->UnitCommand(self, ABILITY_ID::SMART, probetarg->nexus->self, true);
                    }
                    else {
                        agent->Actions()->UnitCommand(self, ABILITY_ID::HARVEST_RETURN);
                    }
                }
                else {
                    if (probetarg->gatherPoint != Point2D()) {
                        agent->Actions()->UnitCommand(self, ABILITY_ID::GENERAL_MOVE, probetarg->gatherPoint);
                        agent->Actions()->UnitCommand(self, ABILITY_ID::HARVEST_GATHER, targ->self, true);
                    }
                    else {
                        agent->Actions()->UnitCommand(self, ABILITY_ID::HARVEST_GATHER, targ->self);
                    }

                }
            }
            else if (orders.size() == 1) {
                if (unit->orders[0].ability_id == ABILITY_ID::HARVEST_GATHER &&
                    unit->orders[0].target_unit_tag != targ->self) {
                    agent->Actions()->UnitCommand(self, ABILITY_ID::HARVEST_GATHER, targ->self);
                }
                else { 
                    //UnitWrappers inRange = SpatialHashGrid::findInRadiusSelfLoose(target->pos(agent), 8);
                    //bool foundNexus = false;
                    //for (const auto& unit : inRange) {
                    //    if (unit->getStorageType() == UNIT_TYPEID::PROTOSS_NEXUS) {
                    //        foundNexus = true;
                    //        break;
                    //    }
                    //}
                    if (probetarg->nexus != nullptr) { //URGENT: FIGURE OUT WHY MINERALS DONT HAVE THEIR NEXUSES MARKED
                        float distFromHarvest = DistanceSquared2D(probetarg->gatherPoint, pos(agent));
                        float distFromReturn = DistanceSquared2D(probetarg->returnPoint, pos(agent));
                        bool notAtHarvest = orders[0].ability_id == ABILITY_ID::HARVEST_GATHER && distFromHarvest > SpeedMining::GATHER_RANGE;
                        bool notAtReturn = orders[0].ability_id == ABILITY_ID::HARVEST_RETURN && distFromReturn > SpeedMining::GATHER_RANGE;
                        if (notAtHarvest && distFromHarvest < SpeedMining::DECCEL_RANGE_2 && probetarg->gatherPoint != Point2D()) {
                            agent->Actions()->UnitCommand(self, ABILITY_ID::GENERAL_MOVE, probetarg->gatherPoint);
                            agent->Actions()->UnitCommand(self, ABILITY_ID::HARVEST_GATHER, targ->self, true);
                        }
                        else if (notAtReturn && distFromReturn < SpeedMining::DECCEL_RANGE_2 && probetarg->nexus != nullptr) {
                            agent->Actions()->UnitCommand(self, ABILITY_ID::GENERAL_MOVE, probetarg->returnPoint);
                            agent->Actions()->UnitCommand(self, ABILITY_ID::SMART, probetarg->nexus->self, true);
                        }
                    }
                }
            }
            //else if (orders.size() == 2) {
            //    if (orders[0].ability_id == ABILITY_ID::GENERAL_MOVE && (orders[1].ability_id == ABILITY_ID::HARVEST_GATHER || orders[1].ability_id == ABILITY_ID::HARVEST_RETURN)) {
            //        if (found) { //make sure that this persists until lad is no longer in range
            //            agent->Actions()->UnitCommand(self, ABILITY_ID::SMART, probetarg->nexus->self, true);
            //        }
            //    }
            //}
        }
#else
        //if unit has no buildings
        //and if unit has either no orders, or the order is harvest
        else if((unit->orders.size() == 0 || unit->orders[0].ability_id == ABILITY_ID::HARVEST_GATHER) && buildings.size() == 0) {
            UnitWrapperPtr targ = getTargetTag(agent);
            //if unit has no target
            //and if unit has either no orders, or the order is harvest to the wrong target
            if (targ != nullptr && (unit->orders.size() == 0 || (unit->orders[0].ability_id == ABILITY_ID::HARVEST_GATHER &&
                unit->orders[0].target_unit_tag != targ->self))) {
                agent->Actions()->UnitCommand(self, ABILITY_ID::HARVEST_GATHER, targ->self);
            }
        }
#endif

        //render the pos
        if (target != nullptr) {
            UnitWrapperPtr targ = getTargetTag(agent);
            ProbeTargetPtr probetarg = std::static_pointer_cast<ProbeTarget>(targ);

            float epsilon = 0.03;
            if (getReturn(agent)->is_selected) {
                printf("");
                epsilon = 1.5;
            }
            Point2D pt = probetarg->gatherPoint;
            DebugBox(agent, AP3D(pt) + Point3D{ -0.1, -0.1, -0.1 }, AP3D(pt) + Point3D{ 0.1, 0.1, epsilon });

            Point3D targetPos = target->pos3D(agent);
            DebugBox(agent, targetPos + Point3D{ -1.375, -0.875, -0.1 }, targetPos + Point3D{ 1.375, 0.875, 0.02 }, Colors::Red);

            UnitWrapperPtr nexus = probetarg->nexus;
            if (nexus != nullptr) {
                Point2D rt = SpeedMining::calculateReturnPoint(agent, pt, nexus->pos(agent));
                DebugBox(agent, AP3D(rt) + Point3D{ -0.1, -0.1, -0.1 }, AP3D(rt) + Point3D{ 0.1, 0.1, epsilon }, Colors::Teal);
                //SendDebug(agent);
            }
        }
        if (getReturn(agent)->orders.size() != 0 && getReturn(agent)->orders[0].ability_id == ABILITY_ID::HARVEST_GATHER) {
            DebugBox(agent, pos3D(agent) + Point3D{ -0.15,-0.15,1 - 0.15 }, pos3D(agent) + Point3D{ 0.15,0.15,1 + 0.15 }, Colors::Purple);
        }else if (getReturn(agent)->orders.size() != 0 && getReturn(agent)->orders[0].ability_id == ABILITY_ID::HARVEST_RETURN) {
            DebugBox(agent, pos3D(agent) + Point3D{ -0.15,-0.15,1 - 0.15 }, pos3D(agent) + Point3D{ 0.15,0.15,1 + 0.15 }, Colors::Yellow);
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