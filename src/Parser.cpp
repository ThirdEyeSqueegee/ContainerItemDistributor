#include "Parser.h"

constexpr DistrType Parser::ClassifyString(const std::string_view& str) noexcept
{
    const auto minus{ str.find('-') };
    const auto caret{ str.find('^') };
    const auto bar_count{ std::ranges::count(str, '|') };

    if (!is_present(minus) && !is_present(caret) && bar_count <=> 1 == 0)
        return DistrType::Add;
    if (is_present(minus) && !is_present(caret) && bar_count <=> 1 == 0)
        return DistrType::Remove;
    if (is_present(minus) && !is_present(caret) && bar_count <=> 0 == 0)
        return DistrType::RemoveAll;
    if (!is_present(minus) && is_present(caret) && bar_count <=> 2 == 0)
        return DistrType::Replace;
    if (!is_present(minus) && is_present(caret) && bar_count <=> 1 == 0)
        return DistrType::ReplaceAll;

    return DistrType::Error;
}

void Parser::ParseINIs(CSimpleIniA& ini) noexcept
{
    const std::filesystem::path data_dir{ R"(.\Data)" };
    const std::regex            p{ "_CID.ini$" };

    if (!exists(data_dir))
    {
        logger::error("ERROR: Failed to find Data directory");
        stl::report_and_fail(fmt::format("{}: Failed to find Data directory", SKSE::PluginDeclaration::GetSingleton()->GetName()));
    }

    for (std::error_code ec{}; const auto& file : std::filesystem::directory_iterator{ data_dir, ec })
    {
        if (ec.value())
        {
            logger::debug("ERROR CODE: {}", ec.value());
            continue;
        }

        const auto filepath{ file.path().string() };
        if (!std::regex_search(filepath, p))
            continue;

        logger::info("Loading config file: {}", filepath);

        ini.LoadFile(filepath.c_str());

        CSimpleIniA::TNamesDepend keys{};
        ini.GetAllKeys("General", keys);

        const auto ini_file_name{ file.path().filename().string() };

        logger::debug("{} has {} keys", ini_file_name, keys.size());

        for (const auto& [key, key_order, key_count] : keys)
        {
            CSimpleIniA::TNamesDepend values{};
            ini.GetAllValues("General", key, values);

            logger::debug("Key {} has {} values", key, values.size());

            for (const auto& [val, order, val_count] : values)
            {
                auto       value{ std::string{ val } + "/" + ini_file_name };
                const auto distr_type = ClassifyString(value);

                auto* map_to_use = &Maps::add_conflict_test_map;
                switch (distr_type)
                {
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

                if (!map_to_use)
                {
                    logger::error("ERROR: Could not find conflict test map to use");
                    continue;
                }
                if (!map_to_use->contains(key))
                {
                    Maps::TDistrTokenVec new_vec;
                    new_vec.emplace_back(Tokenize(value, key));
                    map_to_use->emplace(key, new_vec);
                }
                else
                {
                    auto& conflict_test_vec{ map_to_use->at(key) };
                    conflict_test_vec.emplace_back(Tokenize(value, key));
                }
            }
        }
        ini.Reset();
    }

    logger::debug("Finished building conflict test maps\n");

    logger::debug("ADD conflict test map:");
    for (const auto& [key, distr_token_vec] : Maps::add_conflict_test_map)
    {
        logger::debug("\t* Key {}:", key);
        for (const auto& [type, filename, to_identifier, identifier, count, rhs, rhs_count] : distr_token_vec)
            logger::debug("\t\t+ ADD {} {} to {}", count.value_or(-1), identifier, to_identifier);
    }

    logger::debug("REMOVE conflict test map:");
    for (const auto& [key, distr_token_vec] : Maps::remove_conflict_test_map)
    {
        logger::debug("\t* Key {}:", key);
        for (const auto& [type, filename, to_identifier, identifier, count, rhs, rhs_count] : distr_token_vec)
        {
            if (count.has_value())
                logger::debug("\t\t- REMOVE {} {} from {}", count.value(), identifier, to_identifier);
            else
                logger::debug("\t\t- REMOVE ALL {} from {}", identifier, to_identifier);
        }
    }

    logger::debug("REPLACE conflict test map:");
    for (const auto& [key, distr_token_vec] : Maps::replace_conflict_test_map)
    {
        logger::debug("\t* Key {}:", key);
        for (const auto& [type, filename, to_identifier, identifier, count, rhs, rhs_count] : distr_token_vec)
        {
            if (count.has_value())
                logger::debug("\t\t^ REPLACE {} {} with {} {} in {}", count.value(), identifier, rhs_count.value(), rhs.value(), to_identifier);
            else
                logger::debug("\t\t^ REPLACE ALL {} with {} {} in {}", identifier, rhs_count.value(), rhs.value(), to_identifier);
        }
    }
}

DistrToken Parser::Tokenize(const std::string& str, const std::string& to) noexcept
{
    const auto bar{ str.find('|') };
    const auto caret{ str.find('^') };
    const auto slash{ str.find('/') };
    const auto filename{ str.substr(slash + 1) };

    const auto lhs{ str.substr(0, caret) };
    const auto rhs{ str.substr(caret + 1) };
    const auto rhs_bar{ rhs.find('|') };
    const auto rhs_distr{ rhs.substr(0, rhs_bar) };
    const auto rhs_count{ to_int32(rhs.substr(rhs_bar + 1)) };

    switch (ClassifyString(str))
    {
    case DistrType::Add: {
        const auto identifier{ str.substr(0, bar) };
        const auto count{ to_int32(str.substr(bar + 1)) };
        DistrToken distr_token{ DistrType::Add, filename, to, identifier, count, std::nullopt, std::nullopt };
        return distr_token;
    }
    case DistrType::Remove: {
        const auto identifier{ str.substr(1, bar - 1) };
        const auto count{ to_int32(str.substr(bar + 1)) };
        DistrToken distr_token{ DistrType::Remove, filename, to, identifier, count, std::nullopt, std::nullopt };
        return distr_token;
    }
    case DistrType::RemoveAll: {
        DistrToken distr_token{ DistrType::RemoveAll, filename, to, str.substr(1), std::nullopt, std::nullopt, std::nullopt };
        return distr_token;
    }
    case DistrType::Replace: {
        const auto lhs_bar{ lhs.find('|') };
        if (is_present(lhs_bar))
        {
            const auto lhs_distr{ lhs.substr(0, lhs_bar) };
            const auto lhs_count{ to_int32(lhs.substr(lhs_bar + 1)) };
            DistrToken distr_token{ DistrType::Replace, filename, to, lhs_distr, lhs_count, rhs_distr, rhs_count };
            return distr_token;
        }
        DistrToken distr_token{ DistrType::Replace, filename, to, lhs, std::nullopt, rhs_distr, rhs_count };
        return distr_token;
    }
    case DistrType::ReplaceAll: {
        DistrToken distr_token{ DistrType::ReplaceAll, filename, to, lhs, std::nullopt, rhs_distr, rhs_count };
        return distr_token;
    }
    default:
        break;
    }

    logger::error("ERROR: Failed to tokenize {}", str);
    return { DistrType::Error, filename, to, "", std::nullopt, std::nullopt, std::nullopt };
}
