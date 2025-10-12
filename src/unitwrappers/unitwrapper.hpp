#pragma once

#include <sc2api/sc2_agent.h>
#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>

#include <unordered_set>
#include <map>
#include "../auxiliary/helpers.hpp"
#include "../auxiliary/primordialstar.hpp"

using namespace sc2;

struct MacroActionData {
    int index;

    std::string name;
    char dependencyFlag;
    UnitTypeID type;

    MacroActionData() : index(0), name(""), dependencyFlag(0), type(UNIT_TYPEID::INVALID) {

    }

    //MacroActionData(int index_, std::string name_ = "", int32_t data1_ = 0, int32_t data2_ = 0) : index(index_), name(name_), data1(data1_), data2(data2_), type(UNIT_TYPEID::INVALID) {

    //}

    MacroActionData(std::string name_, char dependencyFlag_ = 0) : index(-1), name(name_), dependencyFlag(dependencyFlag_), type(UNIT_TYPEID::INVALID) {

    }
};

class UnitWrapper : public std::enable_shared_from_this<UnitWrapper> {
private:
    std::string name;

    UnitTypeID storageType;
    Unit::Alliance team;
    bool isBuilding_;
    CompositionAsTarget comp;

    uint32_t lastCached_loop;
    //cache data
    UnitTypeID recentType_cache;
    float radius_cache;
    bool flying_cache;

    float health_cache;
    float healthMax_cache;
    float shields_cache;
    float shieldsMax_cache;
    float energy_cache;
    float energyMax_cache;

    int32_t attackUpgradeLevel_cache;    //! Level of weapon upgrades.
    int32_t armorUpgradeLevel_cache;    //! Level of armor upgrades.
    int32_t shieldUpgradeLevel_cache;    //! Level of shield upgrades.

    Point3D pos_cache;
    //std::vector<AbilityID> abilities_cache;
    std::vector<AvailableAbility> abilities_cache;

    bool dead; //is dead? (unused probably idk)
    bool isHallucination_; //is hallucination?

    int finished_frames; //what is this?

public:
    Tag self;
    MacroActionData creationData;

    UnitWrapper(const Unit* unit_, UnitTypeID sType) :
        storageType(sType), team(unit_->alliance), self(unit_->tag),
        recentType_cache(unit_->unit_type), radius_cache(unit_->radius), 
        health_cache(unit_->health), healthMax_cache(unit_->health_max),
        shields_cache(unit_->shield), shieldsMax_cache(unit_->shield_max),
        energy_cache(unit_->energy), energyMax_cache(unit_->energy_max), pos_cache(unit_->pos), dead(false),
        attackUpgradeLevel_cache(0), armorUpgradeLevel_cache(0), shieldUpgradeLevel_cache(0),
        finished_frames(0), isBuilding_(unit_->is_building), flying_cache(unit_->is_flying), creationData(), lastCached_loop(0){
        //self = unit_->tag;
        name = "";
    }
    ~UnitWrapper() {
        abilities_cache.clear();
    }

    inline void updateUnitWrapperPtr(const Unit* unit, uint32_t loop) {
        lastCached_loop = loop;

        recentType_cache = unit->unit_type;
        radius_cache = unit->radius;
        flying_cache = unit->is_flying;

        health_cache = unit->health;
        healthMax_cache = unit->health_max;
        shields_cache = unit->shield;
        shieldsMax_cache = unit->shield_max;
        energy_cache = unit->energy;
        energyMax_cache = unit->energy_max;

        attackUpgradeLevel_cache = unit->attack_upgrade_level;
        armorUpgradeLevel_cache = unit->armor_upgrade_level;
        shieldUpgradeLevel_cache = unit->shield_upgrade_level;

        pos_cache = unit->pos;

        if (pos_cache.x == 0) {
            printf("");
        }

        isHallucination_ = unit->is_hallucination;

        name = UnitTypeToName(recentType_cache);

        if (unit->unit_type == UNIT_TYPEID::PROTOSS_COLOSSUS) {
            comp = CompositionAsTarget::Any;
        }
        else if (unit->is_flying) {
            comp = CompositionAsTarget::Air;
        }
        else {
            comp = CompositionAsTarget::Ground;
        }
    }

    inline void get(Agent* const agent) {
        //FUNCTION_LOG();
        uint32_t loop = agent->Observation()->GetGameLoop();
        if (loop != lastCached_loop) {
            const Unit* unit = agent->Observation()->GetUnit(self);
            if (unit != nullptr) {
                updateUnitWrapperPtr(unit, loop);
            }
            else if (unit == nullptr && agent->Observation()->GetVisibility(pos_cache) == Visibility::Visible) {
                pos_cache = { 0.0F, 0.0F, 0.0F };
            }
        }
    }

