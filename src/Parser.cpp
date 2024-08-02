#include "Parser.h"

#include "Utility.h"

DistrType Parser::ClassifyString(const std::string_view s) noexcept
{
    const auto has_leading_minus{ s.starts_with('-') };
    const auto has_caret{ s.contains('^') };
    const auto bar_count{ std::ranges::count(s, '|') };

    if (!has_leading_minus && !has_caret && bar_count == 1) {
        return DistrType::Add;
    }
    if (has_leading_minus && !has_caret && bar_count == 1) {
        return DistrType::Remove;
    }
    if (has_leading_minus && !has_caret && bar_count == 0) {
        return DistrType::RemoveAll;
    }
    if (!has_leading_minus && has_caret && bar_count == 2) {
        return DistrType::Replace;
    }
    if (!has_leading_minus && has_caret && bar_count <= 1) {
        return DistrType::ReplaceAll;
    }

    return DistrType::Error;
}

DistrToken Parser::Tokenize(const std::string& s, const std::string_view to_container, const DistrType distr_type) noexcept
{
    const auto slash_pos{ s.find('/') };

    if (slash_pos == std::string_view::npos) {
        logger::error("ERROR: Failed to find slash in {}", s);
    }

    const auto filename{ s.substr(slash_pos + 1) };

    switch (distr_type) {
    case DistrType::Add: {
        const auto bar_pos{ s.find('|') };
        const auto identifier{ s.substr(0, bar_pos) };
        const auto count{ Maps::ToUnsignedInt(s.substr(bar_pos + 1)) };

        DistrToken distr_token{ DistrType::Add, filename, to_container.data(), identifier, count, std::nullopt, std::nullopt, Utility::GetChance(s) };

        return distr_token;
    }
    case DistrType::Remove: {
        const auto bar_pos{ s.find('|') };
        const auto identifier{ s.substr(1, bar_pos - 1) };
        const auto count{ Maps::ToUnsignedInt(s.substr(bar_pos + 1)) };

        DistrToken distr_token{ DistrType::Remove, filename, to_container.data(), identifier, count, std::nullopt, std::nullopt, Utility::GetChance(s) };

        return distr_token;
    }
    case DistrType::RemoveAll: {
        DistrToken distr_token{ DistrType::RemoveAll, filename, to_container.data(), s.substr(1), std::nullopt, std::nullopt, std::nullopt, Utility::GetChance(s) };

        return distr_token;
    }
    case DistrType::Replace: {
        const auto caret_pos{ s.find('^') };

        const auto lhs{ s.substr(0, caret_pos) };
        const auto rhs{ s.substr(caret_pos + 1) };

        const auto lhs_bar_pos{ lhs.find('|') };
        const auto rhs_bar_pos{ rhs.find('|') };

        const auto lhs_distr{ lhs.substr(0, lhs_bar_pos) };
        const auto lhs_count{ Maps::ToUnsignedInt(lhs.substr(lhs_bar_pos + 1)) };
        const auto rhs_distr{ rhs.substr(0, rhs_bar_pos) };
        const auto rhs_count{ Maps::ToUnsignedInt(rhs.substr(rhs_bar_pos + 1)) };

        DistrToken distr_token{ DistrType::Replace, filename, to_container.data(), lhs_distr, lhs_count, rhs_distr, rhs_count, Utility::GetChance(s) };

        return distr_token;
    }
    case DistrType::ReplaceAll: {
        const auto caret_pos{ s.find('^') };

        const auto& lhs{ s.substr(0, caret_pos) };
        const auto& rhs{ s.substr(caret_pos + 1) };

        if (const auto rhs_bar_pos{ rhs.find('|') }; rhs_bar_pos != std::string_view::npos) {
            const auto& rhs_distr{ rhs.substr(0, rhs_bar_pos) };
            const auto  rhs_count{ Maps::ToUnsignedInt(rhs.substr(rhs_bar_pos + 1)) };

            DistrToken distr_token{ DistrType::ReplaceAll, filename, to_container.data(), lhs, std::nullopt, rhs_distr, rhs_count, Utility::GetChance(s) };

            return distr_token;
        }
        DistrToken distr_token{ DistrType::ReplaceAll, filename, to_container.data(), lhs, std::nullopt, rhs, std::nullopt, Utility::GetChance(s) };

        return distr_token;
    }
    default:
        break;
    }
    logger::error("ERROR: Failed to tokenize {}", s);

    return { DistrType::Error, filename, to_container.data(), "", std::nullopt, std::nullopt, std::nullopt };
}

