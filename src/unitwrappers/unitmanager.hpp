#include "unitwrapper.hpp"
#include <iostream>

using UnitWrapperMap = std::map<UnitTypeID, UnitWrappers>;

namespace UnitManager {
    UnitWrapperMap self_units;
    UnitWrapperMap neutral_units;
    UnitWrapperMap enemy_units;

    std::map<UnitTypeID, UnitTypeID> superTypes = {
        {UNIT_TYPEID::PROTOSS_ASSIMILATORRICH, UNIT_TYPEID::PROTOSS_ASSIMILATOR},
        {UNIT_TYPEID::PROTOSS_WARPPRISMPHASING, UNIT_TYPEID::PROTOSS_WARPPRISM},
        {UNIT_TYPEID::PROTOSS_DISRUPTORPHASED, UNIT_TYPEID::PROTOSS_DISRUPTOR},
        {UNIT_TYPEID::PROTOSS_OBSERVERSIEGEMODE, UNIT_TYPEID::PROTOSS_OBSERVER},

        {UNIT_TYPEID::TERRAN_BARRACKSREACTOR, UNIT_TYPEID::TERRAN_REACTOR},
        {UNIT_TYPEID::TERRAN_BARRACKSTECHLAB, UNIT_TYPEID::TERRAN_TECHLAB},
        {UNIT_TYPEID::TERRAN_FACTORYREACTOR, UNIT_TYPEID::TERRAN_REACTOR},
        {UNIT_TYPEID::TERRAN_FACTORYTECHLAB, UNIT_TYPEID::TERRAN_TECHLAB},
        {UNIT_TYPEID::TERRAN_STARPORTREACTOR, UNIT_TYPEID::TERRAN_REACTOR},
        {UNIT_TYPEID::TERRAN_STARPORTTECHLAB, UNIT_TYPEID::TERRAN_TECHLAB},
        {UNIT_TYPEID::TERRAN_REFINERYRICH, UNIT_TYPEID::TERRAN_REFINERY},
        {UNIT_TYPEID::TERRAN_BARRACKSFLYING, UNIT_TYPEID::TERRAN_BARRACKS},
        {UNIT_TYPEID::TERRAN_COMMANDCENTERFLYING, UNIT_TYPEID::TERRAN_COMMANDCENTER},
        {UNIT_TYPEID::TERRAN_ORBITALCOMMAND, UNIT_TYPEID::TERRAN_COMMANDCENTER},
        {UNIT_TYPEID::TERRAN_ORBITALCOMMANDFLYING, UNIT_TYPEID::TERRAN_COMMANDCENTER},
        {UNIT_TYPEID::TERRAN_PLANETARYFORTRESS, UNIT_TYPEID::TERRAN_COMMANDCENTER},
        {UNIT_TYPEID::TERRAN_FACTORYFLYING, UNIT_TYPEID::TERRAN_FACTORY},
        {UNIT_TYPEID::TERRAN_HELLIONTANK, UNIT_TYPEID::TERRAN_HELLION},
        {UNIT_TYPEID::TERRAN_LIBERATORAG, UNIT_TYPEID::TERRAN_LIBERATOR},
        {UNIT_TYPEID::TERRAN_VIKINGASSAULT, UNIT_TYPEID::TERRAN_VIKINGFIGHTER},
        {UNIT_TYPEID::TERRAN_HELLIONTANK, UNIT_TYPEID::TERRAN_HELLION},
        {UNIT_TYPEID::TERRAN_SIEGETANKSIEGED, UNIT_TYPEID::TERRAN_SIEGETANK},
        {UNIT_TYPEID::TERRAN_STARPORTFLYING, UNIT_TYPEID::TERRAN_STARPORT},
        {UNIT_TYPEID::TERRAN_SUPPLYDEPOTLOWERED, UNIT_TYPEID::TERRAN_SUPPLYDEPOT},
        {UNIT_TYPEID::TERRAN_THORAP, UNIT_TYPEID::TERRAN_THOR},
        {UNIT_TYPEID::TERRAN_WIDOWMINEBURROWED, UNIT_TYPEID::TERRAN_WIDOWMINE},

        {UNIT_TYPEID::ZERG_EXTRACTORRICH, UNIT_TYPEID::ZERG_EXTRACTOR},
        {UNIT_TYPEID::ZERG_BANELINGBURROWED, UNIT_TYPEID::ZERG_BANELING},
        {UNIT_TYPEID::ZERG_BANELINGCOCOON, UNIT_TYPEID::ZERG_BANELING},
        {UNIT_TYPEID::ZERG_CHANGELINGMARINE, UNIT_TYPEID::ZERG_CHANGELING},
        {UNIT_TYPEID::ZERG_CHANGELINGMARINESHIELD, UNIT_TYPEID::ZERG_CHANGELING},
        {UNIT_TYPEID::ZERG_CHANGELINGZEALOT, UNIT_TYPEID::ZERG_CHANGELING},
        {UNIT_TYPEID::ZERG_CHANGELINGZERGLING, UNIT_TYPEID::ZERG_CHANGELING},
        {UNIT_TYPEID::ZERG_CHANGELINGZERGLINGWINGS, UNIT_TYPEID::ZERG_CHANGELING},
        {UNIT_TYPEID::ZERG_CREEPTUMORBURROWED, UNIT_TYPEID::ZERG_CREEPTUMOR},
        {UNIT_TYPEID::ZERG_CREEPTUMORQUEEN, UNIT_TYPEID::ZERG_CREEPTUMOR},
        {UNIT_TYPEID::ZERG_DRONEBURROWED, UNIT_TYPEID::ZERG_DRONE},
        {UNIT_TYPEID::ZERG_HYDRALISKBURROWED, UNIT_TYPEID::ZERG_HYDRALISK},
        {UNIT_TYPEID::ZERG_INFESTORBURROWED, UNIT_TYPEID::ZERG_INFESTOR},
        {UNIT_TYPEID::ZERG_LOCUSTMPFLYING, UNIT_TYPEID::ZERG_LOCUSTMP},
        {UNIT_TYPEID::ZERG_LURKERMPEGG, UNIT_TYPEID::ZERG_LURKERMP},
        {UNIT_TYPEID::ZERG_LURKERMPBURROWED, UNIT_TYPEID::ZERG_LURKERMP},
        {UNIT_TYPEID::ZERG_OVERLORDCOCOON, UNIT_TYPEID::ZERG_OVERLORD},
        {UNIT_TYPEID::ZERG_OVERSEERSIEGEMODE, UNIT_TYPEID::ZERG_OVERSEER},
        {UNIT_TYPEID::ZERG_QUEENBURROWED, UNIT_TYPEID::ZERG_QUEEN},
        {UNIT_TYPEID::ZERG_RAVAGERCOCOON, UNIT_TYPEID::ZERG_RAVAGER},
        {UNIT_TYPEID::ZERG_ROACHBURROWED, UNIT_TYPEID::ZERG_ROACH},
        {UNIT_TYPEID::ZERG_SPINECRAWLERUPROOTED, UNIT_TYPEID::ZERG_SPINECRAWLER},
        {UNIT_TYPEID::ZERG_SPORECRAWLERUPROOTED, UNIT_TYPEID::ZERG_SPORECRAWLER},
        {UNIT_TYPEID::ZERG_SWARMHOSTBURROWEDMP, UNIT_TYPEID::ZERG_SWARMHOSTMP},
        {UNIT_TYPEID::ZERG_ULTRALISKBURROWED, UNIT_TYPEID::ZERG_ULTRALISK},
        {UNIT_TYPEID::ZERG_TRANSPORTOVERLORDCOCOON, UNIT_TYPEID::ZERG_OVERLORDTRANSPORT},
        {UNIT_TYPEID::ZERG_ZERGLINGBURROWED, UNIT_TYPEID::ZERG_ZERGLING},
        {UNIT_TYPEID::ZERG_LAIR, UNIT_TYPEID::ZERG_HATCHERY},
        {UNIT_TYPEID::ZERG_HIVE, UNIT_TYPEID::ZERG_HATCHERY},
    };

