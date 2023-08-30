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

    for (const auto& ini_file : std::filesystem::directory_iterator{ plugins_dir })
    {
        const auto ini_path{ ini_file.path().generic_string() };
        if (!std::regex_search(ini_path, p))
            continue;

        logger::info("Loading config file: {}", ini_path);

        ini.LoadFile(ini_path.c_str());

        CSimpleIniA::TNamesDepend keys{};
        ini.GetAllKeys("General", keys);

        const auto ini_file_name{ ini_file.path().filename().generic_string() };

        logger::debug("{} has {} keys", ini_file_name, keys.size());

        for (const auto& [cont_edid, key_order, key_count] : keys)
        {
            CSimpleIniA::TNamesDepend values{};
            ini.GetAllValues("General", cont_edid, values);

            logger::debug("{} has {} values", cont_edid, values.size());

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
                if (!map_to_use->contains(cont_edid))
                {
                    std::vector<std::string> new_vec;
                    new_vec.emplace_back(val_with_filename);
                    map_to_use->emplace(cont_edid, new_vec);
                }
                else
                {
                    auto& conflict_test_vec{ map_to_use->at(cont_edid) };
                    conflict_test_vec.emplace_back(val_with_filename);
                }
            }
        }

        logger::debug("");
        ini.Reset();
    }

    logger::debug("Finished building conflict test maps");

    logger::debug("ADD CT map:");
    for (const auto& [k, v] : Utility::add_conflict_test_map)
    {
        logger::debug("{}:", k);
        for (const auto& token : v)
        {
            logger::debug("\t{}", token);
        }
    }

    logger::debug("REMOVE CT map:");
    for (const auto& [k, v] : Utility::remove_conflict_test_map)
    {
        logger::debug("{}:", k);
        for (const auto& token : v)
        {
            logger::debug("\t{}", token);
        }
    }

    logger::debug("SWAP CT map:");
    for (const auto& [k, v] : Utility::swap_conflict_test_map)
    {
        logger::debug("{}:", k);
        for (const auto& token : v)
        {
            logger::debug("\t{}", token);
        }
    }

    logger::info("Loaded settings");
}