void Parser::ParseINIs(CSimpleIniA& ini) noexcept
{
    const std::filesystem::path data_dir{ R"(.\Data)" };
    const auto                  pattern{ L"_CID.ini" };

    if (!exists(data_dir)) {
        logger::error("ERROR: Failed to find Data directory");
        stl::report_and_fail(fmt::format("{}: Failed to find Data directory", SKSE::PluginDeclaration::GetSingleton()->GetName()));
    }

    logger::info(">-------------------------------------------------------------Parsing _CID.ini files...--------------------------------------------------------------<");
    logger::info("");

    std::set<std::filesystem::path> cid_inis;
    for (std::error_code ec{}; const auto& file : std::filesystem::directory_iterator{ data_dir, ec }) {
        if (ec.value()) {
            logger::debug("ERROR CODE: {}", ec.value());
            continue;
        }

        const auto& path{ file.path() };

        if (path.extension() != ".ini") {
            continue;
        }

        const auto filename{ path.filename() };
        const auto filename_str{ filename.string() };
        const auto filename_w{ filename.wstring() };

        if (!filename_w.ends_with(pattern)) {
            continue;
        }

        if (cid_inis.contains(path)) {
            logger::warn("WARNING: Found duplicate _CID.ini file: {}", filename_str);
            continue;
        }

        cid_inis.insert(path);
    }

    for (const auto& f : cid_inis) {
        const auto filename{ f.filename().string() };

        logger::info("Loading config file: {}", filename);

        ini.LoadFile(f.wstring().data());

        CSimpleIniA::TNamesDepend keys{};
        ini.GetAllKeys("General", keys);

        logger::debug("");
        logger::debug("{} has {} keys", filename, keys.size());

        for (const auto& k : keys) {
            CSimpleIniA::TNamesDepend values{};
            ini.GetAllValues("General", k.pItem, values);

            logger::debug("\tKey {} has {} values:", k.pItem, values.size());

            for (const auto& v : values) {
                auto        value{ std::string{ v.pItem } + '/' + filename };
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

                logger::debug("\t\t* {} {}", distr_type, value);

                if (!map_to_use) {
                    logger::error("ERROR: Could not find conflict test map to use");
                    continue;
                }
                if (!map_to_use->contains(k.pItem)) {
                    Maps::TDistrTokenVec new_vec;
                    new_vec.emplace_back(Tokenize(value, k.pItem, distr_type));
                    map_to_use->emplace(k.pItem, new_vec);
                }
                else {
                    map_to_use->at(k.pItem).emplace_back(Tokenize(value, k.pItem, distr_type));
                }
            }
            logger::debug("-------");
        }
        logger::debug("");

        ini.Reset();
    }

    logger::info("");
    logger::info(">----------------------------------------------------------Finished parsing _CID.ini files-----------------------------------------------------------<");
    logger::info("");

    logger::debug("ADD conflict test map:");
    for (const auto& [key, distr_token_vec] : Maps::add_conflict_test_map) {
        logger::debug("\t* Key {}:", key);
        for (const auto& [type, filename, to_identifier, identifier, count, rhs, rhs_count, chance] : distr_token_vec) {
            logger::debug("\t\t+ ADD {} {} to {} with chance {}", count.value_or(0), identifier, to_identifier, chance.value_or(100));
        }
    }

    logger::debug("");
    logger::debug("REMOVE conflict test map:");
    for (const auto& [key, distr_token_vec] : Maps::remove_conflict_test_map) {
        logger::debug("\t* Key {}:", key);
        for (const auto& [type, filename, to_identifier, identifier, count, rhs, rhs_count, chance] : distr_token_vec) {
            if (count.has_value()) {
                logger::debug("\t\t- REMOVE {} {} from {} with chance {}", count.value_or(0), identifier, to_identifier, chance.value_or(100));
            }
            else {
                logger::debug("\t\t- REMOVE ALL {} from {} with chance {}", identifier, to_identifier, chance.value_or(100));
            }
        }
    }

    logger::debug("");
    logger::debug("REPLACE conflict test map:");
    for (const auto& [key, distr_token_vec] : Maps::replace_conflict_test_map) {
        logger::debug("\t* Key {}:", key);
        for (const auto& [type, filename, to_identifier, identifier, count, rhs, rhs_count, chance] : distr_token_vec) {
            if (count.has_value()) {
                logger::debug("\t\t^ REPLACE {} {} with {} {} in {} with chance {}", count.value_or(0), identifier, rhs_count.value_or(0), rhs.value(), to_identifier,
                              chance.value_or(100));
            }
            else {
                logger::debug("\t\t^ REPLACE ALL {} with {} {} in {} with chance {}", identifier, rhs_count.value_or(0), rhs.value(), to_identifier, chance.value_or(100));
            }
        }
    }
}
