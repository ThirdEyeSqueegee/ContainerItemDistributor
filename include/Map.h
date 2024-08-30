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
    u32         count{};
    u32         chance{};
};

struct DistrObject
{
    DistrType           type{};
    RE::TESBoundObject* bound_object{};
    u32                 count{};
    RE::FormID          container_form_id{};
    u32                 chance{};
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
    static auto ToFormID(const std::string& s) noexcept { return static_cast<RE::FormID>(std::stoul(s, nullptr, 16)); }

    static auto ToUnsignedInt(const std::string& s) noexcept { return static_cast<u32>(std::stoul(s)); }

    inline static map<RE::FormID, DistrVecs> distr_map;

    inline static map<RE::FormID, std::pair<RE::TESLevItem*, u32>> leveled_distr_map;

    inline static map<RE::FormID, map<RE::TESBoundObject*, u32>> leveled_reset_map;

    inline static set<RE::FormID> processed_containers;

    inline static set<RE::FormID> respawn_containers;
};

inline std::string GetFormEditorID(const RE::TESForm* form) noexcept
{
    using TGetFormEditorID = const char* (*)(std::uint32_t);

    switch (form->GetFormType()) {
    case RE::FormType::Keyword:
    case RE::FormType::LocationRefType:
    case RE::FormType::Action:
    case RE::FormType::MenuIcon:
    case RE::FormType::Global:
    case RE::FormType::HeadPart:
    case RE::FormType::Race:
    case RE::FormType::Sound:
    case RE::FormType::Script:
    case RE::FormType::Navigation:
    case RE::FormType::Cell:
    case RE::FormType::WorldSpace:
    case RE::FormType::Land:
    case RE::FormType::NavMesh:
    case RE::FormType::Dialogue:
    case RE::FormType::Quest:
    case RE::FormType::Idle:
    case RE::FormType::AnimatedObject:
    case RE::FormType::ImageAdapter:
    case RE::FormType::VoiceType:
    case RE::FormType::Ragdoll:
    case RE::FormType::DefaultObject:
    case RE::FormType::MusicType:
    case RE::FormType::StoryManagerBranchNode:
    case RE::FormType::StoryManagerQuestNode:
    case RE::FormType::StoryManagerEventNode:
    case RE::FormType::SoundRecord:
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
    const auto& [type, to_identifier, identifier, count, chance]{ token };
    return fmt::format("[Type: {} / To: {} / Identifier: {} / Count: {} / Chance: {}]", type, to_identifier, identifier, count, chance);
}

inline auto format_as(const DistrObject& obj) noexcept
{
    const auto& [type, bound_object, count, container_form_id, chance]{ obj };
    return fmt::format("[Type: {} / Bound object: {} ({:#x}) / Count: {} / Container: {:#x} / Chance: {}]", type, GetFormEditorID(bound_object),
                       bound_object ? bound_object->GetFormID() : 0, count, container_form_id, chance);
}
