#pragma once

#include "parallel_hashmap/phmap.h"

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

    DistrType                  type{};
    std::string                filename{};
    std::string                to_identifier{};
    std::string                identifier{};
    std::optional<int>         count{};
    std::optional<std::string> rhs{};
    std::optional<int>         rhs_count{};
};

class Container
{
public:
    explicit operator bool() const { return container; }

    RE::TESContainer* container{};
    RE::FormID        container_form_id{};
    RE::FormType      container_type{};
    std::string       container_name{};
};

struct DistrObject
{
    DistrType                          type{};
    RE::TESBoundObject*                bound_object{};
    std::string                        filename{};
    std::optional<int>                 count{};
    std::optional<RE::TESBoundObject*> replace_with_object{};
    std::optional<int>                 replace_with_count{};
    std::optional<Container>           container{};
};

struct FormIDAndPluginName
{
    RE::FormID  form_id{};
    std::string plugin_name{};
};

class Maps : public Singleton<Maps>
{
public:
    static int ToInt(const std::string_view& s) { return static_cast<int>(std::strtol(s.data(), nullptr, 0)); }

    static unsigned ToUnsignedInt(const std::string_view& s) { return static_cast<unsigned>(std::strtol(s.data(), nullptr, 0)); }

    static std::size_t GetPos(const std::string_view s, const char c)
    {
        const auto ptr{ std::strrchr(s.data(), c) };

        return ptr ? static_cast<std::size_t>(ptr - s.data()) : ULLONG_MAX;
    }

    using TDistrTokenVec   = std::vector<DistrToken>;
    using TConflictTestMap = phmap::parallel_flat_hash_map<std::string, TDistrTokenVec>;

    inline static TConflictTestMap add_conflict_test_map;

    inline static TConflictTestMap remove_conflict_test_map;

    inline static TConflictTestMap replace_conflict_test_map;

    using TDistrVec = std::vector<DistrObject>;
    inline static TDistrVec distr_object_vec;
};

// fmt helpers
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
    return fmt::format("[Type: {} / Filename: {} / To: {} / Identifier: {} / Count: {} / RHS: {} / RHS Count: {}]", type, filename, to_identifier, identifier,
                       count.value_or(-1), rhs.value_or("null"), rhs_count.value_or(-1));
}

inline auto format_as(const DistrObject& obj)
{
    const auto& [type, bound_object, filename, count, replace_with_obj, replace_with_count, container]{ obj };
    return fmt::format("[Type: {} / Filename: {} / Bound object: {} (0x{:x}) / Count: {} / Replace with: {} (0x{:x}) / Replace count: {} / Container: {} "
                       "(0x{:x}) ({})]",
                       type, filename, bound_object ? bound_object->GetName() : "null", bound_object ? bound_object->GetFormID() : 0, count.value_or(-1),
                       replace_with_obj.has_value() ? (replace_with_obj.value() ? replace_with_obj.value()->GetName() : "null") : "null",
                       replace_with_obj.has_value() ? (replace_with_obj.value() ? replace_with_obj.value()->GetFormID() : 0) : 0, replace_with_count.value_or(-1),
                       container.has_value() ? container.value().container_name : "null", container.has_value() ? container.value().container_form_id : 0,
                       container.has_value() ? container->container_type : RE::FormType::Container);
}
