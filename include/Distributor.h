#pragma once

class Distributor : public Singleton<Distributor>
{
public:
    static void Distribute(RE::TESObjectREFR* a_ref, bool is_reset = false, bool is_merchant = false) noexcept;

    static void MerchantDistribute() noexcept;
};
