#pragma once

#include "Maps.h"

class Parser : public Singleton<Parser>
{
public:
    static DistrType ClassifyString(std::string_view str) noexcept;

    static void ParseINIs(CSimpleIniA& ini) noexcept;

    static DistrToken Tokenize(const std::string& str, std::string_view to_container) noexcept;
};
