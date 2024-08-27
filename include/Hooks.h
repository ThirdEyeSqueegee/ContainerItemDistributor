#pragma once

namespace Hooks
{
    void Install() noexcept;

    class Load3D : public Singleton<Load3D>
    {
    public:
        static RE::NiAVObject* Thunk(RE::TESObjectREFR* a_this, bool a_backgroundLoading) noexcept;

        inline static REL::Relocation<decltype(&Thunk)> func;

        static constexpr std::size_t idx{ 106 }; // 0x6a
    };

    class ResetInventory : public Singleton<ResetInventory>
    {
    public:
        static void Thunk(RE::TESObjectREFR* a_this, bool a_leveledOnly) noexcept;

        inline static REL::Relocation<decltype(&Thunk)> func;

        static constexpr std::size_t idx{ 138 }; // 0x8a
    };
} // namespace Hooks
