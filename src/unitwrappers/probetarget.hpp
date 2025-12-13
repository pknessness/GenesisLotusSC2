#pragma once
#include <sc2api/sc2_api.h>
#include <map>
#include "../auxiliary/helpers.hpp"
#include "../auxiliary/debugging.hpp"
#include "unitmanager.hpp"

//speedmining: 
// https://discord.com/channels/350289306763657218/350289596661104641/896296558528700426
// https://discord.com/channels/350289306763657218/350289596661104641/896469082122117170

//
//from sc2.position import Point2
//import numpy as np
//from typing import List
//
//GATHER_RANGE = 0.04424
//
//
//def line(p1, p2) :
//    A = (p1[1] - p2[1])
//    B = (p2[0] - p1[0])
//    C = (p1[0] * p2[1] - p2[0] * p1[1])
//    return A, B, -C
//
//
//    def intersection(L1, L2) :
//    D = L1[0] * L2[1] - L1[1] * L2[0]
//    Dx = L1[2] * L2[1] - L1[1] * L2[2]
//    Dy = L1[0] * L2[2] - L1[2] * L2[0]
//    if D != 0 :
//        x = Dx / D
//        y = Dy / D
//        return x, y
//    else:
//return False
//
//
//def to_line(p1, l1, l2) :
//    p1 = np.array(l1)
//    p2 = np.array(l2)
//
//    p3 = np.array(p1)
//    return np.abs(np.linalg.norm(np.cross(p2 - p1, p1 - p3))) / np.linalg.norm(p2 - p1)
//
//
//    def in_rect(rect, pt) :
//    return pt and rect[0] <= pt[0] <= rect[2] and rect[1] <= pt[1] <= rect[3]
//
//
//    async def on_start(self) :
//    self.client.game_step = 1
//    await self.assign_workers_game_start()
//
//
//    def calculate_gather_points(agent, minerals, townhall)->List[List] :
//    gather_points = []
//    for mineral in minerals :
//min_center = (mineral.position.x, mineral.position.y)
//th_center = (townhall.position.x, townhall.position.y)
//min_to_th = line(min_center, th_center)
//
//min_x1 = mineral.position.x - 1
//min_x2 = mineral.position.x + 1
//min_y1 = mineral.position.y - 0.5
//min_y2 = mineral.position.y + 0.5
//
//# corners of mineral
//min_tl = (min_x1, min_y2)
//min_tr = (min_x2, min_y2)
//min_bl = (min_x1, min_y1)
//min_br = (min_x2, min_y1)
//
//# find intersection of line from townhall to mineral centre
//# on edgesof mineral field
//inters = [intersection(min_to_th, line(min_tl, min_tr)),
//intersection(min_to_th, line(min_bl, min_br)),
//intersection(min_to_th, line(min_tl, min_bl)),
//intersection(min_to_th, line(min_tr, min_br))]
//
//rect = [min_x1, min_y1, min_x2, min_y2]
//inters_valid = [x for x in inters if in_rect(rect, x)]
//
//inters_dist = [townhall.distance_to(Point2(inter)) for inter in inters_valid]
//
//inters_zipped = zip(inters_valid, inters_dist)
//inters_sorted = sorted(inters_zipped, key = lambda x : x[1])
//
//# closest point
//worker_offset = 0.375 + GATHER_RANGE  # worker radius + gather range
//closest_inter = Point2(inters_sorted[0][0])
//
//gather_point = closest_inter.towards(townhall, worker_offset)
//
//if agent.in_pathing_grid(gather_point) :
//    gather_points.append([mineral, gather_point])
//else:
//# TODO : IF SOMETHING GOES WRONG THIS IS THE CAUSE IM SURE
//#       check if can be blocked on y axis
//blocking_mineral_pos = minerals.filter(lambda x : x != mineral).closest_to(gather_point).position
//dx = mineral.position.x - blocking_mineral_pos.position.x
//offset = 0.4 if dx == 0 else 0.4 * dx / abs(dx)
//offset_point = gather_point + Point2((offset, 0))
//gather_points.append([mineral, offset_point])
//
//return gather_points
//
//
//def sort_minerals_to_townhall(agent, minerals, townhall) :
//    gather_points = calculate_gather_points(agent, minerals, townhall)
//    minerals_sorted_zipped = sorted(gather_points, key = lambda x : x[1].distance_to(townhall))
//    minerals_sorted = [m[0] for m in minerals_sorted_zipped]
//    return minerals_sorted



