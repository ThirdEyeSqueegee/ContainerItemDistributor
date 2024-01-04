#include "Parser.h"
#include "Utility.h"

DistrType Parser::ClassifyString(const std::string_view str) noexcept
{
    const auto minus{ std::strchr(str.data(), '-') };
    const auto is_leading_minus{ str[0] <=> '-' == 0 };
    const auto caret{ std::strchr(str.data(), '^') };
    const auto bar_count{ std::ranges::count(str, '|') };

    if (!minus && !caret && bar_count <=> 1 == 0)
        return DistrType::Add;
    if (minus && is_leading_minus && !caret && bar_count <=> 1 == 0)
        return DistrType::Remove;
    if (minus && is_leading_minus && !caret && bar_count <=> 0 == 0)
        return DistrType::RemoveAll;
    if (!minus && caret && bar_count <=> 2 == 0)
        return DistrType::Replace;
    if (!minus && caret && bar_count <=> 1 <= 0)
        return DistrType::ReplaceAll;

    return DistrType::Error;
}

void Parser::ParseINIs(CSimpleIniA& ini) noexcept
{
    const std::filesystem::path data_dir{ R"(.\Data)" };
    const auto                  pattern{ L"_CID.ini" };

    if (!exists(data_dir)) {
        logger::error("ERROR: Failed to find Data directory");
        stl::report_and_fail(fmt::format("{}: Failed to find Data directory", SKSE::PluginDeclaration::GetSingleton()->GetName()));
    }

    logger::info(">--------------------------------Parsing _CID.ini files...---------------------------------<");
    logger::info("");

    const auto start_time{ std::chrono::system_clock::now() };

    for (std::error_code ec{}; const auto& file : std::filesystem::directory_iterator{ data_dir, ec }) {
        if (ec.value()) {
            logger::debug("ERROR CODE: {}", ec.value());
            continue;
        }

        if (file.path().extension() != ".ini")
            continue;

        const auto& filepath{ file.path() };
        const auto  filename{ filepath.filename() };
        const auto  filename_cstr{ filepath.filename().c_str() };
        if (!std::wcsstr(filename_cstr, pattern))
            continue;

        if (const auto underscore{ std::wcschr(filename_cstr, '_') }; underscore && std::wcscmp(underscore, pattern) != 0)
            continue;

        logger::info("Loading config file: {}", filename.string());

        ini.LoadFile(filepath.c_str());

        CSimpleIniA::TNamesDepend keys{};
        ini.GetAllKeys("General", keys);

        logger::debug("");
        logger::debug("{} has {} keys", filename.string(), keys.size());

        for (const auto& [key, key_order, key_count] : keys) {
            CSimpleIniA::TNamesDepend values{};
            ini.GetAllValues("General", key, values);

            logger::debug("Key {} has {} values:", key, values.size());

            for (const auto& [val, order, val_count] : values) {
                auto        value{ std::string{ val } + "/" + filename.string() };
                const auto& distr_type{ ClassifyString(value) };

                auto* map_to_use{ &Maps::add_conflict_test_map };
                switch (distr_type) {
                case DistrType::Add:
                    break;
                case DistrType::Remove:
                case DistrType::RemoveAll:
                    map_to_use = &Maps::remove_conflict_test_map;
                    break;
                case DistrType::Replace:
                case DistrType::ReplaceAll:
                    map_to_use = &Maps::replace_conflict_test_map;
                    break;
                default:
                    break;
                }

                logger::debug("\t* {} {}", distr_type, value);

                if (!map_to_use) {
                    logger::error("ERROR: Could not find conflict test map to use");
                    continue;
                }
                if (!map_to_use->contains(key)) {
                    Maps::TDistrTokenVec new_vec;
                    new_vec.emplace_back(Tokenize(value, key));
                    map_to_use->emplace(key, new_vec);
                }
                else {
                    auto& conflict_test_vec{ map_to_use->at(key) };
                    conflict_test_vec.emplace_back(Tokenize(value, key));
                }
            }
            logger::debug("-------");
        }
        ini.Reset();
    }

    const auto elapsed{ std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time) };
    logger::info("");
    logger::info(">-------------------------Finished parsing _CID.ini files in {} us-------------------------<", elapsed.count());
    logger::info("");

    logger::debug("ADD conflict test map:");
    for (const auto& [key, distr_token_vec] : Maps::add_conflict_test_map) {
        logger::debug("\t* Key {}:", key);
        for (const auto& [type, filename, to_identifier, identifier, count, rhs, rhs_count, chance] : distr_token_vec)
            logger::debug("\t\t+ ADD {} {} to {} with chance {}", count.value_or(-1), identifier, to_identifier, chance.value_or(100));
    }

    logger::debug("REMOVE conflict test map:");
    for (const auto& [key, distr_token_vec] : Maps::remove_conflict_test_map) {
        logger::debug("\t* Key {}:", key);
        for (const auto& [type, filename, to_identifier, identifier, count, rhs, rhs_count, chance] : distr_token_vec) {
            if (count.has_value())
                logger::debug("\t\t- REMOVE {} {} from {} with chance {}", count.value(), identifier, to_identifier, chance.value_or(100));
            else
                logger::debug("\t\t- REMOVE ALL {} from {} with chance {}", identifier, to_identifier, chance.value_or(100));
        }
    }

    logger::debug("REPLACE conflict test map:");
    for (const auto& [key, distr_token_vec] : Maps::replace_conflict_test_map) {
        logger::debug("\t* Key {}:", key);
        for (const auto& [type, filename, to_identifier, identifier, count, rhs, rhs_count, chance] : distr_token_vec) {
            if (count.has_value())
                logger::debug("\t\t^ REPLACE {} {} with {} {} in {} with chance {}", count.value(), identifier, rhs_count.value_or(-1), rhs.value(), to_identifier, chance.value_or(100));
            else
                logger::debug("\t\t^ REPLACE ALL {} with {} {} in {} with chance {}", identifier, rhs_count.value_or(-1), rhs.value(), to_identifier, chance.value_or(100));
        }
    }
}

