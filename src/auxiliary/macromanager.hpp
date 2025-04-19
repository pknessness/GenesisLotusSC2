#pragma once

#include <sc2api/sc2_api.h>
#include <iostream>
#include <unordered_set>
#include <queue> 

#include "../unitwrappers/unitwrapper.hpp"
#include "helpers.hpp"
#include "profiler.hpp"
#include "../unitwrappers/probe.hpp"
#include "../unitwrappers/vespene.hpp"
#include "debugging.hpp"

using namespace sc2;

#define ACTION_CHECK_DT 10
#define PROBE_CHECK_DT 30
#define PROBE_CHECK_DACTION 3

struct MacroActionData {
	int x;
	int y;
	int z;

	MacroActionData() {

	}
};

struct MacroAction {
	static int globalIndex;
	
	int index;

	mutable AbilityID ability;

	mutable UnitTypeID executor;
	mutable UnitWrapperPtr executorPtr;

	//mutable Point2D pos;
	mutable Aux::PointArea position;
	mutable UnitWrapperPtr target;
	
	mutable int dependency;

	mutable bool chronoBoost;

	mutable MacroActionData extraData;

	MacroAction(UnitTypeID unit_type_, AbilityID ability_, Aux::PointArea pos_, bool chronoBoost_ = false, MacroActionData extraData_ = MacroActionData(), int dependency_ = -1, int index_ = -1)
		: executor( unit_type_), ability(ability_), position(pos_), 
		chronoBoost(chronoBoost_), target(nullptr), index(globalIndex),
		dependency(dependency_), executorPtr(nullptr) {
		if (index_ == -1) {
			globalIndex++;
		}
		else {
			index = index_;
		}
	}

	MacroAction(UnitTypeID unit_type_, AbilityID ability_, UnitWrapperPtr target_, bool chronoBoost_ = false, MacroActionData extraData_ = MacroActionData(), int dependency_ = -1, int index_ = -1)
		: executor(unit_type_), ability(ability_), position(),
		chronoBoost(chronoBoost_), target(target_), index(globalIndex),
		dependency(dependency_), executorPtr(nullptr) {
		if (index_ == -1) {
			globalIndex++;
		}
		else {
			index = index_;
		}
	}

	MacroAction(UnitTypeID unit_type_, AbilityID ability_, bool chronoBoost_ = false, MacroActionData extraData_ = MacroActionData(), int dependency_ = -1, int index_ = -1)
		: executor(unit_type_), ability(ability_), position(),
		chronoBoost(chronoBoost_), target(nullptr), index(globalIndex),
		dependency(dependency_), executorPtr(nullptr) {
		if (index_ == -1) {
			index = globalIndex;
			globalIndex++;
		}
		else {
			index = index_;
		}
	}

	operator Building() const {
		return Building{ ability, position.pos };
	}
};

struct MacroBuilding : MacroAction {
	MacroBuilding(AbilityID ability_, Aux::PointArea pos_ = Aux::PointDefault(), MacroActionData extraData_ = MacroActionData(), int dependency_ = -1, int index_ = -1) : MacroAction(UNIT_TYPEID::PROTOSS_PROBE, ability_, pos_, false, extraData_, dependency_, index_){
		
	}
};

struct MacroGateway : MacroAction {
	MacroGateway(AbilityID ability_, bool chronoBoost_ = false, MacroActionData extraData_ = MacroActionData(), int dependency_ = -1, int index_ = -1) : MacroAction(UNIT_TYPEID::PROTOSS_GATEWAY, ability_, chronoBoost_, extraData_, dependency_, index_) {

	}
};

struct MacroRobo : MacroAction {
	MacroRobo(AbilityID ability_, bool chronoBoost_ = false, MacroActionData extraData_ = MacroActionData(), int dependency_ = -1, int index_ = -1) : MacroAction(UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY, ability_, chronoBoost_, extraData_, dependency_, index_) {

	}
};

struct MacroStargate : MacroAction {
	MacroStargate(AbilityID ability_, bool chronoBoost_ = false, MacroActionData extraData_ = MacroActionData(), int dependency_ = -1, int index_ = -1) : MacroAction(UNIT_TYPEID::PROTOSS_STARGATE, ability_, chronoBoost_, extraData_, dependency_, index_) {

	}
};

int MacroAction::globalIndex = 0;

namespace MacroManager {