#define SPEEDMINING

namespace SpeedMining {

    constexpr float GATHER_RANGE = 0.04424;
    constexpr float GATHER_RANGE_2 = GATHER_RANGE * GATHER_RANGE;

    constexpr float DECCEL_RANGE = 1.5;
    constexpr float DECCEL_RANGE_2 = DECCEL_RANGE * DECCEL_RANGE;

    constexpr float FLOATING_POINT_BULLSHIT_EPSILON = 0.0001;

    constexpr float PROBE_AVOID_SEARCH_RANGE = 0.375 + 0.1; //0.375 + tolerance (0.1)
    constexpr float PROBE_AVOID_RANGE_CENTER = 0.375 + 0.375 + 0.1; //0.375 + 0.375 + tolerance (0.1)
    constexpr float PROBE_AVOID_RANGE_CENTER_2 = PROBE_AVOID_RANGE_CENTER * PROBE_AVOID_RANGE_CENTER;

    //form is (y_coef)(y) + (x_coef)(x) = y_intercept_neg
    //y = (y_intercept_neg - (x_coef)(x))/y_coef
    struct Line {
        float x_coef; //rise
        float y_coef; //run
        float y_intercept_neg;

    };

    Line line(Point2D point1, Point2D point2) {
        return Line{ (point1.y - point2.y), (point2.x - point1.x), -(point1.x * point2.y - point2.x * point1.y) };
    }

    Point2D towards(Point2D source, Point2D destination, float distance) {
        Point2D vector = Aux::normalize(destination - source);
        return source + (vector * distance);
    }

    // x_min, y_min, x_max, y_max;
    struct Rect {
        float x_min;
        float y_min;
        float x_max;
        float y_max;

        Point2D tl() {
            return Point2D{ x_min, y_max };
        }

        Point2D tr() {
            return Point2D{ x_max, y_max };
        }

        Point2D bl() {
            return Point2D{ x_min, y_min };
        }

        Point2D br() {
            return Point2D{ x_max, y_min };
        }

        bool isInside(Point2D point) {
            return point != Point2D() && x_min <= point.x && point.x <= x_max && y_min <= point.y && point.y <= y_max;
        }
    };

    struct NexusOctagon {
        Point2D p0;
        Point2D p1;
        Point2D p2;
        Point2D p3;
        Point2D p4;
        Point2D p5;
        Point2D p6;
        Point2D p7;

        Point2D center;

        NexusOctagon(Point2D center_) {
            center = center_;

            p0 = center + Point2D{ 2.5,1 };
            p1 = center + Point2D{ 1,2.5 };
            p2 = center + Point2D{ -1,2.5 };
            p3 = center + Point2D{ -2.5,1 };
            p4 = center + Point2D{ -2.5,-1 };
            p5 = center + Point2D{ -1,-2.5 };
            p6 = center + Point2D{ 1,-2.5 };
            p7 = center + Point2D{ 2.5,-1 };
        }

        Line lineTR() {
            return line(p0, p1);
        }

        Line lineT() {
            return line(p1, p2);
        }

        Line lineTL() {
            return line(p2, p3);
        }

        Line lineL() {
            return line(p3, p4);
        }

        Line lineBL() {
            return line(p4, p5);
        }

        Line lineB() {
            return line(p5, p6);
        }

        Line lineBR() {
            return line(p6, p7);
        }

        Line lineR() {
            return line(p7, p0);
        }

        //y = (y_intercept_neg - (x_coef)(x))/y_coef
        float eval(Line l, float x) {
            return (l.y_intercept_neg - (l.x_coef) * x) / l.y_coef;
        }

