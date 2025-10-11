#pragma once

#include "helpers.hpp"
#include "../unitwrappers/unitmanager.hpp"

namespace WeaponGrid {
    constexpr int DAMAGECELL_MULT = 7; //multiplier to map the damage range (9217) to the uint16 range (2^16 - 1)
    constexpr int DAMAGENET_PRECISION = 2; //how many subdivisions to do of each edge of a cell for damagegrid (higher is more expensive by n^2)

    //damaging spells:
    //feedback - HT
    //pulsar beam +7 light - Oracle
    //Psi Storm - HT

    //steady targetting +40 Psionic - Ghost
    //EMP (shields only) - Ghost

    //Parasitic bomb - Viper
    //Fungal Growth - Infestor

    constexpr int STATIC_DAMAGECELL_SIZE = 32;
    //struct WeaponSource {
    //    UnitTypeID unitType;
    //    int8_t unitIndex;
    //};

    //int weaponSourceToInt(WeaponSource in) {
    //    return (in.unitType << 8) | in.unitIndex;
    //}

    //WeaponSource intToInbuiltSource(int in) {
    //    return { UnitTypeID(in >> 8), int8_t(in & 0xFF) };
    //}

    //struct DamageSourceID {
    //    UnitTypeID unitType;
    //    int8_t unitIndex;

    //    int damageSourceIndex;
    //};

    ////bool operator< (WeaponSource a, WeaponSource b) { return weaponSourceToInt(a) < weaponSourceToInt(b); }

    //static std::map <UnitTypeID, std::vector<DamageSourceID>> damageSources;
    //static std::map <EffectID, uint8_t> damageEffects;

    //static std::vector < Aux::ExtraWeapon > extraWeapons;

    //static void addDamageSource(UnitTypeID unitType, int8_t unitIndex) {
    //    int index = damageSources.size();
    //    if (index >= STATIC_DAMAGECELL_SIZE) {
    //        printf("STATIC_DAMAGECELL_SIZE is too small %d %d\n", index, STATIC_DAMAGECELL_SIZE);
    //        throw 40;
    //    }
    //    //damageSources.push_back({ unitType, unitIndex, index });
    //    DamageSourceID dS{ unitType, unitIndex, index };
    //    if (damageSources.find(unitType) == damageSources.end()) {
    //        damageSources[unitType] = { dS };
    //    }
    //    else {
    //        damageSources[unitType].push_back(dS);
    //    }
    //    
    //}

    //static void addDamageSource(UnitTypeID unitType, Aux::ExtraWeapon w) {
    //    int index = extraWeapons.size();
    //    int8_t uIndex = -extraWeapons.size();
    //    extraWeapons.push_back(w);
    //    if (index >= STATIC_DAMAGECELL_SIZE) {
    //        printf("STATIC_DAMAGECELL_SIZE is too small %d %d\n", index, STATIC_DAMAGECELL_SIZE);
    //        throw 41;
    //    }
    //    //damageSources.push_back({ unitType, unitIndex, index });
    //    DamageSourceID dS{ unitType, uIndex, index };
    //    if (damageSources.find(unitType) == damageSources.end()) {
    //        damageSources[unitType] = { dS };
    //    }
    //    else {
    //        damageSources[unitType].push_back(dS);
    //    }
    //}

    //static void addDamageEffect(EffectID effectID, Aux::ExtraWeapon w) {
    //    int index = damageEffects.size();
    //    int8_t uIndex = -extraWeapons.size();
    //    extraWeapons.push_back(w);
    //    if (index >= STATIC_DAMAGECELL_SIZE) {
    //        printf("STATIC_DAMAGECELL_SIZE is too small %d %d\n", index, STATIC_DAMAGECELL_SIZE);
    //        throw 41;
    //    }
    //    //damageSources.push_back({ unitType, unitIndex, index });
    //    if (damageSources.find(effectID) == damageSources.end()) {
    //        damageSources[unitType] = { dS };
    //    }
    //    else {
    //        damageSources[unitType].push_back(dS);
    //    }
    //}
    
    //static std::vector < Aux::ExtraWeapon > getWeapons(UnitTypeID unitType, Agent* const agent) {
    //    if (damageSources.find(unitType) == damageSources.end()) {
    //        return std::vector<Aux::ExtraWeapon>();
    //    }
    //    else {
    //        std::vector < Aux::ExtraWeapon > weapons;
    //        std::vector<DamageSourceID> damages = damageSources[unitType];
    //        for (int i = 0; i < damages.size(); i++) {
    //            UnitTypeData unitStats = Aux::getStats(unitType, agent);
    //            if (damages[i].unitIndex >= 0) {
    //                weapons.push_back(Aux::ExtraWeapon(unitStats.weapons[damages[i].unitIndex]));
    //            }
    //            else {
    //                weapons.push_back(extraWeapons[-damages[i].unitIndex]);
    //            }
    //        }
    //    }
    //    
    //}

