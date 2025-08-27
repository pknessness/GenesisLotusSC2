#pragma once

#include "helpers.hpp"
#include "../unitwrappers/unitmanager.hpp"

namespace DamageGrid {
    constexpr int DAMAGECELL_MULT = 7; //multiplier to map the damage range (9217) to the uint16 range (2^16 - 1)
    constexpr int DAMAGENET_PRECISION = 2; //how many subdivisions to do of each edge of a cell for damagegrid (higher is more expensive by n^2)

	struct DamageCell {
        uint16_t normal_gnd;
        uint16_t bonus_armored_gnd;
        uint16_t bonus_light_gnd;
        uint16_t bonus_biological_gnd;
        uint16_t bonus_mechanical_gnd;
        uint16_t bonus_massive_gnd;
        uint16_t bonus_psionic_gnd;
        uint16_t bonus_shield_gnd;

        uint16_t normal_air;
        uint16_t bonus_armored_air;
        uint16_t bonus_light_air;
        uint16_t bonus_biological_air;
        uint16_t bonus_mechanical_air;
        uint16_t bonus_massive_air;
        uint16_t bonus_psionic_air;
        uint16_t bonus_shield_air;
        //max number can be 9217, and that *7 < 2^16

        void clear() {
            normal_gnd = 0;
            bonus_armored_gnd = 0;
            bonus_light_gnd = 0;
            bonus_biological_gnd = 0;
            bonus_mechanical_gnd = 0;
            bonus_massive_gnd = 0;
            bonus_psionic_gnd = 0;
            bonus_shield_gnd = 0;

            normal_air = 0;
            bonus_armored_air = 0;
            bonus_light_air = 0;
            bonus_biological_air = 0;
            bonus_mechanical_air = 0;
            bonus_massive_air = 0;
            bonus_psionic_air = 0;
            bonus_shield_air = 0;
        }


        DamageCell() {
            clear();
        }

        void add(Weapon w) {
            uint16_t bonus_armored = 0;
            uint16_t bonus_light = 0;
            uint16_t bonus_biological = 0;
            uint16_t bonus_mechanical = 0;
            uint16_t bonus_massive = 0;
            uint16_t bonus_psionic = 0;
            uint16_t bonus_shield = 0;

            for (DamageBonus bonus : w.damage_bonus) {
                uint16_t bonusDMG = (uint16_t)((bonus.bonus * w.attacks * DAMAGECELL_MULT) / (w.speed / timeSpeed));
                if (bonus.attribute == Attribute::Light) {
                    bonus_light += bonusDMG;
                }
                else if (bonus.attribute == Attribute::Armored) {
                    bonus_armored += bonusDMG;
                }
                else if (bonus.attribute == Attribute::Biological) {
                    bonus_biological += bonusDMG;
                }
                else if (bonus.attribute == Attribute::Mechanical) {
                    bonus_mechanical += bonusDMG;
                }
                else if (bonus.attribute == Attribute::Massive) {
                    bonus_massive += bonusDMG;
                }
                else if (bonus.attribute == Attribute::Psionic) {
                    bonus_psionic += bonusDMG;
                }
                else {
                    printf("ATTRIBUTE NOT PROGRAMMED %ud\n", bonus.attribute);
                }
            }

            uint16_t DMG = (uint16_t)((w.damage_ * w.attacks * DAMAGECELL_MULT) / (w.speed / timeSpeed));

            if (w.type == Weapon::TargetType::Air || w.type == Weapon::TargetType::Any) {
                normal_air += DMG;
                bonus_armored_air = bonus_armored;
                bonus_light_air = bonus_light;
                bonus_biological_air = bonus_biological;
                bonus_mechanical_air = bonus_mechanical;
                bonus_massive_air = bonus_massive;
                bonus_psionic_air = bonus_psionic;
                bonus_shield_air = bonus_shield;
            }
            if (w.type == Weapon::TargetType::Ground || w.type == Weapon::TargetType::Any) {
                normal_gnd += DMG;
                bonus_armored_gnd += bonus_armored;
                bonus_light_gnd += bonus_light;
                bonus_biological_gnd += bonus_biological;
                bonus_mechanical_gnd += bonus_mechanical;
                bonus_massive_gnd += bonus_massive;
                bonus_psionic_gnd += bonus_psionic;
                bonus_shield_gnd += bonus_shield;
            }
        }
	};

    static std::shared_ptr < map2d<DamageCell> > damageMap_enemy;
    static std::shared_ptr < map2d<uint8_t> > damageMap_modify;
    static std::shared_ptr < map2d<uint8_t> > damageMap_valid;
    static std::shared_ptr < map2d<uint16_t> > damageMap_heal;

    static void init() {
        damageMap_enemy = std::make_shared<map2d<DamageCell>>(Aux::mapWidth_cache * DAMAGENET_PRECISION, Aux::mapHeight_cache * DAMAGENET_PRECISION, true);
        damageMap_valid = std::make_shared<map2d<uint8_t>>(Aux::mapWidth_cache * DAMAGENET_PRECISION, Aux::mapHeight_cache * DAMAGENET_PRECISION, true);
        damageMap_modify = std::make_shared<map2d<uint8_t>>(Aux::mapWidth_cache * DAMAGENET_PRECISION, Aux::mapHeight_cache * DAMAGENET_PRECISION, true);
        damageMap_heal = std::make_shared<map2d<uint16_t>>(Aux::mapWidth_cache * DAMAGENET_PRECISION, Aux::mapHeight_cache * DAMAGENET_PRECISION, true);
    }

