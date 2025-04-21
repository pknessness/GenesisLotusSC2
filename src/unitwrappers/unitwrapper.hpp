#pragma once

#include <sc2api/sc2_agent.h>
#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>

#include <unordered_set>
#include <map>

using namespace sc2;

struct MacroActionData {
    int index;

    std::string name;
    int32_t data1;
    int32_t data2;

    MacroActionData() : index(0), name(""), data1(0), data2(0) {

    }

    MacroActionData(int index_, std::string name_ = "", int32_t data1_ = 0, int32_t data2_ = 0) : index(index_), name(name_), data1(data1_), data2(data2_) {

    }
};

class UnitWrapper {
private:
    UnitTypeID storageType;
    Unit::Alliance team;

    //cache data
    UnitTypeID recentType_cache;
    float radius_cache;

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

    bool dead;

    int finished_frames;
    
public:
    Tag self;
    bool construction_finished;
    MacroActionData creationData;

    UnitWrapper(const Unit* unit_, UnitTypeID sType) :
        storageType(sType), team(unit_->alliance), self(unit_->tag),
        recentType_cache(unit_->unit_type), radius_cache(unit_->radius), 
        health_cache(unit_->health), healthMax_cache(unit_->health_max),
        shields_cache(unit_->shield), shieldsMax_cache(unit_->shield_max),
        energy_cache(unit_->energy), energyMax_cache(unit_->energy_max), pos_cache(unit_->pos), dead(false),
        attackUpgradeLevel_cache(0), armorUpgradeLevel_cache(0), shieldUpgradeLevel_cache(0),
        finished_frames(0), construction_finished(!unit_->is_building), creationData(){
        //self = unit_->tag;
    }
    ~UnitWrapper() {
        abilities_cache.clear();
    }

    inline const Unit* get(Agent* const agent) {
        const Unit* unit = agent->Observation()->GetUnit(self);
        if (unit != nullptr) {
            recentType_cache = unit->unit_type;
            radius_cache = unit->radius;

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
        }
        return unit;
    }

    inline Point2D pos(Agent* const agent) {
        get(agent);
        return pos_cache;
    }

    inline Point2D posCache() {
        if (team != Unit::Self) {
            throw 7;
        }
        return pos_cache;
    }

    inline Point3D pos3D(Agent* const agent) {
        get(agent);
        return pos_cache;
    }

    virtual void atk(Agent* const agent, Point2D point) {

    }

    virtual void atk(Agent* const agent, UnitWrapper* target) {

    }

    virtual void mov(Agent* const agent, Point2D point) {

    }

    //virtual void execute(Agent* const agent) {

    //}

    void setConstructed(Agent* const agent) {
        if (!construction_finished && finished_frames >= 5) {
            construction_finished = true;
        }
        else if(!construction_finished){
            const Unit* unit = get(agent);
            if (unit != nullptr && unit->build_progress == 1.0) {
                finished_frames += 1;
            }
        }
    }

    virtual std::vector<Point2D> getPath(Agent* const agent, Point2D point) {
        return std::vector<Point2D>();
    }

    UnitTypeID getStorageType() {
        return storageType;
    }

    virtual float getPathLength(Agent* const agent, Point2D point) {
        const Unit* unit = get(agent);
        if (unit == nullptr) return -1;
        return agent->Query()->PathingDistance(unit, point);
    }

    void setDead() {
        dead = true;
    }

    float getHealth(Agent* const agent) {
        const Unit* unit = get(agent);
        if (unit != nullptr) {
            health_cache = unit->health;
        }
        return health_cache;
    }

    float getHealthMax(Agent* const agent) {
        const Unit* unit = get(agent);
        if (unit != nullptr) {
            healthMax_cache = unit->health_max;
        }
        return healthMax_cache;
    }

    float getShields(Agent* const agent) {
        const Unit* unit = get(agent);
        if (unit != nullptr) {
            shields_cache = unit->shield;
        }
        return shields_cache;
    }

    float getShieldsMax(Agent* const agent) {
        const Unit* unit = get(agent);
        if (unit != nullptr) {
            shieldsMax_cache = unit->shield_max;
        }
        return shieldsMax_cache;
    }

    float getEnergy(Agent* const agent) {
        const Unit* unit = get(agent);
        if (unit != nullptr) {
            energy_cache = unit->energy;
        }
        return energy_cache;
    }

    float getEnergyMax(Agent* const agent) {
        const Unit* unit = get(agent);
        if (unit != nullptr) {
            energyMax_cache = unit->energy_max;
        }
        return energyMax_cache;
    }

    int32_t getAttackUpgradeLevel(Agent* const agent) {
        const Unit* unit = get(agent);
        if (unit != nullptr) {
            attackUpgradeLevel_cache = unit->attack_upgrade_level;
        }
        return attackUpgradeLevel_cache;
    }

    int32_t getArmorUpgradeLevel(Agent* const agent) {
        const Unit* unit = get(agent);
        if (unit != nullptr) {
            armorUpgradeLevel_cache = unit->armor_upgrade_level;
        }
        return armorUpgradeLevel_cache;
    }

    int32_t getShieldsUpgradeLevel(Agent* const agent) {
        const Unit* unit = get(agent);
        if (unit != nullptr) {
            shieldUpgradeLevel_cache = unit->shield_upgrade_level;
        }
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