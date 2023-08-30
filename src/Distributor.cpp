#include "Distributor.h"

void Distributor::CheckConflicts() noexcept
{
    logger::info("Checking conflicts...");
    BuildConflictMap(Utility::add_conflict_test_map, Utility::add_conflict_map, DistrType::Add);
    BuildConflictMap(Utility::remove_conflict_test_map, Utility::remove_conflict_map, DistrType::Remove);
    BuildConflictMap(Utility::swap_conflict_test_map, Utility::swap_conflict_map, DistrType::Swap);

    Utility::LogConflictMap(Utility::add_conflict_map);
    Utility::LogConflictMap(Utility::remove_conflict_map);
    Utility::LogConflictMap(Utility::swap_conflict_map);
}

template <typename T>
constexpr void Distributor::BuildConflictMap(const Utility::TConflictTestMap& test_map, T& conflict_map, const DistrType type) noexcept
{
    for (auto& [cont_edid, vec] : test_map)
    {
        logger::debug("Processing conflicts for {} ({})", cont_edid, static_cast<int>(type));
        for (const auto& val_with_filename : vec)
        {
            auto                    vec_copy{ vec };
            Utility::TStringPairVec conflict_providers{};
            const auto              slash{ val_with_filename.find('/') };
            const auto              full_token{ val_with_filename.substr(0, slash) };
            const auto              filename{ val_with_filename.substr(slash + 1) };
            const auto              bar{ full_token.find('|') };
            const auto              distr_token{ full_token.substr(0, bar) };

            std::string lhs{};
            std::string rhs{};
            switch (type)
            {
            case DistrType::Add:
            case DistrType::Remove:
                break;
            case DistrType::Swap: {
                const auto caret{ full_token.find('^') };
                lhs = full_token.substr(0, caret);
                rhs = full_token.substr(caret + 1);
                break;
            }
            default:
                break;
            }

            if (auto it{ std::ranges::find_if(
                    vec_copy, [&](const std::string& s) { return s <=> val_with_filename != 0 && s.substr(0, s.find('|')) <=> distr_token == 0; }) };
                it != vec_copy.end())
            {
                conflict_providers.emplace_back(filename, full_token);
                while (it != vec_copy.end())
                {
                    const auto delim{ it->find('/') };
                    conflict_providers.emplace_back(it->substr(delim + 1), it->substr(0, delim));
                    std::erase(vec_copy, *it);
                    it = std::ranges::find_if(vec_copy, [&](const std::string& s) {
                        return s <=> val_with_filename != 0 && s.substr(0, s.find('|')) <=> distr_token == 0;
                    });
                }
                if constexpr (std::is_same_v<T, Utility::TSwapConflictMap>)
                    conflict_map.emplace(std::pair(std::pair(lhs, rhs), cont_edid), conflict_providers);
                else
                    conflict_map.emplace(std::pair(distr_token, cont_edid), conflict_providers);
            }
        }
    }
}

void Distributor::PrepareDistribution() noexcept
{
    logger::info("Preparing distribution...");
    PrepareAddRemoveDistribution(Utility::add_conflict_test_map, Utility::add_conflict_map, DistrType::Add);
    PrepareAddRemoveDistribution(Utility::remove_conflict_test_map, Utility::remove_conflict_map, DistrType::Remove);
    PrepareSwapDistribution(Utility::swap_conflict_test_map, Utility::swap_conflict_map);

    for (const auto& [k, v] : Utility::add_map)
    {
        logger::info("ADD map for {}:", k);
        for (const auto& [obj, count] : v)
            logger::info("\t{} (0x{:x}), count: {}", obj->GetName(), obj->GetFormID(), count);
    }

    for (const auto& [k, v] : Utility::remove_map)
    {
        logger::info("REMOVE map for {}:", k);
        for (const auto& [obj, count] : v)
            logger::info("\t{} (0x{:x}), count: {}", obj->GetName(), obj->GetFormID(), count);
    }

    for (const auto& [k, v] : Utility::swap_map)
    {
        logger::info("SWAP map for {}:", k);
        for (const auto& [lhs, rhs] : v)
        {
            const auto& [lhs_obj, lhs_count]{ lhs };
            const auto& [rhs_obj, rhs_count]{ rhs };
            logger::info("\tLHS: {} (0x{:x}), count: {}", lhs_obj->GetName(), lhs_obj->GetFormID(), lhs_count);
            logger::info("\tRHS: {} (0x{:x}), count: {}", rhs_obj->GetName(), rhs_obj->GetFormID(), rhs_count);
        }
    }
}

