#pragma once
#include <chrono>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#define lastX 50

using timeus = std::chrono::time_point<std::chrono::steady_clock>;

struct lastfew {
    long long last[lastX] = { 0 };
    int arm = 0;

    void add(long long l) {
        last[arm] = l;
        arm++;
        if (arm == lastX)
            arm = 0;
    }

    long long time() {
        long long t = 0;
        for (int i = 0; i < lastX; i++)
            t += last[i];
        return t / lastX;
    }
};

std::map<std::string, long long> profilerMap;
std::map<std::string, long long> profilerMax;
std::map<std::string, int> profilerCount;
std::map<std::string, lastfew> profilerLast;

class Profiler {
public:
    std::string name;
    timeus mid_time;
    timeus start_time;
    bool enabled;

    Profiler(std::string name_) : enabled(true) {
        name = name_;
        start_time = std::chrono::steady_clock::now();
        mid_time = start_time;
    }

    void disable() {
        enabled = false;
    }

    void enable() {
        enabled = true;
    }

    void subScope() {
        if (!enabled)
            return;
        mid_time = std::chrono::steady_clock::now();
    }

    void addCall(std::string sname, long long dt) {
        if (profilerMap.find(sname) == profilerMap.end()) {
            profilerMap[sname] = dt;
            profilerCount[sname] = 1;
            profilerLast[sname].add(dt);
            profilerMax[sname] = dt;
        }
        else {
            profilerMap[sname] += dt;
            profilerCount[sname] += 1;
            profilerLast[sname].add(dt);
            profilerMax[sname] = std::max(profilerMax[sname], dt);
        }
    }

    void midLog(std::string mid) {
        if (!enabled)
            return;
        timeus now = std::chrono::steady_clock::now();
        long long dt = std::chrono::duration_cast<std::chrono::microseconds>(now - mid_time).count();
        addCall(mid, dt);
        mid_time = now;
    }

    long long getFullDT() {
        timeus now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(now - start_time).count();
    }

    ~Profiler() {
        if (!enabled)
            return;
        timeus now = std::chrono::steady_clock::now();
        long long dt = std::chrono::duration_cast<std::chrono::microseconds>(now - start_time).count();
        addCall("-" + name, dt);
    }
};