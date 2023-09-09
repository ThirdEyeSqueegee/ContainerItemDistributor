#pragma once

#include "Utility.h"

class Distributor : public Singleton<Distributor>
{
public:
    static void Distribute() noexcept;

    static void AddDistribute(const Maps::TDistrVec& distr_vec) noexcept;

    static void RemoveDistribute(const Maps::TDistrVec& distr_vec) noexcept;

    static void ReplaceDistribute(const Maps::TDistrVec& distr_vec) noexcept;
};