	Point2D getPylonLocation(Point2D pos = { 0,0 }, float radius = 0) {
		if (pos == Point2D{ 0, 0 }) {
			UnitWrappers pylons = UnitManager::getSelf(UNIT_TYPEID::PROTOSS_PYLON);
			for (auto it = pylons.begin(); it != pylons.end(); it++) {
				for (Point2D displaceMent : Aux::possibleNextPylons) {
					Point2D loc = (*it)->posCache() + displaceMent;
					if (Aux::checkStructurePlacement(loc, 2)) {
						return loc;
					}
				}
			}
		}
		else {
			int numP = (int)((radius + 1) * (radius + 1));
			for (int i = 0; i < numP; i++) {
				Point2D p = Aux::getRandomPointRadius(p, radius);
				if (Aux::checkStructurePlacement(p, 2)) {
					return p;
				}
			}
		}
		return Point2D{ -1, -1 };
	}

	Point2D getBuildingLocation(Point2D pos = { 0,0 }, float radius = 0) { //TODO: ACCOUNT FOR HEIGHT, PYLONS CAN ONLY POWER DOWNWARDS NOT UPWARDS
		if (pos == Point2D{ 0, 0 }) {
			UnitWrappers pylons = UnitManager::getSelf(UNIT_TYPEID::PROTOSS_PYLON);
			for (auto it = pylons.begin(); it != pylons.end(); it++) {
				for (Point2D displaceMent : Aux::possibleNextBuildings) {
					Point2D loc = (*it)->posCache() + displaceMent;
					if (Aux::checkStructurePlacement(loc, 3)) {
						return loc;
					}
				}
			}
		}
		else {
			int numP = (int)((radius + 1) * (radius + 1));
			for (int i = 0; i < numP; i++) {
				Point2D p = Aux::getRandomPointRadius(p, radius);
				if (Aux::checkStructurePlacement(p, 3)) {
					return p;
				}
			}
		}
		return Point2D{ -1, -1 };
	}

	class MacroActionCompare {
	public:
		bool operator()(const MacroAction a, const MacroAction b) const
		{
			return a.index < b.index;
		}
	};
	class MacroActionPtrCompare {
	public:
		bool operator()(const MacroAction* a, const MacroAction* b) const
		{
			return a->index < b->index;
		}
	};

	//std::map<UnitTypeID, std::priority_queue<MacroAction, std::vector<MacroAction>, MacroActionCompare>> allActions;
	std::map<UnitTypeID, std::multiset<MacroAction, MacroActionCompare>> allActions;
	
	int lastPylonTriggered_frames = 0;
	int macroExecuteCooldown_frames = 0;
	std::string diagnostics = "";

	std::unordered_set<int> finishedActions;

	inline bool isActionCompleted(int action) {
		return finishedActions.count(action);
	}

	void addAction(MacroAction action) {
		allActions[action.executor].insert(action);
	}

