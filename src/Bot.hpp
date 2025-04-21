// The MIT License (MIT)
//
// Copyright (c) 2021-2024 Alexander Kurbatov

#pragma once

#include <sc2api/sc2_agent.h>
#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>
#include <iostream>

#include "auxiliary/helpers.hpp"
#include "auxiliary/profiler.hpp"
#include "auxiliary/debugging.hpp"
#include "unitwrappers/unitmanager.hpp"
#include "auxiliary/macromanager.hpp"
#include "unitwrappers/nexus.hpp"
#include "unitwrappers/vespene.hpp"

#define DEBUG

namespace UnitManager {
    void encode(UnitWrapperPtr wrap, const Unit* unit_) {
        if (unit_->orders.size() != 0 && unit_->orders[0].target_pos != Point2D()) {
            Aux::encoding2D point(unit_->orders[0].target_pos);
            if (MacroManager::dataEncoding.find(point) != MacroManager::dataEncoding.end()) {
                wrap->creationData = MacroManager::dataEncoding.at(point);
                MacroManager::dataEncoding.erase(point);
            }
            else {
                printf("missed production\n");
            }
        }
        else {
            Aux::encoding2D point(unit_->pos);
            bool encoded = false;
            for (auto it = MacroManager::dataEncoding.begin(); it != MacroManager::dataEncoding.end(); it++) {
                if (DistanceSquared2D((*it).first, point) < 2) {
                    wrap->creationData = MacroManager::dataEncoding.at((*it).first);
                    MacroManager::dataEncoding.erase((*it).first);
                    encoded = true;
                    break;
                }
            }
            if (!encoded) {
                printf("missed production 2\n");
            }
        }
        

    }

    void add(UnitWrapperMap& units, const Unit* unit_, Agent* const agent) {
        UnitTypeID stype = getSuperType(unit_->unit_type);
        if (unit_->alliance == Unit::Self) {
            if (stype == UNIT_TYPEID::PROTOSS_PROBE) {
                ProbePtr probe = std::make_shared<Probe>(unit_);
                encode(probe, unit_);
                units[stype].insert(probe);
            }else if (stype == UNIT_TYPEID::PROTOSS_NEXUS) {
                NexusPtr nexus = std::make_shared<Nexus>(unit_);
                nexus->init(agent);
                encode(nexus, unit_);
                units[stype].insert(nexus);
            }
            else if (stype == UNIT_TYPEID::PROTOSS_ASSIMILATOR) {
                UnitWrapperPtr assimilator = std::make_shared<UnitWrapper>(unit_, stype);
                UnitWrappers nexuses = UnitManager::getSelf(UNIT_TYPEID::PROTOSS_NEXUS);
                for (UnitWrapperPtr nexus : nexuses) {
                    if (DistanceSquared2D(assimilator->pos(agent), nexus->pos(agent)) < 100) {
                        std::static_pointer_cast<Nexus>(nexus)->addAssimilator(assimilator);
                        break;
                    }
                }
                encode(assimilator, unit_);
                units[stype].insert(assimilator);
            }
            else {
                UnitWrapperPtr selfUnit = std::make_shared<UnitWrapper>(unit_, stype);
                encode(selfUnit, unit_);
                units[stype].insert(selfUnit);
            }
            agent->Actions()->UnitCommand(unit_->tag, ABILITY_ID::GENERAL_MOVE, unit_->pos);
            return;
        }
        else if (unit_->alliance == Unit::Neutral) {
            if (stype == UNIT_TYPEID::NEUTRAL_VESPENEGEYSER) {
                units[stype].insert(std::make_shared<Vespene>(unit_, stype));
            }
            else if (stype == UNIT_TYPEID::NEUTRAL_MINERALFIELD) {
                UnitWrapperPtr mineral = std::make_shared<UnitWrapper>(unit_, stype);
                UnitWrappers nexuses = UnitManager::getSelf(UNIT_TYPEID::PROTOSS_NEXUS);
                for (UnitWrapperPtr nexus : nexuses) {
                    if (DistanceSquared2D(mineral->pos(agent), nexus->pos(agent)) < 100) {
                        std::static_pointer_cast<Nexus>(nexus)->addMineral(mineral);
                        break;
                    }
                }
                units[stype].insert(mineral);
            }
            return;
        }
        units[stype].insert(std::make_shared<UnitWrapper>(unit_, stype));
    }