    DamageCell getRawCell(int x, int y) {
        if (imRef(damageMap_valid, x, y)) {
            return imRef(damageMap_enemy, x, y);
        }
        return DamageCell{};
    }

    DamageCell getCell(Point2D point) {
        int x = int(point.x * DAMAGENET_PRECISION);
        int y = int(point.y * DAMAGENET_PRECISION);
        return getRawCell(x, y);
    }

    void addWeaponToRawCell(int x, int y, Weapon w) {
        if (imRef(damageMap_valid, x, y)) {
            imRef(damageMap_enemy, x, y).add(w);
        }
        else {
            imRef(damageMap_valid, x, y) = 1;
            imRef(damageMap_enemy, x, y).clear();
            imRef(damageMap_enemy, x, y).add(w);
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
        for (int i = x; i <= xmax; i++) {
            for (int j = y; j <= ymax; j++) {
                //DebugLine(agent,Point3D{ (float)(i) / damageNetPrecision, (float)(j) / damageNetPrecision, 0.0F }, Point3D{ (float)(i) / damageNetPrecision, (float)(j) / damageNetPrecision, 13.0F });
                float f = Distance2D(pos * DAMAGENET_PRECISION, Point2D{ (float)i,(float)j });
                if (i > 1 && i < damageMap_modify->width() && j > 1 && j < damageMap_modify->height() && f < radius * DAMAGENET_PRECISION) {
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

        for (int p = 0; p < (((radius + 2) * DAMAGENET_PRECISION) * 6); p++) {
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
                    float distPoint = Distance2D(pos, testPoint);
                    if (distPoint < (radius + 0.5F / DAMAGENET_PRECISION)) {
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

    void setEnemyDamageRadius(Point2D pos,  Weapon w) {
        //Profiler profiler("DamageGridF");
        damageMap_modify->clear();

        int xmin = std::max(int((pos.x - w.range) * DAMAGENET_PRECISION), 0);
        int ymin = std::max(int((pos.y - w.range) * DAMAGENET_PRECISION), 0);
        int xmax = std::min(int((pos.x + w.range) * DAMAGENET_PRECISION), damageMap_modify->width());
        int ymax = std::min(int((pos.y + w.range) * DAMAGENET_PRECISION), damageMap_modify->height());

        fillDamageModify(pos, w.range);

        for (int i = xmin; i <= xmax; i++) {
            for (int j = ymin; j <= ymax; j++) {
                //DebugLine(agent,Point3D{ (float)(i) / damageNetPrecision, (float)(j) / damageNetPrecision, 0.0F }, Point3D{ (float)(i) / damageNetPrecision, (float)(j) / damageNetPrecision, 13.0F });
                if (imRef(damageMap_modify, i, j)) {
                    addWeaponToRawCell(i, j, w);
                }
            }
        }
    }

    void update(Agent* agent) {
        clearDamageGrid();
        for (auto it = UnitManager::enemy_units.begin(); it != UnitManager::enemy_units.end(); it++) {
            for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
                if ((*it2)->isHallucination() || (*it2)->get(agent) == nullptr) {
                    continue; 
                }
                std::vector<Weapon> weapons = Aux::getStats((*it2)->getActualType(agent), agent).weapons;
                //TODO:
                // add psi storm (more damage the more time it has left, prioritzed more since its constant dmag)
                // add helion line, lurker line, liberator circle
                // add ravager artillery, tank
                //switch (uint32_t((*it2)->getActualType(agent))) {
                //case (uint32_t(UNIT_TYPEID::PROTOSS_ORACLE)): {
                //    if ((*it2)->get(agent)->energy > Aux::extraWeapons[ABILITY_ID::BEHAVIOR_PULSARBEAMON].energyCostStatic) {
                //        weapons.clear();
                //        weapons.push_back(Aux::extraWeapons[ABILITY_ID::BEHAVIOR_PULSARBEAMON].w);
                //    }
                //} break;
                //case (uint32_t(UNIT_TYPEID::ZERG_LURKERMPBURROWED)): {
                //    weapons.push_back(Aux::extraWeapons[ABILITY_ID::BEHAVIOR_HOLDFIREON_LURKER].w);
                //} break;
                //case (uint32_t(UNIT_TYPEID::ZERG_BANELING)): {
                //    weapons.push_back(Aux::extraWeapons[ABILITY_ID::EFFECT_EXPLODE].w);
                //} break;
                //default: {
                //}
                //}
                for (Weapon w : weapons) {
                    setEnemyDamageRadius((*it2)->pos(agent), w);
                    //printf("%s %Ix set %.1f,%.1f  %.1f,%.1f  %.1f,%.1f\n", UnitTypeToName((*it2)->type), (*it2)->self, d.ground, d.air, d.groundlight, d.airlight, d.groundarmored, d.airarmored);
                }
            }
        }
    }

    void saveDamageMapEnemyBitmap(std::string fileName_without_extension) {
        FUNCTION_LOG();
        saveBitmap(fileName_without_extension + "_main.bmp", damageMap_enemy->width(), damageMap_enemy->height(), [](int i, int j) {
            DamageCell cell = getRawCell(i, j);
            return Color{ (uint8_t)(cell.normal_air / 2),0, (uint8_t)(cell.normal_gnd / 2) };
            });
    }
}