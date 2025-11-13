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

    Strategy test_plusone_atk;

    Strategy shit_stalker_colossus;

    Strategy zuka_proxy_tempest;//https://lotv.spawningtool.com/build/150094/


    void load(){
        FUNCTION_LOG();
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

        test_plusone_atk.build_order = {
            MacroBuilding(ABILITY_ID::BUILD_PYLON, Aux::criticalPoints[Aux::SELF_FIRSTPYLON_POINT], MacroActionData("Pylar")),
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY, Aux::PointDefault(), MacroActionData("Alice")),
            MacroBuilding(ABILITY_ID::BUILD_FORGE, Aux::PointDefault(), MacroActionData("Frank")),
            MacroBuilding(ABILITY_ID::GENERAL_MOVE, Aux::criticalPoints[Aux::ENEMY_STARTLOC_POINT]),
            MacroBuilding(ABILITY_ID::BUILD_ASSIMILATOR),
            MacroBuilding(ABILITY_ID::BUILD_ASSIMILATOR),
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY, Aux::PointDefault(), MacroActionData("Bob")),
            MacroBuilding(ABILITY_ID::BUILD_CYBERNETICSCORE),
            MacroBuilding(ABILITY_ID::BUILD_PYLON),
            MacroAction(UNIT_TYPEID::PROTOSS_CYBERNETICSCORE, ABILITY_ID::RESEARCH_WARPGATE),
            MacroAction(UNIT_TYPEID::PROTOSS_FORGE, ABILITY_ID::RESEARCH_PROTOSSGROUNDWEAPONS),
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
        };

        shit_stalker_colossus.build_order = {
            MacroBuilding(ABILITY_ID::BUILD_PYLON, Aux::criticalPoints[Aux::SELF_FIRSTPYLON_POINT]),
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY),
            MacroBuilding(ABILITY_ID::GENERAL_MOVE, Aux::criticalPoints[Aux::ENEMY_STARTLOC_POINT]),
            MacroBuilding(ABILITY_ID::BUILD_ASSIMILATOR),
            MacroBuilding(ABILITY_ID::BUILD_CYBERNETICSCORE),
            MacroBuilding(ABILITY_ID::BUILD_NEXUS),
            MacroAction(UNIT_TYPEID::PROTOSS_GATEWAY, ABILITY_ID::TRAIN_STALKER),
            MacroAction(UNIT_TYPEID::PROTOSS_CYBERNETICSCORE, ABILITY_ID::RESEARCH_WARPGATE),
            MacroBuilding(ABILITY_ID::BUILD_ASSIMILATOR),
            MacroBuilding(ABILITY_ID::BUILD_PYLON),
            MacroAction(UNIT_TYPEID::PROTOSS_GATEWAY, ABILITY_ID::TRAIN_STALKER),
            MacroAction(UNIT_TYPEID::PROTOSS_GATEWAY, ABILITY_ID::TRAIN_STALKER),
            MacroBuilding(ABILITY_ID::BUILD_ROBOTICSFACILITY),
            MacroAction(UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY, ABILITY_ID::TRAIN_OBSERVER),
            MacroAction(UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY, ABILITY_ID::TRAIN_IMMORTAL),
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY),
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY),
            MacroBuilding(ABILITY_ID::BUILD_ROBOTICSBAY),
            MacroAction(UNIT_TYPEID::PROTOSS_GATEWAY, ABILITY_ID::TRAIN_STALKER),
            MacroAction(UNIT_TYPEID::PROTOSS_GATEWAY, ABILITY_ID::TRAIN_STALKER),
            MacroAction(UNIT_TYPEID::PROTOSS_GATEWAY, ABILITY_ID::TRAIN_STALKER),
            MacroAction(UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL, ABILITY_ID::RESEARCH_BLINK),
            MacroBuilding(ABILITY_ID::BUILD_ASSIMILATOR),
            MacroBuilding(ABILITY_ID::BUILD_TWILIGHTCOUNCIL),
            MacroBuilding(ABILITY_ID::BUILD_ASSIMILATOR),
            MacroAction(UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY, ABILITY_ID::TRAIN_COLOSSUS),
            MacroAction(UNIT_TYPEID::PROTOSS_ROBOTICSBAY, ABILITY_ID::RESEARCH_EXTENDEDTHERMALLANCE),
            MacroAction(UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY, ABILITY_ID::TRAIN_COLOSSUS),
            MacroAction(UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY, ABILITY_ID::TRAIN_IMMORTAL),
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
            MacroAction(UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY, ABILITY_ID::TRAIN_WARPPRISM, Aux::PointDefault(), false, MacroActionData("Prism", 'p')),
            MacroBuilding(ABILITY_ID::BUILD_NEXUS, Aux::PointDefault(), MacroActionData(), 'p'),
        };
        shit_stalker_colossus.unitRatio.stalker = 8;
        shit_stalker_colossus.unitRatio.immortal = 4;
        shit_stalker_colossus.unitRatio.colossus = 2;
        shit_stalker_colossus.unitRatio.observer = 1;
        shit_stalker_colossus.unitRatio.warpprism = 1;
        shit_stalker_colossus.armyAttackNum = 10;
        shit_stalker_colossus.commit = true;

        auto it = Aux::enemyRankedExpansions.begin();
        it++; it++; it++;
        Point2D proxyPoint = Aux::P2D(Aux::expansions[(*it).expansionIndex].pos);
        zuka_proxy_tempest.build_order = {
            // ===== Early Economy ===== (0:00-1:21)
            MacroBuilding(ABILITY_ID::BUILD_PYLON, Aux::criticalPoints[Aux::SELF_FIRSTPYLON_POINT]),  // 12 probes @ 0:08
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY),                  // 13 probes @ 0:26
            MacroBuilding(ABILITY_ID::GENERAL_PATROL, proxyPoint + Point2D{0, -3.5}),
            MacroBuilding(ABILITY_ID::GENERAL_MOVE, Aux::criticalPoints[Aux::ENEMY_STARTLOC_POINT]),
            MacroBuilding(ABILITY_ID::BUILD_ASSIMILATOR),              // 14 probes @ 0:41
            MacroBuilding(ABILITY_ID::BUILD_ASSIMILATOR),              // 16 probes @ 0:51
            MacroBuilding(ABILITY_ID::BUILD_CYBERNETICSCORE),          // 18 probes @ 1:14
            MacroBuilding(ABILITY_ID::BUILD_PYLON, proxyPoint),                    // 18 probes @ 1:21

            // ===== Tech Transition ===== (1:21-2:26)
            MacroBuilding(ABILITY_ID::BUILD_STARGATE, proxyPoint + Point2D{2.5, 0.5}),                 // 20 probes @ 1:51
            MacroAction(UNIT_TYPEID::PROTOSS_GATEWAY, ABILITY_ID::TRAIN_ADEPT, true), // 21 probes @ 1:54 (Chrono)
            MacroAction(UNIT_TYPEID::PROTOSS_GATEWAY, ABILITY_ID::TRAIN_ADEPT), // 25 probes @ 2:17
            MacroBuilding(ABILITY_ID::BUILD_PYLON,proxyPoint + Point2D{-2, -2}),                    // 26 probes @ 2:23
            MacroAction(UNIT_TYPEID::PROTOSS_STARGATE, ABILITY_ID::TRAIN_VOIDRAY, true), // 26 probes @ 2:26 (Chrono)

            // ===== Defensive Setup ===== (2:26-3:57)
            MacroBuilding(ABILITY_ID::BUILD_SHIELDBATTERY, proxyPoint + Point2D{-2, 0}),            // 30 probes @ 2:42 (x2)
            MacroBuilding(ABILITY_ID::BUILD_SHIELDBATTERY, proxyPoint + Point2D{-2, 2}),            // 30 probes @ 2:42 (x2)
            MacroAction(UNIT_TYPEID::PROTOSS_STARGATE, ABILITY_ID::TRAIN_VOIDRAY), // 30 probes @ 2:57
            MacroBuilding(ABILITY_ID::BUILD_SHIELDBATTERY, proxyPoint + Point2D{0, 2}),            // 34 probes @ 3:09
            MacroBuilding(ABILITY_ID::BUILD_FLEETBEACON),              // 34 probes @ 3:28
            MacroBuilding(ABILITY_ID::BUILD_SHIELDBATTERY, proxyPoint + Point2D{0, -2}),            // 34 probes @ 3:57

            // ===== Tempest Transition ===== (4:12-5:02)
            MacroBuilding(ABILITY_ID::BUILD_PYLON),                    // 35 probes @ 4:12
            MacroBuilding(ABILITY_ID::BUILD_SHIELDBATTERY),            // 36 probes @ 4:24
            MacroAction(UNIT_TYPEID::PROTOSS_STARGATE, ABILITY_ID::TRAIN_TEMPEST, true), // 36 probes @ 4:29 (Chrono)
            MacroBuilding(ABILITY_ID::BUILD_SHIELDBATTERY),            // 36 probes @ 4:30
            MacroBuilding(ABILITY_ID::BUILD_SHIELDBATTERY),            // 41 probes @ 4:39
            MacroAction(UNIT_TYPEID::PROTOSS_CYBERNETICSCORE, ABILITY_ID::RESEARCH_WARPGATE), // 41 probes @ 4:48
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY),                  // 41 probes @ 4:49
            MacroAction(UNIT_TYPEID::PROTOSS_STARGATE, ABILITY_ID::TRAIN_TEMPEST, true), // 41 probes @ 5:02 (Chrono)

            // ===== Mid-Game Push Prep ===== (5:21-6:16)
            MacroBuilding(ABILITY_ID::BUILD_PYLON),                    // 46 probes @ 5:21
            MacroBuilding(ABILITY_ID::BUILD_SHIELDBATTERY),            // 46 probes @ 5:24
            MacroAction(UNIT_TYPEID::PROTOSS_STARGATE, ABILITY_ID::TRAIN_TEMPEST, true), // 42 probes @ 5:38 (Chrono)
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY),                  // 47 probes @ 5:45
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY),                  // 43 probes @ 6:12
            MacroAction(UNIT_TYPEID::PROTOSS_STARGATE, ABILITY_ID::TRAIN_TEMPEST, true), // 43 probes @ 6:16 (Chrono)

            // ===== Late-Game Army ===== (6:38-7:57)
            MacroAction(UNIT_TYPEID::PROTOSS_GATEWAY, ABILITY_ID::TRAIN_STALKER), // 48 probes @ 6:38 (x2)
            MacroAction(UNIT_TYPEID::PROTOSS_GATEWAY, ABILITY_ID::TRAIN_STALKER), // 48 probes @ 6:38 (x2)
            MacroAction(UNIT_TYPEID::PROTOSS_STARGATE, ABILITY_ID::TRAIN_TEMPEST, true), // 52 probes @ 7:01 (Chrono)
            MacroAction(UNIT_TYPEID::PROTOSS_GATEWAY, ABILITY_ID::TRAIN_STALKER), // 57 probes @ 7:25 (x4)
            MacroAction(UNIT_TYPEID::PROTOSS_GATEWAY, ABILITY_ID::TRAIN_STALKER), // 57 probes @ 7:25 (x4)
            MacroAction(UNIT_TYPEID::PROTOSS_GATEWAY, ABILITY_ID::TRAIN_STALKER), // 57 probes @ 7:25 (x4)
            MacroAction(UNIT_TYPEID::PROTOSS_GATEWAY, ABILITY_ID::TRAIN_STALKER), // 57 probes @ 7:25 (x4)
            MacroAction(UNIT_TYPEID::PROTOSS_GATEWAY, ABILITY_ID::TRAIN_STALKER), // 61 probes @ 7:57 (x4)
            MacroAction(UNIT_TYPEID::PROTOSS_GATEWAY, ABILITY_ID::TRAIN_STALKER), // 61 probes @ 7:57 (x4)
            MacroAction(UNIT_TYPEID::PROTOSS_GATEWAY, ABILITY_ID::TRAIN_STALKER), // 61 probes @ 7:57 (x4)
            MacroAction(UNIT_TYPEID::PROTOSS_GATEWAY, ABILITY_ID::TRAIN_STALKER)  // 61 probes @ 7:57 (x4)
        };
        zuka_proxy_tempest.unitRatio.adept = 2;
        zuka_proxy_tempest.unitRatio.voidray = 2;
        zuka_proxy_tempest.unitRatio.tempest = 5;
        zuka_proxy_tempest.unitRatio.stalker = 10;
        zuka_proxy_tempest.armyAttackNum = 1;

    }

}