#pragma once

#include <sc2api/sc2_api.h>
#include <iostream>
#include <unordered_set>
#include <queue> 

#include "../unitwrappers/unitwrapper.hpp"
#include "helpers.hpp"
#include "profiler.hpp"

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
};

struct MacroBuilding : MacroAction {
	MacroBuilding(AbilityID ability_, Aux::PointArea pos_ = Aux::PointArea(), MacroActionData extraData_ = MacroActionData(), int dependency_ = -1, int index_ = -1) : MacroAction(UNIT_TYPEID::PROTOSS_PROBE, ability_, pos_, false, extraData_, dependency_, index_){
		
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
	class MacroActionCompare {
	public:
		bool operator()(const MacroAction a, const MacroAction b)
		{
			return a.index > b.index;
		}
	};
	class MacroActionPtrCompare {
	public:
		bool operator()(const MacroAction* a, const MacroAction* b) const
		{
			return a->index > b->index;
		}
	};

	std::map<UnitTypeID, std::priority_queue<MacroAction, std::vector<MacroAction>, MacroActionCompare>> allActions;
	
	int lastPylonTriggered_frames = 0;
	int macroExecuteCooldown_frames = 0;
	std::string diagnostics = "";

	std::unordered_set<int> finishedActions;

	inline bool isActionCompleted(int action) {
		return finishedActions.count(action);
	}

	void addAction(MacroAction action) {
		allActions[action.executor].push(action);
	}

	void execute(Agent* const agent) {
		Profiler macroProfiler("mac.exec");
		if (macroExecuteCooldown_frames > 0) {
			macroExecuteCooldown_frames--;
			return;
		}
		
		//std::multiset<const MacroAction*, std::vector<const MacroAction*>, MacroActionPtrCompare> topActions;
		std::multiset<const MacroAction*, MacroActionPtrCompare> topActions;
			
		for (auto it = allActions.begin(); it != allActions.end(); it++) {
			if (it->second.size() == 0) {
				continue;
			}
			topActions.insert(&(it->second.top()));
		}

		std::string diagnostics = "";

		for (auto it = topActions.begin(); it != topActions.end(); it++) {
			const MacroAction* currentAction = *it;
			
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
				if ((*it)->get(agent) != nullptr && (*it)->get(agent)->orders.size() == 0) {
					possibleUnits.insert(*it);
				}
			}

			//Check if there are any units post-filtering
			if (possibleUnits.size() == 0) {
				diagnostics += "NO FREE EXECUTOR\n\n";
				continue;
			}

			if (currentAction->position.pa_type == Aux::PointArea::DEFAULT_FINDOUT) {
				if (currentAction->executor == UNIT_TYPEID::PROTOSS_PROBE) {
					if (currentAction->ability == ABILITY_ID::BUILD_PYLON) {

					}
				}
			}

			diagnostics += "SEMI SUCCESS\n\n";
		}

		DebugText(agent, diagnostics, Point2D{ 0.01,0.1 }, {150, 100, 200});
	}
};