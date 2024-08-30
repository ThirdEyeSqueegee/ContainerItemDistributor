#include "Distributor.h"

#include "Map.h"
#include "Utility.h"

void Distributor::Distribute(RE::TESObjectREFR* a_ref, const bool is_reset) noexcept
{
    const auto form_id{ a_ref->GetFormID() };
    const auto base_form_id{ a_ref->GetBaseObject()->GetFormID() };
    const auto edid{ GetFormEditorID(a_ref) };

    if (is_reset && !Map::respawn_containers.contains(form_id)) {
        return;
    }

    if (is_reset) {
        if (Map::leveled_reset_map.contains(form_id)) {
            Utility::RemoveResolvedList(a_ref, Map::leveled_reset_map[form_id]);
            const auto& [lev_item, count]{ Map::leveled_distr_map[form_id] };
            Utility::AddObjectsFromResolvedList(a_ref, lev_item, count);
            return;
        }
    }

    if (Map::processed_containers.contains(form_id)) {
        return;
    }

    const DistrVecs* to_modify{};

    if (Map::distr_map.contains(form_id)) {
        to_modify = &Map::distr_map[form_id];
    }
    else if (Map::distr_map.contains(base_form_id)) {
        to_modify = &Map::distr_map[base_form_id];
    }

    if (!to_modify) {
        return;
    }

    Map::processed_containers.insert(form_id);

    for (const auto& distr_obj : to_modify->to_add) {
        if (const auto& [type, bound_object, count, container, chance]{ distr_obj }; Utility::GetRandomChance() <= chance) {
            if (const auto lev_item{ bound_object->As<RE::TESLevItem>() }) {
                Utility::AddObjectsFromResolvedList(a_ref, lev_item, count);
                Map::leveled_distr_map[form_id] = std::make_pair(lev_item, count);
            }
            else {
                a_ref->AddObjectToContainer(bound_object, nullptr, count, nullptr);
                logger::info("+ {} / Container ref: {} ({:#x})", distr_obj, edid, form_id);
                logger::info("");
            }
        }
    }

    for (const auto& distr_obj : to_modify->to_remove) {
        if (const auto& [type, bound_object, count, container, chance]{ distr_obj }; Utility::GetRandomChance() <= chance) {
            a_ref->RemoveItem(bound_object, count, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
            logger::info("- {} / Container ref: {} ({:#x})", distr_obj, edid, form_id);
            logger::info("");
        }
    }

    for (const auto& distr_obj : to_modify->to_remove_all) {
        if (const auto& [type, bound_object, count, container, chance]{ distr_obj }; Utility::GetRandomChance() <= chance) {
            if (const auto lev_item{ bound_object->As<RE::TESLevItem>() }) {
                a_ref->RemoveItem(lev_item, 999, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
                logger::info("- {} / Container ref: {} ({:#x})", distr_obj, edid, form_id);
                logger::info("");

                continue;
            }
            const auto inv_map{ a_ref->GetInventoryCounts() };
            if (!inv_map.contains(bound_object)) {
                logger::error("ERROR: Could not find {} ({:#x}) in inventory counts map of {} ({:#x})", GetFormEditorID(bound_object), bound_object->GetFormID(), edid, form_id);
                continue;
            }
            const auto inv_count{ inv_map.at(bound_object) };

            a_ref->RemoveItem(bound_object, inv_count, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
            logger::info("- {} / Container ref: {} ({:#x})", distr_obj, edid, form_id);
            logger::info("");
        }
    }
}
