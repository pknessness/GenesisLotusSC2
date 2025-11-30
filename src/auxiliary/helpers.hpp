#pragma once
#include <sc2api/sc2_api.h>
#include <sc2lib/sc2_lib.h>

#include <string>
#include "debugging.hpp"
#include "map2d.hpp"
#include "bitmap.hpp"
#include "profiler.hpp"
#include "upgrademanager.hpp"

#ifndef FLT_MAX
#define FLT_MAX 3.402823466e+38F
#endif

using namespace sc2;
using CompositionAsTarget = sc2::Weapon::TargetType;

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

constexpr float GS_PI = 3.14159274F;
constexpr float timeSpeed = 1.4F;
constexpr float native_fps = 16;
constexpr float fps = native_fps * timeSpeed;

//henceforth, i will try to put all time variables in units of either native frames _natframes, faster frames _fframes, native seconds _natsec, or faster seconds _fsec

//timeSpeed = 1.4F; native_fps = 16;
//natsec (native seconds) / timeSpeed = fsec (faster seconds)
//natsec (native seconds) * native_fps = natframes (native frames)
//fsec (faster seconds) * native_fps = fframes (faster frames)
//natframes (native frames) / timeSpeed = fframes (faster frames)

//are fframes the same as natframes?

//timeSpeed = 1.4F; native_fps = 16; fps = 22.4
//natsec (native seconds) / timeSpeed = fsec (faster seconds)
//natsec (native seconds) * native_fps = frames (frames)
//fsec (faster seconds) * fps = frames (frames)

//fsec [REALTIME SPEED, LIQUPEDIA]
//frames [unit.weapon_cooldown (frames until next attack)]
//natsec [unit_stats.movement_speed (cells/natsec), q.speed (time between attacks in natsec)]

//1.07 fsec -> 23.968 frames (colossus)

constexpr float MINERALS_PER_PROBE_PER_SEC = 55.0F / 60;
constexpr float VESPENE_PER_PROBE_PER_SEC = 61.0F / 60;

constexpr float PYLON_RADIUS = 6.0F;
constexpr float PYLON_RADIUS_REAL = 6.5F;
constexpr float PYLON_RADIUS_SQUARED = PYLON_RADIUS * PYLON_RADIUS;
constexpr float PYLON_RADIUS_SQUARED_REAL = PYLON_RADIUS_REAL * PYLON_RADIUS_REAL;

constexpr float PRISM_RADIUS = 3.25F;
constexpr float PRISM_RADIUS_REAL = 3.75F;
constexpr float PRISM_RADIUS_SQUARED = PRISM_RADIUS * PRISM_RADIUS;
constexpr float PRISM_RADIUS_SQUARED_REAL = PRISM_RADIUS_REAL * PRISM_RADIUS_REAL;


constexpr float EPSILON = 0.1; //time between attacks for one time attacks

constexpr float MY_PI = 3.141592653589;
constexpr float MY_2PI = MY_PI * 2;
constexpr float MY_PI2 = MY_PI / 2;

namespace Aux {
	GameInfo gameInfo_cache;
	int mapWidth_cache;
	int mapHeight_cache;

	Race opponent = Race::Random;

	std::map<UnitTypeID, UnitTypeData> statsMap = std::map<UnitTypeID, UnitTypeData>();
	bool init_data = false;
	UnitTypes cached_data;

	std::vector<Point2D> criticalPoints;

	int effectiveMinerals;
	int effectiveVespene;
	//int effectivePsi;

	enum CrucialPoints {
		PLACEHOLDER_POINT,
		SELF_STARTLOC_POINT,
		SELF_FIRSTPYLON_POINT,
		SELF_RALLY_POINT,
		SELF_PROXY_POINT,
		SELF_SHIELDBATTERY_POINT,
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

	bool isPathable(Point2D p) {
		return isPathable(int(p.x), int(p.y));
	}

	bool withinBounds(Point2D p) {
		return p.x > 0 && p.y > 0 && p.x < mapWidth_cache && p.y < mapHeight_cache;
	}

	bool isPathableTile(int i, int j) {
		ObstacleInfo obstacle = getObstacle(i, j);
		return !((imRef(masterMap, i, j) & 0x01));
	}

	bool isPlacable(int i, int j, bool ignoreBuildingReserve = false) {
		ObstacleInfo obstacle = getObstacle(i, j);
		return !((imRef(masterMap, i, j) & 0x02) || 
			obstacle == SELF_BUILDINGS ||
			obstacle == ENEMY_BUILDINGS ||
			obstacle == CLIFF_UNPATHABLE ||
			obstacle == CLIFF_PATHABLE ||
			obstacle == MINERALS ||
			obstacle == UNPATHABLE_ROCKS ||
			obstacle == PATHABLE_ROCKS || 
			obstacle == VESPENE ||
			(obstacle == BUILDING_RESERVE && !ignoreBuildingReserve));
	}

	struct Cost {
		unsigned int minerals = 0;
		unsigned int vespene = 0;
		unsigned int energy = 0;
		int psi = 0;
	};

	struct EnergyCost {
		float energyCostStatic;
		float energyCostPerFrame;
	};

	inline int damageExtraPerUpgrade(float baseDamage) {
		if (baseDamage >= 45) {
			return 5;
		}
		else if (baseDamage >= 36) {
			return 4;
		}
		else if (baseDamage >= 24) {
			return 3;
		}
		else if (baseDamage >= 15) {
			return 2;
		}
		return 1;
	}

	//Weapon::TargetType type_, float damage_, uint32_t attacks_, float range_, float speed_, EnergyCost energyCost_ = { static, per frame }, bool spell_ = false
	struct ExtraWeapon : Weapon {
		EnergyCost energyCost;
		bool spell;
		ExtraWeapon() : energyCost{ 0, 0 } {

		}

		ExtraWeapon(Weapon::TargetType type_, float damage__, uint32_t attacks_, float range_, float speed_, EnergyCost energyCost_ = { 0, 0 }, bool spell_ = false) {
			type = type_;
			damage_ = damage__;
			attacks = attacks_;
			range = range_;
			speed = speed_;
			energyCost = energyCost_;
			spell = spell_;
		}

