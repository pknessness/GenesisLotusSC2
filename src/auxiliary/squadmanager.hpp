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

	class Squad {
		UnitWrapperPtr core;
		StrategyManager::UnitRatio unitComp;
		
		std::map<UnitWrapperPtr, char> squadStates;
		std::map<UnitWrapperPtr, char> unitStates;

		UnitWrappers armyContents;
		UnitWrappers squadTargets;

	public:
		SquadMode squadMode;
		Point2D targetPosition;

		Squad() : squadMode(INVALID) {

		}

		void add(UnitWrapperPtr armyUnit) {
			armyContents.insert(armyUnit);
		}

		void remove(UnitWrapperPtr armyUnit) {
			armyContents.erase(armyUnit);
		}

		UnitWrapperPtr getCore(Agent* const agent) {
			if (core == nullptr || core->get(agent) == nullptr) {
				if (armyContents.size() > 0) {
					core = *armyContents.begin();
				}
			}
			return core;
		}

		Point2D getCorePosition(Agent* const agent) {
			if (getCore(agent) != nullptr) {
				return core->pos(agent);
			}
			return Point2D{ 0,0 };
		}

		inline float armyballRadius() {
			return armyContents.size() * 4;
		}

		inline float armyballSquaredRadius() {
			return std::sqrt(armyballRadius());
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
			Point2D corePos = getCorePosition(agent);
			int count = 0;
			for (auto it = armyContents.begin(); it != armyContents.end(); it++) {
				if (DistanceSquared2D((*it)->pos(agent), corePos) < armyballSquaredRadius()) {
					count++;
				}
			}
			return count;
		}
	};
}