    inline const Unit* getReturn(Agent* const agent) {
        //FUNCTION_LOG();
        const Unit* unit = agent->Observation()->GetUnit(self);
        if (unit != nullptr) {
            uint32_t loop = agent->Observation()->GetGameLoop();
            if (loop != lastCached_loop) {
                updateUnitWrapperPtr(unit, loop);
            }
        }
        else if (unit == nullptr && agent->Observation()->GetVisibility(pos_cache) == Visibility::Visible) {
            pos_cache = { 0.0F, 0.0F, 0.0F };
        }
        return unit;
    }

    inline Point2D pos(Agent* const agent) {
        get(agent);
        return pos_cache;
    }

    //inline Point2D posCache() {
    //    if (team != Unit::Self) {
    //        throw 7;
    //    }
    //    return pos_cache;
    //}

    inline Point3D pos3D(Agent* const agent) {
        get(agent);
        return pos_cache;
    }

    inline bool isFlying(Agent* const agent) {
        get(agent);
        return flying_cache;
    }

    inline UnitTypeID getActualType(Agent* const agent) {
        get(agent);
        return recentType_cache;
    }

    CompositionAsTarget getCompositionAsTarget(Agent* agent) {
        get(agent);
        return comp;
    }

    inline float radius(Agent* const agent) {
        get(agent);
        return radius_cache;
    }

    inline bool isBuilding() {
        return isBuilding_;
    }

    inline bool isDead() {
        return dead;
    }

    inline bool isHallucination() {
        return isHallucination_;
    }

    virtual void execute(Agent* const agent) {

    }

    UnitTypeID getStorageType() {
        return storageType;
    }

    static inline float getPathLengthGroundAStar(Point2D start, Point2D end, float radius_, Agent* const agent) {
        //return agent->Query()->PathingDistance(start, end);
        return PrimordialStar::getPathLengthAStar(start, end, radius_, agent);
    }

    static inline float getPathLengthAir(Point2D start, Point2D end) {
        return Distance2D(start, end);
    }

    static inline float getPathLengthGroundDijkstra(Point2D start, Point2D end, float radius_, Agent* const agent) {
        //return agent->Query()->PathingDistance(start, end);
        return PrimordialStar::getPathLengthDijkstra(start, end, radius_, agent);
    }

    //virtual std::vector<Point2D> getPath(Agent* const agent, Point2D point) {
    //    const Unit* unit = get(agent);
    //    if (unit == nullptr) return std::vector<Point2D>();
    //    if (unit->is_flying) {
    //        return { unit->pos, point };
    //    }
    //    return UnitWrapper::getPathLengthGroundAStar(unit->pos, point, unit->radius, agent);//->Query()->PathingDistance(unit, point);
    //}

    virtual float getPathLength(Agent* const agent, Point2D point) {
        get(agent);
        if (flying_cache) {
            return getPathLengthAir(pos_cache, point);
        }
        return UnitWrapper::getPathLengthGroundAStar(pos_cache, point, radius_cache, agent);//->Query()->PathingDistance(unit, point);
    }

    virtual std::vector<Point2D> getPathUniversal(Agent* const agent, Point2D point) {
        get(agent);
        if (flying_cache) {
            return { pos_cache, point};
        }
        return PrimordialStar::getPathAStar(pos_cache, point, radius_cache, agent);//->Query()->PathingDistance(unit, point);
    }

    void setDead() {
        dead = true;
    }

    float getHealth(Agent* const agent) {
        get(agent);
        return health_cache;
    }

    float getHealthMax(Agent* const agent) {
        get(agent);
        return healthMax_cache;
    }

    float getShields(Agent* const agent) {
        get(agent);
        return shields_cache;
    }

    float getShieldsMax(Agent* const agent) {
        get(agent);
        return shieldsMax_cache;
    }

    float getEnergy(Agent* const agent) {
        get(agent);
        return energy_cache;
    }

    float getEnergyMax(Agent* const agent) {
        get(agent);
        return energyMax_cache;
    }

    int32_t getAttackUpgradeLevel(Agent* const agent) {
        get(agent);
        return attackUpgradeLevel_cache;
    }

    int32_t getArmorUpgradeLevel(Agent* const agent) {
        get(agent);
        return armorUpgradeLevel_cache;
    }

    int32_t getShieldsUpgradeLevel(Agent* const agent) {
        get(agent);
        return shieldUpgradeLevel_cache;
    }

    bool operator==(const UnitWrapper& other) const
    {
        return self == other.self;
    }
};

typedef std::shared_ptr<UnitWrapper> UnitWrapperPtr;


struct UnitWrappersHashFunction
{
    size_t operator()(const UnitWrapperPtr& unitWrap) const
    {
        return unitWrap->self;
    }
};

struct UnitWrappersEqFunction
{
    size_t operator()(const UnitWrapperPtr& a, const UnitWrapperPtr& b) const
    {
        return a->self == b->self;
    }
};

using UnitWrappers = std::unordered_set<UnitWrapperPtr, UnitWrappersHashFunction, UnitWrappersEqFunction>;