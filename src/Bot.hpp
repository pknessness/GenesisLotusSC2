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

        std::cout << "New game started!" << std::endl;
        MacroManager::addAction(MacroBuilding(ABILITY_ID::BUILD_PYLON, Aux::PointDefault()));
        MacroManager::addAction(MacroBuilding(ABILITY_ID::BUILD_GATEWAY, Aux::PointDefault()));
        MacroManager::addAction(MacroBuilding(ABILITY_ID::BUILD_ASSIMILATOR, Aux::PointDefault()));
        MacroManager::addAction(MacroGateway(ABILITY_ID::TRAIN_ADEPT));
        //std::vector<MacroAction> build_order = {
        //    MacroBuilding(ABILITY_ID::BUILD_PYLON, P2D(Aux::staging_location)),
        //    MacroBuilding(ABILITY_ID::BUILD_GATEWAY, {-1,-1}),
        //    MacroBuilding(ABILITY_ID::GENERAL_MOVE, Aux::enemyLoc),
        //    MacroBuilding(ABILITY_ID::BUILD_ASSIMILATOR, {-1,-1}),
        //    MacroBuilding(ABILITY_ID::BUILD_CYBERNETICSCORE, {-1, -1}),
        //    MacroBuilding(ABILITY_ID::BUILD_NEXUS, P2D(Aux::rankedExpansions[0])),
        //    MacroAction(UNIT_TYPEID::PROTOSS_GATEWAY, ABILITY_ID::TRAIN_STALKER, nullptr),
        //    MacroAction(UNIT_TYPEID::PROTOSS_CYBERNETICSCORE, ABILITY_ID::RESEARCH_WARPGATE, nullptr),
        //    MacroBuilding(ABILITY_ID::BUILD_ASSIMILATOR, { -1,-1 }),
        //    MacroBuilding(ABILITY_ID::BUILD_PYLON, {-1, -1}),
        //    MacroAction(UNIT_TYPEID::PROTOSS_GATEWAY, ABILITY_ID::TRAIN_STALKER, nullptr),
        //    MacroAction(UNIT_TYPEID::PROTOSS_GATEWAY, ABILITY_ID::TRAIN_STALKER, nullptr),
        //    MacroBuilding(ABILITY_ID::BUILD_ROBOTICSFACILITY, {-1, -1}),
        //    MacroAction(UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY, ABILITY_ID::TRAIN_OBSERVER, nullptr),
        //    MacroAction(UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY, ABILITY_ID::TRAIN_IMMORTAL, nullptr),
        //    MacroBuilding(ABILITY_ID::BUILD_GATEWAY, {-1, -1}),
        //    MacroBuilding(ABILITY_ID::BUILD_GATEWAY, {-1, -1}),
        //    MacroBuilding(ABILITY_ID::BUILD_ROBOTICSBAY, {-1, -1}),
        //    MacroAction(UNIT_TYPEID::PROTOSS_GATEWAY, ABILITY_ID::TRAIN_STALKER, nullptr),
        //    MacroAction(UNIT_TYPEID::PROTOSS_GATEWAY, ABILITY_ID::TRAIN_STALKER, nullptr),
        //    MacroAction(UNIT_TYPEID::PROTOSS_GATEWAY, ABILITY_ID::TRAIN_STALKER, nullptr),
        //    MacroAction(UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL, ABILITY_ID::RESEARCH_BLINK, nullptr),
        //    MacroBuilding(ABILITY_ID::BUILD_ASSIMILATOR, { -1,-1 }),
        //    MacroBuilding(ABILITY_ID::BUILD_TWILIGHTCOUNCIL, {-1, -1}),
        //    MacroBuilding(ABILITY_ID::BUILD_ASSIMILATOR, { -1,-1 }),
        //    MacroAction(UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY, ABILITY_ID::TRAIN_COLOSSUS, nullptr),
        //    MacroAction(UNIT_TYPEID::PROTOSS_ROBOTICSBAY, ABILITY_ID::RESEARCH_EXTENDEDTHERMALLANCE, nullptr),
        //    MacroAction(UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY, ABILITY_ID::TRAIN_COLOSSUS, nullptr),
        //    MacroAction(UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY, ABILITY_ID::TRAIN_IMMORTAL, nullptr),
        //    MacroBuilding(ABILITY_ID::BUILD_GATEWAY, { -1,-1 }),
        //    MacroBuilding(ABILITY_ID::BUILD_GATEWAY, { -1,-1 }),
        //    MacroBuilding(ABILITY_ID::BUILD_GATEWAY, { -1,-1 }),
        //    MacroBuilding(ABILITY_ID::BUILD_GATEWAY, { -1,-1 }),
        //    MacroBuilding(ABILITY_ID::BUILD_GATEWAY, { -1,-1 }),
        //    MacroBuilding(ABILITY_ID::BUILD_GATEWAY, { -1,-1 }),
        //    MacroBuilding(ABILITY_ID::BUILD_PYLON, {-1, -1}),
        //    MacroBuilding(ABILITY_ID::BUILD_PYLON, {-1, -1}),
        //    MacroBuilding(ABILITY_ID::BUILD_PYLON, {-1, -1}),
        //    MacroBuilding(ABILITY_ID::BUILD_PYLON, {-1, -1}),
        //    MacroBuilding(ABILITY_ID::BUILD_PYLON, {-1, -1}),
        //    MacroAction(UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY, ABILITY_ID::TRAIN_WARPPRISM, nullptr),
        //    MacroBuilding(ABILITY_ID::BUILD_NEXUS, P2D(Aux::rankedExpansions[1])),
        //};
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
        
        DebugText(this, strprintf("%.3fms", lastDT / 1000.0));

        Aux::displayExpansions(this);

        onStepProfiler.midLog("oS-Debug");

        SendDebug(this);

        onStepProfiler.midLog("oS-SendDebug");

        lastDT = onStepProfiler.getFullDT();
    }

    //! Called when a Unit has been created by the player.
    //!< \param unit The created unit.
    void OnUnitCreated(const sc2::Unit* unit_){
        std::cout << sc2::UnitTypeToName(unit_->unit_type) <<
            "(" << unit_->tag << ") was created" << std::endl;
        //UnitWrapper u(unit_);
        //UnitManager::self_units.insert(std::make_shared<UnitWrapper>(unit_));
        UnitManager::add(UnitManager::self_units, unit_);
        Aux::loadUnitPlacement(Aux::SELF_BUILDINGS, unit_->pos, unit_->unit_type);
    }

    //! Called when an enemy unit enters vision from out of fog of war.
    //!< \param unit The unit entering vision.
    virtual void OnUnitEnterVision(const sc2::Unit* unit_) {
        std::cout << sc2::UnitTypeToName(unit_->unit_type) <<
            "(" << unit_->tag << ") was created E" << std::endl;

        UnitManager::add(UnitManager::enemy_units, unit_);
        Aux::loadUnitPlacement(Aux::ENEMY_BUILDINGS, unit_->pos, unit_->unit_type); //TODO: ENEMY UNITS WHEN get() is called WILL CHECK IF THEIR POSITION IS DIFFERENT FROM THEIR OLD POSITION AND IF SO, REWORK THE PATHING GRID
    }

    //!  Called when a neutral unit is created. For example, mineral fields observed for the first time
    //!< \param unit The observed unit.
    virtual void OnNeutralUnitCreated(const sc2::Unit* unit_) {
        UnitManager::add(UnitManager::neutral_units, unit_);
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
