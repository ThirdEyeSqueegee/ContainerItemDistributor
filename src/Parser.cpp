#include "Parser.h"

#include "Utility.h"

DistrType Parser::ClassifyString(const std::string_view s) noexcept
{
    const auto has_leading_minus{ s.starts_with('-') };
    const auto bar_count{ std::ranges::count(s, '|') };

    if (!has_leading_minus && bar_count == 1) {
        return DistrType::Add;
    }
    if (has_leading_minus && bar_count == 1) {
        return DistrType::Remove;
    }
    if (has_leading_minus && bar_count == 0) {
        return DistrType::RemoveAll;
    }

    return DistrType::Error;
}

DistrToken Parser::Tokenize(const std::string& s, const std::string& to_container, const DistrType distr_type) noexcept
{
    using enum DistrType;
    switch (distr_type) {
    case Add: {
        const auto bar_pos{ s.find('|') };

        DistrToken distr_token{ .type          = Add,
                                .to_identifier = to_container,
                                .identifier    = s.substr(0, bar_pos),
                                .count         = Map::ToUnsignedInt(s.substr(bar_pos + 1)),
                                .chance        = Utility::GetChanceFromToken(s) };

        return distr_token;
    }
    case Remove: {
        const auto bar_pos{ s.find('|') };

        DistrToken distr_token{ .type          = Remove,
                                .to_identifier = to_container,
                                .identifier    = s.substr(1, bar_pos - 1),
                                .count         = Map::ToUnsignedInt(s.substr(bar_pos + 1)),
                                .chance        = Utility::GetChanceFromToken(s) };

        return distr_token;
    }
    case RemoveAll: {
        DistrToken distr_token{ .type = RemoveAll, .to_identifier = to_container, .identifier = s.substr(1), .count = 0, .chance = Utility::GetChanceFromToken(s) };

        return distr_token;
    }
    default:
        break;
    }
    logger::error("ERROR: Failed to tokenize {}", s);

    return { .type = Error, .to_identifier = to_container, .identifier = "", .count = 0, .chance = 0 };
}

void Parser::ParseINIs(CSimpleIniA& ini) noexcept
{
    ini.SetMultiKey();

    const std::filesystem::path data_dir{ R"(.\Data)" };
    const auto                  pattern{ L"_CID.ini" };

    if (!exists(data_dir)) {
        logger::error("ERROR: Failed to find Data directory");
        stl::report_and_fail(fmt::format("{}: Failed to find Data directory", SKSE::PluginDeclaration::GetSingleton()->GetName()));
    }

    logger::info(">------------------------------------------------------------ Parsing _CID.ini files... -------------------------------------------------------------<");
    logger::info("");

    std::vector<std::filesystem::path> cid_inis;
    for (std::error_code ec{}; const auto& file : std::filesystem::directory_iterator{ data_dir, ec }) {
        if (ec.value()) {
            logger::error("ERROR CODE: {}", ec.value());
            continue;
        }

        const auto& path{ file.path() };

        if (path.extension() != ".ini") {
            continue;
        }

        if (!path.filename().wstring().ends_with(pattern)) {
            continue;
        }

        cid_inis.emplace_back(path);
    }

    std::sort(std::execution::par, cid_inis.begin(), cid_inis.end());

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
                const auto& distr_type{ ClassifyString(v.pItem) };
                logger::debug("\t\t* {} {}", distr_type, v.pItem);

                auto distr_obj{ Utility::BuildDistrObject(Tokenize(v.pItem, k.pItem, distr_type)) };

                if (distr_obj.type == DistrType::Error) {
                    continue;
                }

                const auto cont_form_id{ distr_obj.container_form_id };

                using enum DistrType;
                switch (distr_obj.type) {
                case Add:
                    Map::distr_map[cont_form_id].to_add.emplace_back(distr_obj);
                    break;
                case Remove:
                    Map::distr_map[cont_form_id].to_remove.emplace_back(distr_obj);
                    break;
                case RemoveAll:
                    Map::distr_map[cont_form_id].to_remove_all.emplace_back(distr_obj);
                    break;
                default:
                    break;
                }
            }
        }
        logger::debug("");

        ini.Reset();
    }

    logger::info("");
    logger::info(">--------------------------------------------------------- Finished parsing _CID.ini files ----------------------------------------------------------<");
    logger::info("");
}
