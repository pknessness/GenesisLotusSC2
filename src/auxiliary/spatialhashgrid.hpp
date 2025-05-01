//I dedicate this class to Thom
#pragma once
#include <sc2api/sc2_api.h>

#include <map>
#include "map2d.hpp"

#include "helpers.hpp"
#include "../unitwrappers/unitmanager.hpp"

struct Circle {
	Point2D pos;
	float radius;
};

using Circles = std::unordered_set<Circle>;

namespace SpatialHashGrid {

	constexpr int8_t spatialCellSize = 3;

	//struct spatialHashCell {
	//	UnitWrappers wrappers;

	//	spatialHashCell(){

	//	}
	//};

	static map2d<UnitWrappers>* spatialGridSelf;
    static map2d<UnitWrappers>* spatialGridEnemy;
    
    //Bit 0: Self Valid
    //Bit 1: Enemy Valid
    //Bit 2: Grid Modify
    static map2d<uint8_t>* gridModify;

    struct Bounds {
        int xmin;
        int xmax;
        int ymin;
        int ymax;

        Bounds operator+(const Bounds& b) {
            Bounds nu = { xmin,xmax,ymin,ymax };
            if (b.xmin < nu.xmin) {
                nu.xmin = b.xmin;
            }
            if (b.xmax > nu.xmax) {
                nu.xmax = b.xmax;
            }
            if (b.ymin < nu.ymin) {
                nu.ymin = b.ymin;
            }
            if (b.ymax > nu.ymax) {
                nu.ymax = b.ymax;
            }
            return nu;
        }

        void operator+=(const Bounds& b) {
            if (b.xmin < xmin) {
                xmin = b.xmin;
            }
            if (b.xmax > xmax) {
                xmax = b.xmax;
            }
            if (b.ymin < ymin) {
                ymin = b.ymin;
            }
            if (b.ymax > ymax) {
                ymax = b.ymax;
            }
        }
    };

    int realScaleToSpacial(int dimension) {
        return dimension / spatialCellSize;
    }

    Point2D realScaleToSpacial(Point2D dimension) {
        return dimension / spatialCellSize;
    }

    Circle realScaleToSpacial(Circle circle) {
        return Circle{ realScaleToSpacial(circle.pos), circle.radius / spatialCellSize };
    }

    static void init() {
        spatialGridSelf = new map2d<UnitWrappers>(Aux::mapWidth_cache, Aux::mapHeight_cache, false);
        spatialGridEnemy = new map2d<UnitWrappers>(Aux::mapWidth_cache, Aux::mapHeight_cache, false);
        gridModify = new map2d<uint8_t>(Aux::mapWidth_cache, Aux::mapHeight_cache, true);

        for (int i = 0; i < Aux::mapWidth_cache; i++) {
            for (int j = 0; j < Aux::mapHeight_cache; j++) {
                imRef(spatialGridSelf, i, j) = UnitWrappers();
                imRef(spatialGridEnemy, i, j) = UnitWrappers();
            }
        }
    }

    static void resetSelf() {
        for (int i = 0; i < Aux::mapWidth_cache; i++) {
            for (int j = 0; j < Aux::mapHeight_cache; j++) {
                //imRef(gridModify, i, j) &= 0xFE;
                imRef(spatialGridSelf, i, j).clear();
            }
        }
    }

    static void resetEnemy() {
        for (int i = 0; i < Aux::mapWidth_cache; i++) {
            for (int j = 0; j < Aux::mapHeight_cache; j++) {
                //imRef(gridModify, i, j) &= 0xFD;
                imRef(spatialGridEnemy, i, j).clear();
            }
        }
    }

    static void resetModify() {
        //for (int i = 0; i < Aux::mapWidth_cache; i++) {
        //    for (int j = 0; j < Aux::mapHeight_cache; j++) {
        //        //imRef(gridModify, i, j) &= 0xFB;
        //    }
        //}
        gridModify->clear();
    }

    static Bounds fillSpacialMapRadius(Point2D pos, float radius, std::function<void(int, int)> action) {
        int x = (int)((pos.x - radius) / spatialCellSize);
        int y = (int)((pos.y - radius) / spatialCellSize);
        int xmax = (int)((pos.x + radius) / spatialCellSize) + 1;
        int ymax = (int)((pos.y + radius) / spatialCellSize) + 1;
        for (int i = x; i < xmax; i++) {
            for (int j = y; j < ymax; j++) {
                Point2D blockCenter = Point2D{ (float)i + 0.5F,(float)j + 0.5F };
                float f = Distance2D(pos / spatialCellSize, blockCenter) - 1.414 / 2; //sqrt 2 / 2 because its only half of the square 

                if (i > 1 && i < gridModify->width() && j > 1 && j < gridModify->height() && f < (radius / spatialCellSize)) {
                    action(i, j);
                }
            }
        }
        return { x, xmax, y, ymax };
    }

    static inline Bounds fillSpacialModify(Point2D pos, float radius) {
        return fillSpacialMapRadius(pos, radius, [](int i, int j) {imRef(gridModify, i, j) = 1;});
    }

    static inline Bounds fillSpacialModify(Circle c) {
        return fillSpacialModify(c.pos, c.radius);
    }

    static void updateSelf(Agent* const agent) {
        resetSelf();
        for (auto it = UnitManager::self_units.begin(); it != UnitManager::self_units.end(); it++) {
            for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
                Bounds b = fillSpacialMapRadius((*it2)->pos(agent), (*it2)->radius(agent), [it2](int i, int j) {imRef(spatialGridSelf, i, j).insert(*it2);});
                DebugSphere(agent, (*it2)->pos3D(agent), (*it2)->radius(agent));
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
            }else if (imRef(spatialGridEnemy, realScaleToSpacial(i), realScaleToSpacial(j)).size() > 0) {
                return Color{ 215, 55, 25 };
            }
                return Color{ 255, 255, 255 };
            }, spatialCellSize);
    }
}