    //UnitType -> all the weapons id
    //Effect -> the relevant weapon id
    //id -> weapon

    struct DamageSourceID {
        Race race;
        uint8_t weaponIndex;
    };

    static std::map <UnitTypeID, std::vector<DamageSourceID>> unitDamageSources;
    static std::map <EffectID, DamageSourceID> effectDamageSources;

    static Aux::ExtraWeapon* emptyWeapon;

    static std::vector < Aux::ExtraWeapon > allWeaponsProtoss;
    static std::vector < Aux::ExtraWeapon > allWeaponsTerran;
    static std::vector < Aux::ExtraWeapon > allWeaponsZerg;

    static void addDamageSource(UnitTypeID unitType, Race race, Aux::ExtraWeapon w) {
        uint8_t index = 255;
        if (race == Race::Protoss) {
            index = allWeaponsProtoss.size();
            allWeaponsProtoss.push_back(w);
        }
        else if (race == Race::Terran) {
            index = allWeaponsTerran.size();
            allWeaponsTerran.push_back(w);
        }
        else if (race == Race::Zerg) {
            index = allWeaponsZerg.size();
            allWeaponsZerg.push_back(w);
        }
        if (index >= STATIC_DAMAGECELL_SIZE) {
            printf("STATIC_DAMAGECELL_SIZE is too small %d %d\n", index, STATIC_DAMAGECELL_SIZE);
            throw 41;
        }
        if (unitDamageSources.find(unitType) == unitDamageSources.end()) {
            unitDamageSources[unitType] = { {race, index} };
        }
        else {
            unitDamageSources[unitType].push_back({ race, index });
        }
    }
    

    static void addDamageSource(EffectID effectType, Race race, Aux::ExtraWeapon w) {
        uint8_t index = 255;
        if (race == Race::Protoss) {
            index = allWeaponsProtoss.size();
            allWeaponsProtoss.push_back(w);
        }
        else if (race == Race::Terran) {
            index = allWeaponsTerran.size();
            allWeaponsTerran.push_back(w);
        }
        else if (race == Race::Zerg) {
            index = allWeaponsZerg.size();
            allWeaponsZerg.push_back(w);
        }
        if (index >= STATIC_DAMAGECELL_SIZE) {
            printf("STATIC_DAMAGECELL_SIZE is too small %d %d\n", index, STATIC_DAMAGECELL_SIZE);
            throw 41;
        }
        effectDamageSources[effectType] = { race, index };
    }

    std::vector< DamageSourceID> getWeapons(UnitTypeID unitType) {
        if (unitDamageSources.find(unitType) == unitDamageSources.end()) {
            return std::vector< DamageSourceID>();
        }
        else {
            return unitDamageSources[unitType];
        }
    }

    DamageSourceID getWeapon(EffectID effectType) {
        if (effectDamageSources.find(effectType) == effectDamageSources.end()) {
            return { Race::Random, 0 };
        }
        else {
            return effectDamageSources[effectType];
        }
    }

    //Only if Aux::opponent != Race::Random
    static std::vector < Aux::ExtraWeapon > *getAllWeapons() {
        if (Aux::opponent == Race::Protoss) {
            return &allWeaponsProtoss;
        }
        else if (Aux::opponent == Race::Terran) {
            return &allWeaponsTerran;
        }
        else if (Aux::opponent == Race::Zerg) {
            return &allWeaponsZerg;
        }
        else {
            throw 51;
            //return nullptr;
        }
    }

    //Only if Aux::opponent != Race::Random
    Aux::ExtraWeapon* getEnemyWeaponFromIndex(uint8_t weaponIndex) {
        if (Aux::opponent == Race::Protoss) {
            return &(allWeaponsProtoss[weaponIndex]);
        }
        else if (Aux::opponent == Race::Terran) {
            return &(allWeaponsTerran[weaponIndex]);
        }
        else if (Aux::opponent == Race::Zerg) {
            return &(allWeaponsZerg[weaponIndex]);
        }
        else {
            throw 52;
            //return Aux::ExtraWeapon();
        }
    }

    Aux::ExtraWeapon* getSelfWeaponFromIndex(uint8_t weaponIndex) {
        return &(allWeaponsProtoss[weaponIndex]);
    }

    struct relevantTargetDamageInfo {
        UnitTypeData* targetStats;
        CompositionAsTarget c; 
        float shields; 
        int32_t shieldsUpgrade; 
        int32_t armorUpgrade; 
        float energy; 
        bool hallucination;
    };

