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
		CLIFF_PATHABLE = 0x4,    // 0100: Cliff Pathable by Reapers/Colossus
		MINERALS = 0x5,          // 0101: Minerals
		UNPATHABLE_ROCKS = 0x7,  // 0111: Unpathable Rocks
		PATHABLE_ROCKS = 0x8,    // 1000: Pathable Rocks
		VESPENE = 0x9,           // 1001: Vespene
		// Reserved values
		RESERVED_A = 0xA,        // 1010: Reserved
		RESERVED_B = 0xB,        // 1011: Reserved
		RESERVED_C = 0xC,        // 1100: Reserved
		RESERVED_D = 0xD,        // 1101: Reserved
		RESERVED_E = 0xE,        // 1110: Reserved
		RESERVED_F = 0xF         // 1111: Reserved
	};

	ObstacleInfo getObstacle(int i, int j) {
		return (ObstacleInfo)(imRef(masterMap, i, j) >> 2);
	}

	bool isPathable(int i, int j) {
		ObstacleInfo obstacle = getObstacle(i, j);
		return !((imRef(masterMap, i, j) & 0x01) || 
			obstacle == SELF_BUILDINGS || 
			obstacle == ENEMY_BUILDINGS || 
			obstacle == CLIFF_UNPATHABLE || 
			obstacle == CLIFF_PATHABLE ||
			obstacle == MINERALS || 
			obstacle == UNPATHABLE_ROCKS ||
			obstacle == VESPENE);
	}

	bool isPlacable(int i, int j) {
		ObstacleInfo obstacle = getObstacle(i, j);
		return !(imRef(masterMap, i, j) & 0x02 || 
			obstacle == SELF_BUILDINGS ||
			obstacle == ENEMY_BUILDINGS ||
			obstacle == CLIFF_UNPATHABLE ||
			obstacle == CLIFF_PATHABLE ||
			obstacle == MINERALS ||
			obstacle == UNPATHABLE_ROCKS ||
			obstacle == PATHABLE_ROCKS || 
			obstacle == VESPENE);
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

	static bool isMineralType(UnitTypeID type) {
		return (type == UNIT_TYPEID::NEUTRAL_MINERALFIELD || type == UNIT_TYPEID::NEUTRAL_LABMINERALFIELD ||
			type == UNIT_TYPEID::NEUTRAL_MINERALFIELD750 || type == UNIT_TYPEID::NEUTRAL_LABMINERALFIELD750 ||
			type == UNIT_TYPEID::NEUTRAL_MINERALFIELD450 || type == UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD ||
			type == UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD750);
	}

	static bool isVespeneType(UnitTypeID type) {
		return (type == UNIT_TYPEID::NEUTRAL_VESPENEGEYSER || type == UNIT_TYPEID::NEUTRAL_PROTOSSVESPENEGEYSER ||
			type == UNIT_TYPEID::NEUTRAL_PURIFIERVESPENEGEYSER || type == UNIT_TYPEID::NEUTRAL_RICHVESPENEGEYSER ||
			type == UNIT_TYPEID::NEUTRAL_SHAKURASVESPENEGEYSER || type == UNIT_TYPEID::NEUTRAL_SPACEPLATFORMGEYSER);
	}

	Point2D cliffCheckDisplace[8] = {
		Point2D{-1,1} , Point2D{0,1} , Point2D{1,1} ,
		Point2D{-1,0} ,                 Point2D{1,0} ,
		Point2D{-1,-1} , Point2D{0,-1} , Point2D{1,-1} };

	int8_t diagULBR[10][10] = {
		{0,0,0,0,0,0,1,1,1,0},
		{0,0,0,0,0,1,1,1,1,1},
		{0,0,0,0,1,1,1,1,1,1},
		{0,0,0,1,1,1,1,1,1,1},
		{0,0,1,1,1,1,1,1,1,0},
		{0,1,1,1,1,1,1,1,0,0},
		{1,1,1,1,1,1,1,0,0,0},
		{1,1,1,1,1,1,0,0,0,0},
		{1,1,1,1,1,0,0,0,0,0},
		{0,1,1,1,0,0,0,0,0,0},
	};

	int8_t diagBLUR[10][10] = {
		{0,1,1,1,0,0,0,0,0,0},
		{1,1,1,1,1,0,0,0,0,0},
		{1,1,1,1,1,1,0,0,0,0},
		{1,1,1,1,1,1,1,0,0,0},
		{0,1,1,1,1,1,1,1,0,0},
		{0,0,1,1,1,1,1,1,1,0},
		{0,0,0,1,1,1,1,1,1,1},
		{0,0,0,0,1,1,1,1,1,1},
		{0,0,0,0,0,1,1,1,1,1},
		{0,0,0,0,0,0,1,1,1,0},
	};

	std::map<UnitTypeID, int> structureDiameter = {
		// Protoss Structures
		{ UNIT_TYPEID::PROTOSS_PYLON, 2 },
		{ UNIT_TYPEID::PROTOSS_GATEWAY, 3 },
		{ UNIT_TYPEID::PROTOSS_NEXUS, 5 },
		{ UNIT_TYPEID::PROTOSS_ASSIMILATOR, 3 },
		{ UNIT_TYPEID::PROTOSS_ASSIMILATORRICH, 3 },
		{ UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY, 3 },
		{ UNIT_TYPEID::PROTOSS_WARPGATE, 3 },
		{ UNIT_TYPEID::PROTOSS_ASSIMILATORRICH, 3 },
		{ UNIT_TYPEID::PROTOSS_FORGE, 3 },
		{ UNIT_TYPEID::PROTOSS_ROBOTICSBAY, 3 },
		{ UNIT_TYPEID::PROTOSS_SHIELDBATTERY, 2 },
		{ UNIT_TYPEID::PROTOSS_DARKSHRINE, 2 },
		{ UNIT_TYPEID::PROTOSS_PHOTONCANNON, 2 },
		{ UNIT_TYPEID::PROTOSS_STARGATE, 3 },
		{ UNIT_TYPEID::PROTOSS_TEMPLARARCHIVE, 3 },
		{ UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL, 3 },
		{ UNIT_TYPEID::PROTOSS_CYBERNETICSCORE, 3 },

		// Zerg Structures
		{ UNIT_TYPEID::ZERG_BANELINGNEST, 3 },
		{ UNIT_TYPEID::ZERG_EVOLUTIONCHAMBER, 3 },
		{ UNIT_TYPEID::ZERG_EXTRACTOR, 3 },
		{ UNIT_TYPEID::ZERG_EXTRACTORRICH, 3 },
		{ UNIT_TYPEID::ZERG_GREATERSPIRE, 3 },
		{ UNIT_TYPEID::ZERG_HATCHERY, 5 },
		{ UNIT_TYPEID::ZERG_HIVE, 5 },
		{ UNIT_TYPEID::ZERG_HYDRALISKDEN, 3 },
		{ UNIT_TYPEID::ZERG_INFESTATIONPIT, 3 },
		{ UNIT_TYPEID::ZERG_LAIR, 5 },
		{ UNIT_TYPEID::ZERG_LURKERDENMP, 3 },
		{ UNIT_TYPEID::ZERG_NYDUSCANAL, 2 },
		{ UNIT_TYPEID::ZERG_NYDUSNETWORK, 3 },
		{ UNIT_TYPEID::ZERG_ROACHWARREN, 3 },
		{ UNIT_TYPEID::ZERG_SPAWNINGPOOL, 3 },
		//{ UNIT_TYPEID::ZERG_SPINECRAWLER, -1 }, //2 //TODO: account for 
		{ UNIT_TYPEID::ZERG_SPIRE, 3 },
		//{ UNIT_TYPEID::ZERG_SPORECRAWLER, -1 }, //2 //TODO: account for 
		{ UNIT_TYPEID::ZERG_ULTRALISKCAVERN, 3 },

		// Terran Structures
		{ UNIT_TYPEID::TERRAN_ARMORY, 3 },
		{ UNIT_TYPEID::TERRAN_AUTOTURRET, 2 },
		{ UNIT_TYPEID::TERRAN_BARRACKS, 3 },
		//{ UNIT_TYPEID::TERRAN_BARRACKSFLYING, 3 }, //TODO: account for 
		{ UNIT_TYPEID::TERRAN_BARRACKSREACTOR, 2 },
		{ UNIT_TYPEID::TERRAN_BARRACKSTECHLAB, 2 },
		{ UNIT_TYPEID::TERRAN_BUNKER, 3 },
		{ UNIT_TYPEID::TERRAN_COMMANDCENTER, 5 },
		//{ UNIT_TYPEID::TERRAN_COMMANDCENTERFLYING, 5 }, //TODO: account for 
		{ UNIT_TYPEID::TERRAN_ENGINEERINGBAY, 3 },
		{ UNIT_TYPEID::TERRAN_FACTORY, 3 },
		//{ UNIT_TYPEID::TERRAN_FACTORYFLYING, 3 }, //TODO: account for 
		{ UNIT_TYPEID::TERRAN_FACTORYREACTOR, 2 },
		{ UNIT_TYPEID::TERRAN_FACTORYTECHLAB, 2 },
		{ UNIT_TYPEID::TERRAN_FUSIONCORE, 3 },
		{ UNIT_TYPEID::TERRAN_GHOSTACADEMY, 3 },
		{ UNIT_TYPEID::TERRAN_MISSILETURRET, 2 },
		{ UNIT_TYPEID::TERRAN_ORBITALCOMMAND, 5 },
		//{ UNIT_TYPEID::TERRAN_ORBITALCOMMANDFLYING, 5 }, //TODO: account for 
		{ UNIT_TYPEID::TERRAN_PLANETARYFORTRESS, 5 },
		{ UNIT_TYPEID::TERRAN_REFINERY, 3 },
		{ UNIT_TYPEID::TERRAN_REFINERYRICH, 3 },
		{ UNIT_TYPEID::TERRAN_SENSORTOWER, 2 },
		{ UNIT_TYPEID::TERRAN_STARPORT, 3 },
		//{ UNIT_TYPEID::TERRAN_STARPORTFLYING, 3 }, //TODO: account for 
		{ UNIT_TYPEID::TERRAN_STARPORTREACTOR, 2 },
		{ UNIT_TYPEID::TERRAN_STARPORTTECHLAB, 2 },
		{ UNIT_TYPEID::TERRAN_SUPPLYDEPOT, 2 },
		//{ UNIT_TYPEID::TERRAN_SUPPLYDEPOTLOWERED, 2 }, //TODO: account for 
		{ UNIT_TYPEID::TERRAN_TECHLAB, 2 },

		// Neutral/Destructible structures
		{ UNIT_TYPEID::NEUTRAL_DESTRUCTIBLECITYDEBRIS6X6, 6 },
		{ UNIT_TYPEID::NEUTRAL_DESTRUCTIBLEDEBRIS6X6, 6 },
		{ UNIT_TYPEID::NEUTRAL_DESTRUCTIBLEROCK6X6, 6 },
		{ UNIT_TYPEID::NEUTRAL_UNBUILDABLEBRICKSDESTRUCTIBLE, -2 },
		{ UNIT_TYPEID::NEUTRAL_UNBUILDABLEPLATESDESTRUCTIBLE, -2 },
		{ UNIT_TYPEID::UNBUILDABLEROCKSDESTRUCTIBLE, -2 },
		{ UNIT_TYPEID::DEBRIS2X2NONCONJOINED, 2 },
		{ UNIT_TYPEID::DESTRUCTIBLEROCKEX16X6, 6 },
		{ UNIT_TYPEID::DESTRUCTIBLEROCKEX14X4, 4 },
		{ UNIT_TYPEID::DESTRUCTIBLEDEBRIS4X4, 4 },

		// Special cases with -1 diameter
		{ UNIT_TYPEID::NEUTRAL_DESTRUCTIBLEDEBRISRAMPDIAGONALHUGEBLUR, -1 },
		{ UNIT_TYPEID::NEUTRAL_DESTRUCTIBLEDEBRISRAMPDIAGONALHUGEULBR, -1 },
		{ UNIT_TYPEID::NEUTRAL_DESTRUCTIBLEROCKEX1DIAGONALHUGEBLUR, -1 },
		{ UNIT_TYPEID::DESTRUCTIBLERAMPDIAGONALHUGEBLUR, -1 },
		{ UNIT_TYPEID::DESTRUCTIBLEROCKEX1HORIZONTALHUGE, -1 },
		{ UNIT_TYPEID::DESTRUCTIBLEROCKEX1VERTICALHUGE, -1 },
	};

	static void loadUnitPlacement(ObstacleInfo obstacle, Point2D pos, int sizeX, int sizeY, int8_t(*pattern)[10][10] = nullptr) {
		int x = (int)(pos.x - (sizeX / 2) + ((sizeX % 2 == 0) ? 0.5F : 0.0F));
		int y = (int)(pos.y - (sizeY / 2) + ((sizeY % 2 == 0) ? 0.5F : 0.0F));
		for (int i = 0; i < sizeX; i++) {
			for (int j = 0; j < sizeY; j++) {
				if ((sizeX > 5 && sizeY > 5) && (i == 0 || i == sizeX - 1) && (j == 0 || j == sizeY - 1)) {
					continue;
				}
				if (pattern == nullptr || (*pattern)[i][j] == 1) {
					if (obstacle == NOTHING) {
						imRef(masterMap, i + x, j + y) &= 0xC0;
					}
					else {
						if (obstacle == VESPENE) {
							imRef(masterMap, i + x, j + y) |= 0x3;
						}
						imRef(masterMap, i + x, j + y) &= 0xC0;
						imRef(masterMap, i + x, j + y) |= ((uint8_t)(obstacle) << 2);
						//imRef(masterMap, i + x, j + y) &= 0xFC;
					}

				}
			}
		}
	}

	static void loadUnitPlacement(ObstacleInfo obstacle, Point2D pos, UnitTypeID unit_type, int8_t(*pattern)[10][10] = nullptr) {
		if (structureDiameter.find(unit_type) != structureDiameter.end()) {
			float diam = structureDiameter[unit_type];
			if (diam > 0) {
				loadUnitPlacement(obstacle, pos, diam, diam);
			}
			else {
				printf("NOT FOUND STRUCTURE ID 2%s\n", UnitTypeToName(unit_type));
			}
		}
	}

	static void loadNeutralUnitPlacement(Point2D pos, UnitTypeID unit_type) {
		//int diam = structureDiameter(unit_type);
		if (isMineralType(unit_type)) {
			loadUnitPlacement(MINERALS, pos, 2, 1);
		}
		else if(isVespeneType(unit_type)) {
			loadUnitPlacement(VESPENE, pos, 3, 3);
		}
		else if (structureDiameter.find(unit_type) != structureDiameter.end()) {
			float diam = structureDiameter[unit_type];
			if (diam > 0) {
				loadUnitPlacement(UNPATHABLE_ROCKS, pos, diam, diam);
			}
			else if (unit_type == UNIT_TYPEID::DESTRUCTIBLEROCKEX1HORIZONTALHUGE) {
				loadUnitPlacement(UNPATHABLE_ROCKS, pos, 12, 4);
			}
			else if (unit_type == UNIT_TYPEID::DESTRUCTIBLEROCKEX1VERTICALHUGE) {
				loadUnitPlacement(UNPATHABLE_ROCKS, pos, 4, 12);
			}
			else if (unit_type == UNIT_TYPEID::NEUTRAL_DESTRUCTIBLEDEBRISRAMPDIAGONALHUGEBLUR) {
				loadUnitPlacement(UNPATHABLE_ROCKS, pos, 10, 10, &diagBLUR);
			}
			else if (unit_type == UNIT_TYPEID::NEUTRAL_DESTRUCTIBLEDEBRISRAMPDIAGONALHUGEULBR) {
				loadUnitPlacement(UNPATHABLE_ROCKS, pos, 10, 10, &diagULBR);
			}
			else if (unit_type == UNIT_TYPEID::NEUTRAL_DESTRUCTIBLEROCKEX1DIAGONALHUGEBLUR) {
				loadUnitPlacement(UNPATHABLE_ROCKS, pos, 10, 10, &diagBLUR);
			}
			else if (unit_type == UNIT_TYPEID::DESTRUCTIBLERAMPDIAGONALHUGEBLUR) {
				loadUnitPlacement(UNPATHABLE_ROCKS, pos, 10, 10, &diagBLUR);
			}
			else if (unit_type == UNIT_TYPEID::NEUTRAL_UNBUILDABLEBRICKSDESTRUCTIBLE) {
				loadUnitPlacement(PATHABLE_ROCKS, pos, 2, 2);
			}
			else if (unit_type == UNIT_TYPEID::NEUTRAL_UNBUILDABLEPLATESDESTRUCTIBLE) {
				loadUnitPlacement(PATHABLE_ROCKS, pos, 2, 2);
			}
			else if (unit_type == UNIT_TYPEID::UNBUILDABLEROCKSDESTRUCTIBLE) {
				loadUnitPlacement(PATHABLE_ROCKS, pos, 2, 2);
			}
			else {
				printf("UNSUPPORTED STRUCTURE ID %s\n", UnitTypeToName(unit_type));
			}
		}
		else {
			printf("NOT FOUND STRUCTURE ID %s\n", UnitTypeToName(unit_type));
		}
	}

	void reloadMasterMap(Agent* const agent, Point2D pos, int sizeX, int sizeY) {
		int x = (int)(pos.x - (sizeX / 2) + ((sizeX % 2 == 0) ? 0.5F : 0.0F));
		int y = (int)(pos.y - (sizeY / 2) + ((sizeY % 2 == 0) ? 0.5F : 0.0F));
		for (int i = 0; i < sizeX; i++) {
			for (int j = 0; j < sizeY; j++) {
				//uint8_t p = imRef(masterMap, i + x, j + y);
				imRef(masterMap, i + x, j + y) &= 0xFC;
				Point2D check = Point2D{ i + x + 0.1F, j + y + 0.1F };
				if (!agent->Observation()->IsPlacable(check)) {
					imRef(masterMap, i + x, j + y) |= 0x2; //second bit is unplacable
				}
				if (!agent->Observation()->IsPathable(check)) {
					imRef(masterMap, i + x, j + y) |= 0x1; //first bit is unpathable
				}
			}
		}
	}

	static void unloadNeutralUnitPlacement(Agent* const agent, Point2D pos, UnitTypeID unit_type) {
		//int diam = structureDiameter(unit_type);
		if (isMineralType(unit_type)) {
			loadUnitPlacement(NOTHING, pos, 2, 1);
		}else if (structureDiameter.find(unit_type) != structureDiameter.end()) {
			float diam = structureDiameter[unit_type];
			if (diam > 0) {
				loadUnitPlacement(NOTHING, pos, diam, diam);
				reloadMasterMap(agent, pos, diam, diam);
			}
			else if (unit_type == UNIT_TYPEID::DESTRUCTIBLEROCKEX1HORIZONTALHUGE) {
				loadUnitPlacement(NOTHING, pos, 12, 4);
				reloadMasterMap(agent, pos, 12, 4);
			}
			else if (unit_type == UNIT_TYPEID::DESTRUCTIBLEROCKEX1VERTICALHUGE) {
				loadUnitPlacement(NOTHING, pos, 4, 12);
				reloadMasterMap(agent, pos, 4, 12);
			}
			else if (unit_type == UNIT_TYPEID::NEUTRAL_DESTRUCTIBLEDEBRISRAMPDIAGONALHUGEBLUR) {
				loadUnitPlacement(NOTHING, pos, 10, 10);
				reloadMasterMap(agent, pos, 10, 10);
			}
			else if (unit_type == UNIT_TYPEID::NEUTRAL_DESTRUCTIBLEDEBRISRAMPDIAGONALHUGEULBR) {
				loadUnitPlacement(NOTHING, pos, 10, 10);
				reloadMasterMap(agent, pos, 10, 10);
			}
			else if (unit_type == UNIT_TYPEID::NEUTRAL_DESTRUCTIBLEROCKEX1DIAGONALHUGEBLUR) {
				loadUnitPlacement(NOTHING, pos, 10, 10);
				reloadMasterMap(agent, pos, 10, 10);
			}
			else if (unit_type == UNIT_TYPEID::DESTRUCTIBLERAMPDIAGONALHUGEBLUR) {
				loadUnitPlacement(NOTHING, pos, 10, 10);
				reloadMasterMap(agent, pos, 10, 10);
			}
			else if (unit_type == UNIT_TYPEID::NEUTRAL_UNBUILDABLEBRICKSDESTRUCTIBLE) {
				loadUnitPlacement(NOTHING, pos, 2, 2);
				reloadMasterMap(agent, pos, 2, 2);
			}
			else if (unit_type == UNIT_TYPEID::NEUTRAL_UNBUILDABLEPLATESDESTRUCTIBLE) {
				loadUnitPlacement(NOTHING, pos, 2, 2);
				reloadMasterMap(agent, pos, 2, 2);
			}
			else if (unit_type == UNIT_TYPEID::UNBUILDABLEROCKSDESTRUCTIBLE) {
				loadUnitPlacement(NOTHING, pos, 2, 2);
				reloadMasterMap(agent, pos, 2, 2);
			}
			else {
				printf("UNLOAD: UNSUPPORTED STRUCTURE ID %s\n", UnitTypeToName(unit_type));
			}
		}
		else {
			printf("UNLOAD: NOT FOUND STRUCTURE ID %s\n", UnitTypeToName(unit_type));
		}
	}

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

	void saveMasterBitmap(std::string fileName) {
		saveBitmap(fileName, masterMap->width(), masterMap->height(), [](int i, int j) {
			ObstacleInfo obstacle = getObstacle(i, j);
			uint8_t p = imRef(masterMap, i, j);
			if (isPlacable(i, j) && !isPathable(i, j)) {
				return Color{ 0, 255, 0 };
			}
			else if (obstacle == NOTHING) {
				if (!isPathable(i, j)) {
					return Color{ 0,0,0 };
				}
				else if (!isPlacable(i, j)) {
					return Color{ 100, 100, 100 };
				}
				else {
					return Color{ 255,255,255 };
				}
			}
			else if (obstacle == SELF_BUILDINGS) {
				return Color{ 30, 30, 255 };
			}
			else if (obstacle == ENEMY_BUILDINGS) {
				return Color{ 255, 30, 30 };
			}
			else if (obstacle == CLIFF_UNPATHABLE) {
				return Color{ 200, 100, 200 };
			}
			else if (obstacle == CLIFF_PATHABLE) {
				return Color{ 150, 50, 150 };
			}
			else if (obstacle == MINERALS) {
				return Color{ 50, 50, 150 };
			}
			else if (obstacle == UNPATHABLE_ROCKS) {
				return Color{ 150, 100, 50 };
			}
			else if (obstacle == PATHABLE_ROCKS) {
				return Color{ 150, 150, 50 };
			}
			else if (obstacle == VESPENE) {
				return Color{ 50, 150, 50 };
			}
			else {
				return Color{ 255,70,162 };
			}
			});
	}

	void setupMasterMap(Agent* const agent) {
		Units units = agent->Observation()->GetUnits(sc2::Unit::Alliance::Neutral);
		for (const Unit* unit : units) {
			loadNeutralUnitPlacement(unit->pos, unit->unit_type);
		}

		for (int i = 0; i < masterMap->width(); i++) {
			for (int j = 0; j < masterMap->height(); j++) {
				Point2D check = Point2D{ i + 0.1F, j + 0.1F }; //arbitrary to reduce floating point casting errors.
				if (!agent->Observation()->IsPlacable(check)) {
					imRef(masterMap, i, j) |= 0x2; //second bit is unplacable
				}
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
			}
		}

		loadUnitPlacement(NOTHING, agent->Observation()->GetStartLocation(), 5, 5);

		saveMasterBitmap("masterMap.bmp");
	}
}