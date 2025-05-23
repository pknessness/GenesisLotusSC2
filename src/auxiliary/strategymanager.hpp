#pragma once
#include "macromanager.hpp"

namespace StrategyManager {
    struct UnitRatio {
        int8_t zealot = 0;
        int8_t stalker = 0;
        int8_t sentry = 0;
        int8_t adept = 0;
        int8_t darktemplar = 0;
        int8_t hightemplar = 0;
        int8_t archon = 0;

        int8_t observer = 0;
        int8_t immortal = 0;
        int8_t warpprism = 0;
        int8_t colossus = 0;
        int8_t disruptor = 0;

        int8_t pheonix = 0;
        int8_t oracle = 0;
        int8_t voidray = 0;
        int8_t tempest = 0;
        int8_t carrier = 0;
        int8_t mothership = 0;
    };

    struct UnitRatioFloat {
        float zealot = 0;
        float stalker = 0;
        float sentry = 0;
        float adept = 0;
        float darktemplar = 0;
        float hightemplar = 0;
        float archon = 0;

        float observer = 0;
        float immortal = 0;
        float warpprism = 0;
        float colossus = 0;
        float disruptor = 0;

        float pheonix = 0;
        float oracle = 0;
        float voidray = 0;
        float tempest = 0;
        float carrier = 0;
        float mothership = 0;
    };

    struct Strategy {
        std::vector<MacroAction> build_order;
        UnitRatio unitRatio;
        int armyAttackNum = 12;
        bool commit = false;

        //std::function<void(Agent* const agent)> armyTargetting;
    };

    Strategy glaive_adept_rush_lightwisdom;
    Strategy glaive_adept_rush_hupsaiya;

    void load(){
        glaive_adept_rush_lightwisdom.build_order = {
            //Early Economy
            MacroBuilding(ABILITY_ID::BUILD_PYLON, Aux::criticalPoints[Aux::SELF_FIRSTPYLON_POINT]),
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY, Aux::PointArea(), MacroActionData("Gate1")),
            MacroBuilding(ABILITY_ID::GENERAL_MOVE, Aux::criticalPoints[Aux::ENEMY_STARTLOC_POINT]),
            MacroBuilding(ABILITY_ID::BUILD_ASSIMILATOR),
            MacroBuilding(ABILITY_ID::BUILD_NEXUS),

            //Core Infrastructure
            MacroBuilding(ABILITY_ID::BUILD_PYLON),
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY, Aux::PointArea(), MacroActionData("Gate2")),
            MacroBuilding(ABILITY_ID::BUILD_CYBERNETICSCORE),
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY, Aux::PointArea(), MacroActionData("Gate3")),
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY, Aux::PointArea(), MacroActionData("Gate4")),

            //Warp Gate + Early Units
            MacroAction(UNIT_TYPEID::PROTOSS_CYBERNETICSCORE, ABILITY_ID::RESEARCH_WARPGATE, true), //Chrono Boosted
            MacroAction(UNIT_TYPEID::PROTOSS_CYBERNETICSCORE, ABILITY_ID::RESEARCH_WARPGATE), // 2nd Chrono
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),

            //Mid - Game Setup
            MacroBuilding(ABILITY_ID::BUILD_ASSIMILATOR),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroBuilding(ABILITY_ID::BUILD_TWILIGHTCOUNCIL),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),

            //Power Spike
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroBuilding(ABILITY_ID::BUILD_PYLON),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroAction(UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL, ABILITY_ID::RESEARCH_ADEPTRESONATINGGLAIVES, true), //Chrono Boosted

            // Sustained Production
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroBuilding(ABILITY_ID::BUILD_PYLON),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroBuilding(ABILITY_ID::BUILD_PYLON),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroBuilding(ABILITY_ID::BUILD_PYLON),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT)
        };
        glaive_adept_rush_lightwisdom.unitRatio.adept = 1;
        glaive_adept_rush_lightwisdom.armyAttackNum = 10;
        
        glaive_adept_rush_hupsaiya.build_order = {
            MacroBuilding(ABILITY_ID::BUILD_PYLON, Aux::criticalPoints[Aux::SELF_FIRSTPYLON_POINT], MacroActionData("Pylar")),
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY, Aux::PointDefault(), MacroActionData("Alice")),
            MacroBuilding(ABILITY_ID::GENERAL_MOVE, Aux::criticalPoints[Aux::ENEMY_STARTLOC_POINT]),
            MacroBuilding(ABILITY_ID::BUILD_ASSIMILATOR),
            MacroBuilding(ABILITY_ID::BUILD_ASSIMILATOR),
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY, Aux::PointDefault(), MacroActionData("Bob")),
            MacroBuilding(ABILITY_ID::BUILD_CYBERNETICSCORE),
            MacroBuilding(ABILITY_ID::BUILD_PYLON),
            MacroAction(UNIT_TYPEID::PROTOSS_CYBERNETICSCORE, ABILITY_ID::RESEARCH_WARPGATE),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroBuilding(ABILITY_ID::BUILD_TWILIGHTCOUNCIL),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroBuilding(ABILITY_ID::BUILD_PYLON),
            MacroAction(UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL, ABILITY_ID::RESEARCH_ADEPTRESONATINGGLAIVES),
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY, Aux::PointDefault(), MacroActionData("Chris")),
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY, Aux::PointDefault(), MacroActionData("Donna")),
            MacroBuilding(ABILITY_ID::BUILD_PYLON), //Proxy Pylon near their 3rd base
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY, Aux::PointDefault(), MacroActionData("Edith")),
            MacroBuilding(ABILITY_ID::BUILD_SHIELDBATTERY),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroBuilding(ABILITY_ID::BUILD_PYLON),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
                        MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
                        MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
                        MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
                        MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
                        MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
                        MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
                        MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
                        MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
                        MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
                        MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
                        MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
                        MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
                        MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
                        MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT),
        };
        glaive_adept_rush_hupsaiya.unitRatio.adept = 1;
        glaive_adept_rush_hupsaiya.armyAttackNum = 13; //Hit at 4:37 with 14 Adepts
        glaive_adept_rush_hupsaiya.commit = true;
    }

}