    inline relevantTargetDamageInfo wrapToTargetInfo(UnitWrapperPtr target, Agent* const agent) {
        return { Aux::getStats(target->getActualType(agent), agent), target->getCompositionAsTarget(agent), target->getShields(agent), target->getShieldsUpgradeLevel(agent), target->getArmorUpgradeLevel(agent), target->getEnergy(agent), target->isHallucination() };
    }
    
    //passing it in as parameters for speed
    //https://liquipedia.net/starcraft2/Damage_Calculation
    float DamageCalculation(Aux::ExtraWeapon* w, UnitTypeData* targetStats, CompositionAsTarget c, float shields, int32_t shieldsUpgrade, int32_t armorUpgrade, float energy, bool hallucination, Agent* const agent) {
        if (w->type != CompositionAsTarget::Any && c != CompositionAsTarget::Any && w->type != c) {
            return 0;
        }
        float damage = w->damage_;
        for (int b = 0; b < w->damage_bonus.size(); b++) {
            if (w->damage_bonus[b].attribute == Attribute::Invalid) {
                if (w->damage_bonus[b].bonus == 0) {
                    damage += std::max(4.0F, shields);
                }
                else if (w->damage_bonus[b].bonus == 1) {
                    damage += 0.5 * energy;
                }
                else if (w->damage_bonus[b].bonus == 2) {
                    bool bio = false;
                    bool psionic = false;
                    for (int a = 0; a < targetStats->attributes.size(); a++) {
                        if (targetStats->attributes[a] == Attribute::Biological) {
                            bio = true;
                        }
                        else if (targetStats->attributes[a] == Attribute::Psionic) {
                            psionic = true;
                        }
                    }
                    if (bio) {
                        damage += 130;
                        if (psionic) {
                            damage += 40;
                        }
                    }
                }
                else if (w->damage_bonus[b].bonus == 3) {
                    damage += std::max(100.0F, shields);
                }
            }
            else {
                for (int a = 0; a < targetStats->attributes.size(); a++) {
                    if (w->damage_bonus[b].attribute == targetStats->attributes[a]) {
                        damage += w->damage_bonus[b].bonus;
                    }
                }
            }
        }
        //TODO: add attack upgrades
        float damage_recieved = damage - (shields > 0 ? shieldsUpgrade : targetStats->armor); //TODO: add defense upgrades

        //TODO: add guardian and hardened shield;
        float hardened = 900;
        float guardian = 0; //see if there is a sentry with guardian within range of this tile
        damage_recieved = std::min(damage_recieved, hardened * (hallucination + 1) + 900 * (w->spell));

        float damage_inflicted = std::max(damage_recieved - (guardian * !w->spell * (w->range > 0.1)), 0.5F);

        float total_damage = (damage_inflicted > shields) ? std::max(0.0F, damage_inflicted - shields - targetStats->armor) : damage_inflicted;

        return total_damage * w->attacks;
    }

    inline float DamageCalculation(Aux::ExtraWeapon* w, relevantTargetDamageInfo info, Agent* const agent) {
        return DamageCalculation(w, info.targetStats, info.c, info.shields, info.shieldsUpgrade, info.armorUpgrade, info.energy, info.hallucination, agent);
    }

    inline float DamageCalculation(Aux::ExtraWeapon* w, UnitWrapperPtr target, Agent* const agent) {
        return DamageCalculation(w, wrapToTargetInfo(target, agent), agent);
    }

    struct DamageCell {
        uint8_t weaponCount[STATIC_DAMAGECELL_SIZE];

        void clear() {
            memset(weaponCount, 0, STATIC_DAMAGECELL_SIZE);
        }


        DamageCell() {
            clear();
        }

        void add(uint8_t weaponID) {
            weaponCount[weaponID]++;
        }

        uint8_t count() {
            uint8_t c = 0;
            for (int i = 0; i < STATIC_DAMAGECELL_SIZE; i++){
                c += weaponCount[i];
            }
            return c;
        }

        float getDPS(relevantTargetDamageInfo targetInfo, Agent* const agent) {
            float DPS = 0;
            if (Aux::opponent != Race::Random) {
                int size = getAllWeapons()->size();
                for (int i = 0; i < size; i++) {
                    if (weaponCount[i] != 0) {
                        //TODO: ONLY COUNT WEAPON WHEN HAS ENERGY REQUIRED
                        Aux::ExtraWeapon* w = getEnemyWeaponFromIndex(i);

                        float total_damage = DamageCalculation(w, targetInfo, agent);

                        DPS += (total_damage / w->speed);
                    }
                }
            }
            return DPS;
        }
    };

    static std::shared_ptr < map2d<DamageCell> > damageMap_enemy;
    static std::shared_ptr < map2d<uint8_t> > damageMap_modify;
    static std::shared_ptr < map2d<uint8_t> > damageMap_valid;
    static std::shared_ptr < map2d<uint16_t> > damageMap_heal;

