#pragma once

#include "Maps.h"

class Conflicts : public Singleton<Conflicts>
{
public:
    static void PrepareDistribution() noexcept;

    static void PrepareDistributionImpl(const Maps::TConflictTestMap& test_map) noexcept;
};