DistrToken Parser::Tokenize(const std::string& str, std::string_view to_container) noexcept
{
    const auto slash_pos{ Maps::GetPos(str, '/') };

    if (slash_pos <=> ULLONG_MAX == 0)
        logger::error("ERROR: Failed to find slash in {}", str);

    const auto filename{ str.substr(slash_pos + 1) };

    switch (ClassifyString(str)) {
    case DistrType::Add: {
        const auto  bar_pos{ Maps::GetPos(str, '|') };
        const auto& identifier{ str.substr(0, bar_pos) };
        const auto  count{ Maps::ToInt(str.substr(bar_pos + 1)) };

        DistrToken distr_token{ DistrType::Add, filename, to_container.data(), identifier, count, std::nullopt, std::nullopt, Utility::GetChance(str)};

        return distr_token;
    }
    case DistrType::Remove: {
        const auto  bar_pos{ Maps::GetPos(str, '|') };
        const auto& identifier{ str.substr(1, bar_pos - 1) };
        const auto  count{ Maps::ToInt(str.substr(bar_pos + 1)) };

        DistrToken distr_token{ DistrType::Remove, filename, to_container.data(), identifier, count, std::nullopt, std::nullopt, Utility::GetChance(str) };

        return distr_token;
    }
    case DistrType::RemoveAll: {
        DistrToken distr_token{ DistrType::RemoveAll, filename, to_container.data(), str.substr(1), std::nullopt, std::nullopt, std::nullopt, Utility::GetChance(str) };

        return distr_token;
    }
    case DistrType::Replace: {
        const auto caret_pos{ Maps::GetPos(str, '^') };

        const auto& lhs{ str.substr(0, caret_pos) };
        const auto& rhs{ str.substr(caret_pos + 1) };

        const auto lhs_bar_pos{ Maps::GetPos(lhs, '|') };
        const auto rhs_bar_pos{ Maps::GetPos(rhs, '|') };

        const auto& lhs_distr{ lhs.substr(0, lhs_bar_pos) };
        const auto  lhs_count{ Maps::ToInt(lhs.substr(lhs_bar_pos + 1)) };
        const auto& rhs_distr{ rhs.substr(0, rhs_bar_pos) };
        const auto  rhs_count{ Maps::ToInt(rhs.substr(rhs_bar_pos + 1)) };

        DistrToken distr_token{ DistrType::Replace, filename, to_container.data(), lhs_distr, lhs_count, rhs_distr, rhs_count, Utility::GetChance(str) };

        return distr_token;
    }
    case DistrType::ReplaceAll: {
        const auto caret_pos{ Maps::GetPos(str, '^') };

        const auto& lhs{ str.substr(0, caret_pos) };
        const auto& rhs{ str.substr(caret_pos + 1) };

        if (const auto rhs_bar_pos{ Maps::GetPos(rhs, '|') }; rhs_bar_pos <=> ULLONG_MAX != 0) {
            const auto& rhs_distr{ rhs.substr(0, rhs_bar_pos) };
            const auto  rhs_count{ Maps::ToInt(rhs.substr(rhs_bar_pos + 1)) };

            DistrToken distr_token{ DistrType::ReplaceAll, filename, to_container.data(), lhs, std::nullopt, rhs_distr, rhs_count, Utility::GetChance(str) };

            return distr_token;
        }
        DistrToken distr_token{ DistrType::ReplaceAll, filename, to_container.data(), lhs, std::nullopt, rhs, std::nullopt, Utility::GetChance(str) };

        return distr_token;
    }
    default:
        break;
    }
    logger::error("ERROR: Failed to tokenize {}", str);

    return { DistrType::Error, filename, to_container.data(), "", std::nullopt, std::nullopt, std::nullopt };
}