    static void init(Agent* const agent) {
        damageMap_enemy = std::make_shared<map2d<DamageCell>>(Aux::mapWidth_cache * DAMAGENET_PRECISION, Aux::mapHeight_cache * DAMAGENET_PRECISION, true);
        damageMap_valid = std::make_shared<map2d<uint8_t>>(Aux::mapWidth_cache * DAMAGENET_PRECISION, Aux::mapHeight_cache * DAMAGENET_PRECISION, true);
        damageMap_modify = std::make_shared<map2d<uint8_t>>(Aux::mapWidth_cache * DAMAGENET_PRECISION, Aux::mapHeight_cache * DAMAGENET_PRECISION, true);
        damageMap_heal = std::make_shared<map2d<uint16_t>>(Aux::mapWidth_cache * DAMAGENET_PRECISION, Aux::mapHeight_cache * DAMAGENET_PRECISION, true);

        emptyWeapon = new Aux::ExtraWeapon();

        for (int i = 0; i < sizeof(Aux::ArmyUnitsProtoss) / sizeof(UnitTypeID); i++) {
            UnitTypeData* d = Aux::getStats(Aux::ArmyUnitsProtoss[i], agent);
            for (int w = 0; w < d->weapons.size(); w++) {
                addDamageSource(Aux::ArmyUnitsProtoss[i], Race::Protoss, d->weapons[w]);
            }
        }
        for (int i = 0; i < sizeof(Aux::ArmyUnitsTerran) / sizeof(UnitTypeID); i++) {
            UnitTypeData* d = Aux::getStats(Aux::ArmyUnitsTerran[i], agent);
            for (int w = 0; w < d->weapons.size(); w++) {
                addDamageSource(Aux::ArmyUnitsTerran[i], Race::Terran, d->weapons[w]);
            }
        }
        for (int i = 0; i < sizeof(Aux::ArmyUnitsZerg) / sizeof(UnitTypeID); i++) {
            UnitTypeData* d = Aux::getStats(Aux::ArmyUnitsZerg[i], agent);
            for (int w = 0; w < d->weapons.size(); w++) {
                addDamageSource(Aux::ArmyUnitsZerg[i], Race::Zerg, d->weapons[w]);
            }
        }

        Aux::ExtraWeapon prismaticBeam(Weapon::TargetType::Any, 6, 1, 6, 0.36F); //void ray main
        prismaticBeam.addDamageBonus(Attribute::Armored, 4); //TODO: Add prismatic alignment
        addDamageSource(UNIT_TYPEID::PROTOSS_VOIDRAY, Race::Protoss, prismaticBeam);

        Aux::ExtraWeapon disruptionBeam(Weapon::TargetType::Any, 6, 1, 5, 0.71F); //sentry main
        disruptionBeam.addDamageBonus(Attribute::Invalid, 0); //INVALID 0 IS +4 ON SHIELDS
        addDamageSource(UNIT_TYPEID::PROTOSS_SENTRY, Race::Protoss, disruptionBeam);

        //https://www.reddit.com/r/starcraft/comments/40pl7l/how_far_can_a_disruptors_purification_nova_travel/?rdt=50754
        Aux::ExtraWeapon novaAura(Weapon::TargetType::Any, 100 * 0.5, 1, 13, 21.4F); //disruptor aura //downgrade by 0.5 to make aura less powerful than the actual
        addDamageSource(UNIT_TYPEID::PROTOSS_DISRUPTOR, Race::Protoss, novaAura);
        Aux::ExtraWeapon purificationNova(Weapon::TargetType::Ground, 100, 1, 1.5, EPSILON); //disruptor ball
        addDamageSource(UNIT_TYPEID::PROTOSS_DISRUPTORPHASED, Race::Protoss, purificationNova);

        Aux::ExtraWeapon volatileBurst(Weapon::TargetType::Ground, 16, 1, 2.2, EPSILON); //baneling
        volatileBurst.addDamageBonus(Attribute::Light, 19);
        addDamageSource(UNIT_TYPEID::ZERG_BANELING, Race::Terran, volatileBurst);
        addDamageSource(UNIT_TYPEID::ZERG_BANELINGBURROWED, Race::Terran, volatileBurst);

        Aux::ExtraWeapon ATSLaserBattery(Weapon::TargetType::Ground, 8, 1, 6, 0.16); //battlecruiser gnd
        addDamageSource(UNIT_TYPEID::TERRAN_BATTLECRUISER, Race::Terran, ATSLaserBattery);
        Aux::ExtraWeapon ATALaserBattery(Weapon::TargetType::Air, 5, 1, 6, 0.16); //battlecruiser air
        addDamageSource(UNIT_TYPEID::TERRAN_BATTLECRUISER, Race::Terran, ATALaserBattery);

        //spells
        Aux::ExtraWeapon pulsarBeam(Weapon::TargetType::Ground, 15, 1, 4, 0.61F, { 25, 0.0875 }, true);
        addDamageSource(UNIT_TYPEID::PROTOSS_ORACLE, Race::Protoss, pulsarBeam);

        Aux::ExtraWeapon feedback(Weapon::TargetType::Any, 0, 1, 10, EPSILON, { 50, 0 }, true);
        feedback.addDamageBonus(Attribute::Invalid, 1); //INVALID 1 IS +0.5 PER ENERGY
        addDamageSource(UNIT_TYPEID::PROTOSS_HIGHTEMPLAR, Race::Protoss, feedback);

        Aux::ExtraWeapon stormAura(Weapon::TargetType::Any, 10 * 0.8, 1, 9 + 1.5, 0.41, { 75, 0 }, true); //downgrade by 0.8 to make aura less powerful than the actual
        Aux::ExtraWeapon psiStorm(Weapon::TargetType::Any, 10, 1, 1.5, 0.41, { 0, 0 }, true); 
        addDamageSource(UNIT_TYPEID::PROTOSS_HIGHTEMPLAR, Race::Protoss, stormAura);
        //addDamageSource(EFFECT_ID::PSISTORM, Race::Protoss, psiStorm); //TODO: UNCOMMENT WHEN FIGURE OUT THE WIERD NONCOMPILE ON LINUX BUG

        Aux::ExtraWeapon steadyTargetting(Weapon::TargetType::Any, 0, 1, 10, EPSILON, { 50, 0 }, true);
        steadyTargetting.addDamageBonus(Attribute::Invalid, 2); //INVALID 2 IS +130 ON BIO AND +40 ON PSIONIC
        addDamageSource(UNIT_TYPEID::TERRAN_GHOST, Race::Terran, steadyTargetting);

        Aux::ExtraWeapon EMPRound(Weapon::TargetType::Any, 0, 1, 10 + 1.5, EPSILON, { 75, 0 }, true);
        EMPRound.addDamageBonus(Attribute::Invalid, 3); //INVALID 3 IS +100 ON SHIELDS
        addDamageSource(UNIT_TYPEID::TERRAN_GHOST, Race::Terran, EMPRound);

        Aux::ExtraWeapon parasiticAura(Weapon::TargetType::Air, 1 * 0.5, 1, 8 + 3, 0.017, { 125, 0 }, true); //downgrade by 0.5 to make aura less powerful than the actual
        Aux::ExtraWeapon parasiticBomb(Weapon::TargetType::Air, 1, 1, 3, 0.017, { 0, 0 }, true); //viper poison
        addDamageSource(UNIT_TYPEID::ZERG_VIPER, Race::Zerg, parasiticAura);
        addDamageSource(UNIT_TYPEID::PARASITICBOMBMISSILE, Race::Zerg, parasiticBomb);

        Aux::ExtraWeapon fungalAura(Weapon::TargetType::Any, 1 * 0.5, 1, 8 + 3, 0.017, { 75, 0 }, true); //downgrade by 0.5 to make aura less powerful than the actual
        //Aux::ExtraWeapon fungalGrowth(Weapon::TargetType::Any, 1, 1, 3, 0.017, { 0, 0 }, true); //infestor blob
        addDamageSource(UNIT_TYPEID::ZERG_INFESTOR, Race::Zerg, fungalAura);
        //addDamageSource(UNIT_TYPEID::ZERG_INFESTOR, Race::Zerg, fungalGrowth);

        Aux::ExtraWeapon corrosiveBile(Weapon::TargetType::Any, 60, 1, 1.5, EPSILON);
        //addDamageSource(EFFECT_ID::CORROSIVEBILE, Race::Zerg, corrosiveBile);//TODO: UNCOMMENT WHEN FIGURE OUT THE WIERD NONCOMPILE ON LINUX BUG

        Aux::ExtraWeapon liberatorDefenderSetup(Weapon::TargetType::Any, 75 * 1.5, 1, 10, 1.14); //upgrade by 1.5 to make aura more powerful than the actual
        //addDamageSource(EFFECT_ID::LIBERATORDEFENDERZONESETUP, Race::Terran, liberatorDefenderSetup);//TODO: UNCOMMENT WHEN FIGURE OUT THE WIERD NONCOMPILE ON LINUX BUG
        Aux::ExtraWeapon liberatorDefender(Weapon::TargetType::Any, 75, 1, 10, 1.14);
        //addDamageSource(EFFECT_ID::LIBERATORDEFENDERZONE, Race::Terran, liberatorDefender);//TODO: UNCOMMENT WHEN FIGURE OUT THE WIERD NONCOMPILE ON LINUX BUG

        Aux::ExtraWeapon nukeIN(Weapon::TargetType::Any, 300 * 0.5, 1, 4, EPSILON);
        Aux::ExtraWeapon nukeMID(Weapon::TargetType::Any, 300 * 0.25, 1, 6, EPSILON);
        Aux::ExtraWeapon nukeOUT(Weapon::TargetType::Any, 300 * 0.25, 1, 8, EPSILON);
        //addDamageSource(EFFECT_ID::NUKEDOT, Race::Terran, nukeIN);//TODO: UNCOMMENT WHEN FIGURE OUT THE WIERD NONCOMPILE ON LINUX BUG
        //addDamageSource(EFFECT_ID::NUKEDOT, Race::Terran, nukeMID);//TODO: UNCOMMENT WHEN FIGURE OUT THE WIERD NONCOMPILE ON LINUX BUG
        //addDamageSource(EFFECT_ID::NUKEDOT, Race::Terran, nukeOUT);//TODO: UNCOMMENT WHEN FIGURE OUT THE WIERD NONCOMPILE ON LINUX BUG

        //have i added all other effects,
        //BLINDINGCLOUD = 10,
        //CORROSIVEBILE = 11,
        //GUARDIANSHIELD = 2,
        //INVALID = 0,
        //LIBERATORDEFENDERZONE = 9,
        //LIBERATORDEFENDERZONESETUP = 8,
        //LURKERSPINES = 12,
        //NUKEDOT = 7,
        //PSISTORM = 1,
        //SCANNERSWEEP = 6,
        //TEMPORALFIELD = 4,
        //TEMPORALFIELDGROWING = 3,
        //THERMALLANCE = 5,

        printf("");
    }

