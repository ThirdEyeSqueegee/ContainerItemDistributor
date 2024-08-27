#include "Distributor.h"

#include "Map.h"
#include "Utility.h"

void Distributor::Distribute(RE::TESObjectREFR* a_ref, const bool is_reset) noexcept
{
    const auto form_id{ a_ref->GetFormID() };

    if (is_reset) {
        if (Map::leveled_reset_map.contains(form_id)) {
            Utility::RemoveResolvedList(a_ref, Map::leveled_reset_map[form_id]);
            const auto& [lev_item, count]{ Map::leveled_distr_map[form_id] };
            Utility::AddObjectsFromResolvedList(a_ref, lev_item, count);
            return;
        }
    }

    const DistrVecs* to_modify{};
    const auto       base_form_id{ a_ref->GetBaseObject()->GetFormID() };

    if (Map::distr_map.contains(form_id)) {
        to_modify = &Map::distr_map[form_id];
    }
    else if (Map::distr_map.contains(base_form_id)) {
        to_modify = &Map::distr_map[base_form_id];
    }

    if (!to_modify) {
        return;
    }

    for (const auto& distr_obj : to_modify->to_add) {
        if (const auto& [type, bound_object, count, container, chance]{ distr_obj }; Utility::GetRandomChance() <= chance) {
            if (const auto lev_item{ bound_object->As<RE::TESLevItem>() }) {
                Utility::AddObjectsFromResolvedList(a_ref, lev_item, count);
                Map::leveled_distr_map[form_id] = std::make_pair(lev_item, count);
            }
            else {
                a_ref->AddObjectToContainer(bound_object, nullptr, count, nullptr);
                logger::info("+ {} / Container ref: {:#x}", distr_obj, form_id);
            }
        }
    }

    for (const auto& distr_obj : to_modify->to_remove) {
        if (const auto& [type, bound_object, count, container, chance]{ distr_obj }; Utility::GetRandomChance() <= chance) {
            a_ref->RemoveItem(bound_object, count, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
            logger::info("- {} / Container ref: {:#x}", distr_obj, form_id);
        }
    }

    for (const auto& distr_obj : to_modify->to_remove_all) {
        if (const auto& [type, bound_object, count, container, chance]{ distr_obj }; Utility::GetRandomChance() <= chance) {
            const auto inv_count{ a_ref->GetInventoryCounts().at(bound_object) };
            a_ref->RemoveItem(bound_object, inv_count, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
            logger::info("- {} / Container ref: {:#x}", distr_obj, form_id);
        }
    }

    Map::processed_refs.insert(form_id);
}
