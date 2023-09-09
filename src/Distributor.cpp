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

    for (const auto& distr_obj : distr_vec)
    {
        const auto& [type, bound_object, filename, count, replace_with_obj, replace_with_count, container]{ distr_obj };
        if (type <=> DistrType::Add != 0)
            continue;

        if (container.has_value())
        {
            const auto& [cont, cont_form_id, cont_type, cont_name]{ container.value() };
            cont->AddObjectToContainer(bound_object, count.value(), nullptr);
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

    for (const auto& distr_obj : distr_vec)
    {
        const auto& [type, bound_object, filename, count, replace_with_obj, replace_with_count, container]{ distr_obj };
        if (type <=> DistrType::Remove != 0 && type <=> DistrType::RemoveAll != 0)
            continue;

        if (container.has_value())
        {
            const auto& [cont, cont_form_id, cont_type, cont_name]{ container.value() };
            if (count.has_value())
            {
                cont->RemoveObjectFromContainer(bound_object, count.value());
                logger::info("\t- {}", distr_obj);
            }
            else
            {
                const auto& num_to_remove{ cont->CountObjectsInContainer(bound_object) };
                cont->RemoveObjectFromContainer(bound_object, num_to_remove);
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

    for (const auto& distr_obj : distr_vec)
    {
        const auto& [type, bound_object, filename, count, replace_with_obj, replace_with_count, container]{ distr_obj };
        if (type <=> DistrType::Replace != 0 && type <=> DistrType::ReplaceAll != 0)
            continue;

        if (container.has_value())
        {
            const auto& [cont, cont_form_id, cont_type, cont_name]{ container.value() };
            if (count.has_value())
            {
                cont->RemoveObjectFromContainer(bound_object, count.value());
                cont->AddObjectToContainer(replace_with_obj.value(), replace_with_count.value(), nullptr);
                logger::info("\t^ {}", distr_obj);
            }
            else
            {
                const auto& num_to_remove{ cont->CountObjectsInContainer(bound_object) };
                cont->RemoveObjectFromContainer(bound_object, num_to_remove);
                cont->AddObjectToContainer(replace_with_obj.value(), replace_with_count.value(), nullptr);
                logger::info("\t^ {}", distr_obj);
            }
        }
    }

    const auto elapsed{ std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time) };
    logger::info("^-------Finished REPLACE distribution in {} us-------^", elapsed.count());
}
