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

    Strategy plusone_glaivedept_allin; //https://lotv.spawningtool.com/build/93501/

    Strategy shit_stalker_colossus;

    Strategy zuka_proxy_tempest;//https://lotv.spawningtool.com/build/150094/

    Strategy classic_colossus_disruptor; //https://lotv.spawningtool.com/build/197604/

    Strategy pig_colossus_timing; //https://lotv.spawningtool.com/build/184604/

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
            //MacroAction(UNIT_TYPEID::PROTOSS_CYBERNETICSCORE, ABILITY_ID::RESEARCH_WARPGATE), // 2nd Chrono
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
        glaive_adept_rush_hupsaiya.armyAttackNum = 2; //Hit at 4:37 with 14 Adepts
        glaive_adept_rush_hupsaiya.commit = true;

        plusone_glaivedept_allin.build_order = {
            // Early Economy
            MacroBuilding(ABILITY_ID::BUILD_PYLON, Aux::criticalPoints[Aux::SELF_FIRSTPYLON_POINT]),  // 14 @ 0:18
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY),                              // 15 @ 0:39
            MacroBuilding(ABILITY_ID::GENERAL_MOVE, Aux::criticalPoints[Aux::ENEMY_STARTLOC_POINT]),
            MacroBuilding(ABILITY_ID::BUILD_ASSIMILATOR),                         // 16 @ 0:48
            MacroBuilding(ABILITY_ID::BUILD_ASSIMILATOR),                         // 18 @ 0:58

            // Early Production
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY),                              // 19 @ 1:13
            MacroBuilding(ABILITY_ID::BUILD_CYBERNETICSCORE),                     // 21 @ 1:30
            MacroBuilding(ABILITY_ID::BUILD_PYLON),                               // 22 @ 1:42

            // Tech & First Units
            MacroAction(UNIT_TYPEID::PROTOSS_CYBERNETICSCORE, ABILITY_ID::RESEARCH_WARPGATE), // 23 @ 1:59
            MacroAction(UNIT_TYPEID::PROTOSS_GATEWAY, ABILITY_ID::TRAIN_STALKER), // 24 @ 2:07
            MacroAction(UNIT_TYPEID::PROTOSS_GATEWAY, ABILITY_ID::TRAIN_SENTRY),  // 24 @ 2:08

            // Expansion Phase
            MacroBuilding(ABILITY_ID::BUILD_PYLON),                               // 29 @ 2:17
            MacroAction(UNIT_TYPEID::PROTOSS_GATEWAY, ABILITY_ID::TRAIN_STALKER), // 28 @ 2:34 (x2)
            MacroAction(UNIT_TYPEID::PROTOSS_GATEWAY, ABILITY_ID::TRAIN_STALKER),
            MacroBuilding(ABILITY_ID::BUILD_NEXUS), // 32 @ 3:00

            // Robotics Transition
            MacroBuilding(ABILITY_ID::BUILD_ROBOTICSFACILITY),                    // 32 @ 3:11
            MacroBuilding(ABILITY_ID::BUILD_PYLON),                               // 35 @ 3:35
            MacroBuilding(ABILITY_ID::BUILD_SHIELDBATTERY),                       // 35 @ 3:43
            MacroBuilding(ABILITY_ID::BUILD_PYLON),                               // 36 @ 3:53
            MacroAction(UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY, ABILITY_ID::TRAIN_IMMORTAL), // 36 @ 3:53

            // Upgrades & Support
            MacroBuilding(ABILITY_ID::BUILD_FORGE),                               // 43 @ 4:20
            MacroAction(UNIT_TYPEID::PROTOSS_GATEWAY, ABILITY_ID::TRAIN_SENTRY),  // 44 @ 4:26
            MacroAction(UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY, ABILITY_ID::TRAIN_OBSERVER), // 48 @ 4:32
            MacroAction(UNIT_TYPEID::PROTOSS_FORGE, ABILITY_ID::RESEARCH_PROTOSSGROUNDWEAPONS), // 48 @ 4:36

            // Adept Transition
            MacroBuilding(ABILITY_ID::BUILD_TWILIGHTCOUNCIL),                     // 53 @ 5:03
            MacroAction(UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY, ABILITY_ID::TRAIN_IMMORTAL), // 53 @ 5:05
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY),                              // 59 @ 5:17
            MacroAction(UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL, ABILITY_ID::RESEARCH_ADEPTRESONATINGGLAIVES), // 58 @ 5:26

            // Mass Gateway Production
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY),                              // 58 @ 5:30 (x2)
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY),
            MacroAction(UNIT_TYPEID::PROTOSS_GATEWAY, ABILITY_ID::TRAIN_ADEPT),  // 58 @ 5:35 (x2)
            MacroAction(UNIT_TYPEID::PROTOSS_GATEWAY, ABILITY_ID::TRAIN_ADEPT),
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY),                              // 60 @ 5:38
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY),                              // 62 @ 5:51

            // Warp Prism & Final Production
            MacroAction(UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY, ABILITY_ID::TRAIN_WARPPRISM), // 62 @ 5:56
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY),                              // 64 @ 5:57
            MacroAction(UNIT_TYPEID::PROTOSS_GATEWAY, ABILITY_ID::TRAIN_ADEPT),  // 64 @ 6:06 (x2)
            MacroAction(UNIT_TYPEID::PROTOSS_GATEWAY, ABILITY_ID::TRAIN_ADEPT),
            MacroAction(UNIT_TYPEID::PROTOSS_GATEWAY, ABILITY_ID::TRAIN_ADEPT),  // 68 @ 6:13
            MacroAction(UNIT_TYPEID::PROTOSS_GATEWAY, ABILITY_ID::TRAIN_ADEPT),  // 70 @ 6:27 (x4)
            MacroAction(UNIT_TYPEID::PROTOSS_GATEWAY, ABILITY_ID::TRAIN_ADEPT),
            MacroAction(UNIT_TYPEID::PROTOSS_GATEWAY, ABILITY_ID::TRAIN_ADEPT),
            MacroAction(UNIT_TYPEID::PROTOSS_GATEWAY, ABILITY_ID::TRAIN_ADEPT),
            MacroAction(UNIT_TYPEID::PROTOSS_GATEWAY, ABILITY_ID::TRAIN_ADEPT)   // 78 @ 6:41
        };

        // Unit Composition
        plusone_glaivedept_allin.unitRatio.adept = 11;
        plusone_glaivedept_allin.armyAttackNum = 13;  // Push with Immortal + Adept timing
        plusone_glaivedept_allin.commit = true;

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
        it++; it++; it++; it++;
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

        classic_colossus_disruptor.build_order = {
            // Early Economy & Expansion
            MacroBuilding(ABILITY_ID::BUILD_PYLON, Aux::criticalPoints[Aux::SELF_FIRSTPYLON_POINT]),  // 14 @ 0:20
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY),                              // 16 @ 0:40
            MacroBuilding(ABILITY_ID::GENERAL_MOVE, Aux::criticalPoints[Aux::ENEMY_STARTLOC_POINT]),
            MacroBuilding(ABILITY_ID::BUILD_ASSIMILATOR),                         // 17 @ 0:50

            // Tech & Production
            MacroBuilding(ABILITY_ID::BUILD_NEXUS), // 20 @ 1:27
            MacroBuilding(ABILITY_ID::BUILD_CYBERNETICSCORE),                     // 21 @ 1:38
            MacroBuilding(ABILITY_ID::BUILD_ASSIMILATOR),                         // 21 @ 1:47
            MacroBuilding(ABILITY_ID::BUILD_PYLON),                               // 22 @ 1:55

            // Early Units & Robotics
            MacroGateway(ABILITY_ID::TRAIN_ADEPT, true), // 25 @ 2:14 (Chrono)
            MacroAction(UNIT_TYPEID::PROTOSS_CYBERNETICSCORE, ABILITY_ID::RESEARCH_WARPGATE), // 26 @ 2:16
            MacroBuilding(ABILITY_ID::BUILD_ROBOTICSFACILITY),                    // 26 @ 2:23
            MacroGateway(ABILITY_ID::TRAIN_STALKER, true), // 29 @ 2:34 (Chrono)

            // Army Composition
            MacroAction(UNIT_TYPEID::PROTOSS_GATEWAY, ABILITY_ID::TRAIN_STALKER), // 36 @ 3:01
            MacroAction(UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY, ABILITY_ID::TRAIN_OBSERVER), // 39 @ 3:12
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY),                              // 41 @ 3:26
            MacroBuilding(ABILITY_ID::BUILD_PYLON),                               // 41 @ 3:28
            MacroBuilding(ABILITY_ID::BUILD_ASSIMILATOR),                         // 43 @ 3:44 (x2)
            MacroBuilding(ABILITY_ID::BUILD_ASSIMILATOR),
            MacroAction(UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY, ABILITY_ID::TRAIN_IMMORTAL), // 47 @ 3:47

            // Expansion & Tech
            MacroBuilding(ABILITY_ID::BUILD_NEXUS), // 49 @ 4:00
            MacroBuilding(ABILITY_ID::BUILD_PYLON),                               // 49 @ 4:08
            MacroBuilding(ABILITY_ID::BUILD_PYLON),                               // 52 @ 4:25
            MacroBuilding(ABILITY_ID::BUILD_ROBOTICSBAY),                         // 54 @ 4:34
            MacroBuilding(ABILITY_ID::BUILD_ROBOTICSFACILITY),                    // 56 @ 4:43
            // MacroAction(UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY, ABILITY_ID::TRAIN_OBSERVER), // 59 @ 4:49

            // Upgrades & Heavy Production
            MacroBuilding(ABILITY_ID::BUILD_FORGE),                               // 66 @ 5:15
            MacroRobo(ABILITY_ID::TRAIN_COLOSSUS, true), // 66 @ 5:32 (Chrono x2)
            MacroRobo(ABILITY_ID::TRAIN_COLOSSUS, true),
            MacroBuilding(ABILITY_ID::BUILD_TWILIGHTCOUNCIL),                     // 72 @ 5:35
            MacroBuilding(ABILITY_ID::BUILD_PYLON),                               // 81 @ 5:38
            MacroBuilding(ABILITY_ID::BUILD_ASSIMILATOR),                         // 83 @ 5:48 (x2)
            MacroBuilding(ABILITY_ID::BUILD_ASSIMILATOR),

            // Late Game Army
            MacroBuilding(ABILITY_ID::BUILD_PYLON),                               // 90 @ 6:13
            MacroAction(UNIT_TYPEID::PROTOSS_FORGE, ABILITY_ID::RESEARCH_PROTOSSGROUNDWEAPONSLEVEL1, Aux::PointArea(), true), // 91 @ 6:16 (Chrono)
            MacroAction(UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL, ABILITY_ID::RESEARCH_CHARGE, Aux::PointArea(), true), // 91 @ 6:17 (Chrono)

            // Mass Gateway Production
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY),                              // 92 @ 6:30 (x7)
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY),
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY),
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY),
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY),
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY),
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY),
            MacroBuilding(ABILITY_ID::BUILD_TEMPLARARCHIVE),                     // 92 @ 6:38
            MacroBuilding(ABILITY_ID::BUILD_PYLON),                               // 96 @ 6:41

            // Disruptor Tech
            MacroRobo(ABILITY_ID::TRAIN_DISRUPTOR, true), // 100 @ 6:43 (Chrono x2)
            MacroRobo(ABILITY_ID::TRAIN_DISRUPTOR, true),

            // Final Production Phase
            MacroRobo(ABILITY_ID::TRAIN_IMMORTAL, true), // 108 @ 7:15 (Chrono x2)
            MacroRobo(ABILITY_ID::TRAIN_IMMORTAL, true),
            MacroBuilding(ABILITY_ID::BUILD_NEXUS), // 108 @ 7:25
            MacroBuilding(ABILITY_ID::BUILD_PYLON),                               // 108 @ 7:30

            // Archon Transition
            MacroRobo(ABILITY_ID::TRAIN_WARPPRISM, true), // 116 @ 7:52 (Chrono)
            MacroRobo(ABILITY_ID::TRAIN_IMMORTAL, true), // 116 @ 7:52 (Chrono)
            MacroAction(UNIT_TYPEID::PROTOSS_GATEWAY, ABILITY_ID::TRAIN_HIGHTEMPLAR), // 116 @ 8:07 (x6)
            MacroAction(UNIT_TYPEID::PROTOSS_GATEWAY, ABILITY_ID::TRAIN_HIGHTEMPLAR),
            MacroAction(UNIT_TYPEID::PROTOSS_GATEWAY, ABILITY_ID::TRAIN_HIGHTEMPLAR),
            MacroAction(UNIT_TYPEID::PROTOSS_GATEWAY, ABILITY_ID::TRAIN_HIGHTEMPLAR),
            MacroAction(UNIT_TYPEID::PROTOSS_GATEWAY, ABILITY_ID::TRAIN_HIGHTEMPLAR),
            MacroAction(UNIT_TYPEID::PROTOSS_GATEWAY, ABILITY_ID::TRAIN_HIGHTEMPLAR),
            MacroAction(UNIT_TYPEID::PROTOSS_HIGHTEMPLAR, ABILITY_ID::MORPH_ARCHON), // Make Archons x3
            MacroAction(UNIT_TYPEID::PROTOSS_HIGHTEMPLAR, ABILITY_ID::MORPH_ARCHON),
            MacroAction(UNIT_TYPEID::PROTOSS_HIGHTEMPLAR, ABILITY_ID::MORPH_ARCHON)
        };

        // Unit Composition
        classic_colossus_disruptor.unitRatio.adept = 1;
        classic_colossus_disruptor.unitRatio.stalker = 2;
        classic_colossus_disruptor.unitRatio.observer = 2;
        classic_colossus_disruptor.unitRatio.immortal = 5;
        classic_colossus_disruptor.unitRatio.colossus = 2;
        classic_colossus_disruptor.unitRatio.disruptor = 2;
        classic_colossus_disruptor.unitRatio.warpprism = 1;
        classic_colossus_disruptor.unitRatio.hightemplar = 6;
        classic_colossus_disruptor.unitRatio.archon = 3;

        // Attack Timing
        classic_colossus_disruptor.armyAttackNum = 9;  // Push with Colossus + Disruptor timing

        pig_colossus_timing.build_order = {
            MacroBuilding(ABILITY_ID::BUILD_PYLON, Aux::criticalPoints[Aux::SELF_FIRSTPYLON_POINT]),
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY),
            MacroBuilding(ABILITY_ID::GENERAL_MOVE, Aux::criticalPoints[Aux::ENEMY_STARTLOC_POINT]),
            MacroBuilding(ABILITY_ID::BUILD_ASSIMILATOR),
            MacroBuilding(ABILITY_ID::BUILD_NEXUS),
            MacroBuilding(ABILITY_ID::BUILD_CYBERNETICSCORE),
            MacroBuilding(ABILITY_ID::BUILD_ASSIMILATOR),
            MacroBuilding(ABILITY_ID::BUILD_PYLON),
            MacroGateway(ABILITY_ID::TRAIN_ADEPT, true),
            MacroAction(UNIT_TYPEID::PROTOSS_CYBERNETICSCORE, ABILITY_ID::RESEARCH_WARPGATE, true),
            MacroBuilding(ABILITY_ID::BUILD_ROBOTICSFACILITY),
            MacroBuilding(ABILITY_ID::BUILD_PYLON),
            MacroGateway(ABILITY_ID::TRAIN_STALKER),
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY),
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY),
            MacroRobo(ABILITY_ID::TRAIN_OBSERVER, true),
            MacroBuilding(ABILITY_ID::BUILD_ROBOTICSBAY),
            MacroGateway(ABILITY_ID::TRAIN_STALKER),
            MacroBuilding(ABILITY_ID::BUILD_ASSIMILATOR),
            MacroBuilding(ABILITY_ID::BUILD_ASSIMILATOR),
            MacroRobo(ABILITY_ID::TRAIN_COLOSSUS, true),
            MacroGateway(ABILITY_ID::TRAIN_STALKER),
            MacroGateway(ABILITY_ID::TRAIN_STALKER),
            MacroGateway(ABILITY_ID::TRAIN_STALKER),
            MacroRobo(ABILITY_ID::TRAIN_COLOSSUS),
            MacroAction(UNIT_TYPEID::PROTOSS_ROBOTICSBAY, ABILITY_ID::RESEARCH_EXTENDEDTHERMALLANCE),
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY),
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY),
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY),
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY),
            MacroBuilding(ABILITY_ID::BUILD_GATEWAY),
            MacroGateway(ABILITY_ID::TRAIN_SENTRY),
            MacroGateway(ABILITY_ID::TRAIN_SENTRY),
            MacroGateway(ABILITY_ID::TRAIN_SENTRY),
            MacroRobo(ABILITY_ID::TRAIN_WARPPRISM),

        };
        pig_colossus_timing.unitRatio.stalker = 6;
        pig_colossus_timing.unitRatio.colossus = 1;
        pig_colossus_timing.unitRatio.sentry = 2;
        pig_colossus_timing.armyAttackNum = 10; 
        pig_colossus_timing.commit = true;
    }

}