    void remove(const Unit* unit_) {
        UnitTypeID stype = getSuperType(unit_->unit_type);
        UnitWrapperMap* units;
        if (unit_->alliance == Unit::Alliance::Self) {
            units = &self_units;
        }
        else if (unit_->alliance == Unit::Alliance::Neutral) {
            units = &neutral_units;
        }
        else if (unit_->alliance == Unit::Alliance::Enemy) {
            units = &enemy_units;
        }
        else {
            printf("NO TEAM, YOU FUCKED UP\n");
            throw 5;
        }
        bool removed = false;
        for (auto it = (*units)[stype].begin(); it != (*units)[stype].end(); it++) {
            if ((*it)->self == unit_->tag) {
                (*it)->setDead();
                (*units)[stype].erase(it);
                removed = true;
                break;
            }
        }
        if (!removed) {
            printf("NOT REMOVED, YOU FUCKED UP super(%s) = %s\n", UnitTypeToName(unit_->unit_type), UnitTypeToName(stype));
            //throw 5;
        }
    }
}

// The main bot class.
struct Bot: sc2::Agent
{
    Bot() = default;
    long long lastDT = 0;

 private:
    //! Called when a game is started or restarted.
    void OnGameStart(){
        Aux::gameInfo_cache = Observation()->GetGameInfo();
        Aux::mapWidth_cache = Aux::gameInfo_cache.width;
        Aux::mapHeight_cache = Aux::gameInfo_cache.height;

        Aux::setupExpansions(this);

        Aux::masterMap = std::make_shared<map2d<uint8_t>>(Aux::mapWidth_cache, Aux::mapHeight_cache, true);
        Aux::setupMasterMap(this);

        Aux::allData(this);

        std::cout << "New game started!" << std::endl;
        //MacroManager::addAction(MacroBuilding(ABILITY_ID::BUILD_PYLON, Aux::criticalPoints[Aux::SELF_FIRSTPYLON_POINT]));
        //MacroManager::addAction(MacroBuilding(ABILITY_ID::BUILD_GATEWAY, Aux::PointDefault()));
        //MacroManager::addAction(MacroBuilding(ABILITY_ID::BUILD_ASSIMILATOR, Aux::PointDefault()));
        //MacroManager::addAction(MacroBuilding(ABILITY_ID::BUILD_CYBERNETICSCORE, Aux::PointDefault()));
        //MacroManager::addAction(MacroGateway(ABILITY_ID::TRAIN_ADEPT));
        std::vector<MacroAction> build_order = {
            MacroAction(UNIT_TYPEID::PROTOSS_NEXUS, ABILITY_ID::TRAIN_PROBE, true),
            MacroBuilding(ABILITY_ID::BUILD_PYLON, Aux::criticalPoints[Aux::SELF_FIRSTPYLON_POINT]),
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY),
            MacroBuilding(ABILITY_ID::GENERAL_MOVE, Aux::criticalPoints[Aux::ENEMY_STARTLOC_POINT]),
            MacroBuilding(ABILITY_ID::BUILD_ASSIMILATOR),
            MacroBuilding(ABILITY_ID::BUILD_CYBERNETICSCORE),
            MacroBuilding(ABILITY_ID::BUILD_NEXUS),
            MacroGateway(ABILITY_ID::TRAIN_STALKER),
            MacroAction(UNIT_TYPEID::PROTOSS_CYBERNETICSCORE, ABILITY_ID::RESEARCH_WARPGATE),
            MacroBuilding(ABILITY_ID::BUILD_ASSIMILATOR),
            MacroBuilding(ABILITY_ID::BUILD_PYLON),
            MacroGateway(ABILITY_ID::TRAIN_STALKER),
            MacroGateway(ABILITY_ID::TRAIN_STALKER),
            MacroBuilding(ABILITY_ID::BUILD_ROBOTICSFACILITY),
            MacroRobo(ABILITY_ID::TRAIN_OBSERVER),
            MacroRobo(ABILITY_ID::TRAIN_IMMORTAL),
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY),
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY),
            MacroBuilding(ABILITY_ID::BUILD_ROBOTICSBAY),
            MacroGateway(ABILITY_ID::TRAIN_STALKER),
            MacroGateway(ABILITY_ID::TRAIN_STALKER),
            MacroGateway(ABILITY_ID::TRAIN_STALKER),
            MacroAction(UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL, ABILITY_ID::RESEARCH_BLINK),
            MacroBuilding(ABILITY_ID::BUILD_ASSIMILATOR),
            MacroBuilding(ABILITY_ID::BUILD_TWILIGHTCOUNCIL),
            MacroBuilding(ABILITY_ID::BUILD_ASSIMILATOR),
            MacroGateway(ABILITY_ID::TRAIN_COLOSSUS),
            MacroAction(UNIT_TYPEID::PROTOSS_ROBOTICSBAY, ABILITY_ID::RESEARCH_EXTENDEDTHERMALLANCE),
            MacroGateway(ABILITY_ID::TRAIN_COLOSSUS),
            MacroGateway(ABILITY_ID::TRAIN_IMMORTAL),
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY),
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY),
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY),
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY),
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY),
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY),
            MacroBuilding(ABILITY_ID::BUILD_PYLON),
            MacroBuilding(ABILITY_ID::BUILD_PYLON),
            MacroBuilding(ABILITY_ID::BUILD_PYLON),
            MacroBuilding(ABILITY_ID::BUILD_PYLON),
            MacroBuilding(ABILITY_ID::BUILD_PYLON),
            MacroRobo(ABILITY_ID::TRAIN_WARPPRISM),
            MacroBuilding(ABILITY_ID::BUILD_NEXUS),
        };

        for (int i = 0; i < build_order.size(); i++) {
            MacroManager::addAction(build_order[i]);
        }
    }

    //! Called when a game has ended.
    void OnGameEnd(){
        std::cout << "Game over!" << std::endl;
        UnitManager::self_units.clear();
        UnitManager::neutral_units.clear();
        UnitManager::enemy_units.clear();
    }

    //! In non realtime games this function gets called after each step as indicated by step size.
    //! In realtime this function gets called as often as possible after request/responses are received from the game gathering observation state.
    void OnStep(){
        Profiler onStepProfiler("onStep");

        UnitWrappers probes = UnitManager::getSelf(UNIT_TYPEID::PROTOSS_PROBE);
        for (auto it = probes.begin(); it != probes.end(); it++) {
            Profiler onStepProfiler("pE");
            UnitWrapperPtr p = *it;
            ProbePtr probe = std::static_pointer_cast<Probe>(p);
            probe->execute(this);
#ifdef DEBUG
            UnitWrapperPtr target = probe->getTargetTag(this);
            if (target != nullptr) {
                DebugLine(this, target->pos3D(this) + Point3D{ 0,0,1 }, probe->pos3D(this) + Point3D{ 0,0,1 });
            }
            if (probe->buildings.size() != 0) {
                DebugText(this, strprintf("%s", AbilityTypeToName(probe->buildings[0].build)), probe->pos3D(this) + Point3D{ 0,0,1 });
            }
#endif
        }

        onStepProfiler.midLog("oS-probeExec");

        int numProbes = 0;
        int numProbesMax = 0;
        for (UnitWrapperPtr nexusWrap : UnitManager::getSelf(UNIT_TYPEID::PROTOSS_NEXUS)) {
            NexusPtr nexus = std::static_pointer_cast<Nexus>(nexusWrap);

            int numProbesN = 0;
            int numProbesMaxN = 0;
            float percentUntilViable = 1.0F - (Aux::getStats(UNIT_TYPEID::PROTOSS_PROBE, this).build_time / Aux::getStats(UNIT_TYPEID::PROTOSS_NEXUS, this).build_time);
            if (nexus->get(this)->build_progress < percentUntilViable) {
                continue;
            }
            if (nexus->assimilator1 != NullTag) {
                numProbesMaxN += 3;
                numProbesN += probeTargetting[nexus->assimilator1->self];
            }
            if (nexus->assimilator2 != NullTag) {
                numProbesMaxN += 3;
                numProbesN += probeTargetting[nexus->assimilator2->self];
            }
            for (int i = 0; i < 8; i++) {
                if (nexus->minerals[i] != nullptr) {
                    if (nexus->minerals[i]->get(this) != nullptr) {
                        numProbesMaxN += 2;
                        numProbesN += probeTargetting[nexus->minerals[i]->self];
                        Point3D po = nexus->minerals[i]->pos3D(this);
                        //Color mineralCapacity
                        DebugBox(this, po + Point3D{ -0.125, -0.125, 3 }, po + Point3D{ 0.125, 0.125, 3.125 }, Colors::Blue);
                        if (probeTargetting[nexus->minerals[i]->self] > 0) {
                            DebugBox(this, po + Point3D{ -0.125, -0.125, 3.125 }, po + Point3D{ 0.125, 0.125, 3.375 }, Colors::Teal);
                        }
                        if (probeTargetting[nexus->minerals[i]->self] > 1) {
                            DebugBox(this, po + Point3D{ -0.125, -0.125, 3.375 }, po + Point3D{ 0.125, 0.125, 3.5 }, Colors::Yellow);
                        }
                        if (probeTargetting[nexus->minerals[i]->self] > 2) {
                            DebugBox(this, po + Point3D{ -0.13, -0.13, 3 }, po + Point3D{ 0.13, 0.13, 3.5 }, Colors::Red);
                        }
                    }
                }
            }
            DebugText(this, strprintf("%d/%d", numProbesN, numProbesMaxN), nexusWrap->pos3D(this) + Point3D{ 0,0, 5.5 });
            numProbes += numProbesN;
            numProbesMax += numProbesMaxN;
        }

        if ((numProbes) < numProbesMax && MacroManager::allActions[UNIT_TYPEID::PROTOSS_NEXUS].size() == 0) {
            MacroManager::addAction(MacroAction(UNIT_TYPEID::PROTOSS_NEXUS, ABILITY_ID::TRAIN_PROBE, false, MacroActionData(), -1, 0));
        }

        onStepProfiler.midLog("oS-probe");

        MacroManager::execute(this);

        onStepProfiler.midLog("oS-macroExec");

#ifndef BUILD_FOR_LADDER
        std::vector<ChatMessage> chats = Observation()->GetChatMessages();
        for (int i = 0; i < chats.size(); i++) {
            printf("[%s]\n", chats[i].message.c_str());
            if (chats[i].message[0] == '.') {
                std::string command = "";
                int commandPtr = 0;
                std::vector<std::string> arguments;
                for (commandPtr = 1; commandPtr < 50; commandPtr++) {
                    char c = chats[i].message[commandPtr];
                    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
                        command += c;
                    }
                    else {
                        commandPtr++;
                        break;
                    }
                }
                while (commandPtr < chats[i].message.size()) {
                    std::string arg = "";
                    for (commandPtr; commandPtr < chats[i].message.size(); commandPtr++) {
                        char c = chats[i].message[commandPtr];
                        if (c != ' ') {
                            arg += c;
                        }
                        if (c == ' ' || commandPtr == chats[i].message.size() - 1) {
                            arguments.push_back(arg);
                            commandPtr++;
                            break;
                        }
                    }
                }

                Actions()->SendChat("Command: " + command);
                for (std::string argument : arguments) {
                    Actions()->SendChat(argument);
                }

                if (command == "s") { //spawn
                    if (spawnCommandMap.find(arguments[0]) != spawnCommandMap.end()) {
                        DebugCreateUnit(this, spawnCommandMap[arguments[0]], Observation()->GetCameraPos(), 1);
                    }
                }
                else if (command == "se") { //spawn enemy
                    if (spawnCommandMap.find(arguments[0]) != spawnCommandMap.end()) {
                        DebugCreateUnit(this, spawnCommandMap[arguments[0]], Observation()->GetCameraPos(), 2);
                    }
                }
                else if (command == "v") { //spawn enemy
                    Debug()->DebugShowMap();
                }
                else if (command == "kn") { //spawn enemy
                    Units units = Observation()->GetUnits(sc2::Unit::Alliance::Neutral);
                    for (const Unit* unit : units) {
                        Debug()->DebugKillUnit(unit);
                    }
                }
                else if (command == "m") { //spawn enemy
                    Aux::saveMasterBitmap("command.bmp");
                }
                else {
                    Actions()->SendChat("Invalid Command " + command);
                }
            }
        }