        void render(Agent* const agent) {
            DebugLine(agent, AP3D(p0) + Point3D{ 0,0,0.02 }, AP3D(p1) + Point3D{ 0,0,0.02 }, Colors::Green);
            DebugLine(agent, AP3D(p1) + Point3D{ 0,0,0.02 }, AP3D(p2) + Point3D{ 0,0,0.02 }, Colors::Green);
            DebugLine(agent, AP3D(p2) + Point3D{ 0,0,0.02 }, AP3D(p3) + Point3D{ 0,0,0.02 }, Colors::Green);
            DebugLine(agent, AP3D(p3) + Point3D{ 0,0,0.02 }, AP3D(p4) + Point3D{ 0,0,0.02 }, Colors::Green);
            DebugLine(agent, AP3D(p4) + Point3D{ 0,0,0.02 }, AP3D(p5) + Point3D{ 0,0,0.02 }, Colors::Green);
            DebugLine(agent, AP3D(p5) + Point3D{ 0,0,0.02 }, AP3D(p6) + Point3D{ 0,0,0.02 }, Colors::Green);
            DebugLine(agent, AP3D(p6) + Point3D{ 0,0,0.02 }, AP3D(p7) + Point3D{ 0,0,0.02 }, Colors::Green);
            DebugLine(agent, AP3D(p7) + Point3D{ 0,0,0.02 }, AP3D(p0) + Point3D{ 0,0,0.02 }, Colors::Green);

        }

        bool isInside(Point2D point) {
            if (point != Point2D()
                && (center.x - 2.5 - FLOATING_POINT_BULLSHIT_EPSILON <= point.x) && (point.x <= center.x + 2.5 + FLOATING_POINT_BULLSHIT_EPSILON)
                && (center.y - 2.5 - FLOATING_POINT_BULLSHIT_EPSILON <= point.y) && (point.y <= center.y + 2.5 + FLOATING_POINT_BULLSHIT_EPSILON)) {
                //bool tr = eval(lineTR(), point.x) >= point.y;
                //bool tl = eval(lineTL(), point.x) >= point.y;
                //bool bl = eval(lineBL(), point.x) <= point.y;
                //bool br = eval(lineBR(), point.x) <= point.y;
                //return (tr && tl && bl && br);

                Line tr_line = lineTR();
                float tr_y = eval(tr_line, point.x);
                bool tr = tr_y >= point.y - FLOATING_POINT_BULLSHIT_EPSILON;

                Line tl_line = lineTL();
                float tl_y = eval(tl_line, point.x);
                bool tl = tl_y >= point.y - FLOATING_POINT_BULLSHIT_EPSILON;

                Line bl_line = lineBL();
                float bl_y = eval(bl_line, point.x);
                bool bl = bl_y <= point.y + FLOATING_POINT_BULLSHIT_EPSILON;

                Line br_line = lineBR();
                float br_y = eval(br_line, point.x);
                bool br = br_y <= point.y + FLOATING_POINT_BULLSHIT_EPSILON;
                return (tr && tl && bl && br);
            }
            return false;
        }
    };

    Point2D intersection(Line line1, Line line2) {
        float D = line1.x_coef * line2.y_coef - line1.y_coef * line2.x_coef;
        if (D != 0) {
            float Dx = line1.y_intercept_neg * line2.y_coef - line1.y_coef * line2.y_intercept_neg;
            float Dy = line1.x_coef * line2.y_intercept_neg - line1.y_intercept_neg * line2.x_coef;
            return Point2D{ (Dx / D), (Dy / D) };
        }
        return Point2D();
    }

