#pragma once
#include <sc2api/sc2_api.h>
#include "../auxiliary/helpers.hpp"
#include "unitwrapper.hpp"
#include "../auxiliary/squadmanager.hpp"
#include "../auxiliary/visiblemap.hpp"
#include "../auxiliary/weapongrid.hpp"

class ArmyUnit : public UnitWrapper {
private:
public:

    SquadManager::Squad* squad;

    Point2D moveLocation;

    ArmyUnit(const Unit* unit, UnitTypeID sType, SquadManager::Squad* squad_) : UnitWrapper(unit, sType), squad(squad_){
        squad->squadMainStates[unit->tag] = 'u';
        squad->unitStates[unit->tag] = ' ';
    }

    virtual void atk(Agent* const agent, Point2D point) {
        agent->Actions()->UnitCommand(self, ABILITY_ID::ATTACK, point);
    }

    virtual void atk(Agent* const agent, UnitWrapper* target) {
        agent->Actions()->UnitCommand(self, ABILITY_ID::ATTACK, target->self);
    }

    virtual void atkmov(Agent* const agent, Point2D point) {
        if (get(agent)->weapon_cooldown > 0) {
            agent->Actions()->UnitCommand(self, ABILITY_ID::GENERAL_MOVE, point);
        }
        else {
            agent->Actions()->UnitCommand(self, ABILITY_ID::ATTACK, point);
        }
    }

    virtual void mov(Agent* const agent, Point2D point) {
        agent->Actions()->UnitCommand(self, ABILITY_ID::GENERAL_MOVE, point);
    }

    //void damageAtCell(DamageGrid::DamageCell cell, Agent* const agent) {
    //    CompositionAsTarget comp = getCompositionAsTarget(agent);
    //    float damage = 0;
    //    if (comp == CompositionAsTarget::Ground || comp == CompositionAsTarget::Any) {
    //        damage += cell.normal_gnd;
    //        for (Attribute a : Aux::getStats(getActualType(agent), agent).attributes) {
    //            switch (a) {
    //            case(Attribute::Light): {
    //                damage += pointDamage.ground.light;
    //            } break;
    //            case(Attribute::Armored): {
    //                damage += pointDamage.ground.armored;
    //            } break;
    //            case(Attribute::Biological): {
    //                damage += pointDamage.ground.biological;
    //            } break;
    //            case(Attribute::Mechanical): {
    //                damage += pointDamage.ground.mechanical;
    //            } break;
    //            case(Attribute::Massive): {
    //                damage += pointDamage.ground.massive;
    //            } break;
    //            case(Attribute::Psionic): {
    //                damage += pointDamage.ground.psionic;
    //            } break;
    //            }
    //        }

    //    }
    //    if (comp == Composition::Air || comp == Composition::Any) {
    //        damage += pointDamage.air.normal;
    //        for (Attribute a : Aux::getStats(unitWrap->getType(agent), agent).attributes) {
    //            switch (a) {
    //            case(Attribute::Light): {
    //                damage += pointDamage.air.light;
    //            } break;
    //            case(Attribute::Armored): {
    //                damage += pointDamage.air.armored;
    //            } break;
    //            case(Attribute::Biological): {
    //                damage += pointDamage.air.biological;
    //            } break;
    //            case(Attribute::Mechanical): {
    //                damage += pointDamage.air.mechanical;
    //            } break;
    //            case(Attribute::Massive): {
    //                damage += pointDamage.air.massive;
    //            } break;
    //            case(Attribute::Psionic): {
    //                damage += pointDamage.air.psionic;
    //            } break;
    //            }
    //        }

    //    }
    //    return damage;
    //}

    virtual void executeAttack(Agent* const agent) {
        FUNCTION_LOG();
        moveLocation = Point2D{ -1, -1 };
        if (squad->squadMainStates[self] == 'u') {
            moveLocation = squad->targetPosition;
            if (get(agent)->weapon_cooldown > 0) {
                squad->unitStates[self] = 'n';
            }
            else {
                squad->unitStates[self] = 'k';
            }
        }
        if (squad->squadMainStates[self] == 'u') {
            if (squad->unitStates[self] = 'n') {
                mov(agent, squad->getCorePosition(agent));
            }
            else if (squad->unitStates[self] == 'k') {
                atk(agent, squad->getCorePosition(agent));
            }
            else {
                atk(agent, squad->getCorePosition(agent));
            }
            
        }
        else if (squad->squadMainStates[self] == 'j') {
            atk(agent, squad->targetPosition);
        }
    }

    virtual void executeHarass(Agent* const agent) {
        FUNCTION_LOG();
        atk(agent, squad->targetPosition);
    }

    virtual void executeDefend(Agent* const agent) {
        FUNCTION_LOG();
        atk(agent, squad->targetPosition);
    }

    float searchCost(Point2D p) {
        FUNCTION_LOG();
        if (p.x == 0 && p.y == 0) {
            return -1;
        }
        return imRef(VisibleMap2D::visibleMap, 
            VisibleMap2D::realScaleToVisMap((int)(p.x)), 
            VisibleMap2D::realScaleToVisMap((int)(p.y)));
    }

    virtual void executeSearch(Agent* const agent) {
        FUNCTION_LOG();
        //atk(agent, squad->targetPosition);
        float cost = -1;
        if (Aux::withinBounds(moveLocation)) {
            cost = searchCost(moveLocation);
        }
        //posTarget = { 0,0 };
        for (int i = 0; i < 5; i++) {
            Point2D check;
            check = Aux::getRandomPathable(agent);
            float cos = searchCost(check);
            if (cos < cost || cost == -1) {
                cost = cos;
                moveLocation = check;
            }
        }
        atkmov(agent, moveLocation);
    }

    virtual void execute(Agent* const agent) {
        FUNCTION_LOG();
        if (get(agent) == nullptr) {
            return;
        }
        if (squad->squadMainStates[self] == 'u' && DistanceSquared2D(pos(agent), squad->getCorePosition(agent)) < squad->armyballSquaredRadius()) {
            squad->squadMainStates[self] = 'j';
        }
        if (squad->squadMode == SquadManager::ATTACK) {
            executeAttack(agent);
        }
        else if (squad->squadMode == SquadManager::HARASS) {
            executeHarass(agent);
        }
        else if (squad->squadMode == SquadManager::DEFEND) {
            executeDefend(agent);
        }
        else if (squad->squadMode == SquadManager::SEARCH) {
            executeSearch(agent);
        }
    }
};

typedef std::shared_ptr<ArmyUnit> ArmyUnitPtr;