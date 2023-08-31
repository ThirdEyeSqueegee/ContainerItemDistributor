#pragma once

#include "parallel_hashmap/phmap.h"

enum class DistrType
{
    Add,
    Remove,
    Swap,
    LeveledList,
    Error
};

class Utility : public Singleton<Utility>
{
public:
    static constexpr auto is_present = [](const std::size_t idx) { return idx <=> std::string::npos != 0; };

    static constexpr DistrType ClassifyToken(const std::string& token)
    {
        const auto minus{ token.find('-') };
        const auto caret{ token.find('^') };
        const auto bar_count{ std::ranges::count(token, '|') };

        if (!is_present(caret) && bar_count <=> 2 == 0)
            return DistrType::LeveledList;
        if (!is_present(minus) && !is_present(caret))
            return DistrType::Add;
        if (is_present(minus) && !is_present(caret))
            return DistrType::Remove;
        if (!is_present(minus) && is_present(caret))
            return DistrType::Swap;

        return DistrType::Error;
    }

    template <typename T>
    static constexpr void LogConflictMap(const T& map)
    {
        for (const auto& [k, v] : map)
        {
            if constexpr (std::is_same_v<T, TSwapConflictMap>)
                logger::info("Found SWAP conflicts for {} and {} ({}):", k.first.first, k.first.second, k.second);
            else
                logger::info("Found conflicts for {} ({}):", k.first, k.second);

            for (const auto& [filename, distr_and_count] : v)
                logger::info("\t{}: {}", filename, distr_and_count);

            logger::info("");
        }
    }

    using TStringVec            = std::vector<std::string>;
    using TStringPair           = std::pair<std::string, std::string>;
    using TStringPairVec        = std::vector<TStringPair>;
    using TBoundObjectCountPair = std::pair<RE::TESBoundObject*, std::int32_t>;
    using TSwapPair             = std::pair<TBoundObjectCountPair, TBoundObjectCountPair>;

    using TConflictTestMap = phmap::parallel_flat_hash_map<std::string, TStringVec>;

    using TConflictMap     = phmap::parallel_flat_hash_map<TStringPair, TStringPairVec>;
    using TSwapConflictMap = phmap::parallel_flat_hash_map<std::pair<TStringPair, std::string>, TStringPairVec>;

    using TAddRemoveMap = phmap::parallel_flat_hash_map<std::string, std::vector<TBoundObjectCountPair>>;
    using TSwapMap      = phmap::parallel_flat_hash_map<std::string, std::vector<TSwapPair>>;

    inline static TConflictTestMap add_conflict_test_map{};
    inline static TConflictTestMap remove_conflict_test_map{};
    inline static TConflictTestMap swap_conflict_test_map{};

    inline static TConflictMap     add_conflict_map{};
    inline static TConflictMap     remove_conflict_map{};
    inline static TSwapConflictMap swap_conflict_map{};

    inline static TAddRemoveMap add_map{};
    inline static TAddRemoveMap remove_map{};
    inline static TSwapMap      swap_map{};
};
