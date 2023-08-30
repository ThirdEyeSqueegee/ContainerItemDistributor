#pragma once

#include "Utility.h"

class Distributor : public Singleton<Distributor>
{
public:
    static void CheckConflicts() noexcept;

    template <typename T>
    static constexpr void BuildConflictMap(const Utility::TConflictTestMap& test_map, T& conflict_map, DistrType type) noexcept;

    static void PrepareDistribution() noexcept;

    static void PrepareAddRemoveDistribution(const Utility::TConflictTestMap& test_map, Utility::TConflictMap& conflict_map, DistrType type) noexcept;

    static void PrepareSwapDistribution(const Utility::TConflictTestMap& test_map, Utility::TSwapConflictMap& conflict_map) noexcept;

    static void Distribute() noexcept;

    static void AddDistribute(const Utility::TAddRemoveMap& add_map) noexcept;

    static void RemoveDistribute(const Utility::TAddRemoveMap& remove_map) noexcept;

    static void SwapDistribute(const Utility::TSwapMap& swap_map) noexcept;
};
