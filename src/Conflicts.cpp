#include "Conflicts.h"

#include "Utility.h"

void Conflicts::PrepareDistribution() noexcept
{
    logger::info(">-------------------------------------------------------------Preparing distribution...--------------------------------------------------------------<");
    logger::info("");

    PrepareDistributionImpl(Maps::add_conflict_test_map);
    PrepareDistributionImpl(Maps::remove_conflict_test_map);
    PrepareDistributionImpl(Maps::replace_conflict_test_map);

    Utility::CachePlayerLevel();

    logger::info("");
    logger::info(">----------------------------------------------------------Finished preparing distribution-----------------------------------------------------------<");
    logger::info("");
}

void Conflicts::PrepareDistributionImpl(const Maps::TConflictTestMap& test_map) noexcept
{
    for (auto distr_token_vec : test_map | std::views::values) {
        for (const auto& distr_token : distr_token_vec) {
            const auto& [type, filename, to_identifier, identifier, count, rhs, rhs_count, chance]{ distr_token };

            DistrObject result;

            if (auto matching{ std::ranges::filter_view(distr_token_vec, [&](const DistrToken& other) { return other != distr_token && other.identifier == identifier; })
                               | std::ranges::to<std::vector>() };
                !matching.empty())
            {
                matching.emplace_back(distr_token);
                logger::info("Found conflicts for {} (origin {})", identifier, filename);
                std::ranges::sort(matching, [](const DistrToken& l, const DistrToken& r) { return l.identifier < r.identifier; });
                const auto& winning{ matching.back() };
                logger::info("\t{} wins for {}", winning.filename, distr_token);
                result = Utility::BuildDistrObject(winning);
                Maps::distr_object_vec.emplace_back(result);
                const auto& [ret, last]{ std::ranges::remove_if(distr_token_vec, [&](const DistrToken& d) { return d.identifier == identifier; }) };
                distr_token_vec.erase(ret, last);
            }
            else {
                result = Utility::BuildDistrObject(distr_token);
                Maps::distr_object_vec.emplace_back(result);
            }
        }
    }
}
