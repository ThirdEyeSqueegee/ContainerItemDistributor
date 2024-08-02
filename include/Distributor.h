#pragma once

#include "Utility.h"

class Distributor : public Singleton<Distributor>
{
public:
    inline static std::set<RE::FormID> processed_refs;

    static void Distribute() noexcept;

    static void AddDistribute(const Maps::TDistrVec& distr_vec) noexcept;

    static void RemoveDistribute(const Maps::TDistrVec& distr_vec) noexcept;

    static void ReplaceDistribute(const Maps::TDistrVec& distr_vec) noexcept;

    static void RuntimeDistribute(RE::TESObjectREFR* a_ref) noexcept;
};
