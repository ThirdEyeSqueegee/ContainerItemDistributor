#include "Distributor.h"

void Distributor::Distribute() noexcept
{
    logger::info(">------------------------------------------------------------------Distributing...-------------------------------------------------------------------<");
    logger::info("");

    AddDistribute(Maps::distr_object_vec);
    RemoveDistribute(Maps::distr_object_vec);
    ReplaceDistribute(Maps::distr_object_vec);

    logger::info("");
    logger::info(">---------------------------------------------------------------Finished distribution----------------------------------------------------------------<");
}

void Distributor::AddDistribute(const Maps::TDistrVec& distr_vec) noexcept
{
    logger::info("+----------------------------------Performing ADD distribution...----------------------------------+");
    logger::info("");

    for (const auto& distr_obj : distr_vec) {
        const auto& [type, bound_object, leveled_list, filename, replace_with_obj, replace_with_list, count, replace_with_count, container, chance]{ distr_obj };
        if (type != DistrType::Add) {
            continue;
        }

        if (container.has_value()) {
            const auto& [cont, cont_form_id, cont_type, cont_name]{ container.value() };

            if (chance.value_or(100) < 100) {
                Maps::runtime_map[cont_form_id].to_add.push_back(distr_obj);
                continue;
            }

            if (bound_object) {
                cont->AddObjectToContainer(bound_object, count.value(), nullptr);
                logger::info("\t+ {}", distr_obj);
            }
            else if (leveled_list) {
                for (const auto& [lobj, lcount] : Utility::ResolveLeveledList(leveled_list)) {
                    cont->AddObjectToContainer(lobj, lcount, nullptr);
                    logger::info("\t+ {}", distr_obj);
                }
            }
            else {
                logger::error("\tERROR: Failed to distribute {}", distr_obj);
            }
        }
    }

    logger::info("");
    logger::info("+------------------------------------Finished ADD distribution-------------------------------------+");
    logger::info("");
}

void Distributor::RemoveDistribute(const Maps::TDistrVec& distr_vec) noexcept
{
    logger::info(">--------------------------------Performing REMOVE distribution...---------------------------------<");
    logger::info("");

    for (const auto& distr_obj : distr_vec) {
        const auto& [type, bound_object, leveled_list, filename, replace_with_obj, replace_with_list, count, replace_with_count, container, chance]{ distr_obj };
        if (type != DistrType::Remove && type != DistrType::RemoveAll) {
            continue;
        }

        if (container.has_value()) {
            const auto& [cont, cont_form_id, cont_type, cont_name]{ container.value() };

            if (count.has_value()) {
                if (chance.value_or(100) < 100) {
                    Maps::runtime_map[cont_form_id].to_remove.push_back(distr_obj);
                    continue;
                }

                if (bound_object) {
                    cont->RemoveObjectFromContainer(bound_object, count.value());
                    logger::info("\t- {}", distr_obj);
                }
                else if (leveled_list) {
                    cont->RemoveObjectFromContainer(leveled_list, count.value());
                    logger::info("\t- {}", distr_obj);
                }
                else {
                    logger::error("\tERROR: Failed to distribute {}", distr_obj);
                }
            }
            else {
                if (chance.value_or(100) < 100) {
                    Maps::runtime_map[cont_form_id].to_remove_all.push_back(distr_obj);
                    continue;
                }

                const auto& num_to_remove{ cont->CountObjectsInContainer(bound_object) };
                if (bound_object) {
                    cont->RemoveObjectFromContainer(bound_object, num_to_remove);
                    logger::info("\t- {}", distr_obj);
                }
                else if (leveled_list) {
                    cont->RemoveObjectFromContainer(leveled_list, num_to_remove);
                    logger::info("\t- {}", distr_obj);
                }
                else {
                    logger::error("\tERROR: Failed to distribute {}", distr_obj);
                }
            }
        }
    }

    logger::info("");
    logger::info(">-----------------------------------Finished REMOVE distribution-----------------------------------<");
    logger::info("");
}

