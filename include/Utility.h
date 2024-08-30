#pragma once

#include "Map.h"

class Utility : public Singleton<Utility>
{
    static auto IsEditorID(const std::string_view identifier) noexcept { return !identifier.contains('~'); }

    static FormIDAndPluginName GetFormIDAndPluginName(const std::string& identifier) noexcept
    {
        if (const auto tilde_pos{ identifier.find('~') }; tilde_pos != std::string_view::npos) {
            const auto form_id{ Map::ToFormID(identifier.substr(0, tilde_pos)) };
            const auto plugin_name{ identifier.substr(tilde_pos + 1) };
            return { .form_id = form_id, .plugin_name = plugin_name };
        }
        logger::error("\t\tERROR: Failed to get FormID and plugin name for {}", identifier);

        return { .form_id = 0x0, .plugin_name = "" };
    }

    static RE::TESBoundObject* GetBoundObject(const std::string& identifier) noexcept
    {
        if (IsEditorID(identifier)) {
            if (const auto bound_obj{ RE::TESForm::LookupByEditorID<RE::TESBoundObject>(identifier) }) {
                return bound_obj;
            }
        }
        else {
            const auto handler{ RE::TESDataHandler::GetSingleton() };
            const auto [form_id, plugin_name]{ GetFormIDAndPluginName(identifier) };
            if (const auto form{ handler->LookupForm(form_id, plugin_name) }) {
                if (const auto bound_obj{ form->As<RE::TESBoundObject>() }) {
                    return bound_obj;
                }
            }
        }
        logger::warn("\t\tWARNING: Failed to find bound object for {}", identifier);

        return nullptr;
    }

    static auto GetContainerFormID(const std::string& to_identifier) noexcept
    {
        if (IsEditorID(to_identifier)) {
            if (const auto form{ RE::TESForm::LookupByEditorID(to_identifier) }) {
                return form->GetFormID();
            }
        }
        else {
            const auto [form_id, plugin_name]{ GetFormIDAndPluginName(to_identifier) };
            const auto handler{ RE::TESDataHandler::GetSingleton() };

            return handler->LookupFormID(form_id, plugin_name);
        }

        return 0x0U;
    }

    static ankerl::unordered_dense::map<RE::TESBoundObject*, u32> ResolveLeveledList(RE::TESLevItem* leveled_list, const u32 count, const bool is_nested = false) noexcept
    {
        if (is_nested) {
            logger::debug("\t\tResolving nested leveled list {} ({:#x})", GetFormEditorID(leveled_list), leveled_list->GetFormID());
        }

        RE::BSScrapArray<RE::CALCED_OBJECT>                    calced_objects;
        ankerl::unordered_dense::map<RE::TESBoundObject*, u32> result;

        if (const auto player{ RE::PlayerCharacter::GetSingleton() }) {
            leveled_list->CalculateCurrentFormList(player->GetLevel(), static_cast<i16>(count), calced_objects, 0, true);
        }
        else {
            logger::error("\t\tERROR: Failed to find player level for resolving leveled list {} ({:#x})", GetFormEditorID(leveled_list), leveled_list->GetFormID());
        }

        for (const auto& c : calced_objects) {
            if (const auto lev_item{ c.form->As<RE::TESLevItem>() }) {
                const auto nested_result{ ResolveLeveledList(lev_item, c.count, true) };
                result.insert(nested_result.begin(), nested_result.end());
            }
            if (const auto bound_obj{ c.form->As<RE::TESBoundObject>() }) {
                result[bound_obj] = c.count;
            }
        }

        return result;
    }

public:
    static auto GetRandomChance() noexcept
    {
        static std::random_device                 rd;
        static std::mt19937                       rng(rd());
        static std::uniform_int_distribution<u32> distr(1, 100);

        return distr(rng);
    }

    static auto GetChanceFromToken(const std::string& s) noexcept
    {
        const auto pos{ s.find('?') };

        return pos == std::string_view::npos ? 100 : Map::ToUnsignedInt(s.substr(pos + 1));
    }

    static auto AddObjectsFromResolvedList(RE::TESObjectREFR* ref, RE::TESLevItem* leveled_list, const u32 count) noexcept
    {
        const auto ref_id{ ref->GetFormID() };
        logger::info("Adding resolved leveled list {} ({:#x}) to ref {} ({:#x})", GetFormEditorID(leveled_list), leveled_list->GetFormID(), GetFormEditorID(ref), ref_id);
        for (const auto& [bound_obj, c] : ResolveLeveledList(leveled_list, count)) {
            ref->AddObjectToContainer(bound_obj, nullptr, c, nullptr);
            logger::info("\t+ {} {} ({:#x})", c, bound_obj->GetName(), bound_obj->GetFormID());
            Map::leveled_reset_map[ref_id][bound_obj] = c;
        }
        logger::info("");
    }

    static auto RemoveResolvedList(RE::TESObjectREFR* ref, const ankerl::unordered_dense::map<RE::TESBoundObject*, u32>& resolved_list) noexcept
    {
        const auto ref_id{ ref->GetFormID() };
        logger::info("\tRemoving previously resolved leveled list from ref {} ({:#x})", GetFormEditorID(ref), ref_id);
        for (const auto& [bound_obj, count] : resolved_list) {
            ref->RemoveItem(bound_obj, count, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
            logger::info("\t\t- {} {} ({:#x})", count, bound_obj->GetName(), bound_obj->GetFormID());
        }
        Map::leveled_reset_map.erase(ref_id);
        logger::info("");
    }

    static DistrObject BuildDistrObject(const DistrToken& distr_token) noexcept
    {
        if (const auto bound_obj{ GetBoundObject(distr_token.identifier) }) {
            return { .type              = distr_token.type,
                     .bound_object      = bound_obj,
                     .count             = distr_token.count,
                     .container_form_id = GetContainerFormID(distr_token.to_identifier),
                     .chance            = distr_token.chance };
        }
        logger::error("\t\tERROR: Failed to build DistrObject for {}", distr_token);

        return { .type = DistrType::Error, .bound_object = nullptr, .count = 0, .container_form_id = 0x0, .chance = 0 };
    }
};