	void execute(Agent* const agent) {
		Profiler macroProfiler("mac.exec");
		if (macroExecuteCooldown_frames > 0) {
			macroExecuteCooldown_frames--;
			return;
		}
		
		std::multiset<const MacroAction*, MacroActionPtrCompare> topActions;
		int currentMinerals = agent->Observation()->GetMinerals();
		int currentVespene = agent->Observation()->GetVespene();
		//TODO: TAKE INTO ACCOUNT PROBE BUILDING STORAGE
			
		for (auto it = allActions.begin(); it != allActions.end(); it++) {
			if (it->second.size() == 0) {
				continue;
			}
			topActions.insert(&(*(it->second.begin())));
		}

		std::string diagnostics = "";

		for (auto it = topActions.begin(); it != topActions.end(); it++) {
			const MacroAction* currentAction = *it;
			int readyInXFrames = 0;

			int foodCap = agent->Observation()->GetFoodCap();
			int foodUsed = agent->Observation()->GetFoodUsed();
			
			diagnostics += AbilityTypeToName((*it)->ability);
			diagnostics += ": ";

			UnitWrappers allPossibleUnits = UnitManager::getSelf(currentAction->executor);

			//Check if there are any units of the relevant type
			if (allPossibleUnits.size() == 0) {
				diagnostics += "NO EXISTING EXECUTOR\n\n";
				continue;
			}

			//Filter units
			UnitWrappers possibleUnits;
			for (auto it = allPossibleUnits.begin(); it != allPossibleUnits.end(); it++) {
				const Unit* unwrapUnit = (*it)->get(agent);
				if (unwrapUnit != nullptr && (unwrapUnit->orders.size() == 0 || currentAction->executor == UNIT_TYPEID::PROTOSS_PROBE) && unwrapUnit->build_progress == 1.0F) {
					possibleUnits.insert(*it);
				}
			}

			//Check if there are any units post-filtering
			if (possibleUnits.size() == 0) {
				diagnostics += "NO FREE EXECUTOR\n\n";
				continue;
			}

			if (currentAction->position.pa_type == Aux::PointArea::DEFAULT_FINDOUT) {
				Point2D p;
				if (currentAction->ability == ABILITY_ID::BUILD_PYLON) {
					p = getPylonLocation();

				} else if (currentAction->ability == ABILITY_ID::BUILD_ASSIMILATOR) {
					UnitWrappers vespenes = UnitManager::getVespene();

					bool hasNexus = false;
					float minDist = -1;
					UnitWrapperPtr nextTarget;

					for (UnitWrapperPtr vespeneW : vespenes) {
						VespenePtr vespene = std::static_pointer_cast<Vespene>(vespeneW);

						if (vespene->taken || (hasNexus && !vespene->nearNexus)) {
							continue;
						}

						float dist = DistanceSquared2D(Aux::criticalPoints[Aux::SELF_FIRSTPYLON_POINT], vespene->pos(agent));

						if (minDist == -1 || (!hasNexus && vespene->nearNexus) || dist < minDist) {
							if (vespene->nearNexus) {
								hasNexus = true;
							}
							minDist = dist;
							nextTarget = vespene;
						}
					}
					p = nextTarget->pos(agent);

				} else if (currentAction->ability == ABILITY_ID::BUILD_NEXUS) {
					for (Aux::ExpansionDistance expansionDist : Aux::selfRankedExpansions) {
						Point2D loc = Aux::expansions[expansionDist.expansionIndex].pos;
						if (Aux::checkStructurePlacement(loc, 5)) {
							p = loc;
							break;
						}
					}
				
				} else{
					p = getBuildingLocation();
				}
				if (p != Point2D{ -1, -1 }) {
					currentAction->position.pa_type = Aux::PointArea::SINGLE_POINT;
					currentAction->position.pos = p;
				}
			}
			else if (currentAction->position.pa_type == Aux::PointArea::POINT_RADIUS) {

			}

			if (currentAction->position.pa_type == Aux::PointArea::DEFAULT_FINDOUT) {
				diagnostics += "DEFAULT_FINDOUT LOCATION\n\n";
				continue;
			}
			UnitTypeID unitToCreate = Aux::buildAbilityToUnit(currentAction->ability);
			UnitTypeData ability_stats = Aux::getStats(unitToCreate, agent);
			UnitTypeID prerequisite = ability_stats.tech_requirement;

			if (unitToCreate != UNIT_TYPEID::INVALID) {
				//TODO: PYLON CREATION WHEN SUPPLY TOO LOW
			}

			Aux::Cost abilityCost = Aux::abilityToCost(currentAction->ability, agent);
			int theoryMin = agent->Observation()->GetMinerals();
			int theoryVesp = agent->Observation()->GetVespene();

			UnitWrappers probes = UnitManager::getSelf(UNIT_TYPEID::PROTOSS_PROBE);
			for (auto it = probes.begin(); it != probes.end(); it++) {
				UnitWrapperPtr p = *it;
				ProbePtr probe = std::static_pointer_cast<Probe>(p);
				for (int b = 0; b < probe->buildings.size(); b++) {
					Aux::Cost g = probe->buildings[b].cost(agent);
					theoryMin -= g.minerals;
					theoryVesp -= g.vespene;
				}
			}

			if (currentAction->executor == UNIT_TYPEID::PROTOSS_PROBE && currentAction->ability != ABILITY_ID::MOVE_MOVE &&
				currentAction->ability != ABILITY_ID::MOVE_MOVEPATROL && currentAction->ability != ABILITY_ID::GENERAL_MOVE) {
				if (prerequisite != UNIT_TYPEID::INVALID && UnitManager::getSelf(prerequisite).size() == 0) {
					//TODO: CHECK PROBE BUILDINGS FOR PREREQ CHECKS
					diagnostics += strprintf("PREREQUISITE REQUIRED: %s\n\n", UnitTypeToName(prerequisite));
					continue;
				}

				if (currentAction->executorPtr == nullptr) {
					currentAction->executorPtr = *possibleUnits.begin();
				}
				UnitWrapperPtr newProbe = UnitManager::getRandomSelf(UNIT_TYPEID::PROTOSS_PROBE);

				float distToTravel = currentAction->executorPtr->getPathLength(agent, currentAction->position.pos);
				float newDist = newProbe->getPathLength(agent, currentAction->position.pos);

				if (distToTravel == 0) {
					distToTravel = Distance2D(currentAction->executorPtr->pos(agent), currentAction->position.pos);
				}
				if (newDist == 0) {
					newDist = Distance2D(newProbe->pos(agent), currentAction->position.pos);
				}

				DebugSphere(agent, AP3D(currentAction->position.pos), 2);

				UnitTypeData probeStats = Aux::getStats(UNIT_TYPEID::PROTOSS_PROBE, agent);

				if (newDist <= distToTravel) {
					currentAction->executorPtr = newProbe;
					distToTravel = newDist;
				}

				float dtTravel = (distToTravel - 2) / (probeStats.movement_speed * timeSpeed);

				if (prerequisite != UNIT_TYPEID::INVALID) {
					UnitTypeData prereqStats = Aux::getStats(prerequisite, agent);

					UnitWrappers allPrereqs = UnitManager::getSelf(prerequisite);

					float ticksToPrereq = -1;
					for (auto it = allPrereqs.begin(); it != allPrereqs.end(); it++) {
						const Unit* prereq = (*it)->get(agent);
						if (prereq != nullptr) {
							float ticks = (1.0 - prereq->build_progress) * prereqStats.build_time;
							if (ticksToPrereq == -1 || ticksToPrereq > ticks) {
								ticksToPrereq = ticks;
							}
						}
					}
					if (ticksToPrereq != -1 && readyInXFrames < ticksToPrereq) {
						readyInXFrames = ticksToPrereq;
					}
				}

				if (currentAction->ability != ABILITY_ID::BUILD_NEXUS &&
					currentAction->ability != ABILITY_ID::BUILD_PYLON &&
					currentAction->ability != ABILITY_ID::BUILD_ASSIMILATOR) {

					if (UnitManager::getSelf(UNIT_TYPEID::PROTOSS_PYLON).size() == 0) {
						diagnostics += "NO PYLONS EXIST\n\n";
						continue;
					}
					UnitWrappers pylons = UnitManager::getSelf(UNIT_TYPEID::PROTOSS_PYLON);
					float ticksToPrereq = -1;

					float pylonBuildTime = Aux::getStats(UNIT_TYPEID::PROTOSS_PYLON, agent).build_time;

					for (auto it = pylons.begin(); it != pylons.end(); it++) {
						const Unit* prereq = agent->Observation()->GetUnit((*it)->self);
						if (prereq != nullptr && DistanceSquared2D(prereq->pos, currentAction->position.pos) < PYLON_RADIUS_SQUARED) {
							float ticks = (1.0 - prereq->build_progress) * pylonBuildTime;
							if (ticksToPrereq == -1 || ticksToPrereq > ticks) {
								ticksToPrereq = ticks;
							}
						}
					}
					if (ticksToPrereq != -1 && readyInXFrames < ticksToPrereq) {
						readyInXFrames = ticksToPrereq;
					}
				}

				int numMineralMiners = 0, numVespeneMiners = 0;
				for (auto it = probes.begin(); it != probes.end(); it++) {
					UnitWrapperPtr target = std::static_pointer_cast<Probe>(*it)->getTargetTag(agent);
					if (target == nullptr)
						continue;
					if (target->getStorageType() == UNIT_TYPEID::NEUTRAL_MINERALFIELD) {
						numMineralMiners++;
					}
					else if (target->getStorageType() == UNIT_TYPEID::PROTOSS_ASSIMILATOR) {
						numVespeneMiners++;
					}
				}

				float dtPrerequisites = readyInXFrames / fps;
				if (dtPrerequisites > dtTravel) {
					diagnostics += "WAITING ON PREREQUISITES TO BE READY\n\n";
					break;
				}
				float dt = dtTravel;

				theoryMin += (int)(dt * MINERALS_PER_PROBE_PER_SEC * numMineralMiners);
				theoryVesp += (int)(dt * VESPENE_PER_PROBE_PER_SEC * numVespeneMiners);
			}
			else {
				if (prerequisite != UNIT_TYPEID::INVALID) {
					if (UnitManager::getSelf(prerequisite).size() == 0) {
						diagnostics += strprintf("PREREQUISITE REQUIRED: %s\n\n", UnitTypeToName(prerequisite));
						continue;
					}	

					UnitWrappers allPrereqs = UnitManager::getSelf(prerequisite);

					bool foundPrereq = false;
					for (auto it = allPrereqs.begin(); it != allPrereqs.end(); it++) {
						const Unit* prereq = (*it)->get(agent);
						if (prereq != nullptr && prereq->build_progress == 1.0) {
							foundPrereq = true;
							break;
						}
					}
					if (!foundPrereq) {
						diagnostics += strprintf("INCOMPLETE PREREQUISITE: %s\n\n", UnitTypeToName(prerequisite));
						continue;
					}
				}
				currentAction->executorPtr = *possibleUnits.begin();
			}

			if (currentAction->executorPtr == nullptr) {
				diagnostics += "NO EXECUTOR\n\n";
				continue;
			}

			if (theoryMin >= int(abilityCost.minerals) && theoryVesp >= int(abilityCost.vespene)) {
				const Unit* unit = currentAction->executorPtr->get(agent);
				AvailableAbilities unitAbilities = agent->Query()->GetAbilitiesForUnit(unit);
				bool hasAbility = false;
				for (int a = 0; a < unitAbilities.abilities.size(); a++) {
					if (unitAbilities.abilities[a].ability_id == currentAction->ability) {
						hasAbility = true;
						break;
					}
				}

				if (!hasAbility) {
					diagnostics += "UNIT DOES NOT HAVE REQUIRED ABILITY\n\n";
					continue;
				}

				if (currentAction->position.pos != Point2D{ 0, 0 }) {
					if (currentAction->position.pos == Point2D{ -1, -1 }) {
						diagnostics += "POS NOT DEFINED EARLIER\n\n";
						continue;
					}
					else {
						if (currentAction->executor == UNIT_TYPEID::PROTOSS_PROBE) {
							std::static_pointer_cast<Probe>(currentAction->executorPtr)->addBuilding(*currentAction);
							Aux::loadUnitPlacement(Aux::BUILDING_RESERVE, currentAction->position.pos, unitToCreate);
						}
						else {
							agent->Actions()->UnitCommand(currentAction->executorPtr->self, currentAction->ability, currentAction->position.pos);
						}
					}
				}
				else {
					agent->Actions()->UnitCommand(currentAction->executorPtr->self, currentAction->ability);
				}
				//if (topAct.chronoBoost) {
				//	Nexus::addChrono(UnitManager::find(getSuperType(actionUnit->unit_type), actionUnit->tag));
				//}
				//if (topAct.unit_type == UNIT_TYPEID::PROTOSS_WARPGATE) {
				//	actions[UNIT_TYPEID::PROTOSS_GATEWAY].erase(actions[UNIT_TYPEID::PROTOSS_GATEWAY].begin());
				//}
				//else {
				//	actions[topAct.unit_type].erase(actions[topAct.unit_type].begin());
				//}
				allActions[currentAction->executor].erase(allActions[currentAction->executor].begin());
				diagnostics += "SUCCESS\n\n";
				break;
			}
			else {
				diagnostics += "NOT ENOUGH RESOURCES\n\n";
				continue;
			}

			diagnostics += "SEMI SUCCESS\n\n";
		}
		
		DebugText(agent, diagnostics, Point2D{ 0.01,0.1 }, {150, 100, 200});
	}

	void displayMacroActions(Agent* const agent) {
		std::string tot = "MACRO:\n";
		for (auto it = allActions.begin(); it != allActions.end(); it++) {
			auto all = it->second;
			std::string type = UnitTypeToName(it->first);
			tot += ("\n" + type + ":\n");
			for (auto it2 = all.begin(); it2 != all.end(); it2++) {
				tot += strprintf("%s %d %.1f,%.1f", AbilityTypeToName(it2->ability), it2->index, it2->position.pos.x, it2->position.pos.y);
				if (it2->chronoBoost) {
					tot += " CHRONO";
				}
				tot += "\n";
			}
		}
		DebugText(agent, tot, Point2D(0.81F, 0.11F), Color(250, 50, 15), 8);
	}
};