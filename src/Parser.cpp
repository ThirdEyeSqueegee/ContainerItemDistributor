#include "Parser.h"

#include "Utility.h"

DistrType Parser::ClassifyString(const std::string_view s) noexcept
{
    const auto has_leading_minus{ s.starts_with('-') };
    const auto bar_count{ std::ranges::count(s, '|') };

    if (!has_leading_minus && bar_count > 0) {
        return DistrType::Add;
    }
    if (has_leading_minus && bar_count > 0) {
        return DistrType::Remove;
    }
    if (has_leading_minus && bar_count == 0) {
        return DistrType::RemoveAll;
    }

    return DistrType::Error;
}

DistrToken Parser::Tokenize(std::string s, const std::string& to_container, const DistrType distr_type) noexcept
{
    auto max_split_size{ 4 };
    auto min_split_size{ 2 };

    using enum DistrType;
    switch (distr_type) {
    case Add: {
        break;
    }
    case Remove: {
        s.erase(0, 1);
        break;
    }
    case RemoveAll: {
        s.erase(0, 1);
        max_split_size = 3;
        min_split_size = 1;
        break;
    }
    default:
        logger::error("ERROR: Failed to tokenize {}", s);

        return { .type = Error, .to_identifier = to_container, .identifier = "", .count = 0, .location = "", .location_keyword = "", .chance = 0 };
    }

    const auto chance_sep{ s.rfind('?') };
    u16        chance{ 100U };
    if (chance_sep != std::string::npos) {
        chance = Map::ToUnsignedInt(s.substr(chance_sep + 1));
        s.erase(chance_sep);
    }

    const auto  location_keyword_sep{ s.rfind('@') };
    std::string location_keyword{};
    if (location_keyword_sep != std::string::npos) {
        location_keyword = s.substr(location_keyword_sep + 1);
        s.erase(location_keyword_sep);
    }

    const auto split{ s | std::ranges::views::split('|') | std::ranges::to<std::vector<std::string>>() };

    if (split.size() > max_split_size || split.size() < min_split_size) {
        logger::error("ERROR: {} is ill-formed", s);

        return { .type = Error, .to_identifier = to_container, .identifier = "", .count = 0, .location = "", .location_keyword = "", .chance = 0 };
    }

    return { .type             = distr_type,
             .to_identifier    = to_container,
             .identifier       = split[0],
             .count            = distr_type != RemoveAll ? Map::ToUnsignedInt(split[1]) : static_cast<u16>(0U),
             .location         = split.size() > 2 ? split[2] : "",
             .location_keyword = location_keyword,
             .chance           = chance };
}

void Parser::ParseINIs() noexcept
{
    const std::filesystem::path data_dir{ R"(.\Data)" };
    const auto                  pattern{ L"_CID.ini" };

    if (!exists(data_dir)) {
        logger::error("ERROR: Failed to find Data directory");
        stl::report_and_fail(std::format("{}: Failed to find Data directory", SKSE::PluginDeclaration::GetSingleton()->GetName()));
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
        CSimpleIniA ini;
        ini.SetUnicode();
        ini.SetMultiKey();

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

                const auto distr_obj{ Utility::BuildDistrObject(Tokenize(v.pItem, k.pItem, distr_type)) };

                if (distr_obj.type == DistrType::Error) {
                    continue;
                }

                if (distr_obj.location && distr_obj.location_keyword) {
                    logger::error("\t\tERROR: {:?} contains both location and location_keyword. Please only define one or the other", v.pItem);
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