		ExtraWeapon(Weapon w) {
			type = w.type;
			damage_ = w.damage_;
			attacks = w.attacks;
			range = w.range;
			speed = w.speed;
			energyCost = { 0, 0 };
			spell = false;
		}

		void setWeapon(Weapon::TargetType type_, float damage_, uint32_t attacks_, float range_, float speed_, EnergyCost energyCost_ = { 0, 0 }, bool spell_ = false) {
			type = type_;
			damage_ = damage_;
			attacks = attacks_;
			range = range_;
			speed = speed_;
			energyCost = energyCost_;
			spell = spell_;
		}

		void addDamageBonus(Attribute a, float bonus) {
			DamageBonus b;
			b.attribute = a;
			b.bonus = bonus;
			damage_bonus.push_back(b);
		}

		ExtraWeapon applySelfUpgrades(uint8_t weapon_level) { //OPTIMISE: this copy is expensive
			ExtraWeapon modified = *(this);
			modified.damage_ += damageExtraPerUpgrade(modified.damage_) * weapon_level;
			for (auto& bonus : modified.damage_bonus) {
				bonus.bonus += damageExtraPerUpgrade(bonus.bonus) * weapon_level;
			}
			if (weapon_level != 0) {
				printf("");
			}
			//TODO: ADD COLOSSUS, 
			return modified;
		}

		ExtraWeapon applyEnemyUpgrades() { //OPTIMISE: this copy is expensive
			ExtraWeapon modified = *(this);
			return modified;
		}
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
		return (abs(l.x - r.x) < 0.001 && abs(l.y - r.y) < 0.001);
	}

	struct encoding2DHash
	{
		std::size_t operator()(const encoding2D& k) const
		{
			return int(k.x) * 120 + int(k.y) * 214012;
		}
	};

	UnitTypeID BuildUnitOrder[18] = {
		UNIT_TYPEID::PROTOSS_ZEALOT,
		UNIT_TYPEID::PROTOSS_STALKER,
		UNIT_TYPEID::PROTOSS_SENTRY,
		UNIT_TYPEID::PROTOSS_ADEPT,
		UNIT_TYPEID::PROTOSS_DARKTEMPLAR,
		UNIT_TYPEID::PROTOSS_HIGHTEMPLAR,
		UNIT_TYPEID::PROTOSS_ARCHON,

		UNIT_TYPEID::PROTOSS_OBSERVER,
		UNIT_TYPEID::PROTOSS_IMMORTAL,
		UNIT_TYPEID::PROTOSS_WARPPRISM, //PHASED
		UNIT_TYPEID::PROTOSS_COLOSSUS,
		UNIT_TYPEID::PROTOSS_DISRUPTOR,

		UNIT_TYPEID::PROTOSS_PHOENIX,
		UNIT_TYPEID::PROTOSS_ORACLE,
		UNIT_TYPEID::PROTOSS_VOIDRAY,
		UNIT_TYPEID::PROTOSS_TEMPEST,
		UNIT_TYPEID::PROTOSS_CARRIER,

		UNIT_TYPEID::PROTOSS_MOTHERSHIP
	};

	UnitTypeID ArmyUnitsProtoss[] = {
		UNIT_TYPEID::PROTOSS_ZEALOT,
		UNIT_TYPEID::PROTOSS_STALKER,
		UNIT_TYPEID::PROTOSS_SENTRY,
		UNIT_TYPEID::PROTOSS_ADEPT,
		UNIT_TYPEID::PROTOSS_DARKTEMPLAR,
		UNIT_TYPEID::PROTOSS_HIGHTEMPLAR,
		UNIT_TYPEID::PROTOSS_ARCHON,

		UNIT_TYPEID::PROTOSS_OBSERVER,
		UNIT_TYPEID::PROTOSS_IMMORTAL,
		UNIT_TYPEID::PROTOSS_WARPPRISM, //PHASED
		UNIT_TYPEID::PROTOSS_COLOSSUS,
		UNIT_TYPEID::PROTOSS_DISRUPTOR,

		UNIT_TYPEID::PROTOSS_PHOENIX,
		UNIT_TYPEID::PROTOSS_ORACLE,
		UNIT_TYPEID::PROTOSS_VOIDRAY,
		UNIT_TYPEID::PROTOSS_TEMPEST,
		UNIT_TYPEID::PROTOSS_CARRIER,

		UNIT_TYPEID::PROTOSS_MOTHERSHIP,

		UNIT_TYPEID::PROTOSS_PROBE,
		UNIT_TYPEID::PROTOSS_PHOTONCANNON,
		UNIT_TYPEID::PROTOSS_SHIELDBATTERY,
	};

	UnitTypeID NonArmyProtoss[] = {
		UNIT_TYPEID::PROTOSS_NEXUS,
		UNIT_TYPEID::PROTOSS_PYLON,
		UNIT_TYPEID::PROTOSS_GATEWAY,
		UNIT_TYPEID::PROTOSS_WARPGATE,
		UNIT_TYPEID::PROTOSS_ASSIMILATOR,
		UNIT_TYPEID::PROTOSS_CYBERNETICSCORE,
		UNIT_TYPEID::PROTOSS_FORGE,
		UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL,
		UNIT_TYPEID::PROTOSS_STARGATE,
		UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY,
		UNIT_TYPEID::PROTOSS_TEMPLARARCHIVE,
		UNIT_TYPEID::PROTOSS_DARKSHRINE,
		UNIT_TYPEID::PROTOSS_FLEETBEACON,
		UNIT_TYPEID::PROTOSS_ROBOTICSBAY,
	};

