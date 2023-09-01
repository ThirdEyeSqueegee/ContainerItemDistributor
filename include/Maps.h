#pragma once

#include "parallel_hashmap/phmap.h"

static constexpr auto is_present{ [](const std::size_t idx) { return idx <=> std::string::npos != 0; } };

static constexpr auto to_int32{ [](const std::string_view& s) { return static_cast<std::int32_t>(std::strtol(s.data(), nullptr, 0)); } };

static constexpr auto to_uint32{ [](const std::string_view& s) { return static_cast<std::uint32_t>(std::strtol(s.data(), nullptr, 0)); } };

enum class DistrType
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
    bool operator==(const DistrToken&) const = default;

    DistrType                   type{};
    std::string                 filename{};
    std::string                 to_identifier{};
    std::string                 identifier{};
    std::optional<std::int32_t> count{};
    std::optional<std::string>  rhs{};
    std::optional<std::int32_t> rhs_count{};
};

class Container
{
public:
    explicit operator bool() const
    {
        return container;
    }

    RE::TESContainer* container{};
    RE::FormID        container_form_id{};
    std::string       container_name{};
};

struct DistrObject
{
    DistrType                          type{};
    RE::TESBoundObject*                bound_object{};
    std::optional<std::int32_t>        count{};
    std::optional<RE::TESBoundObject*> replace_with_object{};
    std::optional<std::int32_t>        replace_with_count{};
    std::optional<Container>           container{};
};

struct FormIDAndPluginName
{
    RE::FormID  form_id;
    std::string plugin_name;
};

class Maps : public Singleton<Maps>
{
public:
    using TDistrTokenVec   = std::vector<DistrToken>;
    using TConflictTestMap = phmap::parallel_flat_hash_map<std::string, TDistrTokenVec>;

    // Container EditorIDs (or FormIDs) to DistrToken vectors
    inline static TConflictTestMap add_conflict_test_map;

    inline static TConflictTestMap remove_conflict_test_map;

    inline static TConflictTestMap replace_conflict_test_map;

    using TDistrVec = std::vector<DistrObject>;
    inline static TDistrVec distr_object_vec;
};

inline auto format_as(const DistrType& type)
{
    switch (type)
    {
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

inline auto format_as(const DistrToken& token)
{
    const auto& [type, filename, to_identifier, identifier, count, rhs, rhs_count]{ token };
    return fmt::format("Type: {} / Filename: {} / To: {} / Identifier: {} / Count: {} / RHS: {} / RHS Count: {}", type, filename, to_identifier,
                       identifier, count.value_or(-1), rhs.value_or("null"), rhs_count.value_or(-1));
}

inline auto format_as(const DistrObject& obj)
{
    const auto& [type, bound_object, count, replace_with_obj, replace_with_count, container]{ obj };
    return fmt::format("Type: {} / Bound object: {} (0x{:x}) / Count: {} / Replace with: {} (0x{:x}) / Replace count: {} / Container: {} (0x{:x})",
                       type, bound_object ? bound_object->GetName() : "null", bound_object ? bound_object->GetFormID() : 0, count.value_or(-1),
                       replace_with_obj.has_value() ? (replace_with_obj.value() ? replace_with_obj.value()->GetName() : "null") : "null",
                       replace_with_obj.has_value() ? (replace_with_obj.value() ? replace_with_obj.value()->GetFormID() : 0) : 0,
                       replace_with_count.value_or(-1), container.has_value() ? container.value().container_name : "null",
                       container.has_value() ? container.value().container_form_id : 0);
}