#endif

        onStepProfiler.midLog("oS-CLI");
        
        for (auto typeIt = UnitManager::self_units.begin(); typeIt != UnitManager::self_units.end(); typeIt++) {
            for (auto it = typeIt->second.begin(); it != typeIt->second.end(); it++) {
                DebugText(this, strprintf("%s", (*it)->creationData.name.c_str()), (*it)->pos3D(this) + Point3D{ 0,0,1 }, Colors::Purple);
            }
        }

        onStepProfiler.midLog("oS-Build");

        DebugText(this, strprintf("%.3fms", lastDT / 1000.0));

        Aux::displayExpansions(this);

        MacroManager::displayMacroActions(this);

        onStepProfiler.midLog("oS-Debug");

        std::string profilestr = "";
        static int max1 = 0;
        static int max2 = 0;
        static int max3 = 0;
        static int max4 = 0;
        static int max5 = 0;
        for (auto itr = profilerMap.begin(); itr != profilerMap.end(); itr++) {
            std::string name = itr->first;
            int strlen = (int)(name.size());
            max1 = std::max(strlen + 1, max1);
            for (int i = 0; i < (max1 - strlen); i++) {
                name += " ";
            }
            std::string dtstr = strprintf("AVG:%.3f", ((double)itr->second) / profilerCount[itr->first] / 1000.0);
            strlen = (int)(dtstr.size());
            max2 = std::max(strlen + 1, max2);
            for (int i = 0; i < (max2 - strlen); i++) {
                dtstr += " ";
            }
            std::string totstr = strprintf("TOT:%.3f/%d", itr->second / 1000.0, profilerCount[itr->first]);
            strlen = (int)(totstr.size());
            max3 = std::max(strlen + 1, max3);
            for (int i = 0; i < (max3 - strlen); i++) {
                totstr += " ";
            }
            std::string lateststr = strprintf("LAT:%.3f", profilerLast[itr->first].time() / 1000.0);
            strlen = (int)(lateststr.size());
            max4 = std::max(strlen + 1, max4);
            for (int i = 0; i < (max4 - strlen); i++) {
                lateststr += " ";
            }
            std::string maxstr = strprintf("MAX:%.3f", profilerMax[itr->first] / 1000.0);
            strlen = (int)(maxstr.size());
            max5 = std::max(strlen + 1, max5);
            for (int i = 0; i < (max4 - strlen); i++) {
                maxstr += " ";
            }
            profilestr += (name + lateststr + dtstr + maxstr + "\n");
        }
        DebugText(this, profilestr, Point2D(0.61F, 0.41F), Color(1, 212, 41), 8);

        onStepProfiler.midLog("oS-logging");

        SendDebug(this);

        onStepProfiler.midLog("oS-SendDebug");

        lastDT = onStepProfiler.getFullDT();
    }

    //! Called when a Unit has been created by the player.
    //!< \param unit The created unit.
    void OnUnitCreated(const sc2::Unit* unit_){
        printf("%s (%Ix) was created ", UnitTypeToName(unit_->unit_type), unit_->tag);
        //UnitWrapper u(unit_);
        //UnitManager::self_units.insert(std::make_shared<UnitWrapper>(unit_));
        if (unit_->tag != 0) {
            UnitManager::add(UnitManager::self_units, unit_, this);
            Aux::loadUnitPlacement(Aux::SELF_BUILDINGS, unit_->pos, unit_->unit_type);
        }

        for(int i = 0; i < unit_->orders.size(); i ++) {
            printf("[%s %.1f,%.1f, %Ix]", AbilityTypeToName(unit_->orders[i].ability_id), unit_->orders[i].target_pos.x, unit_->orders[i].target_pos.y, unit_->orders[i].target_unit_tag);
        }
        printf("\n");
    }

    //! Called when an enemy unit enters vision from out of fog of war.
    //!< \param unit The unit entering vision.
    virtual void OnUnitEnterVision(const sc2::Unit* unit_) {
        std::cout << sc2::UnitTypeToName(unit_->unit_type) <<
            "(" << unit_->tag << ") was created E" << std::endl;

        UnitManager::add(UnitManager::enemy_units, unit_, this);
        Aux::loadUnitPlacement(Aux::ENEMY_BUILDINGS, unit_->pos, unit_->unit_type); //TODO: ENEMY UNITS WHEN get() is called WILL CHECK IF THEIR POSITION IS DIFFERENT FROM THEIR OLD POSITION AND IF SO, REWORK THE PATHING GRID
    }

    //!  Called when a neutral unit is created. For example, mineral fields observed for the first time
    //!< \param unit The observed unit.
    virtual void OnNeutralUnitCreated(const sc2::Unit* unit_) {
        std::cout << sc2::UnitTypeToName(unit_->unit_type) <<
            "(" << unit_->tag << ") was created N" << std::endl;

        UnitManager::add(UnitManager::neutral_units, unit_, this);
    }

    //! Called whenever one of the player's units has been destroyed.
    //!< \param unit The destroyed unit.
    void OnUnitDestroyed(const sc2::Unit* unit_){
        std::cout << sc2::UnitTypeToName(unit_->unit_type) <<
             "(" << unit_->tag << ") was destroyed" << std::endl;
        UnitManager::remove(unit_);
        if (unit_->alliance == Unit::Neutral) {
            Aux::unloadNeutralUnitPlacement(this, unit_->pos, unit_->unit_type);
        }
    }

    //! Called when an upgrade is finished, warp gate, ground weapons, baneling speed, etc.
    //!< \param upgrade The completed upgrade.
    void OnUpgradeCompleted(sc2::UpgradeID id_){
        std::cout << sc2::UpgradeIDToName(id_) << " completed" << std::endl;
    }

    //! Called when the unit in the current observation has lower health or shields than in the previous observation.
    //!< \param unit The damaged unit.
    //!< \param health The change in health (damage is positive)
    //!< \param shields The change in shields (damage is positive)
    virtual void OnUnitDamaged(const sc2::Unit* unit_, float health, float shields) {

    }

    //! Called when a nydus is placed.
    virtual void OnNydusDetected() {

    }

    //! Called when a nuclear launch is detected.
    virtual void OnNuclearLaunchDetected() {

    }

    //! Called for various errors the library can encounter. See ClientError enum for possible errors.
    void OnError(const std::vector<sc2::ClientError>& client_errors, const std::vector<std::string>& protocol_errors = {}){
        for (const auto i : client_errors) {
            std::cerr << "Encountered client error: " <<
                static_cast<int>(i) << std::endl;
        }

        for (const auto& i : protocol_errors)
            std::cerr << "Encountered protocol error: " << i << std::endl;
    }

    //! Called when a unit becomes idle, this will only occur as an event so will only be called when the unit becomes
    //! idle and not a second time. Being idle is defined by having orders in the previous step and not currently having
    //! orders or if it did not exist in the previous step and now does, a unit being created, for instance, will call both
    //! OnUnitCreated and OnUnitIdle if it does not have a rally set.
    //!< \param unit The idle unit.
    void OnUnitIdle(const sc2::Unit* unit_){
        std::cout << sc2::UnitTypeToName(unit_->unit_type) <<
             "(" << unit_->tag << ") is idle" << std::endl;
    }

    //! Called when the unit in the previous step had a build progress less than 1.0 but is greater than or equal to 1.0 in
    // !the current step.
    //!< \param unit The constructed unit.
    void OnBuildingConstructionComplete(const sc2::Unit* building_){
        std::cout << sc2::UnitTypeToName(building_->unit_type) <<
            "(" << building_->tag << ") constructed" << std::endl;
    }
};