void Distributor::ReplaceDistribute(const Maps::TDistrVec& distr_vec) noexcept
{
    logger::info("^--------------------------------Performing REPLACE distribution...--------------------------------^");
    logger::info("");

    for (const auto& distr_obj : distr_vec) {
        const auto& [type, bound_object, leveled_list, filename, replace_with_obj, replace_with_list, count, replace_with_count, container, chance]{ distr_obj };
        if (type != DistrType::Replace && type != DistrType::ReplaceAll) {
            continue;
        }

        if (container.has_value()) {
            const auto& [cont, cont_form_id, cont_type, cont_name]{ container.value() };

            if (count.has_value()) {
                if (chance.value_or(100) < 100) {
                    Maps::runtime_map[cont_form_id].to_replace.push_back(distr_obj);
                    continue;
                }

                if (bound_object) {
                    if (replace_with_obj) {
                        cont->RemoveObjectFromContainer(bound_object, count.value());
                        cont->AddObjectToContainer(replace_with_obj, replace_with_count.value(), nullptr);
                        logger::info("\t^ {}", distr_obj);
                    }
                    else if (replace_with_list) {
                        cont->RemoveObjectFromContainer(bound_object, count.value());
                        for (const auto& [lobj, lcount] : Utility::ResolveLeveledList(replace_with_list)) {
                            cont->AddObjectToContainer(lobj, lcount, nullptr);
                        }
                        logger::info("\t^ {}", distr_obj);
                    }
                }
                else if (leveled_list) {
                    if (replace_with_obj) {
                        cont->RemoveObjectFromContainer(leveled_list, count.value());
                        cont->AddObjectToContainer(replace_with_obj, replace_with_count.value(), nullptr);
                        logger::info("\t^ {}", distr_obj);
                    }
                    else if (replace_with_list) {
                        cont->RemoveObjectFromContainer(leveled_list, count.value());
                        for (const auto& [lobj, lcount] : Utility::ResolveLeveledList(replace_with_list)) {
                            cont->AddObjectToContainer(lobj, lcount, nullptr);
                        }
                        logger::info("\t^ {}", distr_obj);
                    }
                }
                else {
                    logger::error("\tERROR: Failed to distribute {}", distr_obj);
                }
            }
            else {
                if (chance.value_or(100) < 100) {
                    Maps::runtime_map[cont_form_id].to_replace_all.push_back(distr_obj);
                    continue;
                }

                const auto& num_to_remove{ cont->CountObjectsInContainer(bound_object) };
                if (bound_object) {
                    if (replace_with_obj) {
                        cont->RemoveObjectFromContainer(bound_object, num_to_remove);
                        cont->AddObjectToContainer(replace_with_obj, num_to_remove, nullptr);
                        logger::info("\t^ {}", distr_obj);
                    }
                    else if (replace_with_list) {
                        cont->RemoveObjectFromContainer(bound_object, num_to_remove);
                        for (const auto& [lobj, lcount] : Utility::ResolveLeveledList(replace_with_list)) {
                            cont->AddObjectToContainer(lobj, lcount, nullptr);
                        }
                        logger::info("\t^ {}", distr_obj);
                    }
                }
                else if (leveled_list) {
                    if (replace_with_obj) {
                        cont->RemoveObjectFromContainer(leveled_list, num_to_remove);
                        cont->AddObjectToContainer(replace_with_obj, num_to_remove, nullptr);
                        logger::info("\t^ {}", distr_obj);
                    }
                    else if (replace_with_list) {
                        cont->RemoveObjectFromContainer(leveled_list, num_to_remove);
                        for (const auto& [lobj, lcount] : Utility::ResolveLeveledList(replace_with_list)) {
                            cont->AddObjectToContainer(lobj, lcount, nullptr);
                        }
                        logger::info("\t^ {}", distr_obj);
                    }
                }
                else {
                    logger::error("\tERROR: Failed to distribute {}", distr_obj);
                }
            }
        }
    }

    logger::info("");
    logger::info("^----------------------------------Finished REPLACE distribution-----------------------------------^");
}

