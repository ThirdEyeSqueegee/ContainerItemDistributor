#include "Distributor.h"

void Distributor::Distribute() noexcept
{
    logger::info(">--------------------------------Distributing...--------------------------------<");
    logger::info("");

    const auto start_time{ std::chrono::system_clock::now() };

    AddDistribute(Maps::distr_object_vec);
    RemoveDistribute(Maps::distr_object_vec);
    ReplaceDistribute(Maps::distr_object_vec);

    const auto elapsed{ std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time) };
    logger::info("");
    logger::info(">-------------------------Finished distribution in {} us-------------------------<", elapsed.count());
}

void Distributor::AddDistribute(const Maps::TDistrVec& distr_vec) noexcept
{
    logger::info("+---------Performing ADD distribution...----------+");

    const auto start_time{ std::chrono::system_clock::now() };

    for (const auto& distr_obj : distr_vec) {
        const auto& [type, bound_object, leveled_list, filename, replace_with_obj, replace_with_list, count, replace_with_count, container, chance]{ distr_obj };
        if (type <=> DistrType::Add != 0)
            continue;

        if (container.has_value()) {
            const auto& [cont, cont_form_id, cont_type, cont_name]{ container.value() };

            if (chance.value_or(100) < 100) {
                Maps::runtime_map[cont_form_id].to_add.push_back(distr_obj);
                continue;
            }

            if (bound_object)
                cont->AddObjectToContainer(bound_object, count.value(), nullptr);
            else if (leveled_list) {
                for (const auto& [lobj, lcount] : Utility::ResolveLeveledList(leveled_list))
                    cont->AddObjectToContainer(lobj, lcount, nullptr);
            }
            logger::info("\t+ {}", distr_obj);
        }
    }

    const auto elapsed{ std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time) };
    logger::info("+-------Finished ADD distribution in {} us-------+", elapsed.count());
    logger::info("");
}

void Distributor::RemoveDistribute(const Maps::TDistrVec& distr_vec) noexcept
{
    logger::info(">---------Performing REMOVE distribution...----------<");

    const auto start_time{ std::chrono::system_clock::now() };

    for (const auto& distr_obj : distr_vec) {
        const auto& [type, bound_object, leveled_list, filename, replace_with_obj, replace_with_list, count, replace_with_count, container, chance]{ distr_obj };
        if (type <=> DistrType::Remove != 0 && type <=> DistrType::RemoveAll != 0)
            continue;

        if (container.has_value()) {
            const auto& [cont, cont_form_id, cont_type, cont_name]{ container.value() };

            if (count.has_value()) {
                if (chance.value_or(100) < 100) {
                    Maps::runtime_map[cont_form_id].to_remove.push_back(distr_obj);
                    continue;
                }

                if (bound_object)
                    cont->RemoveObjectFromContainer(bound_object, count.value());
                else if (leveled_list)
                    cont->RemoveObjectFromContainer(leveled_list, count.value());
                logger::info("\t- {}", distr_obj);
            }
            else {
                if (chance.value_or(100) < 100) {
                    Maps::runtime_map[cont_form_id].to_remove_all.push_back(distr_obj);
                    continue;
                }

                const auto& num_to_remove{ cont->CountObjectsInContainer(bound_object) };
                if (bound_object)
                    cont->RemoveObjectFromContainer(bound_object, num_to_remove);
                else if (leveled_list)
                    cont->RemoveObjectFromContainer(leveled_list, num_to_remove);
                logger::info("\t- {}", distr_obj);
            }
        }
    }

    const auto elapsed{ std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time) };
    logger::info(">-------Finished REMOVE distribution in {} us-------<", elapsed.count());
    logger::info("");
}

