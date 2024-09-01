#pragma once

#include "ankerl/unordered_dense.h"

enum struct DistrType : u8
{
    Add,
    Remove,
    RemoveAll,
    Error
};

struct DistrToken
{
    DistrType   type{};
    std::string to_identifier{};
    std::string identifier{};
    u16         count{};
    std::string location{};
    std::string location_keyword{};
    u16         chance{};
};

struct DistrObject
{
    DistrType           type{};
    RE::FormID          container_form_id{};
    RE::TESBoundObject* bound_object{};
    u16                 count{};
    RE::BGSLocation*    location{};
    RE::BGSKeyword*     location_keyword{};
    u16                 chance{};
};

struct FormIDAndPluginName
{
    RE::FormID  form_id{};
    std::string plugin_name{};
};

using TDistrVec = std::vector<DistrObject>;

struct DistrVecs
{
    TDistrVec to_add;
    TDistrVec to_remove;
    TDistrVec to_remove_all;
};

class Map : public Singleton<Map>
{
    template <typename K, typename V>
    using map = ankerl::unordered_dense::map<K, V>;

    template <typename K>
    using set = ankerl::unordered_dense::set<K>;

public:
    [[nodiscard]] static auto ToFormID(const std::string& s) noexcept { return static_cast<RE::FormID>(std::stoul(s, nullptr, 16)); }

    [[nodiscard]] static auto ToUnsignedInt(const std::string& s) noexcept { return static_cast<u16>(std::stoul(s)); }

    inline static map<RE::FormID, DistrVecs> distr_map{};

    inline static set<RE::FormID> processed_containers{};

    inline static set<RE::FormID> respawn_containers{};
};

[[nodiscard]] inline std::string GetFormEditorID(const RE::TESForm* form) noexcept
{
    if (!form) {
        return "";
    }

    using TGetFormEditorID = const char* (*)(u32);

    using enum RE::FormType;
    switch (form->GetFormType()) {
    case Keyword:
    case LocationRefType:
    case Action:
    case MenuIcon:
    case Global:
    case HeadPart:
    case Race:
    case Sound:
    case Script:
    case Navigation:
    case Cell:
    case WorldSpace:
    case Land:
    case NavMesh:
    case Dialogue:
    case Quest:
    case Idle:
    case AnimatedObject:
    case ImageAdapter:
    case VoiceType:
    case Ragdoll:
    case DefaultObject:
    case MusicType:
    case StoryManagerBranchNode:
    case StoryManagerQuestNode:
    case StoryManagerEventNode:
    case SoundRecord:
        return form->GetFormEditorID();
    default: {
        static auto po3_tweaks{ REX::W32::GetModuleHandleW(L"po3_Tweaks") };
        static auto func{ reinterpret_cast<TGetFormEditorID>(REX::W32::GetProcAddress(po3_tweaks, "GetFormEditorID")) };
        if (func) {
            return func(form->formID);
        }
        return "";
    }
    }
}

inline auto format_as(const DistrType& type) noexcept
{
    switch (type) {
    case DistrType::Add:
        return "ADD";
    case DistrType::Remove:
        return "REMOVE";
    case DistrType::RemoveAll:
        return "REMOVE ALL";
    default:
        return "ERROR";
    }
}

inline auto format_as(const DistrToken& token) noexcept
{
    const auto& [type, to_identifier, identifier, count, location, location_keyword, chance]{ token };
    return fmt::format("[Type: {} / To: {} / Identifier: {} / Count: {} / Location: {} / Location keyword: {} / Chance: {}]", type, to_identifier, identifier, count, location,
                       location_keyword, chance);
}

inline auto format_as(const DistrObject& obj) noexcept
{
    const auto& [type, container_form_id, bound_object, count, location, location_keyword, chance]{ obj };
    return fmt::format("[Type: {} / Container: {:#x} / Bound object: {} ({:#x}) / Count: {} / Location: {} ({:#x}) / Location keyword: {} ({:#x}) / Chance: {}]", type,
                       container_form_id, GetFormEditorID(bound_object), bound_object ? bound_object->GetFormID() : 0x0U, count, GetFormEditorID(location),
                       location ? location->GetFormID() : 0x0U, GetFormEditorID(location_keyword), location_keyword ? location_keyword->GetFormID() : 0x0U, chance);
}
