#pragma once

#include "Maps.h"

class Utility : public Singleton<Utility>
{
    static bool IsEditorID(const std::string_view identifier)
    {
        return std::strchr(identifier.data(), '~') == nullptr;
    }

    static FormIDAndPluginName GetFormIDAndPluginName(const std::string_view identifier)
    {
        if (const auto tilde{ std::strchr(identifier.data(), '~') })
        {
            const auto tilde_pos{ static_cast<int>(tilde - identifier.data()) };
            return { Maps::ToUint32(identifier.substr(0, tilde_pos)), identifier.substr(tilde_pos + 1).data() };
        }
        return { 0, "" };
    }

    static RE::TESBoundObject* GetBoundObject(const std::string_view identifier)
    {
        if (IsEditorID(identifier))
        {
            if (const auto obj{ RE::TESForm::LookupByEditorID<RE::TESBoundObject>(identifier) })
                return obj;
        }
        else
        {
            const auto handler{ RE::TESDataHandler::GetSingleton() };
            const auto [form_id, plugin_name]{ GetFormIDAndPluginName(identifier) };
            if (const auto obj{ handler->LookupForm(form_id, plugin_name) })
            {
                if (const auto bound_obj{ obj->As<RE::TESBoundObject>() })
                    return bound_obj;
            }
        }
        return nullptr;
    }

    static Container GetContainer(const std::string_view to_identifier)
    {
        if (IsEditorID(to_identifier))
        {
            if (const auto form{ RE::TESForm::LookupByEditorID(to_identifier) })
            {
                if (const auto cont{ form->As<RE::TESContainer>() })
                    return { cont, form->GetFormID(), form->GetFormType(), form->GetName() };
            }
        }
        else
        {
            const auto handler{ RE::TESDataHandler::GetSingleton() };
            const auto [form_id, plugin_name]{ GetFormIDAndPluginName(to_identifier) };
            if (const auto form{ handler->LookupForm(form_id, plugin_name) })
            {
                if (const auto cont{ form->As<RE::TESContainer>() })
                    return { cont, form->GetFormID(), form->GetFormType(), form->GetName() };
            }
        }
        return { nullptr, 0, RE::FormType::Container, "" };
    }

public:
    static DistrObject BuildDistrObject(const DistrToken& distr_token) noexcept
    {
        if (const auto bound_obj{ GetBoundObject(distr_token.identifier) })
        {
            if (const auto cont{ GetContainer(distr_token.to_identifier) }; cont.container)
            {
                if (distr_token.type <=> DistrType::Replace == 0 || distr_token.type <=> DistrType::ReplaceAll == 0)
                {
                    if (const auto replace_with_obj{ GetBoundObject(distr_token.rhs.value()) })
                        return {
                            distr_token.type, bound_obj, distr_token.filename, distr_token.count, replace_with_obj, distr_token.rhs_count, cont
                        };
                }
                return { distr_token.type, bound_obj, distr_token.filename, distr_token.count, std::nullopt, std::nullopt, cont };
            }
        }

        return { DistrType::Error, nullptr, distr_token.filename, std::nullopt, std::nullopt, std::nullopt, std::nullopt };
    }
};
