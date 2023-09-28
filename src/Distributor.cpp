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
        const auto& [type, bound_object, leveled_list, filename, replace_with_obj, replace_with_list, count, replace_with_count, container]{ distr_obj };
        if (type <=> DistrType::Add != 0)
            continue;

        if (container.has_value()) {
            const auto& [cont, cont_form_id, cont_type, cont_name]{ container.value() };
            if (bound_object)
                cont->AddObjectToContainer(bound_object, count.value(), nullptr);
            else if (leveled_list)
                cont->AddObjectToContainer(leveled_list, count.value(), nullptr);
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
        const auto& [type, bound_object, leveled_list, filename, replace_with_obj, replace_with_list, count, replace_with_count, container]{ distr_obj };
        if (type <=> DistrType::Remove != 0 && type <=> DistrType::RemoveAll != 0)
            continue;

        if (container.has_value()) {
            const auto& [cont, cont_form_id, cont_type, cont_name]{ container.value() };
            if (count.has_value()) {
                if (bound_object)
                    cont->RemoveObjectFromContainer(bound_object, count.value());
                else if (leveled_list)
                    cont->RemoveObjectFromContainer(leveled_list, count.value());
                logger::info("\t- {}", distr_obj);
            }
            else {
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
        const auto& [type, bound_object, leveled_list, filename, replace_with_obj, replace_with_list, count, replace_with_count, container]{ distr_obj };
        if (type <=> DistrType::Replace != 0 && type <=> DistrType::ReplaceAll != 0)
            continue;

        if (container.has_value()) {
            const auto& [cont, cont_form_id, cont_type, cont_name]{ container.value() };
            if (count.has_value()) {
                if (bound_object) {
                    if (replace_with_obj) {
                        cont->RemoveObjectFromContainer(bound_object, count.value());
                        cont->AddObjectToContainer(replace_with_obj, replace_with_count.value(), nullptr);
                    }
                    else if (replace_with_list) {
                        cont->RemoveObjectFromContainer(bound_object, count.value());
                        cont->AddObjectToContainer(replace_with_list, replace_with_count.value(), nullptr);
                    }
                }
                else if (leveled_list) {
                    if (replace_with_obj) {
                        cont->RemoveObjectFromContainer(leveled_list, count.value());
                        cont->AddObjectToContainer(replace_with_obj, replace_with_count.value(), nullptr);
                    }
                    else if (replace_with_list) {
                        cont->RemoveObjectFromContainer(leveled_list, count.value());
                        cont->AddObjectToContainer(replace_with_list, replace_with_count.value(), nullptr);
                    }
                }
                logger::info("\t^ {}", distr_obj);
            }
            else {
                const auto& num_to_remove{ cont->CountObjectsInContainer(bound_object) };
                if (bound_object) {
                    if (replace_with_obj) {
                        cont->RemoveObjectFromContainer(bound_object, num_to_remove);
                        cont->AddObjectToContainer(replace_with_obj, num_to_remove, nullptr);
                    }
                    else if (replace_with_list) {
                        cont->RemoveObjectFromContainer(bound_object, num_to_remove);
                        cont->AddObjectToContainer(replace_with_list, num_to_remove, nullptr);
                    }
                }
                else if (leveled_list) {
                    if (replace_with_obj) {
                        cont->RemoveObjectFromContainer(leveled_list, num_to_remove);
                        cont->AddObjectToContainer(replace_with_obj, num_to_remove, nullptr);
                    }
                    else if (replace_with_list) {
                        cont->RemoveObjectFromContainer(leveled_list, num_to_remove);
                        cont->AddObjectToContainer(replace_with_list, num_to_remove, nullptr);
                    }
                }
                logger::info("\t^ {}", distr_obj);
            }
        }
    }

    const auto elapsed{ std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time) };
    logger::info("^-------Finished REPLACE distribution in {} us-------^", elapsed.count());
}
