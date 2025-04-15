#pragma once
#include <sc2api/sc2_api.h>

using namespace sc2;

//! Outputs text at the top, left of the screen.
//!< \param out The string of text to display.
//!< \param color (Optional) Color of the text.
void DebugText(Agent* const agent, const std::string& out, Color color = Colors::White) {
#ifndef BUILD_FOR_LADDER
    agent->Debug()->DebugTextOut(out, color);
#endif
}
//! Outputs text at any 2D point on the screen. Coordinate ranges are 0..1 in X and Y.
//!< \param out The string of text to display.
//!< \param pt_virtual_2D The screen position to draw text at.
//!< \param color (Optional) Color of the text.
//!< \param size (Optional) Pixel height of the text.
void DebugText(Agent* const agent, const std::string& out, const Point2D& pt_virtual_2D, Color color = Colors::White,
    uint32_t size = 8) {
#ifndef BUILD_FOR_LADDER
    agent->Debug()->DebugTextOut(out, pt_virtual_2D, color, size);
#endif
}
//! Outputs text at any 3D point in the game world. Map coordinates are used.
//!< \param out The string of text to display.
//!< \param pt3D The world position to draw text at.
//!< \param color (Optional) Color of the text.
//!< \param size (Optional) Pixel height of the text.
void DebugText(Agent* const agent, const std::string& out, const Point3D& pt3D, Color color = Colors::White,
    uint32_t size = 8) {
#ifndef BUILD_FOR_LADDER
    agent->Debug()->DebugTextOut(out, pt3D, color, size);
#endif
}
//! Outputs a line between two 3D points in the game world. Map coordinates are used.
//!< \param p0 The starting position of the line.
//!< \param p1 The ending position of the line.
//!< \param color (Optional) Color of the line.
void DebugLine(Agent* const agent, const Point3D& p0, const Point3D& p1, Color color = Colors::White) {
#ifndef BUILD_FOR_LADDER
    agent->Debug()->DebugLineOut(p0, p1, color);
#endif
}
//! Outputs a box specified as two 3D points in the game world. Map coordinates are used.
//!< \param p0 One corner of the box.
//!< \param p1 The far corner of the box.
//!< \param color (Optional) Color of the lines.
void DebugBox(Agent* const agent, const Point3D& p_min, const Point3D& p_max, Color color = Colors::White) {
#ifndef BUILD_FOR_LADDER
    agent->Debug()->DebugBoxOut(p_min, p_max, color);
#endif
}

//! Outputs a sphere specified as a 3D point in the game world and a radius. Map coordinates are used.
//!< \param p Center of the sphere.
//!< \param r Radius of the sphere.
//!< \param color (Optional) Color of the lines.
void DebugSphere(Agent* const agent, const Point3D& p, float r, Color color = Colors::White) {
#ifndef BUILD_FOR_LADDER
    agent->Debug()->DebugSphereOut(p, r, color);
#endif
}

void SendDebug(Agent* const agent) {
#ifndef BUILD_FOR_LADDER
    agent->Debug()->SendDebug();
#endif
}