    DamageCell getRawCell(int x, int y) {
        if (imRef(damageMap_valid, x, y)) {
            return imRef(damageMap_enemy, x, y);
        }
        return DamageCell{};
    }

    inline float getRawCellDPS(int x, int y, relevantTargetDamageInfo targetInfo, Agent* const agent) {
        return getRawCell(x, y).getDPS(targetInfo, agent);
    }

    //disabled for passthrough for speed
    //inline float getRawCellDPS(int x, int y, UnitWrapperPtr unitWrap, Agent* const agent) {
    //    return getRawCell(x, y).getDPS(unitWrap, agent);
    //}

    inline DamageCell getCell(Point2D point) {
        return getRawCell(int(point.x * DAMAGENET_PRECISION), int(point.y * DAMAGENET_PRECISION));
    }

    inline float getCellDPS(Point2D point, relevantTargetDamageInfo targetInfo, Agent* const agent) {
        return getCell(point).getDPS(targetInfo, agent);
    }

    //disabled for passthrough for speed
    //inline float getCellDPS(Point2D point, UnitWrapperPtr unitWrap, Agent* const agent) {
    //    return getCell(point).getDPS(unitWrap, agent);
    //}

    void addWeaponToRawCell(int x, int y, uint8_t weaponIndex) {
        if (imRef(damageMap_valid, x, y)) {
            imRef(damageMap_enemy, x, y).add(weaponIndex);
        }
        else {
            imRef(damageMap_valid, x, y) = 1;
            imRef(damageMap_enemy, x, y).clear();
            imRef(damageMap_enemy, x, y).add(weaponIndex);
        }
    }

