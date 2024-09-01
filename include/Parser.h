#pragma once

#include "Map.h"

class Parser : public Singleton<Parser>
{
public:
    [[nodiscard]] static DistrType ClassifyString(std::string_view s) noexcept;

    [[nodiscard]] static DistrToken Tokenize(std::string s, const std::string& to_container, DistrType distr_type) noexcept;

    static void ParseINIs() noexcept;
};
