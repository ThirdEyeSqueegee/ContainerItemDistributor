#pragma once

#include "Map.h"

class Parser : public Singleton<Parser>
{
public:
    static DistrType ClassifyString(std::string_view s) noexcept;

    static DistrToken Tokenize(const std::string& s, const std::string& to_container, DistrType distr_type) noexcept;

    static void ParseINIs(CSimpleIniA& ini) noexcept;
};
