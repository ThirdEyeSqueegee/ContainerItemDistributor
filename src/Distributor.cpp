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
    for (auto& [edid, distr_token_vec] : test_map)
    {
        logger::debug("Processing conflicts for {} ({})", edid, static_cast<int>(type));
        for (const auto& val_with_filename : distr_token_vec)
        {
            auto                    vec_copy{ distr_token_vec };
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
                    conflict_map.emplace(std::pair(std::pair(lhs, rhs), edid), conflict_providers);
                else
                    conflict_map.emplace(std::pair(distr_token, edid), conflict_providers);
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

    for (const auto& [edid, object_count_pair_vec] : Utility::add_map)
    {
        logger::info("ADD map for {}:", edid);
        for (const auto& [obj, count] : object_count_pair_vec)
            logger::info("\t{} (0x{:x}), count: {}", obj->GetName(), obj->GetFormID(), count);
    }

    for (const auto& [edid, object_count_pair_vec] : Utility::remove_map)
    {
        logger::info("REMOVE map for {}:", edid);
        for (const auto& [obj, count] : object_count_pair_vec)
            logger::info("\t{} (0x{:x}), count: {}", obj->GetName(), obj->GetFormID(), count);
    }

    for (const auto& [edid, object_count_pairs_vec] : Utility::swap_map)
    {
        logger::info("SWAP map for {}:", edid);
        for (const auto& [lhs, rhs] : object_count_pairs_vec)
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

    for (const auto& [edid, distr_token_vec] : test_map)
    {
        phmap::parallel_flat_hash_set<std::string> resolved_conflicts;

        std::vector<std::pair<RE::TESBoundObject*, std::int32_t>> objects_and_counts{};
        for (const auto& distr_token : distr_token_vec)
        {
            const auto           slash{ distr_token.find('/') };
            auto                 distr_and_count{ distr_token.substr(0, slash) };
            auto                 bar{ distr_and_count.find('|') };
            bool                 all_of{};
            std::string          distr{};
            std::int32_t         count{};
            Utility::TStringPair distr_and_edid;
            if (!Utility::is_present(bar))
            {
                all_of         = true;
                distr          = distr_and_count;
                distr_and_edid = std::pair{ distr_and_count, edid };
            }
            else
            {
                distr          = distr_and_count.substr(0, bar);
                count          = static_cast<std::int32_t>(std::strtol(distr_token.substr(bar + 1).c_str(), nullptr, 0));
                distr_and_edid = std::pair(distr, edid);
            }

            if (resolved_conflicts.contains(distr))
                continue;

            if (conflict_map.contains(distr_and_edid))
            {
                logger::debug("Conflict map contains {}", distr);
                auto providers{ conflict_map[distr_and_edid] };
                std::ranges::sort(providers, [](const auto& lhs, const auto& rhs) { return lhs.first < rhs.first; });
                logger::debug("{} wins for {}", providers.front().first, distr);
                distr_and_count = providers.front().second;
                if (!all_of)
                {
                    bar   = distr_and_count.find('|');
                    distr = distr_and_count.substr(0, bar);
                    count = static_cast<std::int32_t>(std::strtol(distr_and_count.substr(bar + 1).c_str(), nullptr, 0));
                }
                else
                    distr = distr_and_count;

                conflict_map.erase(distr_and_edid);
                resolved_conflicts.emplace(distr);
            }

            const auto tilde{ distr.find('~') };
            bool       use_edid{};
            if (!Utility::is_present(tilde))
                use_edid = true;

            if (use_edid)
            {
                if (const auto form{ RE::TESForm::LookupByEditorID(distr) })
                {
                    if (const auto bound_obj{ form->As<RE::TESBoundObject>() })
                    {
                        logger::debug("Found object {} (0x{:x}) for EditorID {}", bound_obj->GetName(), bound_obj->GetFormID(), distr);
                        if (all_of)
                        {
                            logger::debug("\tALL OF: Setting count to -1");
                            objects_and_counts.emplace_back(bound_obj, -1);
                        }
                        else
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
                        if (all_of)
                        {
                            logger::debug("\tALL OF: Setting count to -1");
                            objects_and_counts.emplace_back(bound_obj, -1);
                        }
                        else
                            objects_and_counts.emplace_back(bound_obj, count);
                    }
                }
            }
        }
        if (type <=> DistrType::Add == 0)
            Utility::add_map.emplace(edid, objects_and_counts);
        else
            Utility::remove_map.emplace(edid, objects_and_counts);
    }
}

void Distributor::PrepareSwapDistribution(const Utility::TConflictTestMap& test_map, Utility::TSwapConflictMap& conflict_map) noexcept
{
    const auto handler{ RE::TESDataHandler::GetSingleton() };

    for (const auto& [edid, distr_token_vec] : test_map)
    {
        phmap::parallel_flat_hash_set<Utility::TStringPair> resolved_conflicts;

        std::vector<Utility::TSwapPair> objects_and_counts{};
        for (const auto& distr_token : distr_token_vec)
        {
            const auto slash{ distr_token.find('/') };
            const auto ss{ distr_token.substr(0, slash) };

            auto caret{ ss.find('^') };
            auto lhs{ ss.substr(0, caret) };
            auto rhs{ ss.substr(caret + 1) };

            auto         lhs_bar{ lhs.find('|') };
            auto         rhs_bar{ rhs.find('|') };
            bool         all_of{};
            std::string  lhs_distr{};
            std::int32_t lhs_count{};
            if (!Utility::is_present(lhs_bar))
            {
                all_of    = true;
                lhs_distr = lhs;
            }
            else
            {
                lhs_distr = lhs.substr(0, lhs_bar);
                lhs_count = static_cast<std::int32_t>(std::strtol(lhs.substr(lhs_bar + 1).c_str(), nullptr, 0));
            }

            auto rhs_distr{ rhs.substr(0, rhs_bar) };
            auto rhs_count{ static_cast<std::int32_t>(std::strtol(rhs.substr(rhs_bar + 1).c_str(), nullptr, 0)) };

            const auto pair{ std::pair{ lhs, rhs } };
            const auto pair_and_edid_pair{ std::pair{ pair, edid } };
            const auto distr_pair{ std::pair{ lhs_distr, rhs_distr } };
            const auto distr_pair_and_edid_pair{ std::pair{ distr_pair, edid } };

            if (resolved_conflicts.contains(distr_pair))
                continue;

            if (conflict_map.contains(pair_and_edid_pair))
            {
                logger::debug("Conflict map contains {} SWAP {}", distr_pair.first, distr_pair.second);
                auto providers{ conflict_map[pair_and_edid_pair] };
                std::ranges::sort(providers, [](const auto& l, const auto& r) { return l.first < r.first; });
                logger::debug("{} wins for {} SWAP {}", providers.front().first, distr_pair.first, distr_pair.second);
                const auto full_token{ providers.front().second };
                caret = full_token.find('^');
                lhs   = full_token.substr(0, caret);
                rhs   = full_token.substr(caret + 1);
                if (all_of)
                {
                    lhs_distr = lhs;
                    rhs_distr = rhs;
                }
                else
                {
                    lhs_bar   = lhs.find('|');
                    lhs_distr = lhs.substr(0, lhs_bar);
                    lhs_count = static_cast<std::int32_t>(std::strtol(lhs.substr(lhs_bar + 1).c_str(), nullptr, 0));
                }

                rhs_bar   = rhs.find('|');
                rhs_distr = rhs.substr(0, rhs_bar);
                rhs_count = static_cast<std::int32_t>(std::strtol(rhs.substr(rhs_bar + 1).c_str(), nullptr, 0));

                conflict_map.erase(pair_and_edid_pair);
                resolved_conflicts.emplace(distr_pair);
            }

            const auto lhs_tilde{ lhs_distr.find('~') };
            const auto rhs_tilde{ rhs_distr.find('~') };
            bool       use_lhs_edid{}, use_rhs_edid{};
            if (!Utility::is_present(lhs_tilde))
                use_lhs_edid = true;
            if (!Utility::is_present(rhs_tilde))
                use_rhs_edid = true;

            if (use_lhs_edid && use_rhs_edid)
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
                                if (all_of)
                                    objects_and_counts.emplace_back(
                                        std::pair{ std::pair{ lhs_bound_obj, -1 }, std::pair{ rhs_bound_obj, rhs_count } });
                                else
                                    objects_and_counts.emplace_back(
                                        std::pair{ std::pair{ lhs_bound_obj, lhs_count }, std::pair{ rhs_bound_obj, rhs_count } });
                            }
                        }
                    }
                }
            }
            else if (!use_lhs_edid && use_rhs_edid)
            {
                const auto lhs_form_id{ static_cast<std::uint32_t>(std::strtol(lhs_distr.substr(0, lhs_tilde).c_str(), nullptr, 0)) };
                const auto lhs_plugin_name{ lhs_distr.substr(lhs_tilde + 1) };
                if (const auto lhs_form{ handler->LookupForm(lhs_form_id, lhs_plugin_name) })
                {
                    if (const auto lhs_bound_obj{ lhs_form->As<RE::TESBoundObject>() })
                    {
                        if (const auto rhs_form{ RE::TESForm::LookupByEditorID(rhs_distr) })
                        {
                            if (const auto rhs_bound_obj{ rhs_form->As<RE::TESBoundObject>() })
                            {
                                logger::debug("Found LHS swap object {} (0x{:x}) for {}", lhs_bound_obj->GetName(), lhs_bound_obj->GetFormID(),
                                              lhs_distr);
                                logger::debug("Found RHS swap object {} (0x{:x}) for EditorID {}", rhs_bound_obj->GetName(),
                                              rhs_bound_obj->GetFormID(), rhs_distr);
                                if (all_of)
                                    objects_and_counts.emplace_back(
                                        std::pair{ std::pair{ lhs_bound_obj, -1 }, std::pair{ rhs_bound_obj, rhs_count } });
                                else
                                    objects_and_counts.emplace_back(
                                        std::pair{ std::pair{ lhs_bound_obj, lhs_count }, std::pair{ rhs_bound_obj, rhs_count } });
                            }
                        }
                    }
                }
            }
            else if (use_lhs_edid && !use_rhs_edid)
            {
                const auto rhs_form_id{ static_cast<std::uint32_t>(std::strtol(rhs_distr.substr(0, rhs_tilde).c_str(), nullptr, 0)) };
                const auto rhs_plugin_name{ rhs_distr.substr(rhs_tilde + 1) };
                if (const auto lhs_form{ RE::TESForm::LookupByEditorID(lhs_distr) })
                {
                    if (const auto lhs_bound_obj{ lhs_form->As<RE::TESBoundObject>() })
                    {
                        if (const auto rhs_form{ handler->LookupForm(rhs_form_id, rhs_plugin_name) })
                        {
                            if (const auto rhs_bound_obj{ rhs_form->As<RE::TESBoundObject>() })
                            {
                                logger::debug("Found LHS swap object {} (0x{:x}) for EditorID {}", lhs_bound_obj->GetName(),
                                              lhs_bound_obj->GetFormID(), lhs_distr);
                                logger::debug("Found RHS swap object {} (0x{:x}) for {}", rhs_bound_obj->GetName(), rhs_bound_obj->GetFormID(),
                                              rhs_distr);
                                if (all_of)
                                    objects_and_counts.emplace_back(
                                        std::pair{ std::pair{ lhs_bound_obj, -1 }, std::pair{ rhs_bound_obj, rhs_count } });
                                else
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
                                if (all_of)
                                    objects_and_counts.emplace_back(
                                        std::pair{ std::pair{ lhs_bound_obj, -1 }, std::pair{ rhs_bound_obj, rhs_count } });
                                else
                                    objects_and_counts.emplace_back(
                                        std::pair{ std::pair{ lhs_bound_obj, lhs_count }, std::pair{ rhs_bound_obj, rhs_count } });
                            }
                        }
                    }
                }
            }
        }
        Utility::swap_map.emplace(edid, objects_and_counts);
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
    for (const auto& [edid, obj_count_pair_vec] : add_map)
    {
        if (const auto& pair{ forms->find(edid) }; pair != forms->end())
        {
            const std::string found_edid{ pair->first.c_str() };
            const auto        found_form{ pair->second };
            const auto        found_form_id{ found_form->GetFormID() };
            logger::debug("ADD: Found {} (0x{:x}) in forms map", edid, found_form_id);

            if (const auto cont{ found_form->As<RE::TESContainer>() })
            {
                for (const auto& [obj, count] : obj_count_pair_vec)
                {
                    cont->AddObjectToContainer(obj, count, nullptr);

                    logger::info("\tAdded {} {} (0x{:x}) to {} (0x{:x})", count, obj->GetName(), obj->GetFormID(), found_edid, found_form_id);
                }
                logger::info("");
            }
            // else if (const auto leveled_list{ found_form->As<RE::TESLeveledList>() })
            //{
            //     for (const auto& [obj, count] : obj_count_pair_vec)
            //     {
            //         if (leveled_list->GetCanContainFormsOfType(obj->GetFormType()))
            //         {
            //             logger::debug("EDITING LEVELED LIST");
            //             leveled_list->GetContainedForms().emplace_back(obj);
            //             const auto it{ leveled_list->entries.end() };
            //             leveled_list->entries.resize(leveled_list->entries.size() + 1);
            //             it->count = static_cast<uint16_t>(count);
            //             it->form  = obj;
            //             it->level = 50;
            //             leveled_list->numEntries++;

            //            logger::debug("\tAdded {} {} (0x{:x}) to {} (0x{:x})", count, obj->GetName(), obj->GetFormID(), found_edid, found_form_id);
            //        }
            //    }
            //}
        }
    }
}

