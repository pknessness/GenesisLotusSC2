#pragma once
#include <sc2api/sc2_api.h>
#include <sc2lib/sc2_lib.h>

#include <string>
#include "debugging.hpp"
#include "map2d.hpp"
#include "bitmap.hpp"

using namespace sc2;

template< typename... Args >
std::string strprintf(const char* format, Args... args) {
	int length = std::snprintf(nullptr, 0, format, args...);
	assert(length >= 0);

	char* buf = new char[length + 1];
	std::snprintf(buf, length + 1, format, args...);

	std::string str(buf);
	delete[] buf;
	return str;
}

#define AP3D(point) Point3D{point.x, point.y, agent->Observation()->TerrainHeight(point)}

namespace Aux {
	GameInfo gameInfo_cache;
	int mapWidth_cache;
	int mapHeight_cache;

	/*
	* BITS 7 6 5 4 3 2 1 0
	* BIT [0]   | unpathable
	* BIT [1]   | unplacable
	* BIT [5:2] | obstacle info
	* BIT [7:6] | reserved
	* 
	* Obstacle info:
	* 0000: Nothing
	* 0001: Self Buildings
	* 0010: Enemy Buildings
	* 0011: Cliff Unpathable
	* 0100: Cliff Pathable
	* 0101: Minerals
	* 0111: Unpathable Rocks
	* 1000: Pathable Rocks 
	* 1001: Reserved 
	* 1010: Reserved 
	* 1011: Reserved 
	* 1100: Reserved 
	* 1101: Reserved 
	* 1110: Reserved 
	* 1111: Reserved 
	*/ 
	static std::shared_ptr<map2d<uint8_t>> masterMap;

	enum ObstacleInfo {
		NOTHING = 0x0,           // 0000: Nothing
		SELF_BUILDINGS = 0x1,    // 0001: Self Buildings
		ENEMY_BUILDINGS = 0x2,   // 0010: Enemy Buildings
		CLIFF_UNPATHABLE = 0x3,  // 0011: Cliff Unpathable
		CLIFF_PATHABLE = 0x4,    // 0100: Cliff Pathable
		MINERALS = 0x5,          // 0101: Minerals
		UNPATHABLE_ROCKS = 0x7,  // 0111: Unpathable Rocks
		PATHABLE_ROCKS = 0x8,    // 1000: Pathable Rocks
		// Reserved values
		RESERVED_9 = 0x9,        // 1001: Reserved
		RESERVED_A = 0xA,        // 1010: Reserved
		RESERVED_B = 0xB,        // 1011: Reserved
		RESERVED_C = 0xC,        // 1100: Reserved
		RESERVED_D = 0xD,        // 1101: Reserved
		RESERVED_E = 0xE,        // 1110: Reserved
		RESERVED_F = 0xF         // 1111: Reserved
	};

	bool isPathable(int i, int j) {
		return !(imRef(masterMap, i, j) & 0x01);
	}

	bool isPlacable(int i, int j) {
		return !(imRef(masterMap, i, j) & 0x02);
	}

	ObstacleInfo getObstacle(int i, int j) {
		return (ObstacleInfo)((imRef(masterMap, i, j) & 0x03C0) >> 2);
	}

	struct PointArea {
		enum PointAreaType {
			INVALID,
			DEFAULT_FINDOUT,
			SINGLE_POINT,
			POINT_RADIUS
		};

		PointAreaType pa_type;
		Point2D pos;
		float radius;

		PointArea() : pa_type(INVALID), pos({ 0, 0 }), radius(0.0F) {

		}

		PointArea(Point2D pos_) : pa_type(SINGLE_POINT), pos(pos_), radius(0.0F) {

		}

		PointArea(Point2D pos_, float radius_) : pa_type(POINT_RADIUS), pos(pos_), radius(radius_) {

		}

		PointArea(PointAreaType pa_type_, Point2D pos_, float radius_) : pa_type(pa_type_), pos(pos_), radius(radius_) {

		}
	};

	struct PointDefault : PointArea {
		PointDefault() : PointArea(DEFAULT_FINDOUT, Point2D{ -1, -1 }, 0.0F) {

		}
	};

	struct Expansion {
		Point3D pos;
		bool taken;

		//Tag vespene[2];
		//Tag mineral[8];

		Expansion(Point3D pos_) : pos(pos_), taken(false)/*, vespene{0}, mineral{0}*/ {
		}
	};

	struct ExpansionDistance {
		float distance;
		int expansionIndex;
	};

	class ExpansionDistanceCompare {
	public:
		bool operator()(const ExpansionDistance a, const ExpansionDistance b) const
		{
			return a.distance > b.distance;
		}
	};
	
	std::vector<Expansion> expansions;
	std::set<ExpansionDistance, ExpansionDistanceCompare> selfRankedExpansions;
	std::set<ExpansionDistance, ExpansionDistanceCompare> enemyRankedExpansions;