    void clearDamageGrid() {
        damageMap_valid->clear();
    }

    //slower but more reliable on smaller radii
    void fillDamageModifyBase(Point2D pos, float radius) {
        int x = (int)((pos.x - radius) * DAMAGENET_PRECISION);
        int y = (int)((pos.y - radius) * DAMAGENET_PRECISION);
        int xmax = (int)((pos.x + radius) * DAMAGENET_PRECISION);
        int ymax = (int)((pos.y + radius) * DAMAGENET_PRECISION);
        //printf("%d - %d, %d - %d\n", x, xmax, y, ymax);
        imRef(damageMap_modify, int(pos.x * DAMAGENET_PRECISION), int(pos.y * DAMAGENET_PRECISION)) = 1;

        float checkRadius2 = (radius * DAMAGENET_PRECISION) * (radius * DAMAGENET_PRECISION);

        for (int i = x; i <= xmax; i++) {
            for (int j = y; j <= ymax; j++) {
                //DebugLine(agent,Point3D{ (float)(i) / damageNetPrecision, (float)(j) / damageNetPrecision, 0.0F }, Point3D{ (float)(i) / damageNetPrecision, (float)(j) / damageNetPrecision, 13.0F });
                float f = DistanceSquared2D(pos * DAMAGENET_PRECISION, Point2D{ (float)i,(float)j });
                if (i > 1 && i < damageMap_modify->width() && j > 1 && j < damageMap_modify->height() && f < checkRadius2) {
                    imRef(damageMap_modify, i, j) = 1;
                    imRef(damageMap_modify, i, j - 1) = 1;
                    imRef(damageMap_modify, i - 1, j) = 1;
                    imRef(damageMap_modify, i - 1, j - 1) = 1;
                }
            }
        }
    }

