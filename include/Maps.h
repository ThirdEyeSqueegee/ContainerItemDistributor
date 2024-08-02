#pragma once

#include "parallel_hashmap/phmap.h"

using u16 = std::uint16_t;
using u32 = std::uint32_t;
using i32 = std::int32_t;

enum struct DistrType
{
    Add,
    Remove,
    RemoveAll,
    Replace,
    ReplaceAll,
    Error
};

class DistrToken
{
public:
    constexpr bool operator==(const DistrToken&) const noexcept = default;

    DistrType                  type{};
    std::string                filename{};
    std::string                to_identifier{};
    std::string                identifier{};
    std::optional<u32>         count{};
    std::optional<std::string> rhs{};
    std::optional<u32>         rhs_count{};
    std::optional<u32>         chance{};
};

class Container
{
public:
    explicit constexpr operator bool() const noexcept { return container; }

    RE::TESContainer* container{};
    RE::FormID        container_form_id{};
    RE::FormType      container_type{};
    std::string       container_name{};
};

struct DistrObject
{
    DistrType                type{};
    RE::TESBoundObject*      bound_object{};
    RE::TESLevItem*          leveled_list{};
    std::string              filename{};
    RE::TESBoundObject*      replace_with_object{};
    RE::TESLevItem*          replace_with_list{};
    std::optional<i32>       count{};
    std::optional<i32>       replace_with_count{};
    std::optional<Container> container{};
    std::optional<u32>       chance{};
};

struct RuntimeDistrVecs
{
    std::vector<DistrObject> to_add;
    std::vector<DistrObject> to_remove;
    std::vector<DistrObject> to_remove_all;
    std::vector<DistrObject> to_replace;
    std::vector<DistrObject> to_replace_all;
};

struct FormIDAndPluginName
{
    RE::FormID  form_id{};
    std::string plugin_name{};
};

class Maps : public Singleton<Maps>
{
public:
    static auto ToFormID(const std::string& s) noexcept { return static_cast<RE::FormID>(std::stoul(s, nullptr, 16)); }

    static auto ToUnsignedInt(const std::string& s) noexcept { return static_cast<u32>(std::stoul(s)); }

    using TDistrTokenVec   = std::vector<DistrToken>;
    using TConflictTestMap = phmap::parallel_flat_hash_map<std::string, TDistrTokenVec>;

    inline static TConflictTestMap add_conflict_test_map;

    inline static TConflictTestMap remove_conflict_test_map;

    inline static TConflictTestMap replace_conflict_test_map;

    using TDistrVec = std::vector<DistrObject>;

    inline static TDistrVec distr_object_vec;

    using TRuntimeMap = phmap::parallel_flat_hash_map<RE::FormID, RuntimeDistrVecs>;

    inline static TRuntimeMap runtime_map;
};

// fmt helpers
inline auto format_as(const DistrType& type) noexcept
{
    switch (type) {
    case DistrType::Add:
        return "ADD";
    case DistrType::Remove:
        return "REMOVE";
    case DistrType::RemoveAll:
        return "REMOVE ALL";
    case DistrType::Replace:
        return "REPLACE";
    case DistrType::ReplaceAll:
        return "REPLACE ALL";
    default:
        return "ERROR";
    }
}

inline auto format_as(const DistrToken& token) noexcept
{
    const auto& [type, filename, to_identifier, identifier, count, rhs, rhs_count, chance]{ token };
    return fmt::format("[Type: {} / Filename: {} / To: {} / Identifier: {} / Count: {} / RHS: {} / RHS Count: {} / Chance: {}]", type, filename, to_identifier, identifier,
                       count.value_or(-1), rhs.value_or(""), rhs_count.value_or(-1), chance.value_or(100));
}

inline auto format_as(const DistrObject& obj) noexcept
{
    const auto& [type, bound_object, leveled_list, filename, replace_with_obj, replace_with_list, count, replace_with_count, container, chance]{ obj };
    // clang-format off
    return fmt::format("[Type: {} / Filename: {} / Bound object: {} (0x{:x}) / Leveled list: {} (0x{:x}) / Replace with obj: {} (0x{:x}) / Replace with list: {} (0x{:x}) / "
                       "Count: {} / Replace count: {} / Container: {} (0x{:x}) ({}) / Chance: {}]",
        // clang-format on
        type, filename, bound_object ? bound_object->GetName() : "", bound_object ? bound_object->GetFormID() : 0, leveled_list ? leveled_list->GetFormEditorID() : "",
        leveled_list ? leveled_list->GetFormID() : 0, replace_with_obj ? replace_with_obj->GetName() : "", replace_with_obj ? replace_with_obj->GetFormID() : 0,
        replace_with_list ? replace_with_list->GetName() : "", replace_with_list ? replace_with_list->GetFormID() : 0, count.value_or(0), replace_with_count.value_or(0),
        container.has_value() ? container.value().container_name : "", container.has_value() ? container.value().container_form_id : 0,
        container.has_value() ? container->container_type : RE::FormType::Container, chance.value_or(100));
}