    Point2D calculateGatherPoint(Agent* const agent, Point2D mineral_position, Point2D nexus_position) {
        Line mineralToNexus = line(mineral_position, nexus_position);

        Rect mineral_bounds{ mineral_position.x - 1, mineral_position.y - 0.5 , mineral_position.x + 1,mineral_position.y + 0.5 };

        Point2D intersections[4] = {
            intersection(mineralToNexus, line(mineral_bounds.tl(), mineral_bounds.tr())),
            intersection(mineralToNexus, line(mineral_bounds.bl(), mineral_bounds.br())),
            intersection(mineralToNexus, line(mineral_bounds.tl(), mineral_bounds.bl())),
            intersection(mineralToNexus, line(mineral_bounds.tr(), mineral_bounds.br()))
        };

        Point2D closest;
        float closestDistance2 = -1;
        for (int i = 0; i < 4; i++) {
            if (mineral_bounds.isInside(intersections[i])) {
                float dist2 = DistanceSquared2D(intersections[i], nexus_position);
                if (closestDistance2 == -1 || dist2 < closestDistance2) {
                    closestDistance2 = dist2;
                    closest = intersections[i];
                }
            }
        }

        Point2D gatherPoint = towards(closest, nexus_position, 0.375 + GATHER_RANGE); //probe radius + gather range
        if (!Aux::isPathable(gatherPoint)) {
            UnitWrapperPtr blockingMineral = nullptr;
            float nearestDistance2 = -1;
            for (const auto& unit : UnitManager::getMinerals()) {
                if (unit->pos(agent) == mineral_position) {
                    continue;
                }
                float dist2 = DistanceSquared2D(unit->pos(agent), gatherPoint);
                if (blockingMineral == nullptr || dist2 < nearestDistance2) {
                    nearestDistance2 = dist2;
                    blockingMineral = unit;
                }
            }
            if (blockingMineral != nullptr) {
                float distX = mineral_position.x - blockingMineral->pos(agent).x;
                float offset = (distX == 0 ? (0.4) : (distX > 0 ? 0.4 : -0.4));
                return Point2D{ gatherPoint.x + offset, gatherPoint.y };
            }
        }
        return gatherPoint;
    }

    Point2D calculateReturnPoint(Agent* const agent, Point2D gather_position, Point2D nexus_position_) {
        Line nexusToMineral = line(nexus_position_, gather_position);

        NexusOctagon nexus_bounds(nexus_position_);

        Point2D intersections[8] = {
            intersection(nexusToMineral, nexus_bounds.lineTR()),
            intersection(nexusToMineral, nexus_bounds.lineT()),
            intersection(nexusToMineral, nexus_bounds.lineTL()),
            intersection(nexusToMineral, nexus_bounds.lineL()),
            intersection(nexusToMineral, nexus_bounds.lineBL()),
            intersection(nexusToMineral, nexus_bounds.lineB()),
            intersection(nexusToMineral, nexus_bounds.lineBR()),
            intersection(nexusToMineral, nexus_bounds.lineR()),
        };

        Point2D closest;
        float closestDistance2 = -1;
        for (int i = 0; i < 8; i++) {
            //DebugSphere(agent, AP3D(intersections[i]), 0.1, Colors::Red);
            //nexus_bounds.render(agent);
            //SendDebug(agent);
            if (nexus_bounds.isInside(intersections[i])) {
                float dist2 = DistanceSquared2D(intersections[i], gather_position);
                if (closestDistance2 == -1 || dist2 < closestDistance2) {
                    closestDistance2 = dist2;
                    closest = intersections[i];
                }
            }
        }

        Point2D returnPoint = towards(closest, gather_position, 0.375 + GATHER_RANGE); //probe radius + gather range
        return returnPoint;
    }
}

class ProbeTarget : public UnitWrapper {
private:
public:

    UnitWrapperPtr nexus;
    Point2D gatherPoint;
    Point2D returnPoint;

    ProbeTarget(const Unit* unit, UnitTypeID sType) : UnitWrapper(unit, sType), nexus(nullptr), gatherPoint(), returnPoint() {

    }

    void init(Agent* const agent) {\
        for (auto const& expand : Aux::expansions) {
            float dist2 = DistanceSquared2D(expand.pos, pos(agent));
            if (dist2 < 100) {
                if (getStorageType() == UNIT_TYPEID::NEUTRAL_MINERALFIELD) {
                    gatherPoint = SpeedMining::calculateGatherPoint(agent, pos(agent), expand.pos);
                    returnPoint = SpeedMining::calculateReturnPoint(agent, gatherPoint, expand.pos);
                }
                else if (getStorageType() == UNIT_TYPEID::PROTOSS_ASSIMILATOR) {
                    returnPoint = SpeedMining::calculateReturnPoint(agent, pos(agent), expand.pos);
                }

            }
        }
    }

    void addNexus(UnitWrapperPtr nexus_) {
        nexus = nexus_;
    }
};

typedef std::shared_ptr<ProbeTarget> ProbeTargetPtr;