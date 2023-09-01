#pragma once

#include "Maps.h"

class Parser : public Singleton<Parser>
{
public:
    static constexpr DistrType ClassifyString(const std::string_view& str) noexcept;

    static void ParseINIs(CSimpleIniA& ini) noexcept;

    static DistrToken Tokenize(const std::string& str, const std::string& to) noexcept;
};
