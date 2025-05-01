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

    static void init() {
        visibleMap = new map2d<int16_t>(Aux::mapWidth_cache, Aux::mapHeight_cache, true);
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
            for (int i = 0; i < (Aux::mapHeight_cache / visibleCellSize) + 1; i++) {

            }
        }
    }

    static void updateEnemy(Agent* const agent) {
        resetEnemy();
        for (auto it = UnitManager::enemy_units.begin(); it != UnitManager::enemy_units.end(); it++) {
            for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
                Bounds b = fillSpacialMapRadius((*it2)->pos(agent), (*it2)->radius(agent), [it2](int i, int j) {imRef(spatialGridEnemy, i, j).insert(*it2);});
                DebugSphere(agent, (*it2)->pos3D(agent), (*it2)->radius(agent));
            }
        }
    }

    static UnitWrappers findInRadiusSelf_INTERNAL(Bounds b) {
        UnitWrappers found;
        for (int i = b.xmin; i < b.xmax; i++) {
            for (int j = b.ymin; j < b.ymax; j++) {
                if (i < 0 || j < 0 || i >= Aux::mapWidth_cache || j >= Aux::mapHeight_cache) {
                    continue;
                }
                if (imRef(gridModify, i, j) == 1) {
                    for (auto it = imRef(spatialGridSelf, i, j).begin(); it != imRef(spatialGridSelf, i, j).end(); it++) {
                        found.insert(*it);
                    }
                }
            }
        }
        return found;
    }

    static UnitWrappers findInRadiusSelfLoose(Point2D pos, float radius) {
        resetModify();
        Bounds b = fillSpacialModify(pos, radius);

        return findInRadiusSelf_INTERNAL(b);
    }

    static inline UnitWrappers findInRadiusSelfLoose(Circle c) {
        return findInRadiusSelfLoose(c.pos, c.radius);
    }

    static UnitWrappers findInRadiiSelfLoose(Circles circles) {
        if (circles.size() == 0) {
            return UnitWrappers();
        }
        Bounds bound = fillSpacialModify(circles.begin()->pos, circles.begin()->radius);
        for (auto it = circles.begin(); it != circles.end(); it++) {
            bound += fillSpacialModify(*it);
        }

        return findInRadiusSelf_INTERNAL(bound);
    }

    static UnitWrappers findInRadiusEnemy_INTERNAL(Bounds b) {
        UnitWrappers found;
        for (int i = b.xmin; i < b.xmax; i++) {
            for (int j = b.ymin; j < b.ymax; j++) {
                if (i < 0 || j < 0 || i >= Aux::mapWidth_cache || j >= Aux::mapHeight_cache) {
                    continue;
                }
                if (imRef(gridModify, i, j) == 1) {
                    for (int u = 0; u < imRef(spatialGridEnemy, i, j).size(); u++) {
                        for (auto it = imRef(spatialGridEnemy, i, j).begin(); it != imRef(spatialGridEnemy, i, j).end(); it++) {
                            found.insert(*it);
                        }
                    }
                }
            }
        }
        return found;
    }

    static UnitWrappers findInRadiusEnemyLoose(Point2D pos, float radius) {
        resetModify();
        Bounds b = fillSpacialModify(pos, radius);

        return findInRadiusEnemy_INTERNAL(b);
    }

    static inline UnitWrappers findInRadiusEnemyLoose(Circle c) {
        return findInRadiusEnemyLoose(c.pos, c.radius);
    }

    static UnitWrappers findInRadiiEnemyLoose(Circles circles) {
        if (circles.size() == 0) {
            return UnitWrappers();
        }
        Bounds bound = fillSpacialModify(circles.begin()->pos, circles.begin()->radius);
        for (auto it = circles.begin(); it != circles.end(); it++) {
            bound += fillSpacialModify(*it);
        }

        return findInRadiusEnemy_INTERNAL(bound);
    }

    static void debug(Agent* const agent) {
        Aux::gridTemplate(agent, [](int i, int j) {
            if (imRef(spatialGridSelf, realScaleToSpacial(i), realScaleToSpacial(j)).size() > 0) {
                return Color{ 90, 135, 205 };
            }
            else if (imRef(spatialGridEnemy, realScaleToSpacial(i), realScaleToSpacial(j)).size() > 0) {
                return Color{ 215, 55, 25 };
            }
            return Color{ 255, 255, 255 };
            }, spatialCellSize);
    }
}