    UnitTypeID getSuperType(UnitTypeID in) {
        if (superTypes.find(in) == superTypes.end()) {
            return in;
        }
        else {
            return superTypes.at(in);
        }
    }

    void add(UnitWrapperMap& units, const Unit* unit_) {
        UnitTypeID stype = getSuperType(unit_->unit_type);
        units[stype].insert(std::make_shared<UnitWrapper>(unit_, stype));
    }

    void remove(const Unit* unit_) {
        UnitTypeID stype = getSuperType(unit_->unit_type);
        UnitWrapperMap* units;
        if (unit_->alliance == Unit::Alliance::Self) {
            units = &self_units;
        }else if(unit_->alliance == Unit::Alliance::Neutral) {
            units = &neutral_units;
        }else if (unit_->alliance == Unit::Alliance::Enemy) {
            units = &enemy_units;
        }
        else{
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
            printf("NOT REMOVED, YOU FUCKED UP\n");
            throw 5;
        }
    }

    UnitWrappers getSelf(UnitTypeID type) {
        UnitTypeID super = getSuperType(type);
        if (self_units.find(super) == self_units.end()) {
            UnitWrappers empty;
            return empty;
        }
        return self_units[super];
    }

    UnitWrappers getNeutral(UnitTypeID type) {
        UnitTypeID super = getSuperType(type);
        if (neutral_units.find(super) == neutral_units.end()) {
            UnitWrappers empty;
            return empty;
        }
        return neutral_units[super];
    }

    UnitWrappers getEnemy(UnitTypeID type) {
        UnitTypeID super = getSuperType(type);
        if (enemy_units.find(super) == enemy_units.end()) {
            UnitWrappers empty;
            return empty;
        }
        return enemy_units[super];
    }
}