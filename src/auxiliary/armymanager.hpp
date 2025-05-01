#pragma once
#include "squadmanager.hpp"

namespace ArmyManager {
	SquadManager::Squad mainAttackSquad;
    bool commited = false;

	void execute(Agent* const agent, StrategyManager::Strategy& strategem) {
        UnitWrappers buildings;
        for (auto it = UnitManager::enemy_units.begin(); it != UnitManager::enemy_units.end(); it++) {
            for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
                UnitWrapperPtr wrap = *it2;
                if (wrap->isBuilding()) {
                    buildings.insert(wrap);
                }//add break optimization if one is building all are building
            }
        }

        if (buildings.size() > 0) {
            if (mainAttackSquad.squadSize(agent) >= strategem.armyAttackNum || (strategem.commit && commited)) {
                commited = true;
                if (mainAttackSquad.getCore(agent) != nullptr) {
                    float mindist = 512;
                    UnitWrapperPtr min = nullptr;
                    for (UnitWrapperPtr wrap : buildings) {
                        if (wrap->pos(agent) == Point2D{ 0,0 }) {
                            continue;
                        }
                        float dist = UnitWrapper::getPathLengthGround(agent, mainAttackSquad.getCorePosition(agent), wrap->pos(agent));
                        if (dist < mindist) {
                            min = wrap;
                            mindist = dist;
                        }
                    }
                    if (min != nullptr) {
                        mainAttackSquad.doAttack(min->pos(agent));
                    }
                }
            }
            else {
                mainAttackSquad.doDefend(Aux::criticalPoints[Aux::CrucialPoints::SELF_RALLY_POINT]);
            }
        }
        else {
            mainAttackSquad.doSearch();
        }
        mainAttackSquad.execute(agent);
	}
}