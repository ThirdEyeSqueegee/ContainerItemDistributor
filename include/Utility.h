#pragma once

#include "Map.h"

class Utility
{
    [[nodiscard]] static auto IsEditorID(const std::string_view identifier) noexcept { return !identifier.contains('~'); }

    [[nodiscard]] static FormIDAndPluginName GetFormIDAndPluginName(const std::string& identifier) noexcept
    {
        if (const auto tilde_pos{ identifier.find('~') }; tilde_pos != std::string_view::npos) {
            const auto form_id{ Map::ToFormID(identifier.substr(0, tilde_pos)) };
            const auto plugin_name{ identifier.substr(tilde_pos + 1) };
            return { .form_id = form_id, .plugin_name = plugin_name };
        }
        logger::error("\t\tERROR: Failed to get FormID and plugin name for {}", identifier);

        return { .form_id = 0x0, .plugin_name = "" };
    }

    [[nodiscard]] static RE::TESBoundObject* GetBoundObject(const std::string& identifier) noexcept
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

    [[nodiscard]] static RE::FormID GetContainerFormID(const std::string& to_identifier) noexcept
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

    [[nodiscard]] static RE::BGSLocation* GetLocation(const std::string& identifier) noexcept
    {
        if (identifier.empty()) {
            return nullptr;
        }

        if (IsEditorID(identifier)) {
            if (const auto location{ RE::TESForm::LookupByEditorID<RE::BGSLocation>(identifier) }) {
                return location;
            }
        }
        else {
            const auto handler{ RE::TESDataHandler::GetSingleton() };
            const auto [form_id, plugin_name]{ GetFormIDAndPluginName(identifier) };
            if (const auto form{ handler->LookupForm(form_id, plugin_name) }) {
                if (const auto location{ form->As<RE::BGSLocation>() }) {
                    return location;
                }
            }
        }
        logger::warn("\t\tWARNING: Failed to find location for {}", identifier);

        return nullptr;
    }

    [[nodiscard]] static RE::BGSKeyword* GetLocationKeyword(const std::string& identifier) noexcept
    {
        if (identifier.empty()) {
            return nullptr;
        }

        if (IsEditorID(identifier)) {
            if (const auto location_keyword{ RE::TESForm::LookupByEditorID<RE::BGSKeyword>(identifier) }) {
                return location_keyword;
            }
        }
        else {
            const auto handler{ RE::TESDataHandler::GetSingleton() };
            const auto [form_id, plugin_name]{ GetFormIDAndPluginName(identifier) };
            if (const auto form{ handler->LookupForm(form_id, plugin_name) }) {
                if (const auto location_keyword{ form->As<RE::BGSKeyword>() }) {
                    return location_keyword;
                }
            }
        }
        logger::warn("\t\tWARNING: Failed to find location keyword for {}", identifier);

        return nullptr;
    }

    [[nodiscard]] static auto ResolveLeveledList(RE::TESLevItem* leveled_list, const u32 count) noexcept
    {
        RE::BSScrapArray<RE::CALCED_OBJECT>                    calced_objects;
        ankerl::unordered_dense::map<RE::TESBoundObject*, u32> result;

        if (const auto player{ RE::PlayerCharacter::GetSingleton() }) {
            leveled_list->CalculateCurrentFormList(player->GetLevel(), static_cast<i16>(count), calced_objects, 0, true);
        }
        else {
            logger::error("\t\tERROR: Failed to find player level for resolving leveled list {} ({:#x})", GetFormEditorID(leveled_list), leveled_list->GetFormID());
        }

        for (const auto& c : calced_objects) {
            if (const auto bound_obj{ c.form->As<RE::TESBoundObject>() }) {
                result[bound_obj] = c.count;
            }
        }

        return result;
    }

public:
    [[nodiscard]] static auto GetRandomChance() noexcept
    {
        static std::random_device                 rd;
        static std::mt19937                       rng(rd());
        static std::uniform_int_distribution<u16> distr(1, 100);

        return distr(rng);
    }

    static auto AddObjectsFromResolvedList(RE::TESObjectREFR* ref, RE::TESLevItem* leveled_list, const u32 count) noexcept
    {
        logger::info("Adding {} {} to ref {}", count, leveled_list, ref);

        for (const auto& [bound_obj, c] : ResolveLeveledList(leveled_list, count)) {
            ref->AddObjectToContainer(bound_obj, nullptr, c, nullptr);
            Map::added_objects[ref].emplace_back(bound_obj, c);
            logger::info("\t+ {} {}", c, bound_obj);
        }

        logger::info("");
    }

    [[nodiscard]] static DistrObject BuildDistrObject(const DistrToken& distr_token) noexcept
    {
        if (const auto bound_obj{ GetBoundObject(distr_token.identifier) }) {
            return { .type              = distr_token.type,
                     .container_form_id = GetContainerFormID(distr_token.to_identifier),
                     .bound_object      = bound_obj,
                     .count             = distr_token.count,
                     .location          = GetLocation(distr_token.location),
                     .location_keyword  = GetLocationKeyword(distr_token.location_keyword),
                     .chance            = distr_token.chance };
        }
        logger::error("\t\tERROR: Failed to build DistrObject for {}", distr_token);

        return { .type = DistrType::Error, .container_form_id = 0x0U, .bound_object = nullptr, .count = 0U, .location = nullptr, .location_keyword = nullptr, .chance = 0U };
    }

    [[nodiscard]] static auto ShouldSkip(RE::TESObjectREFR* ref, const RE::BGSLocation* location, const RE::BGSKeyword* location_keyword) noexcept
    {
        if (const auto ref_location{ ref->GetCurrentLocation() }) {
            if (location) {
                if (ref_location->GetFormID() == location->GetFormID()) {
                    logger::debug("! Skipping {}, location {} does not match {} ({:#x})", ref, GetFormEditorID(ref->GetCurrentLocation()), GetFormEditorID(location),
                                  location->GetFormID());
                    logger::debug("");
                    return true;
                }
            }
            if (location_keyword) {
                if (!ref_location->HasKeyword(location_keyword)) {
                    logger::debug("! Skipping {}, location {} does not have keyword {} ({:#x})", ref, GetFormEditorID(ref->GetCurrentLocation()), GetFormEditorID(location_keyword),
                                  location_keyword->GetFormID());
                    logger::debug("");
                    return true;
                }
            }
        }
        else {
            logger::error("ERROR: Failed to get current location for {} ({:#x})", GetFormEditorID(ref), ref->GetFormID());
        }

        return false;
    }
};
