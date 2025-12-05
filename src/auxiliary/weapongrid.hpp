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

    bool dmagModified = true;

    //UnitType -> all the weapons id
    //Effect -> the relevant weapon id
    //id -> weapon

    struct DamageSourceID {
        Race race;
        uint8_t weaponIndex;
    };

    static std::map <UnitTypeID, std::vector<DamageSourceID>> unitDamageSources;
    static std::map <EffectID, DamageSourceID> effectDamageSources;

    static Aux::ExtraWeapon emptyWeapon;

    static std::vector < Aux::ExtraWeapon > allWeaponsProtoss;
    static std::vector < Aux::ExtraWeapon > allWeaponsTerran;
    static std::vector < Aux::ExtraWeapon > allWeaponsZerg;

    static uint8_t enemyWeaponUpgrades[STATIC_DAMAGECELL_SIZE];

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

    ////Only if Aux::opponent != Race::Random
    //static std::vector < Aux::ExtraWeapon > *getAllWeapons() {
    //    if (Aux::opponent == Race::Protoss) {
    //        return &allWeaponsProtoss;
    //    }
    //    else if (Aux::opponent == Race::Terran) {
    //        return &allWeaponsTerran;
    //    }
    //    else if (Aux::opponent == Race::Zerg) {
    //        return &allWeaponsZerg;
    //    }
    //    else {
    //        throw 51;
    //        //return nullptr;
    //    }
    //}

        //Only if Aux::opponent != Race::Random
    static int getEnemyWeaponSize() {
        if (Aux::opponent == Race::Protoss) {
            return allWeaponsProtoss.size();
        }
        else if (Aux::opponent == Race::Terran) {
            return allWeaponsTerran.size();
        }
        else if (Aux::opponent == Race::Zerg) {
            return allWeaponsZerg.size();
        }
        else {
            throw 51;
            //return nullptr;
        }
    }

    ////Only if Aux::opponent != Race::Random
    //Aux::ExtraWeapon* getEnemyWeaponPtrFromIndex(uint8_t weaponIndex) {
    //    if (Aux::opponent == Race::Protoss) {
    //        return &(allWeaponsProtoss[weaponIndex]);
    //    }
    //    else if (Aux::opponent == Race::Terran) {
    //        return &(allWeaponsTerran[weaponIndex]);
    //    }
    //    else if (Aux::opponent == Race::Zerg) {
    //        return &(allWeaponsZerg[weaponIndex]);
    //    }
    //    else {
    //        throw 52;
    //        //return Aux::ExtraWeapon();
    //    }
    //}

    //need to handle upgrades per-use of this function
    const Aux::ExtraWeapon getEnemyWeaponFromIndex(uint8_t weaponIndex) {
        assert(weaponIndex < getEnemyWeaponSize());
        if (Aux::opponent == Race::Protoss) {
            return allWeaponsProtoss.at(weaponIndex).applyEnemyUpgrades();
        }
        else if (Aux::opponent == Race::Terran) {
            return allWeaponsTerran.at(weaponIndex).applyEnemyUpgrades();
        }
        else if (Aux::opponent == Race::Zerg) {
            return allWeaponsZerg.at(weaponIndex).applyEnemyUpgrades();
        }
        else {
            throw 53;
            //return Aux::ExtraWeapon();
        }
    }

    //Aux::ExtraWeapon* getSelfWeaponPtrFromIndex(uint8_t weaponIndex) {
    //    return &(allWeaponsProtoss[weaponIndex]);
    //}

    //need to handle upgrades per-use of this function
    const Aux::ExtraWeapon getSelfWeaponFromIndex(uint8_t weaponIndex, uint8_t weaponUpgrades) {
        return allWeaponsProtoss.at(weaponIndex).applySelfUpgrades(weaponUpgrades);
    }

    struct relevantTargetDamageInfo {
        UnitTypeData* targetStats;
        CompositionAsTarget c; 
        float shields; 
        float shieldsMax;
        int32_t shieldsUpgrade; 
        int32_t armorUpgrade; 
        float energy; 
        bool hallucination;
    };

    inline relevantTargetDamageInfo wrapToTargetInfo(UnitWrapperPtr target, Agent* const agent) {
        return { Aux::getStats(target->getActualType(agent), agent), target->getCompositionAsTarget(agent), target->getShields(agent), target->getShieldsMax(agent), target->getShieldsUpgradeLevel(agent), target->getArmorUpgradeLevel(agent), target->getEnergy(agent), target->isHallucination() };
    }
    
    //passing it in as parameters for speed
    //https://liquipedia.net/starcraft2/Damage_Calculation
    float DamageCalculation(const Aux::ExtraWeapon& w, UnitTypeData* targetStats, CompositionAsTarget c, float shields, int32_t shieldsUpgrade, int32_t armorUpgrade, float energy, bool hallucination, Agent* const agent) {
        if (w.type != CompositionAsTarget::Any && c != CompositionAsTarget::Any && w.type != c) {
            return 0;
        }
        float damage = w.damage_;
        for (int b = 0; b < w.damage_bonus.size(); b++) {
            if (w.damage_bonus[b].attribute == Attribute::Invalid) {
                if (w.damage_bonus[b].bonus == 0) {
                    damage += std::max(4.0F, shields);
                }
                else if (w.damage_bonus[b].bonus == 1) {
                    damage += 0.5 * energy;
                }
                else if (w.damage_bonus[b].bonus == 2) {
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
                else if (w.damage_bonus[b].bonus == 3) {
                    damage += std::max(100.0F, shields);
                }
            }
            else {
                for (int a = 0; a < targetStats->attributes.size(); a++) {
                    if (w.damage_bonus[b].attribute == targetStats->attributes[a]) {
                        damage += w.damage_bonus[b].bonus;
                    }
                }
            }
        }
        //TODO: add attack upgrades
        float damage_recieved = damage - (shields > 0 ? shieldsUpgrade : targetStats->armor); //TODO: add defense upgrades

        //TODO: add guardian and hardened shield;
        float hardened = 900;
        float guardian = 0; //see if there is a sentry with guardian within range of this tile
        damage_recieved = std::min(damage_recieved, hardened * (hallucination + 1) + 900 * (w.spell));

        float damage_inflicted = std::max(damage_recieved - (guardian * !w.spell * (w.range > 0.1)), 0.5F);

        float total_damage = 0;
        if (shields > 0) { //using shield armor
            if (damage_inflicted > shields) { //break shields
                float breakthrough = damage_inflicted - shields - targetStats->armor;
                if (breakthrough > 0) {
                    total_damage = shields;
                }
                else {
                    total_damage = damage_inflicted;
                }
            }
            else {
                total_damage = damage_inflicted;
            }
        }
        else {//using regular armor
            total_damage = damage_inflicted;
        }
        //float total_damage = (damage_inflicted > shields) ? (std::max(0.0F, damage_inflicted - shields - targetStats->armor) + ) : damage_inflicted;

        return total_damage * w.attacks;
    }

    inline float DamageCalculation(const Aux::ExtraWeapon& w, relevantTargetDamageInfo info, Agent* const agent) {
        return DamageCalculation(w, info.targetStats, info.c, info.shields, info.shieldsUpgrade, info.armorUpgrade, info.energy, info.hallucination, agent);
    }

    inline float DamageCalculation(const Aux::ExtraWeapon& w, UnitWrapperPtr target, Agent* const agent) {
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
                int size = getEnemyWeaponSize();
                for (int i = 0; i < size; i++) {
                    if (weaponCount[i] != 0) {
                        //TODO: ONLY COUNT WEAPON WHEN HAS ENERGY REQUIRED
                        const Aux::ExtraWeapon w = getEnemyWeaponFromIndex(i);

                        float total_damage = DamageCalculation(w, targetInfo, agent);

                        DPS += (total_damage / w.speed);
                    }
                }
            }
            return DPS;
        }
    };

    static std::shared_ptr < map2d<DamageCell> > damageMap_enemy;
    static std::shared_ptr < map2d<uint8_t> > damageMap_modify;
    static std::shared_ptr < map2d<uint8_t> > damageMap_valid;

    /*
    * BITS 7 6 5 4 3 2 1 0
    * BIT [0]   | reserved
    * BIT [1]   | Shield Battery Heal
    * BIT [2]   | Enemy Cloaked Vision
    * BIT [3]   | Acceleration Zone (35% speedup?)
    * BIT [4:7] | reserved
    */
    static std::shared_ptr < map2d<uint8_t> > damageMap_other;

    enum OtherSourceTags {
        RESERVED_BLANK = 0x0,     // 0000: Nothing
        SHIELDBATTERY_AURA = 0x1,    // 0001: Self Buildings
        ENEMY_CLOAKVISION = 0x2,   // 0010: Enemy Buildings
        ACCELERATION_ZONE = 0x3,  // 0011: Cliff Unpathable
        RESERVED_A = 0x4,    // 0100: Cliff Pathable by Reapers/Colossus
        RESERVED_B = 0x5,          // 0101: Minerals
        RESERVED_C = 0x6,  // 0111: Unpathable Rocks
        RESERVED_D = 0x7,    // 1000: Pathable Rocks
    };

    void otherSourceSet(int i, int j, OtherSourceTags tag) {
        imRef(damageMap_other, i, j) = imRef(damageMap_other, i, j) | (0x1 << uint8_t(tag));
    }

    bool otherSourceGet(int i, int j, OtherSourceTags tag) {
        return imRef(damageMap_other, i, j) & (0x1 << uint8_t(tag));
    }

    void otherSourceReset(int i, int j, OtherSourceTags tag) {
        imRef(damageMap_other, i, j) = imRef(damageMap_other, i, j) & (0x0 << uint8_t(tag));
    }

    static Aux::ExtraWeapon* p;

    static void init(Agent* const agent) {
        damageMap_enemy = std::make_shared<map2d<DamageCell>>(Aux::mapWidth_cache * DAMAGENET_PRECISION, Aux::mapHeight_cache * DAMAGENET_PRECISION, true);
        damageMap_valid = std::make_shared<map2d<uint8_t>>(Aux::mapWidth_cache * DAMAGENET_PRECISION, Aux::mapHeight_cache * DAMAGENET_PRECISION, true);
        damageMap_modify = std::make_shared<map2d<uint8_t>>(Aux::mapWidth_cache * DAMAGENET_PRECISION, Aux::mapHeight_cache * DAMAGENET_PRECISION, true);
        damageMap_other = std::make_shared<map2d<uint8_t>>(Aux::mapWidth_cache * DAMAGENET_PRECISION, Aux::mapHeight_cache * DAMAGENET_PRECISION, true);

        for (int i = 0; i < sizeof(Aux::ArmyUnitsProtoss) / sizeof(UnitTypeID); i++) {
            UnitTypeData* d = Aux::getStats(Aux::ArmyUnitsProtoss[i], agent);
            //if (d->unit_type_id == UNIT_TYPEID::PROTOSS_VOIDRAY) {
            //    continue;
            //}
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

        //p = &(allWeaponsProtoss[17]);

        Aux::ExtraWeapon prismaticBeam(Weapon::TargetType::Any, 6, 1, 6, 0.36F); //void ray main
        prismaticBeam.addDamageBonus(Attribute::Armored, 4); //TODO: Add prismatic alignment
        addDamageSource(UNIT_TYPEID::PROTOSS_VOIDRAY, Race::Protoss, prismaticBeam);

        Aux::ExtraWeapon interceptorAura(Weapon::TargetType::Any, 5, 16, 12, 2.14F); //carrier aura
        addDamageSource(UNIT_TYPEID::PROTOSS_CARRIER, Race::Protoss, interceptorAura);

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
        addDamageSource(UNIT_TYPEID::ZERG_BANELING, Race::Zerg, volatileBurst);
        addDamageSource(UNIT_TYPEID::ZERG_BANELINGBURROWED, Race::Zerg, volatileBurst);

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

        //printf("");
    }

    DamageCell getRawCell(int x, int y) {
        if (imRef(damageMap_valid, x, y)) {
            return imRef(damageMap_enemy, x, y);
        }
        return DamageCell{};
    }

    inline float getRawCellDPS(int x, int y, relevantTargetDamageInfo targetInfo, Agent* const agent) {
        if (targetInfo.shields < targetInfo.shieldsMax && otherSourceGet(x, y, SHIELDBATTERY_AURA)) {
            return getRawCell(x, y).getDPS(targetInfo, agent) - 50; //shield battery is 50 shields per second
        }
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
        if (targetInfo.shields < targetInfo.shieldsMax && otherSourceGet(int(point.x * DAMAGENET_PRECISION), int(point.y * DAMAGENET_PRECISION), SHIELDBATTERY_AURA)) {
            return getCell(point).getDPS(targetInfo, agent) - 50; //shield battery is 50 shields per second
        }
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
            imRef(damageMap_valid, x, y) = 0xFF;
            imRef(damageMap_enemy, x, y).clear();
            imRef(damageMap_enemy, x, y).add(weaponIndex);
        }
    }

    void clearDamageGrid() {
        damageMap_valid->clear();
        damageMap_other->clear();
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

    void setOtherRadius(Point2D pos, OtherSourceTags otherTag, float radius) {
        //Profiler profiler("DamageGridF");
        damageMap_modify->clear();


        int xmin = std::max(int((pos.x - radius) * DAMAGENET_PRECISION), 0);
        int ymin = std::max(int((pos.y - radius) * DAMAGENET_PRECISION), 0);
        int xmax = std::min(int((pos.x + radius) * DAMAGENET_PRECISION), damageMap_modify->width());
        int ymax = std::min(int((pos.y + radius) * DAMAGENET_PRECISION), damageMap_modify->height());

        fillDamageModify(pos, radius);

        for (int i = xmin; i <= xmax; i++) {
            for (int j = ymin; j <= ymax; j++) {
                //DebugLine(agent,Point3D{ (float)(i) / damageNetPrecision, (float)(j) / damageNetPrecision, 0.0F }, Point3D{ (float)(i) / damageNetPrecision, (float)(j) / damageNetPrecision, 13.0F });
                if (imRef(damageMap_modify, i, j)) {
                    otherSourceSet(i, j, otherTag);
                }
            }
        }
    }

    void setEnemyDamageRadius(Point2D pos, uint8_t weaponIndex, float enemyRadius) {
        //Profiler profiler("DamageGridF");
        damageMap_modify->clear();

        const Aux::ExtraWeapon w = getEnemyWeaponFromIndex(weaponIndex);
        float radius = enemyRadius + w.range;

        int xmin = std::max(int((pos.x - radius) * DAMAGENET_PRECISION), 0);
        int ymin = std::max(int((pos.y - radius) * DAMAGENET_PRECISION), 0);
        int xmax = std::min(int((pos.x + radius) * DAMAGENET_PRECISION), damageMap_modify->width());
        int ymax = std::min(int((pos.y + radius) * DAMAGENET_PRECISION), damageMap_modify->height());

        fillDamageModify(pos, radius);

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

        float dps_max;
        for (int i = xmin; i <= xmax; i++) {
            for (int j = ymin; j <= ymax; j++) {
                //DebugLine(agent,Point3D{ (float)(i) / damageNetPrecision, (float)(j) / damageNetPrecision, 0.0F }, Point3D{ (float)(i) / damageNetPrecision, (float)(j) / damageNetPrecision, 13.0F });
                if (imRef(damageMap_modify, i, j)) {
                    //DamageLocation pointDmag = imRef(enemyDamageNet, i, j);
                    float dps = std::max(dps, getRawCellDPS(i, j, targetInfo, agent));
                    if (dps > dps_max) {
                        dps_max = dps;
                    }
                }
            }
        }
        return dps_max;
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
            if (Aux::opponent == Random) {
                UnitTypeData* stats = Aux::getStats(it->first, agent);
                Aux::opponent = stats->race;
            }
            for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
                std::vector<DamageSourceID> weapons = getWeapons((*it2)->getActualType(agent));
                if ((*it2)->isHallucination()/* || (*it2)->getReturn(agent) == nullptr*/) {
                    continue;
                }
                
                if ((*it2)->getActualType(agent) == UNIT_TYPEID::TERRAN_SIEGETANKSIEGED) {
                    printf("");
                } 

                //TODO:
                // add psi storm (more damage the more time it has left, prioritzed more since its constant dmag)
                // add helion line, lurker line, liberator circle
                // add ravager artillery, siege tank AOE

                for (DamageSourceID d : weapons) {
#ifdef BUILD_FOR_LADDER
                    if (Aux::opponent != Aux::getStats((*it2)->getActualType(agent), agent)->race || d.weaponIndex >= getEnemyWeaponSize()) {
                        printf("%u %s: Weapon %d\n", (*it2)->getActualType(agent), UnitTypeToName((*it2)->getActualType(agent)), d.weaponIndex);
                    }
#endif
                    setEnemyDamageRadius((*it2)->pos(agent), d.weaponIndex, (*it2)->radius(agent));
                }
            }
        }

        if (UnitManager::self_units.find(UNIT_TYPEID::PROTOSS_SHIELDBATTERY) != UnitManager::self_units.end()) {
            for (auto it = UnitManager::self_units[UNIT_TYPEID::PROTOSS_SHIELDBATTERY].begin(); it != UnitManager::self_units[UNIT_TYPEID::PROTOSS_SHIELDBATTERY].end(); it++) {
                if ((*it)->getEnergy(agent) > 0) {
                    setOtherRadius((*it)->pos(agent), SHIELDBATTERY_AURA, 6);
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


    constexpr int16_t maxDamageOnGrid = 32;

    void showDamageGrid(UnitWrapperPtr unit, Agent* const agent) {
        relevantTargetDamageInfo TDI = wrapToTargetInfo(unit, agent);
        Aux::gridTemplatePrecise(agent, [TDI, agent](int i, int j) {
            Color c(0,0,0);
            float damage = getRawCellDPS(i, j, TDI, agent);
            int dmg = (int)damage;
            int mult = 255 / maxDamageOnGrid;
            if (damage < maxDamageOnGrid && damage > 0) {
                c = { (uint8_t)(dmg * mult), (uint8_t)(dmg * mult), 255};
            }
            else if (damage < (maxDamageOnGrid * 2) && damage > 0) {
                c = { 255, (uint8_t)(255 - (uint8_t)(dmg * mult)), (uint8_t)(255 - (uint8_t)(dmg * mult)) };
            }
            else if (damage < 0) {
                c = { 0, (uint8_t)(dmg * mult), (uint8_t)(255 - (uint8_t)(dmg * mult)) };
            }
            else {
                c = { 255, 0, 0 };
            }
            return c;
        }, DAMAGENET_PRECISION);
    }


    //https://docs.novatel.com/OEM7/Content/Messages/32_Bit_CRC.htm
    #define CRC32_POLYNOMIAL 0xEDB88320L
    /* --------------------------------------------------------------------------
    Calculate a CRC value to be used by CRC calculation functions.
    -------------------------------------------------------------------------- */
    unsigned long CRC32Value(int i) {
        int j;
        unsigned long ulCRC;
        ulCRC = i;

        for (j = 8; j > 0; j--) {
            if (ulCRC & 1)
                ulCRC = (ulCRC >> 1) ^ CRC32_POLYNOMIAL;
            else
                ulCRC >>= 1;
        }
        return ulCRC;
    }

    /* --------------------------------------------------------------------------
    Calculates the CRC-32 of a block of data all at once
    ulCount - Number of bytes in the data block
    ucBuffer - Data block
    -------------------------------------------------------------------------- */
    unsigned long CalculateBlockCRC32(unsigned long ulCount, unsigned char* ucBuffer) {
        unsigned long ulTemp1;
        unsigned long ulTemp2;
        unsigned long ulCRC = 0;

        while (ulCount-- != 0) {
            ulTemp1 = (ulCRC >> 8) & 0x00FFFFFFL;
            ulTemp2 = CRC32Value(((int)ulCRC ^ *ucBuffer++) & 0xFF);
            ulCRC = ulTemp1 ^ ulTemp2;
        }
        return(ulCRC);
    }

    //needs to be called after initialization of map size
    FILE* createDMAGFile(const char* dmagFileName, std::string mapName, Race opponent) {
        FILE* dmagFile = fopen(dmagFileName, "wb");

        uint8_t header[27] = { 0 }; //4 bytes dmag, 2 bytes size (H, W), 20 bytes map name //1 byte opponent id
        const char* dmag = "DMAG";
        uint8_t sizeX = Aux::mapWidth_cache;
        uint8_t sizeY = Aux::mapHeight_cache;
        memcpy(header, dmag, 4);
        memcpy(header + 4, &sizeX, 1);
        memcpy(header + 5, &sizeY, 1);
        memcpy(header + 6, mapName.c_str(), mapName.size());
        memcpy(header + 26, &opponent, 1);

        fwrite(header, 1, 27, dmagFile);
        return dmagFile;
    }

    unsigned long CRC_prev = 0;

    void updateDMAGFile(FILE* filePtr, uint32_t gameloop) {
        FUNCTION_LOG();
        Profiler p("updateDMAG");
        unsigned long CRC = CalculateBlockCRC32(damageMap_valid->rawSize(), (uint8_t*)damageMap_valid->data);
        p.midLog("CRC");
        if (CRC_prev != CRC) {
            //dmagModified = false;
            p.subScope();
            fwrite(&gameloop, 4, 1, filePtr);
            //for (int i = 0; i < damageMap_enemy->rawSize(); i++) {
            //    uint8_t item = ((uint8_t*)damageMap_enemy->data)[i] ^ damageMap_valid->data[uint32_t(i / sizeof(DamageCell))];
            //    fwrite(&item, 1, 1, filePtr);
            //}
            for (int i = 0; i < damageMap_valid->rawSize(); i ++) {
                if (damageMap_valid) {
                    fwrite(&(damageMap_enemy->data[i]), sizeof(DamageCell), 1, filePtr);
                }
            }
            p.midLog("writeDamageMap");
            CRC_prev = CRC;
        }
    }

    void closeDMAGFile(FILE* filePtr) {
        fclose(filePtr);
    }
}