void Distributor::PrepareAddRemoveDistribution(const Utility::TConflictTestMap& test_map, Utility::TConflictMap& conflict_map,
                                               const DistrType type) noexcept
{
    const auto handler{ RE::TESDataHandler::GetSingleton() };

    for (const auto& [cont_edid, vec] : test_map)
    {
        phmap::parallel_flat_hash_set<std::string> resolved_conflicts;

        std::vector<std::pair<RE::TESBoundObject*, std::int32_t>> objects_and_counts{};
        for (const auto& s : vec)
        {
            const auto slash{ s.find('/') };
            auto       distr_and_count{ s.substr(0, slash) };
            auto       bar{ distr_and_count.find('|') };
            auto       distr{ distr_and_count.substr(0, bar) };
            auto       count{ static_cast<std::int32_t>(std::strtol(s.substr(bar + 1).c_str(), nullptr, 0)) };
            auto       distr_and_cont_edid{ std::pair(distr, cont_edid) };

            if (resolved_conflicts.contains(distr))
                continue;

            if (conflict_map.contains(distr_and_cont_edid))
            {
                logger::debug("Conflict map contains {}", distr);
                auto providers{ conflict_map[distr_and_cont_edid] };
                std::ranges::sort(providers, [](const auto& lhs, const auto& rhs) { return lhs.first < rhs.first; });
                logger::debug("{} wins for {}", std::get<0>(providers.front()), distr);
                distr_and_count = std::get<1>(providers.front());
                bar             = distr_and_count.find('|');
                distr           = distr_and_count.substr(0, bar);
                count           = static_cast<std::int32_t>(std::strtol(distr_and_count.substr(bar + 1).c_str(), nullptr, 0));
                conflict_map.erase(distr_and_cont_edid);
                resolved_conflicts.emplace(distr);
            }

            const auto tilde{ distr.find('~') };
            bool       use_edid{};
            if (tilde <=> std::string::npos == 0)
                use_edid = true;

            if (use_edid)
            {
                if (const auto form{ RE::TESForm::LookupByEditorID(distr) })
                {
                    if (const auto bound_obj{ form->As<RE::TESBoundObject>() })
                    {
                        logger::debug("Found object {} (0x{:x}) for EditorID {}", bound_obj->GetName(), bound_obj->GetFormID(), distr);
                        objects_and_counts.emplace_back(bound_obj, count);
                    }
                }
            }
            else
            {
                const auto form_id{ static_cast<std::uint32_t>(std::strtol(distr.substr(0, tilde).c_str(), nullptr, 0)) };
                const auto plugin_name{ distr.substr(tilde + 1) };
                if (const auto obj{ handler->LookupForm(form_id, plugin_name) })
                {
                    if (const auto bound_obj{ obj->As<RE::TESBoundObject>() })
                    {
                        logger::debug("Found object {} (0x{:x}) for 0x{:x}~{}", bound_obj->GetName(), bound_obj->GetFormID(), form_id, plugin_name);
                        objects_and_counts.emplace_back(bound_obj, count);
                    }
                }
            }
        }
        if (type <=> DistrType::Add == 0)
            Utility::add_map.emplace(cont_edid, objects_and_counts);
        else
            Utility::remove_map.emplace(cont_edid, objects_and_counts);
    }
}

