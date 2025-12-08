#pragma once
#include <sc2api/sc2_api.h>
#include <map>
#include "armyunit.hpp"
#include "../auxiliary/helpers.hpp"
#include "../auxiliary/debugging.hpp"
#include "unitmanager.hpp"

constexpr int WARPPRISM_COOLDOWN_FRAMES = 50;

class WarpPrism : public ArmyUnit {
private:
public:

    WarpPrism(const Unit* unit, SquadManager::Squad* squad_) : ArmyUnit(unit, UNIT_TYPEID::PROTOSS_WARPPRISM, squad_) {

    }

    virtual void execute(Agent* const agent) {

        float moveLocationDPS = WeaponGrid::getRadiusAvgDPS(moveLocation, radius(agent) + 3.5, WeaponGrid::wrapToTargetInfo(shared_from_this(), agent), agent);


        if (cooldownFrames > 0) {
            if ((target != nullptr && target->isDead()) || getReturn(agent)->orders.size() == 0) {
                cooldownFrames = 0;
            }
            else if (moveLocationDPS > prevMoveLocationDPS) {
                cooldownFrames /= 2;
                return;
            }
            else {
                cooldownFrames--;
                return;
            }
        }

        prevMoveLocationDPS = moveLocationDPS;

        //ArmyUnit::execute(agent);
        bool unitsWarpingInRange = false;
        UnitWrappers wraps = SpatialHashGrid::findInRadiusSelfLoose(pos(agent), PRISM_RADIUS_REAL);
        for (const auto& wrap : wraps) {
            if (wrap->getReturn(agent) != nullptr && !wrap->getReturn(agent)->is_building && !wrap->getReturn(agent)->IsBuildFinished()) {
                unitsWarpingInRange = true;
                break;
            }
        }
        if ((DistanceSquared2D(pos(agent), squad->getCorePosition(agent)) > 300 && !unitsWarpingInRange) || getEnemyDPS(pos(agent), 3.0F, agent) > 0 ) {
            bool unmorph = getActualType(agent) == UNIT_TYPEID::PROTOSS_WARPPRISMPHASING;
            if (unmorph){
                agent->Actions()->UnitCommand(self, ABILITY_ID::MORPH_WARPPRISMTRANSPORTMODE);
            }
            movSafely(agent, squad->getCorePosition(agent), 10, 14, 5.0F, unmorph);
            agent->Actions()->UnitCommand(self, ABILITY_ID::MORPH_WARPPRISMPHASINGMODE, true);
        }
        else if (DistanceSquared2D(pos(agent), squad->getCorePosition(agent)) < 200 && getActualType(agent) != UNIT_TYPEID::PROTOSS_WARPPRISMPHASING) {
            agent->Actions()->UnitCommand(self, ABILITY_ID::MORPH_WARPPRISMPHASINGMODE);
        }
        cooldownFrames = WARPPRISM_COOLDOWN_FRAMES;
    }

    //virtual void executeAttack(Agent* const agent) {
    //    ArmyUnit::executeAttack(agent);
    //}
};

typedef std::shared_ptr<WarpPrism> WarpPrismPtr;