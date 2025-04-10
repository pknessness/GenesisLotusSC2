// The MIT License (MIT)
//
// Copyright (c) 2021-2024 Alexander Kurbatov

#pragma once

#include <sc2api/sc2_agent.h>
#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>
#include <iostream>

// The main bot class.
struct Bot: sc2::Agent
{
    Bot() = default;

 private:
    void OnGameStart(){
        std::cout << "New game started!" << std::endl;
    }

    void OnGameEnd(){
        std::cout << "Game over!" << std::endl;
    }

    void OnStep(){
        std::cout << "OnStep" << std::endl;
    }

    void OnBuildingConstructionComplete(const sc2::Unit* building_){
        std::cout << sc2::UnitTypeToName(building_->unit_type) <<
            "(" << building_->tag << ") constructed" << std::endl;
    }

    void OnUnitCreated(const sc2::Unit* unit_){
        std::cout << sc2::UnitTypeToName(unit_->unit_type) <<
            "(" << unit_->tag << ") was created" << std::endl;
    }

    void OnUnitIdle(const sc2::Unit* unit_){
        std::cout << sc2::UnitTypeToName(unit_->unit_type) <<
             "(" << unit_->tag << ") is idle" << std::endl;
    }

    void OnUnitDestroyed(const sc2::Unit* unit_){
        std::cout << sc2::UnitTypeToName(unit_->unit_type) <<
             "(" << unit_->tag << ") was destroyed" << std::endl;
    }

    void OnUpgradeCompleted(sc2::UpgradeID id_){
        std::cout << sc2::UpgradeIDToName(id_) << " completed" << std::endl;
    }

    void OnError(const std::vector<sc2::ClientError>& client_errors, const std::vector<std::string>& protocol_errors = {}){
        for (const auto i : client_errors) {
            std::cerr << "Encountered client error: " <<
                static_cast<int>(i) << std::endl;
        }

        for (const auto& i : protocol_errors)
            std::cerr << "Encountered protocol error: " << i << std::endl;
    }
};