void Distributor::PrepareSwapDistribution(const Utility::TConflictTestMap& test_map, Utility::TSwapConflictMap& conflict_map) noexcept
{
    const auto handler{ RE::TESDataHandler::GetSingleton() };

    for (const auto& [cont_edid, vec] : test_map)
    {
        phmap::parallel_flat_hash_set<Utility::TStringPair> resolved_conflicts;

        std::vector<Utility::TSwapPair> objects_and_counts{};
        for (const auto& s : vec)
        {
            const auto slash{ s.find('/') };
            const auto ss{ s.substr(0, slash) };

            auto caret{ ss.find('^') };
            auto lhs{ ss.substr(0, caret) };
            auto rhs{ ss.substr(caret + 1) };

            auto lhs_bar{ lhs.find('|') };
            auto lhs_distr{ lhs.substr(0, lhs_bar) };
            auto lhs_count{ static_cast<std::int32_t>(std::strtol(lhs.substr(lhs_bar + 1).c_str(), nullptr, 0)) };

            auto rhs_bar{ rhs.find('|') };
            auto rhs_distr{ rhs.substr(0, rhs_bar) };
            auto rhs_count{ static_cast<std::int32_t>(std::strtol(rhs.substr(rhs_bar + 1).c_str(), nullptr, 0)) };

            const auto pair{ std::pair{ lhs, rhs } };
            const auto pair_and_cont_edid_pair{ std::pair{ pair, cont_edid } };
            const auto distr_pair{ std::pair{ lhs_distr, rhs_distr } };
            const auto distr_pair_and_cont_edid_pair{ std::pair{ distr_pair, cont_edid } };

            if (resolved_conflicts.contains(distr_pair))
                continue;

            if (conflict_map.contains(pair_and_cont_edid_pair))
            {
                logger::debug("Conflict map contains {} SWAP {}", distr_pair.first, distr_pair.second);
                auto providers{ conflict_map[pair_and_cont_edid_pair] };
                std::ranges::sort(providers, [](const auto& l, const auto& r) { return l.first < r.first; });
                logger::debug("{} wins for {} SWAP {}", providers.front().first, distr_pair.first, distr_pair.second);
                const auto full_token{ providers.front().second };
                caret     = full_token.find('^');
                rhs       = full_token.substr(0, caret);
                lhs_bar   = lhs.find('|');
                lhs_distr = lhs.substr(0, lhs_bar);
                lhs_count = static_cast<std::int32_t>(std::strtol(lhs.substr(lhs_bar + 1).c_str(), nullptr, 0));

                rhs       = full_token.substr(caret + 1);
                rhs_bar   = rhs.find('|');
                rhs_distr = rhs.substr(0, rhs_bar);
                rhs_count = static_cast<std::int32_t>(std::strtol(rhs.substr(rhs_bar + 1).c_str(), nullptr, 0));

                conflict_map.erase(pair_and_cont_edid_pair);
                resolved_conflicts.emplace(distr_pair);
            }

            const auto lhs_tilde{ lhs_distr.find('~') };
            const auto rhs_tilde{ rhs_distr.find('~') };
            bool       use_edid{};
            if (lhs_tilde <=> std::string::npos == 0)
                use_edid = true;

            if (use_edid)
            {
                if (const auto lhs_form{ RE::TESForm::LookupByEditorID(lhs_distr) })
                {
                    if (const auto lhs_bound_obj{ lhs_form->As<RE::TESBoundObject>() })
                    {
                        if (const auto rhs_form{ RE::TESForm::LookupByEditorID(rhs_distr) })
                        {
                            if (const auto rhs_bound_obj{ rhs_form->As<RE::TESBoundObject>() })
                            {
                                logger::debug("Found LHS swap object {} (0x{:x}) for EditorID {}", lhs_bound_obj->GetName(),
                                              lhs_bound_obj->GetFormID(), lhs_distr);
                                logger::debug("Found RHS swap object {} (0x{:x}) for EditorID {}", rhs_bound_obj->GetName(),
                                              rhs_bound_obj->GetFormID(), rhs_distr);
                                objects_and_counts.emplace_back(
                                    std::pair{ std::pair{ lhs_bound_obj, lhs_count }, std::pair{ rhs_bound_obj, rhs_count } });
                            }
                        }
                    }
                }
            }
            else
            {
                const auto lhs_form_id{ static_cast<std::uint32_t>(std::strtol(lhs_distr.substr(0, lhs_tilde).c_str(), nullptr, 0)) };
                const auto lhs_plugin_name{ lhs_distr.substr(lhs_tilde + 1) };
                const auto rhs_form_id{ static_cast<std::uint32_t>(std::strtol(rhs_distr.substr(0, rhs_tilde).c_str(), nullptr, 0)) };
                const auto rhs_plugin_name{ lhs_distr.substr(rhs_tilde + 1) };
                if (const auto lhs_form{ handler->LookupForm(lhs_form_id, lhs_plugin_name) })
                {
                    if (const auto lhs_bound_obj{ lhs_form->As<RE::TESBoundObject>() })
                    {
                        if (const auto rhs_form{ handler->LookupForm(rhs_form_id, rhs_plugin_name) })
                        {
                            if (const auto rhs_bound_obj{ rhs_form->As<RE::TESBoundObject>() })
                            {
                                logger::debug("Found LHS swap object {} (0x{:x}) for 0x{:x}~{}", lhs_bound_obj->GetName(), lhs_bound_obj->GetFormID(),
                                              lhs_form_id, lhs_plugin_name);
                                logger::debug("Found RHS swap object {} (0x{:x}) for 0x{:x}~{}", rhs_bound_obj->GetName(), rhs_bound_obj->GetFormID(),
                                              rhs_form_id, rhs_plugin_name);
                                objects_and_counts.emplace_back(
                                    std::pair{ std::pair{ lhs_bound_obj, lhs_count }, std::pair{ rhs_bound_obj, rhs_count } });
                            }
                        }
                    }
                }
            }
        }
        Utility::swap_map.emplace(cont_edid, objects_and_counts);
    }
}