	UnitTypeID ArmyUnitsTerran[] = {
		UNIT_TYPEID::TERRAN_MARINE,
		UNIT_TYPEID::TERRAN_MARAUDER,
		UNIT_TYPEID::TERRAN_REAPER,

		UNIT_TYPEID::TERRAN_GHOST,

		UNIT_TYPEID::TERRAN_HELLION,
		UNIT_TYPEID::TERRAN_HELLIONTANK,
		UNIT_TYPEID::TERRAN_WIDOWMINE,
		UNIT_TYPEID::TERRAN_CYCLONE,
		UNIT_TYPEID::TERRAN_SIEGETANK,
		UNIT_TYPEID::TERRAN_SIEGETANKSIEGED,

		UNIT_TYPEID::TERRAN_THOR,
		UNIT_TYPEID::TERRAN_THORAP,

		UNIT_TYPEID::TERRAN_VIKINGFIGHTER,
		UNIT_TYPEID::TERRAN_VIKINGASSAULT,
		UNIT_TYPEID::TERRAN_MEDIVAC,
		UNIT_TYPEID::TERRAN_LIBERATOR, 
		UNIT_TYPEID::TERRAN_LIBERATORAG, 

		UNIT_TYPEID::TERRAN_RAVEN,
		UNIT_TYPEID::TERRAN_AUTOTURRET,
		UNIT_TYPEID::TERRAN_BANSHEE,

		UNIT_TYPEID::TERRAN_BATTLECRUISER,

		UNIT_TYPEID::TERRAN_SCV,
		UNIT_TYPEID::TERRAN_MULE,
		UNIT_TYPEID::TERRAN_MISSILETURRET,
		UNIT_TYPEID::TERRAN_PLANETARYFORTRESS,
		UNIT_TYPEID::TERRAN_BUNKER,
		UNIT_TYPEID::TERRAN_KD8CHARGE,
	};

	UnitTypeID ArmyUnitsZerg[] = {  //BURROWED, COCOON
		UNIT_TYPEID::ZERG_ZERGLING,
		UNIT_TYPEID::ZERG_QUEEN,

		UNIT_TYPEID::ZERG_ROACH,
		UNIT_TYPEID::ZERG_RAVAGER,

		UNIT_TYPEID::ZERG_BANELING,

		UNIT_TYPEID::ZERG_HYDRALISK,

		UNIT_TYPEID::ZERG_INFESTOR,
		UNIT_TYPEID::ZERG_SWARMHOSTMP,
		UNIT_TYPEID::ZERG_LOCUSTMP,

		UNIT_TYPEID::ZERG_MUTALISK,
		UNIT_TYPEID::ZERG_CORRUPTOR,

		UNIT_TYPEID::ZERG_LURKERMP,
		UNIT_TYPEID::ZERG_LURKERMPBURROWED,

		UNIT_TYPEID::ZERG_VIPER,

		UNIT_TYPEID::ZERG_NYDUSCANAL,

		UNIT_TYPEID::ZERG_ULTRALISK,

		UNIT_TYPEID::ZERG_BROODLORD,
		UNIT_TYPEID::ZERG_BROODLING,

		UNIT_TYPEID::ZERG_DRONE,
		UNIT_TYPEID::ZERG_OVERLORD,
		UNIT_TYPEID::ZERG_OVERLORDTRANSPORT,
		UNIT_TYPEID::ZERG_OVERSEER,

		UNIT_TYPEID::ZERG_SPINECRAWLER,
		UNIT_TYPEID::ZERG_SPORECRAWLER,
	};

	AbilityID UnitCreationAbility[18] = {
		ABILITY_ID::TRAIN_ZEALOT,
		ABILITY_ID::TRAIN_STALKER,
		ABILITY_ID::TRAIN_SENTRY,
		ABILITY_ID::TRAIN_ADEPT,
		ABILITY_ID::TRAIN_DARKTEMPLAR,
		ABILITY_ID::TRAIN_HIGHTEMPLAR,
		ABILITY_ID::TRAIN_ARCHON,

		ABILITY_ID::TRAIN_OBSERVER,
		ABILITY_ID::TRAIN_IMMORTAL,
		ABILITY_ID::TRAIN_WARPPRISM,
		ABILITY_ID::TRAIN_COLOSSUS,
		ABILITY_ID::TRAIN_DISRUPTOR,

		ABILITY_ID::TRAIN_PHOENIX,
		ABILITY_ID::TRAIN_ORACLE,
		ABILITY_ID::TRAIN_VOIDRAY,
		ABILITY_ID::TRAIN_TEMPEST,
		ABILITY_ID::TRAIN_CARRIER,

		ABILITY_ID::TRAIN_MOTHERSHIP
	};

