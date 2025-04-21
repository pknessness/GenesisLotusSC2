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

constexpr float M_PI = 3.14159274F;
constexpr float timeSpeed = 1.4F;
constexpr float fps = 16 * timeSpeed;

constexpr float MINERALS_PER_PROBE_PER_SEC = 55.0F / 60;
constexpr float VESPENE_PER_PROBE_PER_SEC = 61.0F / 60;

constexpr float PYLON_RADIUS = 6.0F;
constexpr float PYLON_RADIUS_SQUARED = PYLON_RADIUS*PYLON_RADIUS;

namespace Aux {
	GameInfo gameInfo_cache;
	int mapWidth_cache;
	int mapHeight_cache;

	std::map<UnitTypeID, UnitTypeData> statsMap = std::map<UnitTypeID, UnitTypeData>();
	bool init_data = false;
	UnitTypes cached_data;

	std::vector<Point2D> criticalPoints;

	enum CrucialPoints {
		PLACEHOLDER_POINT,
		SELF_STARTLOC_POINT,
		SELF_FIRSTPYLON_POINT,
		SELF_RALLY_POINT,
		SELF_PROXY_POINT,
		ENEMY_STARTLOC_POINT,
		ENEMY_RALLY_POINT
	};

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
		BUILDING_RESERVE = 0xA,  // 1010: Building Reserve
		// Reserved values
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
			obstacle == VESPENE ||
			obstacle == BUILDING_RESERVE);
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
			obstacle == VESPENE ||
			obstacle == BUILDING_RESERVE);
	}

	struct Cost {
		unsigned int minerals = 0;
		unsigned int vespene = 0;
		unsigned int energy = 0;
		int psi = 0;
	};

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

	struct encoding2D : Point2D {
		encoding2D() : Point2D(){
		}

		encoding2D(const Point2D& p) : Point2D(p){
		}
	};

	bool operator<(const encoding2D& l, const encoding2D& r) {
		return (l.x < r.x || (l.x == r.x && l.y < r.y));
	}

	bool operator==(const encoding2D& l, const encoding2D& r) {
		return (l.x == r.x && l.y == r.y);
	}

	struct encoding2DHash
	{
		std::size_t operator()(const encoding2D& k) const
		{
			return k.x * 120 + k.y * 214012;
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
			return a.distance < b.distance;
		}
	};

	std::vector<Expansion> expansions;
	std::set<ExpansionDistance, ExpansionDistanceCompare> selfRankedExpansions;
	std::set<ExpansionDistance, ExpansionDistanceCompare> enemyRankedExpansions;

	UnitTypes allData(Agent* agent) {
		if (!init_data) {
			cached_data = agent->Observation()->GetUnitTypeData();
			init_data = true;
		}
		return cached_data;
	}

	UnitTypeData getStats(UnitTypeID type, Agent* agent) {
		if (statsMap.find(type) == statsMap.end()) {
			try {
				statsMap[type] = allData(agent).at(static_cast<uint32_t>(type));
			}
			catch (...) {
				printf("Errant Type: %s %ud %ul %d\n", UnitTypeToName(type), static_cast<uint32_t>(type), static_cast<uint32_t>(type), static_cast<uint32_t>(type));
				throw 5;
				//return UnitTypeData();//agent->Observation()->GetUnitTypeData().at(static_cast<uint32_t>(type));
			}
			//if (type == UNIT_TYPEID::PROTOSS_VOIDRAY) {
			//	ComplexWeapon prismaticBeam(Weapon::TargetType::Any, 6, 1, 6, 0.36F);
			//	prismaticBeam.addDamageBonus(Attribute::Armored, 4);
			//	statsMap[UNIT_TYPEID::PROTOSS_VOIDRAY].weapons.push_back(prismaticBeam.w);
			//}
			//else if (type == UNIT_TYPEID::PROTOSS_SENTRY) {
			//	ComplexWeapon disruptionBeam(Weapon::TargetType::Any, 6, 1, 5, 0.71F);
			//	statsMap[UNIT_TYPEID::PROTOSS_SENTRY].weapons.push_back(disruptionBeam.w);
			//}
			//else if (type == UNIT_TYPEID::PROTOSS_DISRUPTOR) {
			//	//https://www.reddit.com/r/starcraft/comments/40pl7l/how_far_can_a_disruptors_purification_nova_travel/?rdt=50754
			//	ComplexWeapon novaAura(Weapon::TargetType::Any, 100, 1, 13, 21.4F);
			//	statsMap[UNIT_TYPEID::PROTOSS_DISRUPTOR].weapons.push_back(novaAura.w);
			//}
			//else if (type == UNIT_TYPEID::PROTOSS_DISRUPTORPHASED) {
			//	statsMap[UNIT_TYPEID::PROTOSS_DISRUPTOR].weapons.push_back(extraWeapons[ABILITY_ID::EFFECT_PURIFICATIONNOVA].w);
			//}
		}
		return statsMap[type];
	}

	static UpgradeID researchAbilityToUpgrade(AbilityID build_ability) {
		switch (uint32_t(build_ability)) {
		case (uint32_t(ABILITY_ID::RESEARCH_ADEPTRESONATINGGLAIVES)):
			return UPGRADE_ID::ADEPTKILLBOUNCE;
		case(uint32_t(ABILITY_ID::RESEARCH_BLINK)):
			return UPGRADE_ID::BLINKTECH;
		case(uint32_t(ABILITY_ID::RESEARCH_CHARGE)):
			return UPGRADE_ID::CHARGE;
		case(uint32_t(ABILITY_ID::RESEARCH_EXTENDEDTHERMALLANCE)):
			return UPGRADE_ID::EXTENDEDTHERMALLANCE;
		case(uint32_t(ABILITY_ID::RESEARCH_GRAVITICBOOSTER)):
			return UPGRADE_ID::OBSERVERGRAVITICBOOSTER;
		case(uint32_t(ABILITY_ID::RESEARCH_GRAVITICDRIVE)):
			return UPGRADE_ID::GRAVITICDRIVE;
		case (uint32_t(ABILITY_ID::RESEARCH_PROTOSSAIRARMOR)):
			return UPGRADE_ID::INVALID;
		case(uint32_t(ABILITY_ID::RESEARCH_PROTOSSAIRARMORLEVEL1)):
			return UPGRADE_ID::PROTOSSAIRARMORSLEVEL1;
		case(uint32_t(ABILITY_ID::RESEARCH_PROTOSSAIRARMORLEVEL2)):
			return UPGRADE_ID::PROTOSSAIRARMORSLEVEL2;
		case(uint32_t(ABILITY_ID::RESEARCH_PROTOSSAIRARMORLEVEL3)):
			return UPGRADE_ID::PROTOSSAIRARMORSLEVEL3;
		case(uint32_t(ABILITY_ID::RESEARCH_PROTOSSAIRWEAPONS)):
			return UPGRADE_ID::INVALID;
		case(uint32_t(ABILITY_ID::RESEARCH_PROTOSSAIRWEAPONSLEVEL1)):
			return UPGRADE_ID::PROTOSSAIRWEAPONSLEVEL1;
		case(uint32_t(ABILITY_ID::RESEARCH_PROTOSSAIRWEAPONSLEVEL2)):
			return UPGRADE_ID::PROTOSSAIRWEAPONSLEVEL2;
		case(uint32_t(ABILITY_ID::RESEARCH_PROTOSSAIRWEAPONSLEVEL3)):
			return UPGRADE_ID::PROTOSSAIRWEAPONSLEVEL3;
		case(uint32_t(ABILITY_ID::RESEARCH_PROTOSSGROUNDARMOR)):
			return UPGRADE_ID::INVALID;
		case(uint32_t(ABILITY_ID::RESEARCH_PROTOSSGROUNDARMORLEVEL1)):
			return UPGRADE_ID::PROTOSSGROUNDARMORSLEVEL1;
		case(uint32_t(ABILITY_ID::RESEARCH_PROTOSSGROUNDARMORLEVEL2)):
			return UPGRADE_ID::PROTOSSGROUNDARMORSLEVEL2;
		case(uint32_t(ABILITY_ID::RESEARCH_PROTOSSGROUNDARMORLEVEL3)):
			return UPGRADE_ID::PROTOSSGROUNDARMORSLEVEL3;
		case(uint32_t(ABILITY_ID::RESEARCH_PROTOSSGROUNDWEAPONS)):
			return UPGRADE_ID::INVALID;
		case(uint32_t(ABILITY_ID::RESEARCH_PROTOSSGROUNDWEAPONSLEVEL1)):
			return UPGRADE_ID::PROTOSSGROUNDWEAPONSLEVEL1;
		case(uint32_t(ABILITY_ID::RESEARCH_PROTOSSGROUNDWEAPONSLEVEL2)):
			return UPGRADE_ID::PROTOSSGROUNDWEAPONSLEVEL2;
		case(uint32_t(ABILITY_ID::RESEARCH_PROTOSSGROUNDWEAPONSLEVEL3)):
			return UPGRADE_ID::PROTOSSGROUNDWEAPONSLEVEL3;
		case(uint32_t(ABILITY_ID::RESEARCH_PROTOSSSHIELDS)):
			return UPGRADE_ID::INVALID;
		case(uint32_t(ABILITY_ID::RESEARCH_PROTOSSSHIELDSLEVEL1)):
			return UPGRADE_ID::PROTOSSSHIELDSLEVEL1;
		case(uint32_t(ABILITY_ID::RESEARCH_PROTOSSSHIELDSLEVEL2)):
			return UPGRADE_ID::PROTOSSSHIELDSLEVEL2;
		case(uint32_t(ABILITY_ID::RESEARCH_PROTOSSSHIELDSLEVEL3)):
			return UPGRADE_ID::PROTOSSSHIELDSLEVEL3;
			// case(uint32_t(ABILITY_ID::RESEARCH_PSIONICAMPLIFIERS)):
			//     return UPGRADE_ID::PSIONICAMPLIFIERS;
		case(uint32_t(ABILITY_ID::RESEARCH_PSISTORM)):
			return UPGRADE_ID::PSISTORMTECH;
		case(uint32_t(ABILITY_ID::RESEARCH_SHADOWSTRIKE)):
			return UPGRADE_ID::DARKTEMPLARBLINKUPGRADE;
		case(uint32_t(ABILITY_ID::RESEARCH_TEMPESTRANGEUPGRADE)):
			return UPGRADE_ID::TEMPESTRANGEUPGRADE;
			// case(uint32_t(ABILITY_ID::RESEARCH_TEMPESTRESEARCHGROUNDATTACKUPGRADE)):
			//     return UPGRADE_ID::TEMPESTGROUNDATTACKUPGRADE;  
			// case(uint32_t(ABILITY_ID::RESEARCH_VOIDRAYSPEEDUPGRADE)):
			//     return UPGRADE_ID::VOIDRAYSPEEDUPGRADE;
		case(uint32_t(ABILITY_ID::RESEARCH_WARPGATE)):
			return UPGRADE_ID::WARPGATERESEARCH;
		}
		return 0;
	}

	static UnitTypeID buildAbilityToUnit(AbilityID build_ability) {
		switch (uint32_t(build_ability)) {
		case (uint32_t(ABILITY_ID::BUILD_ASSIMILATOR)):
			return UNIT_TYPEID::PROTOSS_ASSIMILATOR;
		case (uint32_t(ABILITY_ID::BUILD_CYBERNETICSCORE)):
			return UNIT_TYPEID::PROTOSS_CYBERNETICSCORE;
		case (uint32_t(ABILITY_ID::BUILD_DARKSHRINE)):
			return UNIT_TYPEID::PROTOSS_DARKSHRINE;
		case (uint32_t(ABILITY_ID::BUILD_FLEETBEACON)):
			return UNIT_TYPEID::PROTOSS_FLEETBEACON;
		case (uint32_t(ABILITY_ID::BUILD_FORGE)):
			return UNIT_TYPEID::PROTOSS_FORGE;
		case (uint32_t(ABILITY_ID::BUILD_GATEWAY)):
			return UNIT_TYPEID::PROTOSS_GATEWAY;
		case (uint32_t(ABILITY_ID::BUILD_NEXUS)):
			return UNIT_TYPEID::PROTOSS_NEXUS;
		case (uint32_t(ABILITY_ID::BUILD_PHOTONCANNON)):
			return UNIT_TYPEID::PROTOSS_PHOTONCANNON;
		case (uint32_t(ABILITY_ID::BUILD_PYLON)):
			return UNIT_TYPEID::PROTOSS_PYLON;
		case (uint32_t(ABILITY_ID::BUILD_ROBOTICSBAY)):
			return UNIT_TYPEID::PROTOSS_ROBOTICSBAY;
		case (uint32_t(ABILITY_ID::BUILD_ROBOTICSFACILITY)):
			return UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY;
		case (uint32_t(ABILITY_ID::BUILD_SHIELDBATTERY)):
			return UNIT_TYPEID::PROTOSS_SHIELDBATTERY;
		case (uint32_t(ABILITY_ID::BUILD_STARGATE)):
			return UNIT_TYPEID::PROTOSS_STARGATE;
		case (uint32_t(ABILITY_ID::BUILD_TEMPLARARCHIVE)):
			return UNIT_TYPEID::PROTOSS_TEMPLARARCHIVE;
		case (uint32_t(ABILITY_ID::BUILD_TWILIGHTCOUNCIL)):
			return UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL;
		case (uint32_t(ABILITY_ID::TRAIN_ADEPT)):
			return UNIT_TYPEID::PROTOSS_ADEPT;
		case (uint32_t(ABILITY_ID::TRAIN_ARCHON)):
			return UNIT_TYPEID::PROTOSS_ARCHON;
		case (uint32_t(ABILITY_ID::TRAIN_CARRIER)):
			return UNIT_TYPEID::PROTOSS_CARRIER;
		case (uint32_t(ABILITY_ID::TRAIN_COLOSSUS)):
			return UNIT_TYPEID::PROTOSS_COLOSSUS;
		case (uint32_t(ABILITY_ID::TRAIN_DARKTEMPLAR)):
			return UNIT_TYPEID::PROTOSS_DARKTEMPLAR;
		case (uint32_t(ABILITY_ID::TRAIN_DISRUPTOR)):
			return UNIT_TYPEID::PROTOSS_DISRUPTOR;
		case (uint32_t(ABILITY_ID::TRAIN_HIGHTEMPLAR)):
			return UNIT_TYPEID::PROTOSS_HIGHTEMPLAR;
		case (uint32_t(ABILITY_ID::TRAIN_IMMORTAL)):
			return UNIT_TYPEID::PROTOSS_IMMORTAL;
		case (uint32_t(ABILITY_ID::TRAIN_MOTHERSHIP)):
			return UNIT_TYPEID::PROTOSS_MOTHERSHIP;
		case (uint32_t(ABILITY_ID::TRAIN_MOTHERSHIPCORE)):
			return UNIT_TYPEID::PROTOSS_MOTHERSHIPCORE;
		case (uint32_t(ABILITY_ID::TRAIN_OBSERVER)):
			return UNIT_TYPEID::PROTOSS_OBSERVER;
		case (uint32_t(ABILITY_ID::TRAIN_ORACLE)):
			return UNIT_TYPEID::PROTOSS_ORACLE;
		case (uint32_t(ABILITY_ID::TRAIN_PHOENIX)):
			return UNIT_TYPEID::PROTOSS_PHOENIX;
		case (uint32_t(ABILITY_ID::TRAIN_PROBE)):
			return UNIT_TYPEID::PROTOSS_PROBE;
		case (uint32_t(ABILITY_ID::TRAIN_SENTRY)):
			return UNIT_TYPEID::PROTOSS_SENTRY;
		case (uint32_t(ABILITY_ID::TRAIN_STALKER)):
			return UNIT_TYPEID::PROTOSS_STALKER;
		case (uint32_t(ABILITY_ID::TRAIN_TEMPEST)):
			return UNIT_TYPEID::PROTOSS_TEMPEST;
		case (uint32_t(ABILITY_ID::TRAIN_VOIDRAY)):
			return UNIT_TYPEID::PROTOSS_VOIDRAY;
		case (uint32_t(ABILITY_ID::TRAIN_WARPPRISM)):
			return UNIT_TYPEID::PROTOSS_WARPPRISM;
		case (uint32_t(ABILITY_ID::TRAIN_ZEALOT)):
			return UNIT_TYPEID::PROTOSS_ZEALOT;
		case (uint32_t(ABILITY_ID::TRAINWARP_ZEALOT)):
			return UNIT_TYPEID::PROTOSS_ZEALOT;
		case (uint32_t(ABILITY_ID::TRAINWARP_SENTRY)):
			return UNIT_TYPEID::PROTOSS_SENTRY;
		case (uint32_t(ABILITY_ID::TRAINWARP_STALKER)):
			return UNIT_TYPEID::PROTOSS_STALKER;
		case (uint32_t(ABILITY_ID::TRAINWARP_ADEPT)):
			return UNIT_TYPEID::PROTOSS_ADEPT;
		case (uint32_t(ABILITY_ID::TRAINWARP_DARKTEMPLAR)):
			return UNIT_TYPEID::PROTOSS_DARKTEMPLAR;
		case (uint32_t(ABILITY_ID::TRAINWARP_HIGHTEMPLAR)):
			return UNIT_TYPEID::PROTOSS_HIGHTEMPLAR;

		default:
			return 0;
		}
	}

	static Cost buildAbilityToCost(AbilityID build_ability, Agent* agent) {
		//sc2::UnitTypeData unit_stats =
		//    agent->Observation()->GetUnitTypeData().at(static_cast<uint32_t>(buildAbilityToUnit(build_ability)));
		return { getStats(buildAbilityToUnit(build_ability), agent).mineral_cost, getStats(buildAbilityToUnit(build_ability), agent).vespene_cost, 0, int(getStats(buildAbilityToUnit(build_ability), agent).food_required) };
	}

	static Cost unitAbilityToCost(AbilityID build_ability, Agent* agent) {
		if (build_ability == ABILITY_ID::EFFECT_CHRONOBOOSTENERGYCOST) {
			return { 0, 0, 50, 0 };
		}
		else if (build_ability == ABILITY_ID::EFFECT_MASSRECALL_NEXUS) {
			return { 0, 0, 50, 0 };
			//} else if (build_ability == ABILITY_ID::BATTERYOVERCHARGE) { //TODO: REPLACE WITH ENERGY OVERCHARGE
			//    return { 0, 0, 50, 0 };
		}
		else if (build_ability == ABILITY_ID::EFFECT_FORCEFIELD) {
			return { 0, 0, 50, 0 };
		}
		else if (build_ability == ABILITY_ID::EFFECT_GUARDIANSHIELD) {
			return { 0, 0, 75, 0 };
		}
		else if (build_ability == ABILITY_ID::HALLUCINATION_ADEPT) {
			return { 0, 0, 75, 0 };
		}
		else if (build_ability == ABILITY_ID::HALLUCINATION_ARCHON) {
			return { 0, 0, 75, 0 };
		}
		else if (build_ability == ABILITY_ID::HALLUCINATION_COLOSSUS) {
			return { 0, 0, 75, 0 };
		}
		else if (build_ability == ABILITY_ID::HALLUCINATION_DISRUPTOR) {
			return { 0, 0, 75, 0 };
		}
		else if (build_ability == ABILITY_ID::HALLUCINATION_HIGHTEMPLAR) {
			return { 0, 0, 75, 0 };
		}
		else if (build_ability == ABILITY_ID::HALLUCINATION_IMMORTAL) {
			return { 0, 0, 75, 0 };
		}
		else if (build_ability == ABILITY_ID::HALLUCINATION_ORACLE) {
			return { 0, 0, 75, 0 };
		}
		else if (build_ability == ABILITY_ID::HALLUCINATION_PHOENIX) {
			return { 0, 0, 75, 0 };
		}
		else if (build_ability == ABILITY_ID::HALLUCINATION_PROBE) {
			return { 0, 0, 75, 0 };
		}
		else if (build_ability == ABILITY_ID::HALLUCINATION_STALKER) {
			return { 0, 0, 75, 0 };
		}
		else if (build_ability == ABILITY_ID::HALLUCINATION_VOIDRAY) {
			return { 0, 0, 75, 0 };
		}
		else if (build_ability == ABILITY_ID::HALLUCINATION_WARPPRISM) {
			return { 0, 0, 75, 0 };
		}
		else if (build_ability == ABILITY_ID::HALLUCINATION_ZEALOT) {
			return { 0, 0, 75, 0 };
		}
		else if (build_ability == ABILITY_ID::EFFECT_GRAVITONBEAM) {
			return { 0, 0, 50, 0 };
		}
		else if (build_ability == ABILITY_ID::EFFECT_ORACLEREVELATION) {
			return { 0, 0, 25, 0 };
		}
		else if (build_ability == ABILITY_ID::BUILD_STASISTRAP) {
			return { 0, 0, 50, 0 };
		}
		else if (build_ability == ABILITY_ID::EFFECT_FEEDBACK) {
			return { 0, 0, 50, 0 };
		}
		else if (build_ability == ABILITY_ID::EFFECT_PSISTORM) {
			return { 0, 0, 75, 0 };
		}
		else if (build_ability == ABILITY_ID::GENERAL_MOVE) {
			return { 0, 0, 0, 0 };
		}
		else if (build_ability == ABILITY_ID::RESEARCH_PROTOSSGROUNDWEAPONS) {
			return { 0, 0, 0, 0 };
		}
		printf("Unknown Unit Ability %s\n", AbilityTypeToName(build_ability));
		return { 0, 0, 0, 0 };
	}

	static Cost UpgradeToCost(AbilityID research_ability, Agent* agent) {
		UpgradeData upgrade_stats =
			agent->Observation()->GetUpgradeData().at(static_cast<uint32_t>(researchAbilityToUpgrade(research_ability)));
		return { upgrade_stats.mineral_cost, upgrade_stats.vespene_cost, 0 };
	}

	static Cost abilityToCost(AbilityID ability, Agent* agent) {
		if (buildAbilityToUnit(ability) != 0) {
			return buildAbilityToCost(ability, agent);
		}
		else if (researchAbilityToUpgrade(ability)) {
			return UpgradeToCost(ability, agent);
		}
		else {
			return unitAbilityToCost(ability, agent);
		}
		return { 0, 0, 0, 0 };
	}

	void setupExpansions(Agent* const agent) {
		Point3D start = agent->Observation()->GetStartLocation();
		Point3D enemy = AP3D( gameInfo_cache.enemy_start_locations[0] );

		std::vector<Point3D> rawExpands = sc2::search::CalculateExpansionLocations(agent->Observation(), agent->Query());
		std::vector<Point3D> expands;
		for (int i = 0; i < rawExpands.size(); i++) {
			if (rawExpands[i].x != 0 && rawExpands[i].y != 0) {
				expands.push_back(rawExpands[i]);
			}
		}
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
			if (selfDistances[i] != 0.0) {
				selfRankedExpansions.insert(ExpansionDistance{ selfDistances[i], i });
			}
			enemyRankedExpansions.insert(ExpansionDistance{ enemyDistances[i], i });
		}

		//SETUP CRITICAL POINTS
		criticalPoints = std::vector(10, Point2D{ 0,0 });
		criticalPoints[CrucialPoints::PLACEHOLDER_POINT] = Point2D{ 500,500 };
		criticalPoints[CrucialPoints::SELF_STARTLOC_POINT] = agent->Observation()->GetStartLocation();
		criticalPoints[CrucialPoints::ENEMY_STARTLOC_POINT] = gameInfo_cache.enemy_start_locations[0];;

		Point2D py;
		py.x = (criticalPoints[CrucialPoints::SELF_STARTLOC_POINT].x > mapWidth_cache / 2) ? criticalPoints[CrucialPoints::SELF_STARTLOC_POINT].x - 6 : criticalPoints[CrucialPoints::SELF_STARTLOC_POINT].x + 6;
		py.y = (criticalPoints[CrucialPoints::SELF_STARTLOC_POINT].y > mapHeight_cache / 2) ? criticalPoints[CrucialPoints::SELF_STARTLOC_POINT].y - 6 : criticalPoints[CrucialPoints::SELF_STARTLOC_POINT].y + 6;

		//Point2D py2;
		//py2.x = (criticalPoints[CrucialPoints::ENEMY_STARTLOC_POINT].x > mapWidth_cache / 2) ? criticalPoints[CrucialPoints::ENEMY_STARTLOC_POINT].x - 6 : criticalPoints[CrucialPoints::ENEMY_STARTLOC_POINT].x + 6;
		//py2.y = (criticalPoints[CrucialPoints::ENEMY_STARTLOC_POINT].y > mapHeight_cache / 2) ? criticalPoints[CrucialPoints::ENEMY_STARTLOC_POINT].y - 6 : criticalPoints[CrucialPoints::ENEMY_STARTLOC_POINT].y + 6;

		criticalPoints[CrucialPoints::SELF_FIRSTPYLON_POINT] = py;
		//criticalPoints[CrucialPoints::ENEMY_STARTLOC_POINT] = py2;


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

	Point2D getRandomPointRadius(Point2D point, float radius_max) {
		float theta = (2.0F * M_PI * rand()) / RAND_MAX;
		float radius = (radius_max * rand()) / RAND_MAX;
		return point + Point2D{ radius * cos(theta), radius * sin(theta) };
	}

	encoding2D getRandomEncodingPoint() {
		float x = (50 * rand()) / RAND_MAX + 50;
		float y = (50 * rand()) / RAND_MAX + 50;
		return encoding2D(Point2D{ x, y });
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
				if ((sizeX > 6 && sizeY > 6) && (i == 0 || i == sizeX - 1) && (j == 0 || j == sizeY - 1)) {
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

	static bool checkStructurePlacement(Point2D pos, int size) {
		int x = (int)(pos.x - (size / 2) + ((size % 2 == 0) ? 0.5F : 0.0F));
		int y = (int)(pos.y - (size / 2) + ((size % 2 == 0) ? 0.5F : 0.0F));
		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) {
				if (!isPlacable(i + x, j + y)) {
					return false;
				}
			}
		}
		return true;
	}

	static Point2D possibleNextPylons[6] = {Point2D{8, -2}, Point2D{-8, 2}, Point2D{4, 5} , Point2D{-4, -5} , Point2D{4, -7} , Point2D{-4, 7} };
	static Point2D possibleNextBuildings[6] = { Point2D{-2.5, -0.5}, Point2D{0.5, -2.5}, Point2D{5.5, -2.5}, Point2D{4.5, 2.5}, Point2D{1.5, 4.5}, Point2D{-3.5, 4.5} };

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