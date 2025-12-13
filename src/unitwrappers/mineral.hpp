#pragma once
#include <sc2api/sc2_api.h>
#include <map>
#include "../auxiliary/helpers.hpp"
#include "../auxiliary/debugging.hpp"
#include "unitmanager.hpp"


class Mineral : public UnitWrapper {
private:
public:

    UnitWrapperPtr nexus;
    Point2D gatherPoint;
    Point2D returnPoint;

    Mineral(const Unit* unit) : UnitWrapper(unit, UNIT_TYPEID::NEUTRAL_MINERALFIELD), nexus(nullptr), gatherPoint(), returnPoint(){
        nearest
    }

    void addNexus(UnitWrapperPtr nexus_){
        nexus = nexus_;
    }
};

typedef std::shared_ptr<Mineral> MineralPtr;