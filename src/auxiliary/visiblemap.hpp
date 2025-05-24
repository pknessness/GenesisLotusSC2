//I dedicate this class to Thom
#pragma once
#include <sc2api/sc2_api.h>

#include <map>
#include "map2d.hpp"

#include "helpers.hpp"
#include "../unitwrappers/unitmanager.hpp"
#include "spatialhashgrid.hpp"

namespace VisibleMap2D {

    constexpr int8_t visibleCellSize = 3;

    static map2d<int16_t>* visibleMap;

    int realScaleToVisMap(int dimension) {
        return dimension / visibleCellSize;
    }

    Point2D realScaleToVisMap(Point2D dimension) {
        return dimension / visibleCellSize;
    }

    static void init() {
        visibleMap = new map2d<int16_t>(Aux::mapWidth_cache / visibleCellSize + 1, Aux::mapHeight_cache / visibleCellSize + 1, true);
    }

    static void reset() {
        visibleMap->clear();
    }

    static SpatialHashGrid::Bounds fillVisibleMapRadius(Point2D pos, float radius, int16_t value) {
        int x = (int)((pos.x - radius) / visibleCellSize);
        int y = (int)((pos.y - radius) / visibleCellSize);
        int xmax = (int)((pos.x + radius) / visibleCellSize) + 1;
        int ymax = (int)((pos.y + radius) / visibleCellSize) + 1;
        for (int i = x; i < xmax; i++) {
            for (int j = y; j < ymax; j++) {
                Point2D blockCenter = Point2D{ (float)i + 0.5F,(float)j + 0.5F };
                float f = Distance2D(pos / visibleCellSize, blockCenter) - 1.414 / 2; //sqrt 2 / 2 because its only half of the square 

                if (i > 1 && i < visibleMap->width() && j > 1 && j < visibleMap->height() && f < (radius / visibleCellSize)) {
                    imRef(visibleMap, i, j) = value;
                }
            }
        }
        return { x, xmax, y, ymax };
    }

    static void update(Agent* const agent) {
        for (int i = 0; i < (Aux::mapWidth_cache / visibleCellSize) + 1; i++) {
            for (int j = 0; j < (Aux::mapHeight_cache / visibleCellSize) + 1; j++) {
                if (imRef(visibleMap, i, j) > 0) {
                    imRef(visibleMap, i, j) -= 1;
                }
                if (imRef(visibleMap, i, j) < 0) {
                    imRef(visibleMap, i, j) = 0;
                }
            }
        }

        //DO I WANT TO USE UNITWRAPPER FOR THIS OR GETUNITS??
        for (auto it = UnitManager::self_units.begin(); it != UnitManager::self_units.end(); it++) {
            for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
                fillVisibleMapRadius((*it2)->pos(agent), Aux::getStats((*it2)->getActualType(agent), agent).sight_range, 32767);
            }
        }
    }

    static void debug(Agent* const agent) {
        Aux::gridTemplate(agent, [](int i, int j) {
            return Color{ uint8_t(imRef(visibleMap, realScaleToVisMap(i), realScaleToVisMap(j)) / 255),
                uint8_t(imRef(visibleMap, realScaleToVisMap(i), realScaleToVisMap(j)) / 255),
                uint8_t(imRef(visibleMap, realScaleToVisMap(i), realScaleToVisMap(j)) / 255) };
            }, visibleCellSize);
    }
}