	UnitTypeID UnitCreators[18] = {
		UNIT_TYPEID::PROTOSS_GATEWAY,
		UNIT_TYPEID::PROTOSS_GATEWAY,
		UNIT_TYPEID::PROTOSS_GATEWAY,
		UNIT_TYPEID::PROTOSS_GATEWAY,
		UNIT_TYPEID::PROTOSS_GATEWAY,
		UNIT_TYPEID::PROTOSS_GATEWAY,
		UNIT_TYPEID::PROTOSS_GATEWAY,

		UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY,
		UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY,
		UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY,
		UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY,
		UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY,

		UNIT_TYPEID::PROTOSS_STARGATE,
		UNIT_TYPEID::PROTOSS_STARGATE,
		UNIT_TYPEID::PROTOSS_STARGATE,
		UNIT_TYPEID::PROTOSS_STARGATE,
		UNIT_TYPEID::PROTOSS_STARGATE,

		UNIT_TYPEID::PROTOSS_NEXUS
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

	static Point2D getRandomPoint(float startX = -1, float endX = -1, float startY = -1, float endY = -1) {
		FUNCTION_LOG();
		float sX = startX;
		float eX = endX;
		float sY = startY;
		float eY = endY;
		if (sX == -1) sX = 0;
		if (eX == -1) eX = (float)mapWidth_cache;
		if (sY == -1) sY = 0;
		if (eY == -1) eY = (float)mapHeight_cache;
		float x = sX + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (eX - sX - 1)));
		float y = sY + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (eY - sY - 1)));
		return Point2D{ x, y };
	}

	Point2D getRandomPointRadius(Point2D point, float radius_max, float radius_min = 0.0F) {
		float theta = (2.0F * GS_PI * rand()) / RAND_MAX;
		float radius = radius_min + ((radius_max - radius_min) * rand()) / RAND_MAX;
		return point + Point2D{ radius * cos(theta), radius * sin(theta) };
	}

	static Point2D getRandomPathable(float startX = -1, float endX = -1, float startY = -1, float endY = -1) {
		FUNCTION_LOG();
		Point2D p;
		do {
			p = getRandomPoint(startX, endX, startY, endY);
		} while (!Aux::isPathable(p) || !Aux::withinBounds(p));
		return p;
	}

	static Point2D getRandomPathablePointRadius(Point2D center, float radius) {
		FUNCTION_LOG();
		Point2D p;
		do {
			p = getRandomPointRadius(center, radius);
		} while (!Aux::isPathable(p) || !Aux::withinBounds(p));
		return p;
	}

	//static Point2D getRandomNonPathable(Agent* agent, float startX = -1, float endX = -1, float startY = -1, float endY = -1) {

	//	Point2D p;
	//	do {
	//		p = getRandomPoint(agent, startX, endX, startY, endY);
	//	} while (Aux::isPathable(p) || !Aux::withinBounds(p));
	//	return p;
	//}

	UnitTypes allData(Agent* agent) {
		FUNCTION_LOG();
		if (!init_data) {
			cached_data = agent->Observation()->GetUnitTypeData();
			init_data = true;
		}
		return cached_data;
	}

	UnitTypeData* getStats(UnitTypeID type, Agent* agent) {
		//FUNCTION_LOG();
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
			//	ExtraWeapon prismaticBeam(Weapon::TargetType::Any, 6, 1, 6, 0.36F);
			//	prismaticBeam.addDamageBonus(Attribute::Armored, 4);
			//	statsMap[UNIT_TYPEID::PROTOSS_VOIDRAY].weapons.push_back(prismaticBeam);
			//}
			//else if (type == UNIT_TYPEID::PROTOSS_SENTRY) {
			//	ExtraWeapon disruptionBeam(Weapon::TargetType::Any, 6, 1, 5, 0.71F);
			//	statsMap[UNIT_TYPEID::PROTOSS_SENTRY].weapons.push_back(disruptionBeam);
			//}
			//else if (type == UNIT_TYPEID::PROTOSS_DISRUPTOR) {
			//	//https://www.reddit.com/r/starcraft/comments/40pl7l/how_far_can_a_disruptors_purification_nova_travel/?rdt=50754
			//	ExtraWeapon novaAura(Weapon::TargetType::Any, 100, 1, 13, 21.4F);
			//	statsMap[UNIT_TYPEID::PROTOSS_DISRUPTOR].weapons.push_back(novaAura);
			//}
			//else if (type == UNIT_TYPEID::PROTOSS_DISRUPTORPHASED) {
			//	ExtraWeapon purificationNova(Weapon::TargetType::Ground, 100, 1, 1.5, EPSILON);
			//	statsMap[UNIT_TYPEID::PROTOSS_DISRUPTORPHASED].weapons.push_back(purificationNova);
			//}
			//else if (type == UNIT_TYPEID::ZERG_BANELING || type == UNIT_TYPEID::ZERG_BANELINGBURROWED) {
			//	//https://www.reddit.com/r/starcraft/comments/40pl7l/how_far_can_a_disruptors_purification_nova_travel/?rdt=50754
			//	ExtraWeapon volatileBurst(Weapon::TargetType::Ground, 16, 1, 2.2, EPSILON);
			//	volatileBurst.addDamageBonus(Attribute::Light, 19);
			//	statsMap[UNIT_TYPEID::ZERG_BANELING].weapons.push_back(volatileBurst);
			//	statsMap[UNIT_TYPEID::ZERG_BANELINGBURROWED].weapons.push_back(volatileBurst);
			//}
			//else if (type == UNIT_TYPEID::TERRAN_BATTLECRUISER) {
			//	ExtraWeapon ATSLaserBattery(Weapon::TargetType::Ground, 8.0F, 1, 6.0F, 0.16);
			//	ExtraWeapon ATALaserBattery(Weapon::TargetType::Air, 5.0F, 1, 6.0F, 0.16);
			//	statsMap[UNIT_TYPEID::TERRAN_BATTLECRUISER].weapons.push_back(ATSLaserBattery);
			//	statsMap[UNIT_TYPEID::TERRAN_BATTLECRUISER].weapons.push_back(ATALaserBattery);
			//}
		}
		return &(statsMap[type]);
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
			return UNIT_TYPEID::INVALID;
		}
	}

	static Cost buildAbilityToCost(AbilityID build_ability, Agent* agent) {
		FUNCTION_LOG();
		//sc2::UnitTypeData unit_stats =
		//    agent->Observation()->GetUnitTypeData().at(static_cast<uint32_t>(buildAbilityToUnit(build_ability)));
		UnitTypeData* stats = getStats(buildAbilityToUnit(build_ability), agent);
		return { stats->mineral_cost, stats->vespene_cost, 0, int(stats->food_required) };
	}

	static Cost unitAbilityToCost(AbilityID build_ability, Agent* agent) {
		FUNCTION_LOG();
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
		FUNCTION_LOG();
		UpgradeData upgrade_stats =
			agent->Observation()->GetUpgradeData().at(static_cast<uint32_t>(researchAbilityToUpgrade(research_ability)));
		return { upgrade_stats.mineral_cost, upgrade_stats.vespene_cost, 0 };
	}

	static Cost abilityToCost(AbilityID ability, Agent* agent) {
		FUNCTION_LOG();
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

	namespace expansionSearch {

		size_t CalculateQueries(float radius, float step_size, const Point2D& center, std::vector<QueryInterface::PlacementQuery>& queries) {
			Point2D current_grid, previous_grid(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
			size_t valid_queries = 0;
			// Find a buildable location on the circumference of the sphere
			float loc = 0.0f;
			while (loc < 360.0f) {
				Point2D point = Point2D(
					(radius * std::cos((loc * MY_PI) / 180.0f)) + center.x,
					(radius * std::sin((loc * MY_PI) / 180.0f)) + center.y);

				QueryInterface::PlacementQuery query(ABILITY_ID::BUILD_COMMANDCENTER, point);

				current_grid = Point2D(std::floor(point.x), std::floor(point.y));

				if (previous_grid != current_grid) {
					queries.push_back(query);
					++valid_queries;
				}

				previous_grid = current_grid;
				loc += step_size;
			}

			return valid_queries;
		}

		std::vector<std::pair<Point3D, std::vector<Unit> > > Cluster(const Units& units, float distance_apart) {
			float squared_distance_apart = distance_apart * distance_apart;
			std::vector<std::pair<Point3D, std::vector<Unit> > > clusters;
			for (size_t i = 0, e = units.size(); i < e; ++i) {
				const Unit& u = *units[i];

				float distance = std::numeric_limits<float>::max();
				std::pair<Point3D, std::vector<Unit> >* target_cluster = nullptr;
				// Find the cluster this mineral patch is closest to.
				for (auto& cluster : clusters) {
					float d = DistanceSquared3D(u.pos, cluster.first);
					if (d < distance) {
						distance = d;
						target_cluster = &cluster;
					}
				}

				// If the target cluster is some distance away don't use it.
				if (distance > squared_distance_apart) {
					clusters.push_back(std::pair<Point3D, std::vector<Unit> >(u.pos, std::vector<Unit>{u}));
					continue;
				}

				// Otherwise append to that cluster and update it's center of mass.
				target_cluster->second.push_back(u);
				size_t size = target_cluster->second.size();
				target_cluster->first = ((target_cluster->first * (float(size) - 1)) + u.pos) / float(size);
			}

			return clusters;
		}


		std::vector<Point3D> CalculateExpansionLocations(Agent* const agent, sc2::search::ExpansionParameters parameters = sc2::search::ExpansionParameters()) {
			Units resources = agent->Observation()->GetUnits(
				[](const Unit& unit) {
					return unit.unit_type == UNIT_TYPEID::NEUTRAL_MINERALFIELD || unit.unit_type == UNIT_TYPEID::NEUTRAL_MINERALFIELD750 ||
						unit.unit_type == UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD || unit.unit_type == UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD750 ||
						unit.unit_type == UNIT_TYPEID::NEUTRAL_PURIFIERMINERALFIELD || unit.unit_type == UNIT_TYPEID::NEUTRAL_PURIFIERMINERALFIELD750 ||
						unit.unit_type == UNIT_TYPEID::NEUTRAL_PURIFIERRICHMINERALFIELD || unit.unit_type == UNIT_TYPEID::NEUTRAL_PURIFIERRICHMINERALFIELD750 ||
						unit.unit_type == UNIT_TYPEID::NEUTRAL_LABMINERALFIELD || unit.unit_type == UNIT_TYPEID::NEUTRAL_LABMINERALFIELD750 ||
						unit.unit_type == UNIT_TYPEID::NEUTRAL_BATTLESTATIONMINERALFIELD || unit.unit_type == UNIT_TYPEID::NEUTRAL_BATTLESTATIONMINERALFIELD750 ||
						unit.unit_type == UNIT_TYPEID::NEUTRAL_VESPENEGEYSER || unit.unit_type == UNIT_TYPEID::NEUTRAL_PROTOSSVESPENEGEYSER ||
						unit.unit_type == UNIT_TYPEID::NEUTRAL_SPACEPLATFORMGEYSER || unit.unit_type == UNIT_TYPEID::NEUTRAL_PURIFIERVESPENEGEYSER ||
						unit.unit_type == UNIT_TYPEID::NEUTRAL_SHAKURASVESPENEGEYSER || unit.unit_type == UNIT_TYPEID::NEUTRAL_RICHVESPENEGEYSER;
				}
			);

			std::vector<Point3D> expansion_locations;
			std::vector<std::pair<Point3D, std::vector<Unit> > > clustersPre = Cluster(resources, parameters.cluster_distance_);
			std::vector<std::pair<Point3D, std::vector<Unit> > > clusters;

			for (int i = 0; i < clustersPre.size(); ++i) {
				//DebugSphere(agent, clustersPre[i].first, 2.5, Colors::Purple);
				//SendDebug(agent);
				//printf("");
				if (clustersPre[i].second.size() <= 10) {
					clusters.push_back(clustersPre[i]);
				}
			}


			std::vector<size_t> query_size;
			std::vector<QueryInterface::PlacementQuery> queries;
			for (size_t i = 0; i < clusters.size(); ++i) {
				std::pair<Point3D, std::vector<Unit> >& cluster = clusters[i];
				if (parameters.debug_) {
					for (auto r : parameters.radiuses_) {
						parameters.debug_->DebugSphereOut(cluster.first, r, Colors::Green);
					}
				}

				// Get the required queries for this cluster.
				size_t query_count = 0;
				for (auto r : parameters.radiuses_) {
					query_count += CalculateQueries(r, parameters.circle_step_size_, cluster.first, queries);
				}

				query_size.push_back(query_count);
			}

			std::vector<bool> results = agent->Query()->Placement(queries);
			size_t start_index = 0;
			for (int i = 0; i < clusters.size(); ++i) {
				std::pair<Point3D, std::vector<Unit> >& cluster = clusters[i];
				float distance = std::numeric_limits<float>::max();
				Point2D closest;

				// For each query for the cluster minimum distance location that is valid.
				for (size_t j = start_index, e = start_index + query_size[i]; j < e; ++j) {
					if (!results[j]) {
						continue;
					}

					Point2D& p = queries[j].target_pos;

					float d = Distance2D(p, cluster.first);
					if (d < distance) {
						distance = d;
						closest = p;
					}
				}

				Point3D expansion(closest.x, closest.y, cluster.second.begin()->pos.z);

				if (parameters.debug_) {
					parameters.debug_->DebugSphereOut(expansion, 0.35f, Colors::Red);
				}

				expansion_locations.push_back(expansion);
				start_index += query_size[i];
			}

			return expansion_locations;
		}
	}

	void setupExpansions(Agent* const agent) {
		FUNCTION_LOG();
		Point3D start = agent->Observation()->GetStartLocation();
		Point3D enemy = AP3D( gameInfo_cache.enemy_start_locations[0] );

		std::vector<Point3D> rawExpands = expansionSearch::CalculateExpansionLocations(agent);
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
		py.x = (criticalPoints[CrucialPoints::SELF_STARTLOC_POINT].x > mapWidth_cache / 2) ? criticalPoints[CrucialPoints::SELF_STARTLOC_POINT].x - 5.5 : criticalPoints[CrucialPoints::SELF_STARTLOC_POINT].x + 5.5;
		py.y = (criticalPoints[CrucialPoints::SELF_STARTLOC_POINT].y > mapHeight_cache / 2) ? criticalPoints[CrucialPoints::SELF_STARTLOC_POINT].y - 5.5 : criticalPoints[CrucialPoints::SELF_STARTLOC_POINT].y + 5.5;
		criticalPoints[CrucialPoints::SELF_FIRSTPYLON_POINT] = py;

		py.x = (criticalPoints[CrucialPoints::SELF_STARTLOC_POINT].x > mapWidth_cache / 2) ? expansions[selfRankedExpansions.begin()->expansionIndex].pos.x - 6 : expansions[selfRankedExpansions.begin()->expansionIndex].pos.x + 6;
		py.y = (criticalPoints[CrucialPoints::SELF_STARTLOC_POINT].y > mapHeight_cache / 2) ? expansions[selfRankedExpansions.begin()->expansionIndex].pos.y - 6 : expansions[selfRankedExpansions.begin()->expansionIndex].pos.y + 6;
		criticalPoints[CrucialPoints::SELF_RALLY_POINT] = py;

		py.x = (criticalPoints[CrucialPoints::SELF_STARTLOC_POINT].x > mapWidth_cache / 2) ? expansions[selfRankedExpansions.begin()->expansionIndex].pos.x - 4 : expansions[selfRankedExpansions.begin()->expansionIndex].pos.x + 4;
		py.y = (criticalPoints[CrucialPoints::SELF_STARTLOC_POINT].y > mapHeight_cache / 2) ? expansions[selfRankedExpansions.begin()->expansionIndex].pos.y - 4 : expansions[selfRankedExpansions.begin()->expansionIndex].pos.y + 4;
		criticalPoints[CrucialPoints::SELF_SHIELDBATTERY_POINT] = py;

	}

	void displayExpansions(Agent* const agent) {
		FUNCTION_LOG();
		for (int i = 0; i < expansions.size(); i++) {
			DebugSphere(agent, expansions[i].pos, 12, Colors::Yellow);
			DebugBox(agent, expansions[i].pos + Point3D{-2.5,-2.5, 0}, expansions[i].pos + Point3D{ 2.5,2.5, 4 }, Colors::Yellow);
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
			type == UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD750 || type == UNIT_TYPEID::NEUTRAL_PURIFIERMINERALFIELD || 
			type == UNIT_TYPEID::NEUTRAL_PURIFIERMINERALFIELD750 || type == UNIT_TYPEID::NEUTRAL_PURIFIERRICHMINERALFIELD ||
			type == UNIT_TYPEID::NEUTRAL_PURIFIERRICHMINERALFIELD750);
	}

	static bool isVespeneType(UnitTypeID type) {
		return (type == UNIT_TYPEID::NEUTRAL_VESPENEGEYSER || type == UNIT_TYPEID::NEUTRAL_PROTOSSVESPENEGEYSER ||
			type == UNIT_TYPEID::NEUTRAL_PURIFIERVESPENEGEYSER || type == UNIT_TYPEID::NEUTRAL_RICHVESPENEGEYSER ||
			type == UNIT_TYPEID::NEUTRAL_SHAKURASVESPENEGEYSER || type == UNIT_TYPEID::NEUTRAL_SPACEPLATFORMGEYSER);
	}

	encoding2D getRandomEncodingPoint() {
		float x = (50.0 * rand()) / RAND_MAX + 50;
		float y = (50.0 * rand()) / RAND_MAX + 50;
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
		{ UNIT_TYPEID::PROTOSS_FLEETBEACON, 3},

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
		{ UNIT_TYPEID::NEUTRAL_COLLAPSIBLEROCKTOWERDIAGONAL, 5 },
		{ UNIT_TYPEID::NEUTRAL_COLLAPSIBLEROCKTOWERDEBRIS, 5 },
		{ UNIT_TYPEID::COLLAPSIBLEROCKTOWER, 5 },
		{ UNIT_TYPEID::NEUTRAL_COLLAPSIBLETERRANTOWERDIAGONAL, 5 },
		{ UNIT_TYPEID::NEUTRAL_COLLAPSIBLETERRANTOWERDEBRIS, 5 },
		{ UNIT_TYPEID::COLLAPSIBLETERRANTOWER, 5 },
		{ UNIT_TYPEID::NEUTRAL_XELNAGATOWER, 2 },

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
						imRef(masterMap, i + x, j + y) &= 0xC0; //clears all but the highest two bits?
					}
					else {
						if (obstacle == VESPENE) {
							imRef(masterMap, i + x, j + y) |= 0x3; //sets it to unpathable and unplacable
						}
						imRef(masterMap, i + x, j + y) &= 0xC0; //clears all but the highest two bits?
						imRef(masterMap, i + x, j + y) |= ((uint8_t)(obstacle) << 2);
						uint8_t asd = imRef(masterMap, i + x, j + y);
						printf("");
						//imRef(masterMap, i + x, j + y) &= 0xFC;
					}

				}
			}
		}
	}

	static void loadUnitPlacement(ObstacleInfo obstacle, Point2D pos, UnitTypeID unit_type, int8_t(*pattern)[10][10] = nullptr) {
		FUNCTION_LOG();
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
		FUNCTION_LOG();
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

	static bool checkStructurePlacement(Point2D pos, int size, bool ignoreBuildingReserve = false) {
		FUNCTION_LOG();
		int x = (int)(pos.x - (size / 2) + ((size % 2 == 0) ? 0.5F : 0.0F));
		int y = (int)(pos.y - (size / 2) + ((size % 2 == 0) ? 0.5F : 0.0F));
		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) {
				if (!isPlacable(i + x, j + y, ignoreBuildingReserve)) {
					return false;
				}
			}
		}
		return true;
	}

	inline static bool checkStructurePlacement(Point2D pos, UnitTypeID type, bool ignoreBuildingReserve = false) {
		if (structureDiameter.find(type) == structureDiameter.end()) {
			printf("What is the radius of unit id %ul", type);
			throw 71;
		}
		return checkStructurePlacement(pos, structureDiameter[type], ignoreBuildingReserve);
	}

	inline static bool checkStructurePlacement(Point2D pos, AbilityID build, bool ignoreBuildingReserve = false) {
		UnitTypeID type = Aux::buildAbilityToUnit(build);
		return checkStructurePlacement(pos, type, ignoreBuildingReserve);
	}

	static Point2D possibleNextPylons[6] = {Point2D{8, -2}, Point2D{-8, 2}, Point2D{4, 5} , Point2D{-4, -5} , Point2D{4, -7} , Point2D{-4, 7} };
	static Point2D possibleNextBuildings[6] = { Point2D{-2.5, -0.5}, Point2D{0.5, -2.5}, Point2D{5.5, -2.5}, Point2D{4.5, 2.5}, Point2D{1.5, 4.5}, Point2D{-3.5, 4.5} };

	void reloadMasterMap(Agent* const agent, Point2D pos, int sizeX, int sizeY) {
		FUNCTION_LOG();
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
		FUNCTION_LOG();
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

	Color masterGridColor(int i, int j) {
		ObstacleInfo obstacle = getObstacle(i, j);
		uint8_t p = imRef(masterMap, i, j);
		if (isPlacable(i, j) && !isPathable(i, j)) {
			return Color{ 0, 255, 0 };
		}
		else if (obstacle == NOTHING) {
			if (!isPathable(i, j)) {
				return Color{ 1,1,1 };
			}
			else if (!isPlacable(i, j)) {
				return Color{ 100, 100, 100 };
			}
			else {
				return Color{ 254,254,254 };
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
	}

	void saveMasterBitmap(std::string fileName) {
		FUNCTION_LOG();
		saveBitmap(fileName, masterMap->width(), masterMap->height(), masterGridColor);
	}

	void setupMasterMap(Agent* const agent) {
		FUNCTION_LOG();
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

	//white and black ignored
	void gridTemplate(Agent* const agent, std::function<Color(int, int)> color, int cellSize = 1, bool pathableCheck = true, float boxBorder = 0.05F) {
		FUNCTION_LOG();
		Point2D center = agent->Observation()->GetCameraPos();
		int wS = int(center.x) - 10;
		if (wS < 1)
			wS = 1;
		int hS = int(center.y) - 5;
		if (hS < 1)
			hS = 1;
		int wE = int(center.x) + 11;
		if (wE >= (Aux::mapWidth_cache - 2))
			wE = (Aux::mapWidth_cache - 2);
		int hE = int(center.y) + 14;
		if (hE >= (Aux::mapHeight_cache - 2))
			hE = (Aux::mapHeight_cache - 2);

		for (int w = wS; w < wE; w++) {
			for (int h = hS; h < hE; h++) {
				if ((pathableCheck && !Aux::isPathableTile(w, h)) || w % cellSize != 0 || h % cellSize != 0) {
					continue;
				}
				Point2DI point = Point2DI(w, h);
				float boxHeight = 0;
				Color c = color(w, h);

				if (0 || (c.r != 255 && c.r != 0) || (c.g != 255 && c.g != 0) || (c.b != 255 && c.b != 0) || boxHeight != 0) {
					float height = std::max(
						agent->Observation()->TerrainHeight(Point2D{ float(w), float(h) }), 
						agent->Observation()->TerrainHeight(Point2D{ float(w + cellSize), float(h + cellSize) }));

					DebugBox(agent, Point3D(w + boxBorder, h + boxBorder, height + 0.05F),
						Point3D(w + cellSize - boxBorder, h + cellSize - boxBorder, 0), c);

					//DebugText(agent, strprintf("%d, %d", w, h),
					//	Point3D(w + boxBorder, h + 0.2F + boxBorder, height + 0.1F),
					//	Color(200, 90, 15), 8);
				}
			}
		}
	}


	//white and black ignored
	void gridTemplatePrecise(Agent* const agent, std::function<Color(int, int)> color, int precision = 1, bool pathableCheck = true, float boxBorder = 0.05F) {
		FUNCTION_LOG();
		Point2D center = agent->Observation()->GetCameraPos() * precision;
		int wS = int(center.x) - 10 * precision;
		if (wS < 1)
			wS = 1;
		int hS = int(center.y) - 5 * precision;
		if (hS < 1)
			hS = 1;
		int wE = int(center.x) + 11 * precision;
		if (wE >= (Aux::mapWidth_cache - 2) * precision)
			wE = (Aux::mapWidth_cache - 2) * precision;
		int hE = int(center.y) + 14 * precision;
		if (hE >= (Aux::mapHeight_cache - 2) * precision)
			hE = (Aux::mapHeight_cache - 2) * precision;

		float blockSize = 1.0F / precision;

		for (int w = wS; w < wE; w++) {
			for (int h = hS; h < hE; h++) {
				if ((pathableCheck && !Aux::isPathableTile(w / precision, h / precision))) {
					continue;
				}
				Point2D point(w * blockSize, h * blockSize);
				float boxHeight = 0;
				Color c = color(w, h);

				float w_real = (float)w / precision;
				float h_real = (float)h / precision;

				if (0 || (c.r != 255 && c.r != 0) || (c.g != 255 && c.g != 0) || (c.b != 255 && c.b != 0) || boxHeight != 0) {
					float height = std::max(
						agent->Observation()->TerrainHeight(Point2D{ w_real, h_real }),
						agent->Observation()->TerrainHeight(Point2D{ w_real + 1, h_real + 1 }));

					DebugBox(agent, Point3D(w_real + boxBorder, h_real + boxBorder, height + 0.05F),
						Point3D(w_real + blockSize - boxBorder, h_real + blockSize - boxBorder, 0), c);

					//DebugText(agent, strprintf("%d, %d", w, h),
					//	Point3D(w + boxBorder, h + 0.2F + boxBorder, height + 0.1F),
					//	Color(200, 90, 15), 8);
				}
			}
		}
	}

	void displayMasterGrid(Agent* const agent) {
		FUNCTION_LOG();
		gridTemplate(agent, masterGridColor);
	}

	const char* attributeNames[] = {
		"Unknown",
		"Light",
		"Armored",
		"Biological",
		"Mechanical",
		"Robotic",
		"Psionic",
		"Massive",
		"Structure",
		"Hover",
		"Heroic",
		"Summoned",
		"Invalid"
	};

	const char* targetTypeNames[] = {
		"Unknown",
		"Ground",
		"Air",
		"Any",
		"Invalid"
	};

	const char* displayTypeNames[] = {
		"Visible",
		"Snapshot",
		"Hidden",
		"Placeholder"
	};

	const char* allianceNames[] = {
		"Self",
		"Ally",
		"Neutral",
		"Enemy"
	};

	const char* cloakStateNames[] = {
		"CloakedUnknown",
		"Cloaked",
		"CloakedDetected",
		"NotCloaked",
		"CloakedAllied"
	};

	const char* raceNames[] = {
		"Terran",
		"Zerg",
		"Protoss",
		"Random"
	};

	inline const char* AttributeToName(Attribute a) {
		return attributeNames[(int)a];
	}

	inline const char* TargetTypeToName(Weapon::TargetType t) {
		return targetTypeNames[(int)t];
	}

	inline const char* DisplayTypeToName(Unit::DisplayType d) {
		return displayTypeNames[(int)d];
	}

	inline const char* AllianceToName(Unit::Alliance a) {
		return allianceNames[(int)a];
	}

	inline const char* CloakStateToName(Unit::CloakState c) {
		return cloakStateNames[(int)c];
	}

	inline const char* RaceToName(Race r) {
		return raceNames[(int)r];
	}

	inline std::string barToString(float percent_decimal) {
		if (percent_decimal < 0.2) {
			percent_decimal = 0;
		}
		int full = (int)(percent_decimal * 10);
		if (full >= 10) {
			return std::string("##########");
		}
		int half = (int)(((full / 10.0F) - percent_decimal) * 50);
		//std::string result = "";
		char str[11] = { 0 };
		memset(str, ' ', 10);
		memset(str, '#', full);
		char hal = '?';
		if (half == 1) {
			hal = '~';
		}
		else if (half == 2) {
			hal = ':';
		}
		else if (half == 3) {
			hal = '!';
		}
		else if (half == 4) {
			hal = '$';
		}
		else {
			hal = '*';
		}
		memset(str+full, hal, 1);
		return std::string(str);
	}

	inline static Color randomColor() {
		return Color{ uint8_t(255 * rand() / RAND_MAX) , uint8_t(255 * rand() / RAND_MAX) , uint8_t(255 * rand() / RAND_MAX) };
	}

	inline Point3D P3D(Agent* const agent, const Point2D& p) {
		return Point3D(p.x, p.y, agent->Observation()->TerrainHeight(p));
	}

	inline Point2D P2D(const Point2DI& p) {
		return Point2D((float)p.x, (float)p.y);
	}

	inline Point2D P2D(const Point3D& p) {
		return Point2D(p.x, p.y);
	}

	inline Point2DI P2DI(const Point3D& p) {
		return Point2DI((int)p.x, (int)p.y);
	}

	inline Point2DI P2DI(const Point2D& p) {
		return Point2DI((int)p.x, (int)p.y);
	}

	inline Point2D normalize(const Point2D& p) {
		return p / (std::sqrt(p.x * p.x + p.y * p.y));

	}

	float floatmod(float number, float mod) {
		while (number < 0) {
			number += mod;
		}
		while (number >= mod) {
			number -= mod;
		}
		return number;
	}

	inline float atan_scalar_approximation(float x) {
		float a1 = 0.99997726f;
		float a3 = -0.33262347f;
		float a5 = 0.19354346f;
		float a7 = -0.11643287f;
		float a9 = 0.05265332f;
		float a11 = -0.01172120f;

		float x_sq = x * x;

		return x * fmaf(x_sq, fmaf(x_sq, fmaf(x_sq, fmaf(x_sq, fmaf(x_sq, a11, a9), a7), a5), a3), a1);

	}

	float atan2f_auto(float y, float x) {
		//if (x == 0.0F && y == 0.0F) {
		//	return 0;
		//}
		bool swap = fabs(x) < fabs(y);
		float atan_input = (swap ? x : y) / (swap ? y : x);

		// Approximate atan
		float res = atan_scalar_approximation(atan_input);

		// If swapped, adjust atan output
		res = swap ? (atan_input >= 0.0f ? MY_PI2 : -MY_PI2) - res : res;
		// Adjust quadrants
		if (x < 0.0f) {
			if (y >= 0.0f) {
				res = MY_PI + res;
			}
			else {
				res = -MY_PI + res;
			}
		}

		// Store result
		return res;
	}

	inline float atan2f_prim(float y, float x) {
		//return atan2f(y, x);
		return atan2f_auto(y, x);
	}

	constexpr int ATAN2_TEST_NUM = 100000;
	void atanTest() {
		float deviance = 0.0F;
		long long timebuiltin = 0;
		long long timecustom = 0;
		FILE* fp = fopen("data/atanTests.csv", "w+");
		fprintf(fp, "X, Y, builtin, custom, deviance, builtin_ms, custom_ms\n");
		for (int i = 0; i < ATAN2_TEST_NUM; i++) {
			float x = rand() * 3.0 / RAND_MAX - 1.5; 
			float y = rand() * 3.0 / RAND_MAX - 1.5;
			
			timeus start = std::chrono::steady_clock::now();
			float builtin = atan2f(y, x);
			timeus mid = std::chrono::steady_clock::now();
			float auto1 = atan2f_auto(y, x);
			timeus end = std::chrono::steady_clock::now();
			deviance += fabs(builtin - auto1);
			long long b_ms = std::chrono::duration_cast<std::chrono::microseconds>(mid - start).count();
			timebuiltin += b_ms;
			long long c_ms = std::chrono::duration_cast<std::chrono::microseconds>(end - mid).count();
			timecustom += c_ms;
			//fwrite(&c, 1, 1, fp);
			fprintf(fp, "%f, %f, %f, %f, %f, %lld, %lld\n", x, y, builtin, auto1, fabs(builtin - auto1), b_ms, c_ms);
		}
		printf("Time Builtin: %lldus Time Custom: %lldus Deviance: %f rad\n", timebuiltin, timecustom, deviance/ ATAN2_TEST_NUM);
		fclose(fp);
	}
}