    //faster but less reliable on smaller radii
    void fillDamageModify(Point2D pos, float radius) {
        if (radius < 1) {
            fillDamageModifyBase(pos, radius);
            return;
        }
        int center_x = (int)(pos.x * DAMAGENET_PRECISION);
        int center_y = (int)(pos.y * DAMAGENET_PRECISION);

        int xmin = std::max(int((pos.x - radius) * DAMAGENET_PRECISION), 0);
        int ymin = std::max(int((pos.y - radius) * DAMAGENET_PRECISION), 0);
        int xmax = std::min(int((pos.x + radius) * DAMAGENET_PRECISION), damageMap_modify->width());
        int ymax = std::min(int((pos.y + radius) * DAMAGENET_PRECISION), damageMap_modify->height());

        Point2D starting = pos + Point2D{ radius, 0 };
        int operating_x = int(starting.x * DAMAGENET_PRECISION);
        int operating_y = int(starting.y * DAMAGENET_PRECISION);
        if (operating_x >= 0 && operating_x < damageMap_modify->width() && operating_y >= 0 && operating_y < damageMap_modify->height()) {
            imRef(damageMap_modify, operating_x, operating_y) = 1;
        }


        int dir_x = 0;
        int dir_y = 0;
        float checkRadius2 = (radius + 0.5F / DAMAGENET_PRECISION) * (radius + 0.5F / DAMAGENET_PRECISION);
        for (int p = 0; p < (((radius + 2) * DAMAGENET_PRECISION) * 6); p++) { //what is the +2 or *6 for?
            if (operating_x == center_x) {
                if (operating_y > center_y) {
                    dir_x = 1;
                    dir_y = -1;
                }
                else if (operating_y < center_y) {
                    dir_x = -1;
                    dir_y = 1;
                }
                else {
                    printf("CENTER X\n");
                }
            }
            else if (operating_y == center_y) {
                if (operating_x > center_x) {
                    if (dir_x != 0) {
                        dir_x = 2;
                        break;
                    }
                    dir_x = -1;
                    dir_y = -1;
                }
                else if (operating_x < center_x) {
                    dir_x = 1;
                    dir_y = 1;
                }
                else {
                    printf("CENTER Y\n");
                }
            }
            else {
                //printf("CENTER XY %d\n",p);
            }
            int min_x = 0;
            int min_y = 0;
            float dist = -1;
            for (int disp_x : {0, dir_x}) {
                for (int disp_y : {0, dir_y}) {
                    if (disp_x == 0 && disp_y == 0) {
                        continue;
                    }
                    Point2D testPoint{ (operating_x + disp_x + 0.5F) / DAMAGENET_PRECISION, (operating_y + disp_y + 0.5F) / DAMAGENET_PRECISION };
                    float distPoint = DistanceSquared2D(pos, testPoint);
                    if (distPoint < checkRadius2) {
                        if (dist == -1 || dist < distPoint) {
                            dist = distPoint;
                            min_x = disp_x;
                            min_y = disp_y;
                        }
                    }
                }
            }
            operating_x += min_x;
            operating_y += min_y;
            if (operating_x >= 0 && operating_x < damageMap_modify->width() && operating_y >= 0 && operating_y < damageMap_modify->height()) {
                imRef(damageMap_modify, operating_x, operating_y) = 1;
            }

        }

        for (int yi = ymin; yi < ymax; yi++) {
            for (int xi = center_x; xi < xmax; xi++) {
                if (imRef(damageMap_modify, xi, yi)) {
                    break;
                }
                imRef(damageMap_modify, xi, yi) = 1;
            }
            for (int xi = center_x - 1; xi > xmin; xi--) {
                if (imRef(damageMap_modify, xi, yi)) {
                    break;
                }
                imRef(damageMap_modify, xi, yi) = 1;
            }
        }
    }

    void setEnemyDamageRadius(Point2D pos, uint8_t weaponIndex) {
        //Profiler profiler("DamageGridF");
        damageMap_modify->clear();

        Weapon w = *getEnemyWeaponFromIndex(weaponIndex);

        int xmin = std::max(int((pos.x - w.range) * DAMAGENET_PRECISION), 0);
        int ymin = std::max(int((pos.y - w.range) * DAMAGENET_PRECISION), 0);
        int xmax = std::min(int((pos.x + w.range) * DAMAGENET_PRECISION), damageMap_modify->width());
        int ymax = std::min(int((pos.y + w.range) * DAMAGENET_PRECISION), damageMap_modify->height());

        fillDamageModify(pos, w.range);

        for (int i = xmin; i <= xmax; i++) {
            for (int j = ymin; j <= ymax; j++) {
                //DebugLine(agent,Point3D{ (float)(i) / damageNetPrecision, (float)(j) / damageNetPrecision, 0.0F }, Point3D{ (float)(i) / damageNetPrecision, (float)(j) / damageNetPrecision, 13.0F });
                if (imRef(damageMap_modify, i, j)) {
                    addWeaponToRawCell(i, j, weaponIndex);
                }
            }
        }
    }

