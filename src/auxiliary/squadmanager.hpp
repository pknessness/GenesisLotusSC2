#pragma once
#include "strategymanager.hpp"

namespace SquadManager {
	enum SquadMode {
		INVALID,
		ATTACK,
		DEFEND,
		SEARCH,
		HARASS,
	};

	std::map<UnitTypeID, int32_t> priorityMap;

	std::map<UnitTypeID, int32_t> baseUnitValueMap;


	void init() {
		// Terran
		priorityMap[UNIT_TYPEID::TERRAN_WIDOWMINE] = 8;
		priorityMap[UNIT_TYPEID::TERRAN_WIDOWMINEBURROWED] = 10;
		priorityMap[UNIT_TYPEID::TERRAN_SIEGETANK] = 8;
		priorityMap[UNIT_TYPEID::TERRAN_SIEGETANKSIEGED] = 10;
		priorityMap[UNIT_TYPEID::TERRAN_MULE] = 3;
		priorityMap[UNIT_TYPEID::TERRAN_SCV] = 3;
		priorityMap[UNIT_TYPEID::TERRAN_GHOST] = 7;
		priorityMap[UNIT_TYPEID::TERRAN_REAPER] = 4;
		priorityMap[UNIT_TYPEID::TERRAN_MARAUDER] = 4;
		priorityMap[UNIT_TYPEID::TERRAN_MARINE] = 3;
		priorityMap[UNIT_TYPEID::TERRAN_CYCLONE] = 5;
		priorityMap[UNIT_TYPEID::TERRAN_HELLION] = 2;
		priorityMap[UNIT_TYPEID::TERRAN_HELLIONTANK] = 3;
		priorityMap[UNIT_TYPEID::TERRAN_THOR] = 7;
		priorityMap[UNIT_TYPEID::TERRAN_MEDIVAC] = 6;
		priorityMap[UNIT_TYPEID::TERRAN_VIKINGFIGHTER] = 5;
		priorityMap[UNIT_TYPEID::TERRAN_VIKINGASSAULT] = 5;
		priorityMap[UNIT_TYPEID::TERRAN_LIBERATORAG] = 7;
		priorityMap[UNIT_TYPEID::TERRAN_LIBERATOR] = 5;
		priorityMap[UNIT_TYPEID::TERRAN_RAVEN] = 7;
		priorityMap[UNIT_TYPEID::TERRAN_BATTLECRUISER] = 8;
		priorityMap[UNIT_TYPEID::TERRAN_MISSILETURRET] = 1;
		priorityMap[UNIT_TYPEID::TERRAN_BUNKER] = 2;

		// Zerg
		priorityMap[UNIT_TYPEID::ZERG_DRONE] = 4;
		priorityMap[UNIT_TYPEID::ZERG_ZERGLING] = 3;
		priorityMap[UNIT_TYPEID::ZERG_BANELING] = 6;
		priorityMap[UNIT_TYPEID::ZERG_BANELINGCOCOON] = 6;
		priorityMap[UNIT_TYPEID::ZERG_ULTRALISK] = 6;
		priorityMap[UNIT_TYPEID::ZERG_QUEEN] = 5;
		priorityMap[UNIT_TYPEID::ZERG_ROACH] = 6;
		priorityMap[UNIT_TYPEID::ZERG_RAVAGER] = 8;
		priorityMap[UNIT_TYPEID::ZERG_RAVAGERCOCOON] = 8;
		priorityMap[UNIT_TYPEID::ZERG_HYDRALISK] = 7;
		priorityMap[UNIT_TYPEID::ZERG_HYDRALISKBURROWED] = 7;
		priorityMap[UNIT_TYPEID::ZERG_LURKERMP] = 9;
		priorityMap[UNIT_TYPEID::ZERG_LURKERMPEGG] = 9;
		priorityMap[UNIT_TYPEID::ZERG_LURKERMPBURROWED] = 9;
		priorityMap[UNIT_TYPEID::ZERG_INFESTOR] = 10;
		priorityMap[UNIT_TYPEID::ZERG_BROODLORD] = 10;
		priorityMap[UNIT_TYPEID::ZERG_BROODLORDCOCOON] = 10;
		priorityMap[UNIT_TYPEID::ZERG_MUTALISK] = 6;
		priorityMap[UNIT_TYPEID::ZERG_CORRUPTOR] = 8;
		priorityMap[UNIT_TYPEID::ZERG_OVERLORD] = 2;
		priorityMap[UNIT_TYPEID::ZERG_OVERSEER] = 1;
		priorityMap[UNIT_TYPEID::ZERG_VIPER] = 3;
		priorityMap[UNIT_TYPEID::ZERG_LARVA] = -1;
		priorityMap[UNIT_TYPEID::ZERG_EGG] = -1;
		priorityMap[UNIT_TYPEID::ZERG_LOCUSTMP] = -1;

		// Protoss
		priorityMap[UNIT_TYPEID::PROTOSS_SENTRY] = 8;
		priorityMap[UNIT_TYPEID::PROTOSS_PROBE] = 4;
		priorityMap[UNIT_TYPEID::PROTOSS_HIGHTEMPLAR] = 10;
		priorityMap[UNIT_TYPEID::PROTOSS_DARKTEMPLAR] = 9;
		priorityMap[UNIT_TYPEID::PROTOSS_ADEPT] = 4;
		priorityMap[UNIT_TYPEID::PROTOSS_ZEALOT] = 4;
		priorityMap[UNIT_TYPEID::PROTOSS_STALKER] = 5;
		priorityMap[UNIT_TYPEID::PROTOSS_IMMORTAL] = 8;
		priorityMap[UNIT_TYPEID::PROTOSS_COLOSSUS] = 10;
		priorityMap[UNIT_TYPEID::PROTOSS_WARPPRISM] = 8;
		priorityMap[UNIT_TYPEID::PROTOSS_OBSERVER] = 7;
		priorityMap[UNIT_TYPEID::PROTOSS_DISRUPTOR] = 9;
		priorityMap[UNIT_TYPEID::PROTOSS_PHOENIX] = 5;
		priorityMap[UNIT_TYPEID::PROTOSS_VOIDRAY] = 5;
		priorityMap[UNIT_TYPEID::PROTOSS_CARRIER] = 7;
		priorityMap[UNIT_TYPEID::PROTOSS_TEMPEST] = 6;
		priorityMap[UNIT_TYPEID::PROTOSS_MOTHERSHIP] = 9;
		priorityMap[UNIT_TYPEID::PROTOSS_ARCHON] = 6;
		priorityMap[UNIT_TYPEID::PROTOSS_SHIELDBATTERY] = 1;
		priorityMap[UNIT_TYPEID::PROTOSS_PHOTONCANNON] = 1;
		priorityMap[UNIT_TYPEID::PROTOSS_PYLON] = 2;
	}

