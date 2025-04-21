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

	mutable int readyInFrames;

	MacroAction(UnitTypeID unit_type_, AbilityID ability_, Aux::PointArea pos_, bool chronoBoost_ = false, MacroActionData extraData_ = MacroActionData(), int dependency_ = -1, int index_ = -1)
		: executor( unit_type_), ability(ability_), position(pos_), 
		chronoBoost(chronoBoost_), target(nullptr), index(globalIndex),
		dependency(dependency_), executorPtr(nullptr), readyInFrames(-1), extraData(extraData_) {
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
		dependency(dependency_), executorPtr(nullptr), readyInFrames(-1), extraData(extraData_) {
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
		dependency(dependency_), executorPtr(nullptr), readyInFrames(-1), extraData(extraData_) {
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
	std::unordered_map<Aux::encoding2D, MacroActionData, Aux::encoding2DHash> dataEncoding;
	
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

		UnitTypeData pylon_stats = Aux::getStats(UNIT_TYPEID::PROTOSS_PYLON, agent);
		int psi = agent->Observation()->GetFoodUsed();
		int psiCap = agent->Observation()->GetFoodCap();

		std::string diagnostics = "";

		for (auto it = topActions.begin(); it != topActions.end(); it++) {
			const MacroAction* currentAction = *it;

			if (currentAction->executor == UNIT_TYPEID::PROTOSS_GATEWAY &&
				agent->Observation()->GetWarpGateCount() > 0 && currentAction->ability != ABILITY_ID::TRAIN_ARCHON) {
				bool swapped = false;
				if (currentAction->ability == ABILITY_ID::TRAIN_ZEALOT) {
					currentAction->ability = ABILITY_ID::TRAINWARP_ZEALOT;
					swapped = true;
				}
				else if (currentAction->ability == ABILITY_ID::TRAIN_STALKER) {
					currentAction->ability = ABILITY_ID::TRAINWARP_STALKER;
					swapped = true;
				}
				else if (currentAction->ability == ABILITY_ID::TRAIN_ADEPT) {
					currentAction->ability = ABILITY_ID::TRAINWARP_ADEPT;
					swapped = true;
				}
				else if (currentAction->ability == ABILITY_ID::TRAIN_SENTRY) {
					currentAction->ability = ABILITY_ID::TRAINWARP_SENTRY;
					swapped = true;
				}
				else if (currentAction->ability == ABILITY_ID::TRAIN_DARKTEMPLAR) {
					currentAction->ability = ABILITY_ID::TRAINWARP_DARKTEMPLAR;
					swapped = true;
				}
				else if (currentAction->ability == ABILITY_ID::TRAIN_HIGHTEMPLAR) {
					currentAction->ability = ABILITY_ID::TRAINWARP_HIGHTEMPLAR;
					swapped = true;
				}
				else {
					//printf("DELETED WARPGATE ACTION\n");
					//it->second.erase(it->second.begin());
					//continue;
				}

				if (swapped && currentAction->position.pa_type == Aux::PointArea::INVALID) {
					currentAction->position = Aux::PointDefault();
				}
			}

			Aux::Cost abilityCost = Aux::abilityToCost(currentAction->ability, agent);

			diagnostics += AbilityTypeToName((*it)->ability);
			diagnostics += ": ";

			int pylonInX = -1;

			//Check PSI
			if (abilityCost.psi + psi > psiCap && currentAction->readyInFrames != -1 && 
				currentAction->readyInFrames < pylon_stats.build_time && 
				(allActions[UNIT_TYPEID::PROTOSS_PROBE].size() != 0 && allActions[UNIT_TYPEID::PROTOSS_PROBE].begin()->ability != ABILITY_ID::BUILD_PYLON)) {

				UnitWrappers pylons = UnitManager::getSelf(UNIT_TYPEID::PROTOSS_PYLON);

				float pylonBuildTime = pylon_stats.build_time;

				for (auto it = pylons.begin(); it != pylons.end(); it++) {
					const Unit* prereq = agent->Observation()->GetUnit((*it)->self);
					if (prereq != nullptr) {
						float ticks = (1.0 - prereq->build_progress) * pylonBuildTime;
						if (ticks != 0 && (pylonInX == -1 || pylonInX > ticks)) {
							pylonInX = ticks;
						}
					}
				}

				bool pylonRequested = false;
				UnitWrappers probes = UnitManager::getSelf(UNIT_TYPEID::PROTOSS_PROBE);
				for (auto it = probes.begin(); it != probes.end(); it++) {
					UnitWrapperPtr p = *it;
					ProbePtr probe = std::static_pointer_cast<Probe>(p);
					for (int b = 0; b < probe->buildings.size(); b++) {
						if (probe->buildings[b].build == ABILITY_ID::BUILD_PYLON) {
							pylonRequested = true;
							break;
						}
					}
				}
				if (!pylonRequested && pylonInX == -1) {
					addAction(MacroBuilding(ABILITY_ID::BUILD_PYLON, Aux::PointDefault(), MacroActionData(), -1, 0));
					diagnostics += "PYLON REQUESTED\n\n";
					break;
				}

				if (pylonInX == -1) {
					
				}
			}
			if (pylonInX == -1) {
				currentAction->readyInFrames = 0;
			}
			else {
				currentAction->readyInFrames = pylonInX;
			}

			if (currentAction->readyInFrames != 0) {
				diagnostics += strprintf("PYLON IN TRANSIT [%d]\n\n", currentAction->readyInFrames);
				continue;
			}

			UnitWrappers allPossibleUnits = UnitManager::getSelf(currentAction->executor);

			//Check if there are any units of the relevant type
			if (allPossibleUnits.size() == 0) {
				diagnostics += "NO EXISTING EXECUTOR\n\n";
				continue;
			}

			UnitTypeID unitToCreate = Aux::buildAbilityToUnit(currentAction->ability);
			UnitTypeData ability_stats = Aux::getStats(unitToCreate, agent);

			//Filter units
			UnitWrappers possibleUnits;
			float ticksToExecutorReady = -1;
			for (auto it = allPossibleUnits.begin(); it != allPossibleUnits.end(); it++) {
				const Unit* unwrapUnit = (*it)->get(agent);
				if (unwrapUnit != nullptr && (unwrapUnit->orders.size() == 0 || currentAction->executor == UNIT_TYPEID::PROTOSS_PROBE) && unwrapUnit->build_progress == 1.0F) {
					possibleUnits.insert(*it);
					ticksToExecutorReady = 0;
				}
				else if (unwrapUnit != nullptr && unwrapUnit->orders.size() != 0) {
					float ready = (1.0F - unwrapUnit->orders[0].progress) * ability_stats.build_time;
					if (ticksToExecutorReady == -1 || ticksToExecutorReady > ready) {
						ticksToExecutorReady = ready;
					}
				}
				else if (unwrapUnit != nullptr && unwrapUnit->build_progress != 1.0F) {
					UnitTypeData executor_stats = Aux::getStats(unwrapUnit->unit_type, agent);
					float ready = (1.0F - unwrapUnit->build_progress) * executor_stats.build_time;
					if (ticksToExecutorReady == -1 || ticksToExecutorReady > ready) {
						ticksToExecutorReady = ready;
					}
				}
				else {
					//printf("what is this state\n");
				}
			}
			if (ticksToExecutorReady != -1 && currentAction->readyInFrames < ticksToExecutorReady) {
				currentAction->readyInFrames = ticksToExecutorReady;
			}

			//Check if there are any units post-filtering
			if (possibleUnits.size() == 0) {
				diagnostics += strprintf("NO READY EXECUTOR [%d]\n\n", currentAction->readyInFrames);
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

			UnitTypeID prerequisite = ability_stats.tech_requirement;

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
					if (ticksToPrereq != -1 && currentAction->readyInFrames < ticksToPrereq) {
						currentAction->readyInFrames = ticksToPrereq;
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

					float pylonBuildTime = pylon_stats.build_time;

					for (auto it = pylons.begin(); it != pylons.end(); it++) {
						const Unit* prereq = agent->Observation()->GetUnit((*it)->self);
						if (prereq != nullptr && DistanceSquared2D(prereq->pos, currentAction->position.pos) < PYLON_RADIUS_SQUARED) {
							float ticks = (1.0 - prereq->build_progress) * pylonBuildTime;
							if (ticksToPrereq == -1 || ticksToPrereq > ticks) {
								ticksToPrereq = ticks;
							}
						}
					}
					if (ticksToPrereq != -1 && currentAction->readyInFrames < ticksToPrereq) {
						currentAction->readyInFrames = ticksToPrereq;
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

				float dtPrerequisites = currentAction->readyInFrames / fps;
				if (dtPrerequisites > dtTravel) {
					diagnostics += strprintf("WAITING ON PREREQUISITES TO BE READY [%d]\n\n", currentAction->readyInFrames);
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

				if (currentAction->executor != UNIT_TYPEID::PROTOSS_PROBE) {
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
				}
				if (currentAction->ability == 880) {
					printf("ebxus\n");
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
							dataEncoding[currentAction->position.pos] = currentAction->extraData;
						}
						else {
							agent->Actions()->UnitCommand(currentAction->executorPtr->self, currentAction->ability, currentAction->position.pos);
							dataEncoding[currentAction->position.pos] = currentAction->extraData;
						}
					}
				}
				else {
					Aux::encoding2D encodingPoint;
					while (encodingPoint == Aux::encoding2D()) {
						Aux::encoding2D prospectivePoint = Aux::getRandomEncodingPoint();
						if (dataEncoding.find(prospectivePoint) == dataEncoding.end()) {
							encodingPoint = prospectivePoint;
							break;
						}
					}
					agent->Actions()->UnitCommand(currentAction->executorPtr->self, ABILITY_ID::RALLY_BUILDING, encodingPoint);
					agent->Actions()->UnitCommand(currentAction->executorPtr->self, currentAction->ability, true);
					dataEncoding[encodingPoint] = currentAction->extraData;
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
				break;
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