void Distributor::RuntimeDistribute(RE::TESObjectREFR* a_ref) noexcept
{
    const RuntimeDistrVecs* to_modify{};
    const auto              form_id{ a_ref->GetFormID() };
    const auto              base_form_id{ a_ref->GetBaseObject()->GetFormID() };

    if (Maps::runtime_map.contains(form_id)) {
        to_modify = &Maps::runtime_map[form_id];
    }
    else if (Maps::runtime_map.contains(base_form_id)) {
        to_modify = &Maps::runtime_map[base_form_id];
    }

    if (!to_modify) {
        return;
    }

    auto counts{ a_ref->GetInventoryCounts() };

    for (const auto& distr_obj : to_modify->to_add) {
        if (const auto& [type, bound_object, leveled_list, filename, replace_with_obj, replace_with_list, count, replace_with_count, container, chance]{ distr_obj };
            Utility::GetRandomChance() <= chance.value())
        {
            logger::info("x-------------------Performing runtime distribution...--------------------x");
            logger::info("");

            a_ref->GetContainer()->AddObjectToContainer(bound_object, count.value(), nullptr);

            logger::info("\t+ {}", distr_obj);
            logger::info("");
            logger::info("x----------------------Finished runtime distribution----------------------x");
            logger::info("");
        }
    }

    for (const auto& distr_obj : to_modify->to_remove) {
        if (const auto& [type, bound_object, leveled_list, filename, replace_with_obj, replace_with_list, count, replace_with_count, container, chance]{ distr_obj };
            Utility::GetRandomChance() <= chance.value())
        {
            logger::info("x-------------------Performing runtime distribution...--------------------x");
            logger::info("");

            a_ref->RemoveItem(bound_object, count.value(), RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);

            logger::info("\t- {}", distr_obj);
            logger::info("");
            logger::info("x----------------------Finished runtime distribution----------------------x");
            logger::info("");
        }
    }

    for (const auto& distr_obj : to_modify->to_remove_all) {
        if (const auto& [type, bound_object, leveled_list, filename, replace_with_obj, replace_with_list, count, replace_with_count, container, chance]{ distr_obj };
            Utility::GetRandomChance() <= chance.value())
        {
            logger::info("x-------------------Performing runtime distribution...--------------------x");
            logger::info("");

            const auto num_to_remove{ counts[bound_object] };
            a_ref->RemoveItem(bound_object, num_to_remove, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);

            logger::info("\t- {}", distr_obj);
            logger::info("");
            logger::info("x----------------------Finished runtime distribution----------------------x");
            logger::info("");
        }
    }

    for (const auto& distr_obj : to_modify->to_replace) {
        if (const auto& [type, bound_object, leveled_list, filename, replace_with_obj, replace_with_list, count, replace_with_count, container, chance]{ distr_obj };
            Utility::GetRandomChance() <= chance.value())
        {
            logger::info("x-------------------Performing runtime distribution...--------------------x");
            logger::info("");

            a_ref->RemoveItem(bound_object, std::max(counts[bound_object], replace_with_count.value()), RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
            a_ref->GetContainer()->AddObjectToContainer(replace_with_obj, replace_with_count.value(), nullptr);

            logger::info("\t^ {}", distr_obj);
            logger::info("");
            logger::info("x----------------------Finished runtime distribution----------------------x");
            logger::info("");
        }
    }

    for (const auto& distr_obj : to_modify->to_replace_all) {
        if (const auto& [type, bound_object, leveled_list, filename, replace_with_obj, replace_with_list, count, replace_with_count, container, chance]{ distr_obj };
            Utility::GetRandomChance() <= chance.value())
        {
            const auto num_to_swap{ counts[bound_object] };
            logger::info("x-------------------Performing runtime distribution...--------------------x");
            logger::info("");

            a_ref->RemoveItem(bound_object, num_to_swap, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
            a_ref->GetContainer()->AddObjectToContainer(replace_with_obj, num_to_swap, nullptr);

            logger::info("\t^ {}", distr_obj);
            logger::info("");
            logger::info("x----------------------Finished runtime distribution----------------------x");
            logger::info("");
        }
    }
}