std::map<std::string, UnitTypeID> spawnCommandMap = {
    {"zealot", UNIT_TYPEID::PROTOSS_ZEALOT},
    {"stalker", UNIT_TYPEID::PROTOSS_STALKER},
    {"sentry", UNIT_TYPEID::PROTOSS_SENTRY},
    {"adept", UNIT_TYPEID::PROTOSS_ADEPT},
    {"darktemplar", UNIT_TYPEID::PROTOSS_DARKTEMPLAR},
    {"hightemplar", UNIT_TYPEID::PROTOSS_HIGHTEMPLAR},
    {"archon", UNIT_TYPEID::PROTOSS_ARCHON},

    {"observer", UNIT_TYPEID::PROTOSS_OBSERVER},
    {"immortal", UNIT_TYPEID::PROTOSS_IMMORTAL},
    {"warp_prism", UNIT_TYPEID::PROTOSS_WARPPRISM}, //PHASED
    {"colossus", UNIT_TYPEID::PROTOSS_COLOSSUS},
    {"disruptor", UNIT_TYPEID::PROTOSS_DISRUPTOR},

    {"phoenix", UNIT_TYPEID::PROTOSS_PHOENIX},
    {"oracle", UNIT_TYPEID::PROTOSS_ORACLE},
    {"voidray", UNIT_TYPEID::PROTOSS_VOIDRAY},
    {"tempest", UNIT_TYPEID::PROTOSS_TEMPEST},
    {"carrier", UNIT_TYPEID::PROTOSS_CARRIER},

    {"mothership", UNIT_TYPEID::PROTOSS_MOTHERSHIP},

    {"probe", UNIT_TYPEID::PROTOSS_PROBE},
    {"photoncannon", UNIT_TYPEID::PROTOSS_PHOTONCANNON},
    {"shieldbattery", UNIT_TYPEID::PROTOSS_SHIELDBATTERY},

    {"marine", UNIT_TYPEID::TERRAN_MARINE},
    {"marauder", UNIT_TYPEID::TERRAN_MARAUDER},
    {"reaper", UNIT_TYPEID::TERRAN_REAPER},

    {"ghost", UNIT_TYPEID::TERRAN_GHOST},

    {"hellion", UNIT_TYPEID::TERRAN_HELLION}, //TERRAN_HELLIONTANK
    {"widowmine", UNIT_TYPEID::TERRAN_WIDOWMINE},
    {"cyclone", UNIT_TYPEID::TERRAN_CYCLONE},
    {"siegetank", UNIT_TYPEID::TERRAN_SIEGETANK}, //TERRAN_SIEGETANKSIEGED

    {"thor", UNIT_TYPEID::TERRAN_THOR}, //TERRAN_THORAP

    {"viking", UNIT_TYPEID::TERRAN_VIKINGFIGHTER}, //TERRAN_VIKINGASSAULT
    {"medivac", UNIT_TYPEID::TERRAN_MEDIVAC},
    {"liberator", UNIT_TYPEID::TERRAN_LIBERATOR}, //TERRAN_LIBERATORAG
    {"raven", UNIT_TYPEID::TERRAN_RAVEN},
    {"autoturret", UNIT_TYPEID::TERRAN_AUTOTURRET},
    {"banshee", UNIT_TYPEID::TERRAN_BANSHEE},

    {"battlecruiser", UNIT_TYPEID::TERRAN_BATTLECRUISER},

    {"scv", UNIT_TYPEID::TERRAN_SCV},
    {"mule", UNIT_TYPEID::TERRAN_MULE},
    {"missileturret", UNIT_TYPEID::TERRAN_MISSILETURRET},
    {"pf", UNIT_TYPEID::TERRAN_PLANETARYFORTRESS},
    {"bunker", UNIT_TYPEID::TERRAN_BUNKER},
    {"kd8charge", UNIT_TYPEID::TERRAN_KD8CHARGE},

    {"zergling", UNIT_TYPEID::ZERG_ZERGLING},
    {"queen", UNIT_TYPEID::ZERG_QUEEN},

    {"roach", UNIT_TYPEID::ZERG_ROACH},
    {"ravager", UNIT_TYPEID::ZERG_RAVAGER},

    {"baneling", UNIT_TYPEID::ZERG_BANELING},

    {"hydralisk", UNIT_TYPEID::ZERG_HYDRALISK},

    {"infestor", UNIT_TYPEID::ZERG_INFESTOR},
    {"swarmhost", UNIT_TYPEID::ZERG_SWARMHOSTMP},
    {"locust", UNIT_TYPEID::ZERG_LOCUSTMP},

    {"mutalisk", UNIT_TYPEID::ZERG_MUTALISK},
    {"corruptor", UNIT_TYPEID::ZERG_CORRUPTOR},

    {"lurker", UNIT_TYPEID::ZERG_LURKERMP},

    {"viper", UNIT_TYPEID::ZERG_VIPER},

    {"nydus", UNIT_TYPEID::ZERG_NYDUSCANAL},

    {"ultralisk", UNIT_TYPEID::ZERG_ULTRALISK},

    {"broodlord", UNIT_TYPEID::ZERG_BROODLORD},
    {"broodling", UNIT_TYPEID::ZERG_BROODLING},

    {"drone", UNIT_TYPEID::ZERG_DRONE},
    {"overlord", UNIT_TYPEID::ZERG_OVERLORD},
    {"overlordtransport", UNIT_TYPEID::ZERG_OVERLORDTRANSPORT},
    {"overseer", UNIT_TYPEID::ZERG_OVERSEER},

    {"spinecrawler", UNIT_TYPEID::ZERG_SPINECRAWLER},
    {"sporecrawler", UNIT_TYPEID::ZERG_SPORECRAWLER},
};

void DebugCreateUnit(Agent* const agent, UnitTypeID unit_type, const Point2D& p, uint32_t player_id = 1, uint32_t count = 1) {
#ifndef BUILD_FOR_LADDER
    agent->Debug()->DebugCreateUnit(unit_type, p, player_id, count);
#endif
}