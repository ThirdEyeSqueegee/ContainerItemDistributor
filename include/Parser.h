#pragma once

#include "Maps.h"

class Parser : public Singleton<Parser>
{
public:
    static DistrType ClassifyString(std::string_view str) noexcept;

    static DistrToken Tokenize(const std::string& str, std::string_view to_container, DistrType distr_type) noexcept;

    static void ParseINIs(CSimpleIniA& ini) noexcept;
};