	void setupExpansions(Agent* const agent) {
		Point3D start = agent->Observation()->GetStartLocation();
		Point3D enemy = AP3D( gameInfo_cache.enemy_start_locations[0] );

		std::vector<Point3D> expands = sc2::search::CalculateExpansionLocations(agent->Observation(), agent->Query());
		expands.push_back(start);

		std::vector<QueryInterface::PathingQuery> selfQueries;
		std::vector<QueryInterface::PathingQuery> enemyQueries;
		for (int i = 0; i < expands.size(); i++) {
			QueryInterface::PathingQuery query;
			query.start_ = start;
			query.end_ = expands[i];
			selfQueries.push_back(query);
		}

		for (int i = 0; i < expands.size(); i++) {
			QueryInterface::PathingQuery query;
			query.start_ = enemy;
			query.end_ = expands[i];
			enemyQueries.push_back(query);
		}


		std::vector<float> selfDistances = agent->Query()->PathingDistance(selfQueries);
		std::vector<float> enemyDistances = agent->Query()->PathingDistance(enemyQueries);
		for (int i = 0; i < expands.size(); i++) {
			expansions.push_back(Expansion{ expands[i] });
			selfRankedExpansions.insert(ExpansionDistance{ selfDistances[i], i });
			enemyRankedExpansions.insert(ExpansionDistance{ enemyDistances[i], i });
		}
	}

	void displayExpansions(Agent* const agent) {
		for (int i = 0; i < expansions.size(); i++) {
			DebugSphere(agent, expansions[i].pos, 12, Colors::Yellow);
			float selfDist = -1.0F;
			float enemyDist = -1.0F;
			for (auto it = selfRankedExpansions.begin(); it != selfRankedExpansions.end(); it ++) {
				if (it->expansionIndex == i) {
					selfDist = it->distance;
					break;
				}
			}
			for (auto it = enemyRankedExpansions.begin(); it != enemyRankedExpansions.end(); it++) {
				if (it->expansionIndex == i) {
					enemyDist = it->distance;
					break;
				}
			}
			DebugText(agent, strprintf("S:%.1f, E:%.1f", selfDist, enemyDist), expansions[i].pos + Point3D{0,0,1});
		}
	}

	Point2D cliffCheckDisplace[8] = {
		Point2D{-1,1} , Point2D{0,1} , Point2D{1,1} ,
		Point2D{-1,0} ,                 Point2D{1,0} ,
		Point2D{-1,-1} , Point2D{0,-1} , Point2D{1,-1} };

	/*
	* 0 1 2
	* 3 , 4
	* 5 6 7
	*/
	static bool checkCliff(Point2D start, uint8_t slot1, uint8_t slot2, Agent* agent) {
		if (slot1 < 0 || slot1 > 7 || slot2 < 0 || slot2 > 7) {
			throw std::runtime_error("Out of bounds");
		}
		bool path1 = agent->Observation()->IsPathable(start + cliffCheckDisplace[slot1]);
		bool path2 = agent->Observation()->IsPathable(start + cliffCheckDisplace[slot2]);
		if (path1 && path2) {
			float height1 = agent->Observation()->TerrainHeight(start + cliffCheckDisplace[slot1]);
			float height2 = agent->Observation()->TerrainHeight(start + cliffCheckDisplace[slot2]);
			if ((height1 == (float)(int)height1) && (height2 == (float)(int)height2) && height1 != height2) {
				return true;
			}
		}
		return false;
	}

	void setupMasterMap(Agent* const agent) {
		for (int i = 0; i < masterMap->width(); i++) {
			for (int j = 0; j < masterMap->height(); j++) {
				Point2D check = Point2D{ i + 0.1F, j + 0.1F }; //arbitrary to reduce floating point casting errors.
				if (!agent->Observation()->IsPathable(check)) {
					imRef(masterMap, i, j) |= 0x1; //first bit is unpathable
					if (checkCliff(check, 1, 6, agent) || checkCliff(check, 3, 4, agent) ||
						checkCliff(check, 2, 6, agent) || checkCliff(check, 3, 2, agent) ||
						checkCliff(check, 0, 6, agent) || checkCliff(check, 3, 7, agent) ||
						checkCliff(check, 1, 6, agent) || checkCliff(check, 0, 4, agent) ||
						checkCliff(check, 1, 6, agent) || checkCliff(check, 5, 4, agent) ||
						checkCliff(check, 2, 6, agent) || checkCliff(check, 0, 7, agent)) {
						imRef(masterMap, i, j) |= ((uint8_t)(CLIFF_UNPATHABLE) << 2);
						continue;
					}
				}
				if (!agent->Observation()->IsPlacable(check)) {
					imRef(masterMap, i, j) |= 0x2; //second bit is unplacable
				}
			}
		}
		saveBitmap("masterMap.bmp", masterMap->width(), masterMap->height(), [](int i, int j) {
			if (getObstacle(i, j) == NOTHING) {
				if (!isPathable(i, j)) {
					return Color{ 0,0,0 };
				}
				else if (!isPlacable(i, j)) {
					return Color{ 100, 100, 100 };
				}
			} else if (getObstacle(i, j) == SELF_BUILDINGS) {
				return Color{ 30, 30, 255 };
			} else if (getObstacle(i, j) == ENEMY_BUILDINGS) {
				return Color{ 255, 30, 30 };
			} else if (getObstacle(i, j) == CLIFF_UNPATHABLE) {
				return Color{ 100, 100, 50 };
			} else if (getObstacle(i, j) == CLIFF_PATHABLE) {
				return Color{ 150, 150, 50 };
			} else if (getObstacle(i, j) == MINERALS) {
				return Color{ 50, 50, 150 };
			} else if (getObstacle(i, j) == UNPATHABLE_ROCKS) {
				return Color{ 150, 100, 100 };
			} else if (getObstacle(i, j) == PATHABLE_ROCKS) {
				return Color{ 150, 150, 100 };
			}
			return Color{ 255,255,255 };
			});
	}
}