void Distributor::ReplaceDistribute(const Maps::TDistrVec& distr_vec) noexcept
{
    logger::info("^---------Performing REPLACE distribution...---------^");

    const auto start_time{ std::chrono::system_clock::now() };

    for (const auto& distr_obj : distr_vec) {
        const auto& [type, bound_object, leveled_list, filename, replace_with_obj, replace_with_list, count, replace_with_count, container, chance]{ distr_obj };
        if (type <=> DistrType::Replace != 0 && type <=> DistrType::ReplaceAll != 0)
            continue;

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
                    }
                    else if (replace_with_list) {
                        cont->RemoveObjectFromContainer(bound_object, count.value());
                        for (const auto& [lobj, lcount] : Utility::ResolveLeveledList(replace_with_list))
                            cont->AddObjectToContainer(lobj, lcount, nullptr);
                    }
                }
                else if (leveled_list) {
                    if (replace_with_obj) {
                        cont->RemoveObjectFromContainer(leveled_list, count.value());
                        cont->AddObjectToContainer(replace_with_obj, replace_with_count.value(), nullptr);
                    }
                    else if (replace_with_list) {
                        cont->RemoveObjectFromContainer(leveled_list, count.value());
                        for (const auto& [lobj, lcount] : Utility::ResolveLeveledList(replace_with_list))
                            cont->AddObjectToContainer(lobj, lcount, nullptr);
                    }
                }
                logger::info("\t^ {}", distr_obj);
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
                    }
                    else if (replace_with_list) {
                        cont->RemoveObjectFromContainer(bound_object, num_to_remove);
                        for (const auto& [lobj, lcount] : Utility::ResolveLeveledList(replace_with_list))
                            cont->AddObjectToContainer(lobj, lcount, nullptr);
                    }
                }
                else if (leveled_list) {
                    if (replace_with_obj) {
                        cont->RemoveObjectFromContainer(leveled_list, num_to_remove);
                        cont->AddObjectToContainer(replace_with_obj, num_to_remove, nullptr);
                    }
                    else if (replace_with_list) {
                        cont->RemoveObjectFromContainer(leveled_list, num_to_remove);
                        for (const auto& [lobj, lcount] : Utility::ResolveLeveledList(replace_with_list))
                            cont->AddObjectToContainer(lobj, lcount, nullptr);
                    }
                }
                logger::info("\t^ {}", distr_obj);
            }
        }
    }

    const auto elapsed{ std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time) };
    logger::info("^-------Finished REPLACE distribution in {} us-------^", elapsed.count());
}

void Distributor::RuntimeDistribute(RE::TESObjectREFR* a_ref)
{
    if (!a_ref)
        return;

    RuntimeDistrMap* to_modify = nullptr;

    if (Maps::runtime_map.count(a_ref->GetFormID())) {
        to_modify = &Maps::runtime_map[a_ref->GetFormID()];
    }
    else if (Maps::runtime_map.count(a_ref->GetBaseObject()->GetFormID())) {
        to_modify = &Maps::runtime_map[a_ref->GetBaseObject()->GetFormID()];
    }

    if (to_modify) {
        auto counts = a_ref->GetInventoryCounts();

        for (auto& distr_obj : to_modify->to_add) {
            const auto& [type, bound_object, leveled_list, filename, replace_with_obj, replace_with_list, count, replace_with_count, container, chance]{ distr_obj };

            const auto num_to_add = Utility::GetRandomCount(count.value(), chance.value());

            logger::info("Adding {} {} to {}", num_to_add, bound_object->GetFormID(), a_ref->GetFormID());

            a_ref->AddObjectToContainer(bound_object, nullptr, num_to_add, nullptr);
        }

        for (auto& distr_obj : to_modify->to_remove) {
            const auto& [type, bound_object, leveled_list, filename, replace_with_obj, replace_with_list, count, replace_with_count, container, chance]{ distr_obj };

            const auto num_to_remove = Utility::GetRandomCount(count.value(), chance.value());

            logger::info("Removing {} {} from {}", num_to_remove, bound_object->GetFormID(), a_ref->GetFormID());

            a_ref->RemoveItem(bound_object, num_to_remove, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
        }

        for (auto& distr_obj : to_modify->to_remove_all) {
            const auto& [type, bound_object, leveled_list, filename, replace_with_obj, replace_with_list, count, replace_with_count, container, chance]{ distr_obj };

            if (Utility::GetRandomChance() < chance.value()) {
                const auto num_to_remove = counts[bound_object];
                logger::info("Removing {} {} from {}", num_to_remove, bound_object->GetFormID(), a_ref->GetFormID());
                a_ref->RemoveItem(bound_object, num_to_remove, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
            }
        }

        for (auto& distr_obj : to_modify->to_replace) {
            const auto& [type, bound_object, leveled_list, filename, replace_with_obj, replace_with_list, count, replace_with_count, container, chance]{ distr_obj };

            const auto num_to_swap = Utility::GetRandomCount(count.value(), chance.value());

            logger::info("Swapping {} {} for {} from {}", num_to_swap, bound_object->GetFormID(), replace_with_obj->GetFormID(), a_ref->GetFormID());

            a_ref->RemoveItem(bound_object, num_to_swap, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
            a_ref->AddObjectToContainer(replace_with_obj, nullptr, replace_with_count.value(), nullptr);
        }

        for (auto& distr_obj : to_modify->to_replace_all) {
            const auto& [type, bound_object, leveled_list, filename, replace_with_obj, replace_with_list, count, replace_with_count, container, chance]{ distr_obj };

            if (Utility::GetRandomChance() < chance.value()) {
                const auto num_to_swap = counts[bound_object];
                logger::info("Swapping {} {} for {} from {}", num_to_swap, bound_object->GetFormID(), replace_with_obj->GetFormID(), a_ref->GetFormID());

                a_ref->RemoveItem(bound_object, num_to_swap, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
                a_ref->AddObjectToContainer(replace_with_obj, nullptr, num_to_swap, nullptr);
            }
        }
    }
}
