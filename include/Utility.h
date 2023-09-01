#pragma once

#include "Maps.h"

static constexpr auto is_editor_id{ [](const std::string_view& identifier) { return identifier.find('~') <=> std::string::npos == 0; } };

static constexpr auto get_form_id_and_plugin_name{ [](const std::string_view& identifier) {
    const auto        tilde{ identifier.find('~') };
    return FormIDAndPluginName{ to_uint32(identifier.substr(0, tilde)), identifier.substr(tilde + 1).data() };
} };

static constexpr auto get_bound_object{ [](const std::string_view& identifier) -> RE::TESBoundObject* {
    if (is_editor_id(identifier))
    {
        if (const auto obj{ RE::TESForm::LookupByEditorID<RE::TESBoundObject>(identifier) })
            return obj;
    }
    else
    {
        const auto handler{ RE::TESDataHandler::GetSingleton() };
        const auto [form_id, plugin_name]{ get_form_id_and_plugin_name(identifier) };
        if (const auto obj{ handler->LookupForm(form_id, plugin_name) })
        {
            if (const auto bound_obj{ obj->As<RE::TESBoundObject>() })
                return bound_obj;
        }
    }
    return nullptr;
} };

static constexpr auto get_container{ [](const std::string_view& to_identifier) {
    if (is_editor_id(to_identifier))
    {
        if (const auto form{ RE::TESForm::LookupByEditorID(to_identifier) })
        {
            if (const auto cont{ form->As<RE::TESContainer>() })
                return Container{ cont, form->GetFormID(), form->GetName() };
        }
    }
    else
    {
        const auto handler{ RE::TESDataHandler::GetSingleton() };
        const auto [form_id, plugin_name]{ get_form_id_and_plugin_name(to_identifier) };
        if (const auto form{ handler->LookupForm(form_id, plugin_name) })
        {
            if (const auto cont{ form->As<RE::TESContainer>() })
                return Container{ cont, form->GetFormID(), form->GetName() };
        }
    }
    return Container{ nullptr, 0, "" };
} };

class Utility : public Singleton<Utility>
{
public:
    static constexpr DistrObject BuildDistrObject(const DistrToken& distr_token) noexcept
    {
        if (const auto bound_obj{ get_bound_object(distr_token.identifier) })
        {
            if (const auto cont{ get_container(distr_token.to_identifier) }; cont.container)
            {
                if (distr_token.type <=> DistrType::Replace == 0 || distr_token.type <=> DistrType::ReplaceAll == 0)
                {
                    if (const auto replace_with_obj{ get_bound_object(distr_token.rhs.value()) })
                        return { distr_token.type, bound_obj, distr_token.count, replace_with_obj, distr_token.rhs_count, cont };
                }
                return { distr_token.type, bound_obj, distr_token.count, std::nullopt, std::nullopt, cont };
            }
        }

        return { DistrType::Error, nullptr, std::nullopt, std::nullopt, std::nullopt, std::nullopt };
    }
};