    float getRadiusMaxDPS(Point2D pos, float radius, UnitWrapperPtr unitWrap, Agent* const agent) {
        //Profiler profiler("DamageGridF");
        damageMap_modify->clear();

        int xmin = std::max(int((pos.x - radius) * DAMAGENET_PRECISION), 0);
        int ymin = std::max(int((pos.y - radius) * DAMAGENET_PRECISION), 0);
        int xmax = std::min(int((pos.x + radius) * DAMAGENET_PRECISION), damageMap_modify->width());
        int ymax = std::min(int((pos.y + radius) * DAMAGENET_PRECISION), damageMap_modify->height());

        fillDamageModify(pos, radius);

        relevantTargetDamageInfo targetInfo = wrapToTargetInfo(unitWrap, agent);

        float dps;
        for (int i = xmin; i <= xmax; i++) {
            for (int j = ymin; j <= ymax; j++) {
                //DebugLine(agent,Point3D{ (float)(i) / damageNetPrecision, (float)(j) / damageNetPrecision, 0.0F }, Point3D{ (float)(i) / damageNetPrecision, (float)(j) / damageNetPrecision, 13.0F });
                if (imRef(damageMap_modify, i, j)) {
                    //DamageLocation pointDmag = imRef(enemyDamageNet, i, j);
                    dps = std::max(dps, getRawCellDPS(i, j, targetInfo, agent));
                }
            }
        }
        return dps;
    }

    float getRadiusAvgDPS(Point2D pos, float radius, relevantTargetDamageInfo targetInfo, Agent* const agent) {
        //Profiler profiler("DamageGridF");
        damageMap_modify->clear();

        int xmin = std::max(int((pos.x - radius) * DAMAGENET_PRECISION), 0);
        int ymin = std::max(int((pos.y - radius) * DAMAGENET_PRECISION), 0);
        int xmax = std::min(int((pos.x + radius) * DAMAGENET_PRECISION), damageMap_modify->width());
        int ymax = std::min(int((pos.y + radius) * DAMAGENET_PRECISION), damageMap_modify->height());

        fillDamageModify(pos, radius);

        float dps = 0;
        int count = 0;
        for (int i = xmin; i <= xmax; i++) {
            for (int j = ymin; j <= ymax; j++) {
                //DebugLine(agent,Point3D{ (float)(i) / damageNetPrecision, (float)(j) / damageNetPrecision, 0.0F }, Point3D{ (float)(i) / damageNetPrecision, (float)(j) / damageNetPrecision, 13.0F });
                if (imRef(damageMap_modify, i, j)) {
                    //DamageLocation pointDmag = imRef(enemyDamageNet, i, j);
                    dps += getRawCellDPS(i, j, targetInfo, agent);
                    count++;
                }
            }
        }
        return dps / count;
    }

    //disabled for passthrough for speed
    //inline float getRadiusAvgDPS(Point2D pos, float radius, UnitWrapperPtr unitWrap, Agent* const agent) {
    //    return getRadiusAvgDPS(pos, radius, wrapToTargetInfo(unitWrap, agent), agent);
    //}

    void update(Agent* agent) {
        clearDamageGrid();
        for (auto it = UnitManager::enemy_units.begin(); it != UnitManager::enemy_units.end(); it++) {
            std::vector<DamageSourceID> weapons = getWeapons(it->first);
            if (Aux::opponent == Random) {
                UnitTypeData* stats = Aux::getStats(it->first, agent);
                Aux::opponent = stats->race;
            }
            for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
                if ((*it2)->isHallucination() || (*it2)->get(agent) == nullptr) {
                    continue;
                }
                
                //TODO:
                // add psi storm (more damage the more time it has left, prioritzed more since its constant dmag)
                // add helion line, lurker line, liberator circle
                // add ravager artillery, siege tank AOE

                for (DamageSourceID d : weapons) {
                    setEnemyDamageRadius((*it2)->pos(agent), d.weaponIndex);
                }
            }
        }
    }

    void saveDamageMapEnemyBitmap(std::string fileName_without_extension) {
        FUNCTION_LOG();
        saveBitmap(fileName_without_extension + "_main.bmp", damageMap_enemy->width(), damageMap_enemy->height(), [](int i, int j) {
            DamageCell cell = getRawCell(i, j);
            uint8_t number = cell.count();
            return Color{ number, number, number };
            });
    }


}