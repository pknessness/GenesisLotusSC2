#pragma once

#include <unordered_set>

namespace UpgradeManager {
    struct UpgradeIDHashFunction
    {
        size_t operator()(const UpgradeID& id) const
        {
            return (size_t)id;
        }
    };

    struct UpgradeIDEqFunction
    {
        size_t operator()(const UpgradeID& a, const UpgradeID& b) const
        {
            return a == b;
        }
    };

	std::unordered_set<sc2::UpgradeID, UpgradeIDHashFunction, UpgradeIDEqFunction> self_upgrades;
	std::unordered_set<sc2::UpgradeID, UpgradeIDHashFunction, UpgradeIDEqFunction> enemy_upgrades;

	uint8_t self_armor_level_gnd;
	uint8_t self_weapons_level_gnd;
	uint8_t self_armor_level_air;
	uint8_t self_weapons_level_air;
	uint8_t self_shields_level;

	void addSelfUpgrade(UpgradeID id) {
		self_upgrades.insert(id);
        switch ((UPGRADE_ID)id) {
            case UPGRADE_ID::PROTOSSGROUNDWEAPONSLEVEL1:
                self_weapons_level_gnd = 1;
                break;
            case UPGRADE_ID::PROTOSSGROUNDWEAPONSLEVEL2:
                self_weapons_level_gnd = 2;
                break;
            case UPGRADE_ID::PROTOSSGROUNDWEAPONSLEVEL3:
                self_weapons_level_gnd = 3;
                break;
            case UPGRADE_ID::PROTOSSGROUNDARMORSLEVEL1:
                self_armor_level_gnd = 1;
                break;
            case UPGRADE_ID::PROTOSSGROUNDARMORSLEVEL2:
                self_armor_level_gnd = 2;
                break;
            case UPGRADE_ID::PROTOSSGROUNDARMORSLEVEL3:
                self_armor_level_gnd = 3;
                break;
            case UPGRADE_ID::PROTOSSSHIELDSLEVEL1:
                self_shields_level = 1;
                break;
            case UPGRADE_ID::PROTOSSSHIELDSLEVEL2:
                self_shields_level = 2;
                break;
            case UPGRADE_ID::PROTOSSSHIELDSLEVEL3:
                self_shields_level = 3;
                break;
            case UPGRADE_ID::PROTOSSAIRWEAPONSLEVEL1:
                self_weapons_level_air = 1;
                break;
            case UPGRADE_ID::PROTOSSAIRWEAPONSLEVEL2:
                self_weapons_level_air = 2;
                break;
            case UPGRADE_ID::PROTOSSAIRWEAPONSLEVEL3:
                self_weapons_level_air = 3;
                break;
            case UPGRADE_ID::PROTOSSAIRARMORSLEVEL1:
                self_armor_level_air = 1;
                break;
            case UPGRADE_ID::PROTOSSAIRARMORSLEVEL2:
                self_armor_level_air = 2;
                break;
            case UPGRADE_ID::PROTOSSAIRARMORSLEVEL3:
                self_armor_level_air = 3;
                break;
            default:
                // Handle unknown upgrade ID
                break;
        }
	}

	void addEnemysUpgrade(UpgradeID id) {
		enemy_upgrades.insert(id);
	}

    void setEnemyWeapons(UpgradeID id) {
        enemy_upgrades.insert(id);
    }

}