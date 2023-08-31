#include "Settings.h"

#include "Utility.h"

void Settings::LoadSettings() noexcept
{
    logger::info("Loading settings");

    CSimpleIniA ini;

    ini.SetUnicode();
    ini.SetMultiKey();
    ini.LoadFile(R"(.\Data\SKSE\Plugins\ContainerItemDistributor.ini)");

    debug_logging = ini.GetBoolValue("Log", "Debug");

    if (debug_logging)
    {
        spdlog::get("Global")->set_level(spdlog::level::level_enum::debug);
        logger::debug("Debug logging enabled");
    }

    const std::filesystem::path plugins_dir{ R"(.\Data)" };
    const std::regex            p{ "_CID.ini$" };

    if (!exists(plugins_dir))
    {
        logger::error("ERROR: Failed to find Data directory");
        stl::report_and_fail(fmt::format("{}: Failed to find Data directory", SKSE::PluginDeclaration::GetSingleton()->GetName()));
    }

    for (std::error_code ec{}; const auto& file : std::filesystem::directory_iterator{ plugins_dir, ec })
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

        for (const auto& [edid, key_order, key_count] : keys)
        {
            CSimpleIniA::TNamesDepend values{};
            ini.GetAllValues("General", edid, values);

            logger::debug("{} has {} values", edid, values.size());

            for (const auto& [val, order, val_count] : values)
            {
                auto val_with_filename{ std::string{ val } + "/" + ini_file_name };

                const auto distr_type = Utility::ClassifyToken(val);

                auto* map_to_use = &Utility::add_conflict_test_map;
                switch (distr_type)
                {
                case DistrType::Add:
                    break;
                case DistrType::Remove:
                    map_to_use        = &Utility::remove_conflict_test_map;
                    val_with_filename = val_with_filename.substr(1);
                    break;
                case DistrType::Swap:
                    map_to_use = &Utility::swap_conflict_test_map;
                    break;
                case DistrType::Error:
                    map_to_use = nullptr;
                    break;
                }

                logger::debug("\t{}: {}", val, static_cast<int>(distr_type));

                if (!map_to_use)
                {
                    logger::error("ERROR: Could not find conflict test map to use");
                    continue;
                }
                if (!map_to_use->contains(edid))
                {
                    std::vector<std::string> new_vec;
                    new_vec.emplace_back(val_with_filename);
                    map_to_use->emplace(edid, new_vec);
                }
                else
                {
                    auto& conflict_test_vec{ map_to_use->at(edid) };
                    conflict_test_vec.emplace_back(val_with_filename);
                }
            }
        }

        logger::debug("");
        ini.Reset();
    }

    logger::debug("Finished building conflict test maps");

    logger::debug("ADD CT map:");
    for (const auto& [edid, distr_token_vec] : Utility::add_conflict_test_map)
    {
        logger::debug("{}:", edid);
        for (const auto& token : distr_token_vec)
            logger::debug("\t{}", token);
    }

    logger::debug("REMOVE CT map:");
    for (const auto& [edid, distr_token_vec] : Utility::remove_conflict_test_map)
    {
        logger::debug("{}:", edid);
        for (const auto& token : distr_token_vec)
            logger::debug("\t{}", token);
    }

    logger::debug("SWAP CT map:");
    for (const auto& [edid, distr_token_vec] : Utility::swap_conflict_test_map)
    {
        logger::debug("{}:", edid);
        for (const auto& token : distr_token_vec)
            logger::debug("\t{}", token);
    }

    logger::info("Loaded settings");
}