void Distributor::Distribute() noexcept
{
    logger::info("Distributing...");
    AddDistribute(Utility::add_map);
    RemoveDistribute(Utility::remove_map);
    SwapDistribute(Utility::swap_map);
}

void Distributor::AddDistribute(const Utility::TAddRemoveMap& add_map) noexcept
{
    const auto& [forms, lock]{ RE::TESForm::GetAllFormsByEditorID() };

    const RE::BSReadLockGuard read_guard{ lock };
    for (const auto& [k, v] : add_map)
    {
        if (const auto& cont_pair{ forms->find(k) }; cont_pair != forms->end())
        {
            const std::string container_edid{ cont_pair->first.c_str() };
            const auto        container_form{ cont_pair->second };
            const auto        container_form_id{ container_form->GetFormID() };
            logger::debug("ADD: Found container {} (0x{:x}) in forms map", container_edid, container_form_id);

            if (const auto cont{ container_form->As<RE::TESContainer>() })
            {
                for (const auto& [obj, count] : v)
                {
                    cont->AddObjectToContainer(obj, count, nullptr);

                    logger::info("\tAdded {} {} (0x{:x}) to {} (0x{:x})", count, obj->GetName(), obj->GetFormID(), container_edid, container_form_id);
                }
                logger::info("");
            }
        }
    }
}

void Distributor::RemoveDistribute(const Utility::TAddRemoveMap& remove_map) noexcept
{
    const auto& [forms, lock]{ RE::TESForm::GetAllFormsByEditorID() };

    const RE::BSReadLockGuard read_guard{ lock };
    for (const auto& [k, v] : remove_map)
    {
        if (const auto& cont_pair{ forms->find(k) }; cont_pair != forms->end())
        {
            const std::string container_edid{ cont_pair->first.c_str() };
            const auto        container_form{ cont_pair->second };
            const auto        container_form_id{ container_form->GetFormID() };
            logger::debug("REMOVE: Found container {} (0x{:x}) in forms map", container_edid, container_form_id);

            if (const auto cont{ container_form->As<RE::TESContainer>() })
            {
                for (const auto& [obj, count] : v)
                {
                    cont->RemoveObjectFromContainer(obj, count);

                    logger::info("\tRemoved {} {} (0x{:x}) from {} (0x{:x})", count, obj->GetName(), obj->GetFormID(), container_edid,
                                 container_form_id);
                }
                logger::info("");
            }
        }
    }
}

void Distributor::SwapDistribute(const Utility::TSwapMap& swap_map) noexcept
{
    const auto& [forms, lock]{ RE::TESForm::GetAllFormsByEditorID() };

    const RE::BSReadLockGuard read_guard{ lock };
    for (const auto& [k, v] : swap_map)
    {
        if (const auto& cont_pair{ forms->find(k) }; cont_pair != forms->end())
        {
            const std::string container_edid{ cont_pair->first.c_str() };
            const auto        container_form{ cont_pair->second };
            const auto        container_form_id{ container_form->GetFormID() };
            logger::debug("SWAP: Found container {} (0x{:x}) in forms map", container_edid, container_form_id);

            if (const auto cont{ container_form->As<RE::TESContainer>() })
            {
                for (const auto& [lhs, rhs] : v)
                {
                    const auto& [lhs_obj, lhs_count]{ lhs };
                    const auto& [rhs_obj, rhs_count]{ rhs };

                    cont->RemoveObjectFromContainer(lhs_obj, lhs_count);

                    logger::info("\tSWAP: Removed {} {} (0x{:x}) from {} (0x{:x})", lhs_count, lhs_obj->GetName(), lhs_obj->GetFormID(), container_edid,
                                 container_form_id);

                    cont->AddObjectToContainer(rhs_obj, rhs_count, nullptr);

                    logger::info("\tSWAP: Added {} {} (0x{:x}) to {} (0x{:x})", rhs_count, rhs_obj->GetName(), rhs_obj->GetFormID(), container_edid,
                                 container_form_id);
                }
                logger::info("");
            }
        }
    }
}