	class Squad {
		UnitWrapperPtr core;
		StrategyManager::UnitRatio unitComp;

		UnitWrappers armyContents;

	public:
		UnitWrappers squadTargets;
		std::map<Tag, float> squadTargetDamage;

		/*
		* ' ' is without state
		* 'u' is unjoined squad
		* 'j' is joined squad
		*/
		std::map<Tag, char> squadMainStates;

		/*
		* ' ' is without-state
		* 'n' is moving safely
		* 'k' is attack moving
		* 'a' is target attacking
		* 'm' is manual
		*/
		std::map<Tag, char> unitStates;

		SquadMode squadMode;
		Point2D targetPosition;

		Squad() : squadMode(INVALID) {

		}

		void add(UnitWrapperPtr armyUnit) {
			FUNCTION_LOG();
			armyContents.insert(armyUnit);
		}

		void remove(UnitWrapperPtr armyUnit) {
			FUNCTION_LOG();
			armyContents.erase(armyUnit);
		}

		UnitWrapperPtr getCore(Agent* const agent) {
			FUNCTION_LOG();
			if (core == nullptr || core->getReturn(agent) == nullptr) {
				if (armyContents.size() > 0) {
					bool hasGND = false;
					Point2D center;
					int cnt = 0;
					for (auto it = armyContents.begin(); it != armyContents.end(); it++){
						if (1 || !(*it)->isFlying(agent)) {
							hasGND = true;
							center += (*it)->pos(agent);
							cnt++;
						}
					}
					center /= cnt;
					float dist2 = FLT_MAX;
					for (auto it = armyContents.begin(); it != armyContents.end(); it++) {
						float distance2 = DistanceSquared2D((*it)->pos(agent), center);
						if (distance2 < dist2) {
							dist2 = distance2;
							core = *it;
						}
					}
					//core = *armyContents.begin();
				}
			}
			return core;
		}