void Distributor::RemoveDistribute(const Utility::TAddRemoveMap& remove_map) noexcept
{
    const auto& [forms, lock]{ RE::TESForm::GetAllFormsByEditorID() };

    const RE::BSReadLockGuard read_guard{ lock };
    for (const auto& [edid, obj_count_pair_vec] : remove_map)
    {
        if (const auto& pair{ forms->find(edid) }; pair != forms->end())
        {
            const std::string found_edid{ pair->first.c_str() };
            const auto        found_form{ pair->second };
            const auto        found_form_id{ found_form->GetFormID() };
            logger::debug("REMOVE: Found container {} (0x{:x}) in forms map", edid, found_form_id);

            if (const auto cont{ found_form->As<RE::TESContainer>() })
            {
                for (const auto& [obj, count] : obj_count_pair_vec)
                {
                    std::int32_t obj_count{};
                    if (count <=> -1 == 0)
                    {
                        cont->ForEachContainerObject([&](const RE::ContainerObject& cont_obj) {
                            if (cont_obj.obj->GetFormID() <=> obj->GetFormID() == 0)
                            {
                                obj_count = cont_obj.count;
                                return RE::BSContainer::ForEachResult::kStop;
                            }
                            return RE::BSContainer::ForEachResult::kContinue;
                        });
                    }
                    else
                        obj_count = count;
                    cont->RemoveObjectFromContainer(obj, obj_count);

                    logger::info("\tRemoved {} {} (0x{:x}) from {} (0x{:x})", obj_count, obj->GetName(), obj->GetFormID(), edid, found_form_id);
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
    for (const auto& [edid, obj_cont_pairs_vec] : swap_map)
    {
        if (const auto& pair{ forms->find(edid) }; pair != forms->end())
        {
            const std::string found_edid{ pair->first.c_str() };
            const auto        found_form{ pair->second };
            const auto        found_form_id{ found_form->GetFormID() };
            logger::debug("SWAP: Found container {} (0x{:x}) in forms map", edid, found_form_id);

            if (const auto cont{ found_form->As<RE::TESContainer>() })
            {
                for (const auto& [lhs, rhs] : obj_cont_pairs_vec)
                {
                    const auto& [lhs_obj, lhs_count]{ lhs };
                    const auto& [rhs_obj, rhs_count]{ rhs };

                    std::int32_t lhs_obj_count{};
                    if (lhs_count <=> -1 == 0)
                    {
                        cont->ForEachContainerObject([&](const RE::ContainerObject& cont_obj) {
                            if (cont_obj.obj->GetFormID() <=> lhs_obj->GetFormID() == 0)
                            {
                                lhs_obj_count = cont_obj.count;
                                return RE::BSContainer::ForEachResult::kStop;
                            }
                            return RE::BSContainer::ForEachResult::kContinue;
                        });
                    }
                    else
                        lhs_obj_count = lhs_count;
                    cont->RemoveObjectFromContainer(lhs_obj, lhs_obj_count);

                    logger::info("\tSWAP: Removed {} {} (0x{:x}) from {} (0x{:x})", lhs_obj_count, lhs_obj->GetName(), lhs_obj->GetFormID(), edid,
                                 found_form_id);

                    cont->AddObjectToContainer(rhs_obj, rhs_count, nullptr);

                    logger::info("\tSWAP: Added {} {} (0x{:x}) to {} (0x{:x})", rhs_count, rhs_obj->GetName(), rhs_obj->GetFormID(), edid,
                                 found_form_id);
                }
                logger::info("");
            }
        }
    }
}
