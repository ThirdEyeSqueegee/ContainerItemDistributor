#pragma once

class Distributor : public Singleton<Distributor>
{
public:
    static void Distribute(RE::TESObjectREFR* a_ref) noexcept;
};