		Point2D getCorePosition(Agent* const agent) {
			FUNCTION_LOG();
			if (getCore(agent) != nullptr) {
				return core->pos(agent);
			}
			return Point2D{ 0,0 };
		}

		inline float armyballSquaredRadius() {
			FUNCTION_LOG();
			return armyContents.size() * 4;
		}

		inline float armyballRadius() {
			FUNCTION_LOG();
			return  std::sqrt(armyballSquaredRadius());
		}

		inline bool isWithinRadius(Point2D p, Agent* const agent) {
			return DistanceSquared2D(getCorePosition(agent), p) < armyballSquaredRadius();
		}

		inline bool isWithinRadius(UnitWrapperPtr unit, Agent* const agent) {
			return DistanceSquared2D(getCorePosition(agent), unit->pos(agent)) < armyballSquaredRadius();
		}

		void doAttack(Point2D location_) {
			squadMode = ATTACK;
			targetPosition = location_;
		}

		void doDefend(Point2D location_) {
			squadMode = DEFEND;
			targetPosition = location_;
		}

		void doHarass(Point2D location_) {
			squadMode = HARASS;
			targetPosition = location_;
		}

		void doSearch() {
			squadMode = SEARCH;
			targetPosition = Point2D{ 0,0 };
		}

		void execute(Agent* const agent) {
			FUNCTION_LOG();

			squadTargets.clear();
			squadTargetDamage.clear();
			Circles c = {};
			if (squadMode == DEFEND) {
				c.push_back({ targetPosition, 30 });
			}
			else {
				for (auto it = armyContents.begin(); it != armyContents.end(); it++) {
					if (squadMainStates[(*it)->self] == 'u') {
						continue;
					}
					float radius = Aux::getStats((*it)->getActualType(agent), agent)->sight_range + 1;
					c.push_back({ (*it)->pos(agent), radius });
				}
			}
			UnitWrappers wraps = SpatialHashGrid::findInRadiiEnemyLoose(c);
			squadTargets.insert(wraps.begin(), wraps.end());

			if (squadMode == INVALID) {
				
			}
			else if (squadMode == ATTACK) {

			}
			else if (squadMode == DEFEND) {

			}
			else if (squadMode == SEARCH) {

			}
			else if (squadMode == HARASS) {

			}
			std::vector<UnitWrapperPtr> dead;
			for (auto it = armyContents.begin(); it != armyContents.end(); it++) {
				if ((*it)->isDead()) {
					dead.push_back(*it);
				}
			}
			for (int i = 0; i < dead.size(); i++) {
				armyContents.erase(dead[i]);
			}
			for (auto it = armyContents.begin(); it != armyContents.end(); it++) {
				(*it)->execute(agent);
			}
		}

		int squadSize(Agent* const agent) {
			FUNCTION_LOG();
			Point2D corePos = getCorePosition(agent);
			int count = 0;
			for (auto it = armyContents.begin(); it != armyContents.end(); it++) {
				/*if (DistanceSquared2D((*it)->pos(agent), corePos) < armyballSquaredRadius()) {
					count++;
				}*/
				if (squadMainStates[(*it)->self] != 'u') {
					count++;
				}
			}
			return count;
		}

		float getEnemyUnitPriority(UnitWrapperPtr enemyUnit, Agent* const agent) {
			UnitTypeID id = enemyUnit->getActualType(agent);
			//if (priorityMap.find(id) != priorityMap.end()) {
			//	return (float)priorityMap[id];
			//}
			UnitTypeData* stats = Aux::getStats(id, agent);
			return (stats->mineral_cost + 1.7 * stats->vespene_cost) / 100;
		}
	};
}