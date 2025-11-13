// The MIT License (MIT)
//
// Copyright (c) 2021-2024 Alexander Kurbatov

#pragma once

#include <sc2api/sc2_agent.h>
#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>
#include <iostream>
#include <vector>

#ifndef FLT_MAX
#define FLT_MAX 3.402823466e+38F
#endif

#include "auxiliary/helpers.hpp"
#include "auxiliary/profiler.hpp"
#include "auxiliary/debugging.hpp"
#include "unitwrappers/unitmanager.hpp"
#include "auxiliary/macromanager.hpp"

#include "unitwrappers/nexus.hpp"
#include "unitwrappers/vespene.hpp"
#include "unitwrappers/adept.hpp"

#include "auxiliary/armymanager.hpp"
#include "unitwrappers/armyunit.hpp"
#include "auxiliary/spatialhashgrid.hpp"
#include "auxiliary/visiblemap.hpp"
#include "auxiliary/weapongrid.hpp"
#include "auxiliary/primordialstar.hpp"

#define DEBUG
//#define DAMAGEGRID_DEBUG
//#define PROBE_DEBUG

#define ADEPT_RUSH
//#define STALKER_COLOSSUS_TIMING

constexpr int16_t maxDamageOnGrid = 32;

namespace UnitManager {
    void encode(UnitWrapperPtr wrap, const Unit* unit_) {
        FUNCTION_LOG();
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
        FUNCTION_LOG();
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
            else if (stype == UNIT_TYPEID::PROTOSS_ADEPT) {
                AdeptPtr adept = std::make_shared<Adept>(unit_, &ArmyManager::mainAttackSquad);
                //adept->init(agent);
                encode(adept, unit_);
                units[stype].insert(adept);
                ArmyManager::mainAttackSquad.add(adept);

            }
            else if (stype == UNIT_TYPEID::PROTOSS_ADEPTPHASESHIFT) {
                AdeptShadePtr shade = std::make_shared<AdeptShade>(unit_, &ArmyManager::mainAttackSquad);
                shade->init(agent);
                //encode(shade, unit_);
                units[stype].insert(shade);
                ArmyManager::mainAttackSquad.add(shade);

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
            else if(unit_->is_building){
                UnitWrapperPtr selfUnit = std::make_shared<UnitWrapper>(unit_, stype);
                encode(selfUnit, unit_);
                units[stype].insert(selfUnit);
            }
            else {
                ArmyUnitPtr armyUnit = std::make_shared<ArmyUnit>(unit_, stype, &ArmyManager::mainAttackSquad);
                encode(armyUnit, unit_);
                units[stype].insert(armyUnit);
                ArmyManager::mainAttackSquad.add(armyUnit);
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
        FUNCTION_LOG();
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

//float prevMax = 0;

// The main bot class.
struct Bot: sc2::Agent
{
    Bot() = default;
    long long lastDT = 0;

    StrategyManager::Strategy strat;

 private:
    //! Called when a game is started or restarted.
    void OnGameStart(){
        printf("onGameStart\n");
        std::filesystem::create_directory("data");
        FILE* imageFile = fopen("data/functionLogs.txt", "wb");
        fclose(imageFile);

        FUNCTION_LOG();
        Aux::gameInfo_cache = Observation()->GetGameInfo();
        Aux::mapWidth_cache = Aux::gameInfo_cache.width;
        Aux::mapHeight_cache = Aux::gameInfo_cache.height;
        Aux::opponent = Aux::gameInfo_cache.player_info[1].race_requested;

        //printf("0: %s / %s; ", Aux::RaceToName(Aux::gameInfo_cache.player_info[0].race_actual), Aux::RaceToName(Aux::gameInfo_cache.player_info[0].race_requested));
        printf("1: %s / %s\n", Aux::RaceToName(Aux::gameInfo_cache.player_info[1].race_actual), Aux::RaceToName(Aux::gameInfo_cache.player_info[1].race_requested));

        Aux::setupExpansions(this);

        Aux::masterMap = std::make_shared<map2d<uint8_t>>(Aux::mapWidth_cache, Aux::mapHeight_cache, true);
        Aux::setupMasterMap(this);

        Aux::allData(this);

        PrimordialStar::load(this);

        StrategyManager::load();

        SpatialHashGrid::init();
        VisibleMap2D::init();
        WeaponGrid::init(this);

        SquadManager::init();

        strat = StrategyManager::zuka_proxy_tempest; //StrategyManager::shit_stalker_colossus;//StrategyManager::glaive_adept_rush_hupsaiya;//StrategyManager::test_plusone_atk;//

        for (int i = 0; i < strat.build_order.size(); i++) {
            MacroManager::addAction(strat.build_order[i]);
        }

#if 1
        const char* races[] = {
                "protoss",
                "terran",
                "zerg"
        };
        int p_c = sizeof(Aux::ArmyUnitsProtoss) / sizeof(UnitTypeID);
        std::vector<UnitTypeID> p(Aux::ArmyUnitsProtoss, Aux::ArmyUnitsProtoss + p_c);
        int t_c = sizeof(Aux::ArmyUnitsTerran) / sizeof(UnitTypeID);
        std::vector<UnitTypeID> t(Aux::ArmyUnitsTerran, Aux::ArmyUnitsTerran + t_c);
        int z_c = sizeof(Aux::ArmyUnitsZerg) / sizeof(UnitTypeID);
        std::vector<UnitTypeID> z(Aux::ArmyUnitsZerg, Aux::ArmyUnitsZerg + z_c);

        std::vector<UnitTypeID> unitLists[3] = { p, t, z };

        std::string maxDPSPSpName = "";
        float maxDPSPSp = 0;

        int numWeapons = 0;

        FILE* fpa;
        fpa = fopen("data/unitData_all.txt", "w");

        for (int f = 0; f < 3; f++) {
            FILE* fp;
            fp = fopen(strprintf("data/unitData_%s.txt", races[f]).c_str(), "w");
            fprintf(fpa, "%s:\n", races[f]);

            fprintf(fp, "Unit Data:\n");

            //int numU = sizeof(unitLists[f]) / sizeof(UnitTypeID);
            int numU = unitLists[f].size();
            fprintf(fp, "%d Units\n\n---------------\n", numU);
            for (int i = 0; i < numU; i++) {
                UnitTypeData* d = Aux::getStats(unitLists[f][i], this);
                fprintf(fp, "%s:\nAvailable: %c\nCargo: %d\nMineralCost: %d\nVespeneCost: %d\nAttributes: ",
                    UnitTypeToName(unitLists[f][i]),
                    (d->available ? 'Y' : 'N'),
                    d->cargo_size,
                    d->mineral_cost,
                    d->vespene_cost);
                for (int a = 0; a < d->attributes.size(); a++) {
                    fprintf(fp, "%s", Aux::AttributeToName(d->attributes[a]));
                    if (a != d->attributes.size() - 1) {
                        fprintf(fp, ", ");
                    }
                }
                fprintf(fp, "\nMovementSpeed: %.2f\nArmor: %.1f\nWeapons:\n",
                    d->movement_speed * timeSpeed,
                    d->armor);
                for (int w = 0; w < d->weapons.size(); w++) {
                    float atkcd = d->weapons[w].speed / timeSpeed;
                    if (unitLists[f][i] == UNIT_TYPEID::ZERG_ZERGLING) {
                        atkcd -= 0.15; //crack lings
                    }else if (unitLists[f][i] == UNIT_TYPEID::TERRAN_MARINE) {
                        atkcd *= 0.666667; //stim
                    }
                    float dps = (d->weapons[w].damage_ * d->weapons[w].attacks) / (d->weapons[w].speed / timeSpeed);                    
                    float maxdps = ((d->weapons[w].damage_ + 3 * Aux::damageExtraPerUpgrade(d->weapons[w].damage_)) * d->weapons[w].attacks) / atkcd;
                    float dpspsp = dps * 200 / d->food_required;
                    float maxdpspsp = maxdps * 200 / d->food_required;
                    fprintf(fp, "-----\n%s\nDamage: %.2f\nAttacks: %d\nRange: %.2f\nCooldown: %.2fs\nDPS: %.2f\nDPSPSp: %.2f\nMAXDPS: %.2f\nMAXDPSPSp: %.2f\nBonuses: ",
                        Aux::TargetTypeToName(d->weapons[w].type),
                        d->weapons[w].damage_,
                        d->weapons[w].attacks,
                        d->weapons[w].range,
                        d->weapons[w].speed / timeSpeed,
                        dps, 
                        dpspsp,
                        maxdps,
                        maxdpspsp);
                    if (maxDPSPSp < maxdpspsp && d->food_required != 0) {
                        maxDPSPSp = maxdpspsp;
                        maxDPSPSpName = UnitTypeToName(unitLists[f][i]);
                    }
                    for (int b = 0; b < d->weapons[w].damage_bonus.size(); b++) {
                        fprintf(fp, "+%.1f %s", d->weapons[w].damage_bonus[b].bonus, Aux::AttributeToName(d->weapons[w].damage_bonus[b].attribute));
                    }
                    fprintf(fp, "\n-----");
                    numWeapons++;
                }
                fprintf(fp, "\nSupply: %.1f\nSupply+: %.1f\nRace: %d\nBuildTime: %.2f\nSightRange: %.1f\nTechAliases: ",
                    d->food_required,
                    d->food_provided,
                    d->race,
                    d->build_time,
                    d->sight_range);
                for (int a = 0; a < d->tech_alias.size(); a++) {
                    fprintf(fp, "%s", UnitTypeToName(d->tech_alias[a]));
                    if (a != d->tech_alias.size() - 1) {
                        fprintf(fp, ", ");
                    }
                }
                fprintf(fp, "\nAlias: %s\nRequirement: %s\nRequireAttached: %c\n---------------\n",
                    UnitTypeToName(d->unit_alias),
                    UnitTypeToName(d->tech_requirement),
                    (d->require_attached ? 'Y' : 'N'));
            }
            fclose(fp);
            fp = fopen(strprintf("data/unitData_%s.txt", races[f]).c_str(), "r");
            char c = fgetc(fp);
            while (c != EOF) {
                fwrite(&c, 1, 1, fpa);
                c = fgetc(fp);
            }
            fclose(fp);
        }
        fclose(fpa);
        printf("TECHNICALLY MOST EFFECTIVE UNIT: %s @ %.3fdpspsp\nNUMWEAPONS: %d\n", maxDPSPSpName.c_str(), maxDPSPSp, numWeapons);

#endif
    }

    //! Called when a game has ended.
    void OnGameEnd(){
        FUNCTION_LOG();
        std::cout << "Game over!" << std::endl;
        UnitManager::self_units.clear();
        UnitManager::neutral_units.clear();
        UnitManager::enemy_units.clear();
    }

    //! In non realtime games this function gets called after each step as indicated by step size.
    //! In realtime this function gets called as often as possible after request/responses are received from the game gathering observation state.
    void OnStep(){
        //printf("onStep %lld\n", Observation()->GetGameLoop());
        FUNCTION_LOG();
        Profiler onStepProfiler("onStep");
        Aux::effectiveMinerals = Observation()->GetMinerals();
        Aux::effectiveVespene = Observation()->GetVespene();

        UnitWrappers probes = UnitManager::getSelf(UNIT_TYPEID::PROTOSS_PROBE);
        for (auto it = probes.begin(); it != probes.end(); it++) {
            Profiler onStepProfiler("pE");
            UnitWrapperPtr p = *it;
            ProbePtr probe = std::static_pointer_cast<Probe>(p);
            probe->execute(this);
#ifdef PROBE_DEBUG
            UnitWrapperPtr target = probe->getTargetTag(this);
            if (target != nullptr) {
                DebugLine(this, target->pos3D(this) + Point3D{ 0,0,1 }, probe->pos3D(this) + Point3D{ 0,0,1 });
            }
            if (probe->buildings.size() != 0) {
                DebugText(this, strprintf("%s", AbilityTypeToName(probe->buildings[0].build)), probe->pos3D(this) + Point3D{ 0,0,1 });
            }
#endif
        }

        UnitWrappers nexi = UnitManager::getSelf(UNIT_TYPEID::PROTOSS_NEXUS);
        for (auto it = nexi.begin(); it != nexi.end(); it++) {
            NexusPtr nexus = std::static_pointer_cast<Nexus>(*it);
            nexus->execute(this);
        }

        onStepProfiler.midLog("oS-probeExec");

        int numProbes = 0;
        int numProbesMax = 0;
        for (UnitWrapperPtr nexusWrap : UnitManager::getSelf(UNIT_TYPEID::PROTOSS_NEXUS)) {
            NexusPtr nexus = std::static_pointer_cast<Nexus>(nexusWrap);

            int numProbesN = 0;
            int numProbesMaxN = 0;
            UnitTypeData* stats = Aux::getStats(UNIT_TYPEID::PROTOSS_PROBE, this);
            float percentUntilViable = 1.0F - (stats->build_time / stats->build_time);
            if (nexus->getReturn(this)->build_progress < percentUntilViable) {
                continue;
            }
            if (nexus->assimilator1 != nullptr) {
                numProbesMaxN += 3;
                numProbesN += probeTargetting[nexus->assimilator1->self];
            }
            if (nexus->assimilator2 != nullptr) {
                numProbesMaxN += 3;
                numProbesN += probeTargetting[nexus->assimilator2->self];
            }
            for (int i = 0; i < 8; i++) {
                if (nexus->minerals[i] != nullptr) {
                    if (nexus->minerals[i]->getReturn(this) != nullptr) {
                        numProbesMaxN += 2;
                        numProbesN += probeTargetting[nexus->minerals[i]->self];
                        Point3D po = nexus->minerals[i]->pos3D(this);
                        //Color mineralCapacity
#ifdef PROBE_DEBUG
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
#endif
                    }
                }
            }
#ifdef PROBE_DEBUG
            DebugText(this, strprintf("%d/%d", numProbesN, numProbesMaxN), nexusWrap->pos3D(this) + Point3D{ 0,0, 5.5 });
#endif
            numProbes += numProbesN;
            numProbesMax += numProbesMaxN;
        }

        if ((numProbes) < numProbesMax && MacroManager::allActions[UNIT_TYPEID::PROTOSS_NEXUS].size() == 0) {
            MacroManager::addAction(MacroAction(UNIT_TYPEID::PROTOSS_NEXUS, ABILITY_ID::TRAIN_PROBE, false, MacroActionData(), 0, 0));
        }

        onStepProfiler.midLog("oS-probe");

        MacroManager::execute(this);

        onStepProfiler.midLog("oS-macroExec");

        VisibleMap2D::update(this);

        onStepProfiler.midLog("oS-visMap");

        ArmyManager::execute(this, strat);

        onStepProfiler.midLog("oS-armyExec");

        WeaponGrid::update(this);

        onStepProfiler.midLog("oS-weaponGrid");

        if (Observation()->GetGameLoop() > 20) { //TODO: improve for a better ratio on units
            StrategyManager::UnitRatio numUnits;
            StrategyManager::UnitRatioFloat percentageUnits;

            StrategyManager::UnitRatioFloat percentageUnitsStrategy;

            numUnits.zealot = (int8_t)(UnitManager::getSelf(UNIT_TYPEID::PROTOSS_ZEALOT).size());
            numUnits.stalker = (int8_t)(UnitManager::getSelf(UNIT_TYPEID::PROTOSS_STALKER).size());
            numUnits.sentry = (int8_t)(UnitManager::getSelf(UNIT_TYPEID::PROTOSS_SENTRY).size());
            numUnits.adept = (int8_t)(UnitManager::getSelf(UNIT_TYPEID::PROTOSS_ADEPT).size());
            numUnits.darktemplar = (int8_t)(UnitManager::getSelf(UNIT_TYPEID::PROTOSS_DARKTEMPLAR).size());
            numUnits.hightemplar = (int8_t)(UnitManager::getSelf(UNIT_TYPEID::PROTOSS_HIGHTEMPLAR).size());
            numUnits.archon = (int8_t)(UnitManager::getSelf(UNIT_TYPEID::PROTOSS_ARCHON).size());

            numUnits.observer = (int8_t)(UnitManager::getSelf(UNIT_TYPEID::PROTOSS_OBSERVER).size());
            numUnits.immortal = (int8_t)(UnitManager::getSelf(UNIT_TYPEID::PROTOSS_IMMORTAL).size());
            numUnits.warpprism = (int8_t)(UnitManager::getSelf(UNIT_TYPEID::PROTOSS_WARPPRISM).size());
            numUnits.colossus = (int8_t)(UnitManager::getSelf(UNIT_TYPEID::PROTOSS_COLOSSUS).size());
            numUnits.disruptor = (int8_t)(UnitManager::getSelf(UNIT_TYPEID::PROTOSS_DISRUPTOR).size());

            numUnits.pheonix = (int8_t)(UnitManager::getSelf(UNIT_TYPEID::PROTOSS_PHOENIX).size());
            numUnits.oracle = (int8_t)(UnitManager::getSelf(UNIT_TYPEID::PROTOSS_ORACLE).size());
            numUnits.voidray = (int8_t)(UnitManager::getSelf(UNIT_TYPEID::PROTOSS_VOIDRAY).size());
            numUnits.tempest = (int8_t)(UnitManager::getSelf(UNIT_TYPEID::PROTOSS_TEMPEST).size());
            numUnits.carrier = (int8_t)(UnitManager::getSelf(UNIT_TYPEID::PROTOSS_CARRIER).size());
            numUnits.mothership = (int8_t)(UnitManager::getSelf(UNIT_TYPEID::PROTOSS_MOTHERSHIP).size());

            int8_t* numPtr = (int8_t*)&numUnits;
            int8_t* numPtrStrategy = (int8_t*)&(strat.unitRatio);
            float* percentPtr = (float*)&percentageUnits;
            float* percentPtrStrategy = (float*)&percentageUnitsStrategy;

            int totalUnits = 0;
            int totalUnitsStrategy = 0;
            for (int i = 0; i < 18; i++) {
                totalUnits += numPtr[i];
                totalUnitsStrategy += numPtrStrategy[i];
            }

            for (int i = 0; i < 18; i++) {
                percentPtr[i] = ((float)numPtr[i]) / totalUnits;
                percentPtrStrategy[i] = ((float)numPtrStrategy[i]) / totalUnitsStrategy;
            }

            int mindex = -1;
            for (int i = 0; i < 18; i++) {
                if (percentPtrStrategy[i] != 0.0 && ((percentPtrStrategy[i] - percentPtr[i]) > (percentPtrStrategy[mindex] - percentPtr[mindex]) || mindex == -1) && MacroManager::allActions[Aux::UnitCreators[i]].size() == 0) {
                    mindex = i;
                }
            }

            if (mindex != -1 && MacroManager::allActions[Aux::UnitCreators[mindex]].size() == 0) {
                printf("Strategy Profile:\n");
                for (int i = 0; i < 18; i++) {
                    printf("%s   \t%.1f\t%.1f\n", UnitTypeToName(Aux::BuildUnitOrder[i]), percentPtrStrategy[i] * 100, percentPtr[i] * 100);
                }
                printf("%s\n", AbilityTypeToName(Aux::UnitCreationAbility[mindex]));
                MacroManager::addAction( MacroAction( Aux::UnitCreators[mindex], Aux::UnitCreationAbility[mindex] ) );
            }
        }

#ifndef BUILD_FOR_LADDER
        std::vector<ChatMessage> chats = Observation()->GetChatMessages();
        for (int i = 0; i < chats.size(); i++) {
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

                //Actions()->SendChat("Command: " + command);
                //for (std::string argument : arguments) {
                //    Actions()->SendChat(argument);
                //}

                if (command == "s") { //spawn
                    if (spawnCommandMap.find(arguments[0]) != spawnCommandMap.end()) {
                        Actions()->SendChat("Spawning Ally " + spawnCommandMap[arguments[0]]);
                        DebugCreateUnit(this, spawnCommandMap[arguments[0]], Observation()->GetCameraPos(), 1);
                    }
                    else {
                        Actions()->SendChat("Invalid Unit " + spawnCommandMap[arguments[0]]);
                    }
                }
                else if (command == "se") { //spawn enemy
                    if (spawnCommandMap.find(arguments[0]) != spawnCommandMap.end()) {
                        Actions()->SendChat("Spawning Enemy " + spawnCommandMap[arguments[0]]);
                        DebugCreateUnit(this, spawnCommandMap[arguments[0]], Observation()->GetCameraPos(), 2);
                    }
                    else {
                        Actions()->SendChat("Invalid Unit " + spawnCommandMap[arguments[0]]);
                    }
                }
                else if (command == "v") { //show entire map
                    Actions()->SendChat("Showing map");
                    Debug()->DebugShowMap();
                }
                else if (command == "kn") { //kill all neutral
                    Units units = Observation()->GetUnits(sc2::Unit::Alliance::Neutral);
                    for (const Unit* unit : units) {
                        Debug()->DebugKillUnit(unit);
                    }
                    Actions()->SendChat("Killing all neutral");
                }
                else if (command == "m") { //save master bitmap
                    Actions()->SendChat("Saving master bitmap");
                    Aux::saveMasterBitmap("command.bmp");
                }
                else if (command == "dm") { //save main damage bitmap
                    Actions()->SendChat("Saving damage bitmap");
                    WeaponGrid::saveDamageMapEnemyBitmap("damage");
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

        SpatialHashGrid::updateSelf(this);

        onStepProfiler.midLog("oS-SpacSelf");

        SpatialHashGrid::updateEnemy(this);

        onStepProfiler.midLog("oS-SpacEnemy");

        DebugText(this, strprintf("%.3fms %u", lastDT / 1000.0, Observation()->GetGameLoop()));

        Aux::displayExpansions(this);

        MacroManager::displayMacroActions(this);

        std::string selected = "";
        int8_t found = 0;
        std::vector< UnitWrapperMap* > maps = { &UnitManager::self_units, &UnitManager::neutral_units, &UnitManager::enemy_units};
        UnitWrapperPtr select = nullptr;
        for (int i = 0; i < 3; i++) {
            for (auto typeIt = maps[i]->begin(); typeIt != maps[i]->end(); typeIt++) {
                for (auto it = typeIt->second.begin(); it != typeIt->second.end(); it++) {
                    const Unit* selectedUnit = (*it)->getReturn(this);
                    if (selectedUnit != nullptr && selectedUnit->is_selected) {
                        found += 1;
                        if (found > 1) {
                            break;
                        }
                        select = *it;
                        selected = strprintf("%s %llx:\n%s %s\n{%.1f,%.1f,%.1f} H:%.2f\nR:%.1f Radar:%.1f Detect:%.1f\nOnScreen:%c isBlip:%c\n%.1f/%.1fHP %.1f/%.1fSH\n%.1f/%.1fEN\nFLY:%c BUR:%c\nWeaponCD:%.1f\nATK:%x AMR:%x SHIELD:%x",
                            UnitTypeToName(selectedUnit->unit_type), selectedUnit->tag,
                            Aux::DisplayTypeToName(selectedUnit->display_type), Aux::CloakStateToName(selectedUnit->cloak),
                            selectedUnit->pos.x, selectedUnit->pos.y, selectedUnit->pos.z, selectedUnit->facing,
                            selectedUnit->radius, selectedUnit->radar_range, selectedUnit->detect_range,
                            selectedUnit->is_on_screen ? 'Y' : 'N', selectedUnit->is_blip ? 'Y' : 'N',
                            selectedUnit->health, selectedUnit->health_max, selectedUnit->shield, selectedUnit->shield_max,
                            selectedUnit->energy, selectedUnit->energy_max,
                            selectedUnit->is_flying ? 'Y' : 'N', selectedUnit->is_burrowed ? 'Y' : 'N',
                            selectedUnit->weapon_cooldown,
                            selectedUnit->attack_upgrade_level, selectedUnit->armor_upgrade_level, selectedUnit->shield_upgrade_level);

                        if (selectedUnit->alliance == Unit::Alliance::Self && selectedUnit->unit_type == UNIT_TYPEID::PROTOSS_PROBE) {
                            ProbePtr probe = std::static_pointer_cast<Probe>(select);
                            selected += strprintf("\nTarget: %p\n",
                                probe->getTargetTag(this));
                            for (int i = 0; i < probe->buildings.size(); i++) {
                                selected += strprintf("\n%s: %f,%f",
                                    AbilityTypeToName(probe->buildings[i].build), probe->buildings[i].pos.x, probe->buildings[i].pos.y);
                            }
                        }
                    }
                }
                if (found > 1) {
                    break;
                }
            }
            if (found > 1) {
                break;
            }
        }
        if (found == 1) {
            DebugText(this, selected, Point2D(0.01F, 0.21F), Color(132, 67, 135), 8);
        }
        else {
            MacroManager::displayEncodingStack(this);
        }
#ifdef DAMAGEGRID_DEBUG
        if (select != nullptr) {
            Point2D center = select->pos(this); //Observation()->GetCameraPos();
            float wS = center.x - 8;
            if (wS < 0)
                wS = 0;
            float hS = center.y - 8;
            if (hS < 0)
                hS = 0;
            float wE = center.x + 8;
            if (wE > Aux::mapWidth_cache)
                wE = Aux::mapWidth_cache;
            float hE = center.y + 8;
            if (hE > Aux::mapHeight_cache)
                hE = Aux::mapHeight_cache;

            constexpr float BOX_BORDER_S = 0.002F;
            constexpr float BOX_SIZE = (1.0F / WeaponGrid::DAMAGENET_PRECISION);

            for (float w = (int)(wS * WeaponGrid::DAMAGENET_PRECISION) / WeaponGrid::DAMAGENET_PRECISION; w < wE; w += BOX_SIZE) {
                for (float h = (int)(hS * WeaponGrid::DAMAGENET_PRECISION) / WeaponGrid::DAMAGENET_PRECISION; h < hE; h += BOX_SIZE) {
                    Point2D p(w, h);
                    Color c(0, 0, 0);

                    float damage = WeaponGrid::getCellDPS(p, select, this);

                    if (damage != 0) {
                        int dmg = (int)damage;

                        int mult = 255 / maxDamageOnGrid;

                        if (damage < maxDamageOnGrid) {
                            c = { 255, (uint8_t)(dmg * mult), (uint8_t)(dmg * mult) };
                        }
                        else if (damage < (maxDamageOnGrid * 2)) {
                            c = { (uint8_t)(255 - (uint8_t)(dmg * mult)), (uint8_t)(255 - (uint8_t)(dmg * mult)), 255 };
                        }
                        else {
                            c = { 0, 0, 255 };
                        }


                        if (!(c.r == 0 && c.g == 0 && c.b == 0)) {
                            float height = Observation()->TerrainHeight(Point2D{ w + 0.5F, h + 0.5F });

                            DebugBox(this, Point3D(w + BOX_BORDER_S, h + BOX_BORDER_S, height + 0.1F),
                                Point3D(w + BOX_SIZE - BOX_BORDER_S, h + BOX_SIZE - BOX_BORDER_S, height - 0.01F), c);
                        }
                    }

                }
            }
        }
#endif
        

        //SpatialHashGrid::debug(this);

        //VisibleMap2D::debug(this);

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
        DebugText(this, profilestr, Point2D(0.01F, 0.41F), Color(1, 212, 41), 8);

        onStepProfiler.midLog("oS-logging");

        SendDebug(this);

        onStepProfiler.midLog("oS-SendDebug");

        lastDT = onStepProfiler.getFullDT();
    }

    //! Called when a Unit has been created by the player.
    //!< \param unit The created unit.
    void OnUnitCreated(const sc2::Unit* unit_){
        FUNCTION_LOG();
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
        FUNCTION_LOG();
        std::cout << sc2::UnitTypeToName(unit_->unit_type) <<
            "(" << unit_->tag << ") was created E" << std::endl;

        UnitManager::add(UnitManager::enemy_units, unit_, this);
        Aux::loadUnitPlacement(Aux::ENEMY_BUILDINGS, unit_->pos, unit_->unit_type); //TODO: ENEMY UNITS WHEN get() is called WILL CHECK IF THEIR POSITION IS DIFFERENT FROM THEIR OLD POSITION AND IF SO, REWORK THE PATHING GRID
    }

    //!  Called when a neutral unit is created. For example, mineral fields observed for the first time
    //!< \param unit The observed unit.
    virtual void OnNeutralUnitCreated(const sc2::Unit* unit_) {
        FUNCTION_LOG();
        std::cout << sc2::UnitTypeToName(unit_->unit_type) <<
            "(" << unit_->tag << ") was created N" << std::endl;

        UnitManager::add(UnitManager::neutral_units, unit_, this);
    }

    //! Called whenever one of the player's units has been destroyed.
    //!< \param unit The destroyed unit.
    void OnUnitDestroyed(const sc2::Unit* unit_){
        FUNCTION_LOG();
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
        FUNCTION_LOG();
        std::cout << sc2::UpgradeIDToName(id_) << " completed" << std::endl;
    }

    //! Called when the unit in the current observation has lower health or shields than in the previous observation.
    //!< \param unit The damaged unit.
    //!< \param health The change in health (damage is positive)
    //!< \param shields The change in shields (damage is positive)
    virtual void OnUnitDamaged(const sc2::Unit* unit_, float health, float shields) {
        FUNCTION_LOG();
    }

    //! Called when a nydus is placed.
    virtual void OnNydusDetected() {
        FUNCTION_LOG();
    }

    //! Called when a nuclear launch is detected.
    virtual void OnNuclearLaunchDetected() {
        FUNCTION_LOG();
    }

    //! Called for various errors the library can encounter. See ClientError enum for possible errors.
    void OnError(const std::vector<sc2::ClientError>& client_errors, const std::vector<std::string>& protocol_errors = {}){
        FUNCTION_LOG();
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
        FUNCTION_LOG();
        //std::cout << sc2::UnitTypeToName(unit_->unit_type) <<
        //     "(" << unit_->tag << ") is idle" << std::endl;
    }

    //! Called when the unit in the previous step had a build progress less than 1.0 but is greater than or equal to 1.0 in
    // !the current step.
    //!< \param unit The constructed unit.
    void OnBuildingConstructionComplete(const sc2::Unit* building_){
        FUNCTION_LOG();
        std::cout << sc2::UnitTypeToName(building_->unit_type) <<
            "(" << building_->tag << ") constructed" << std::endl;
    }
};
