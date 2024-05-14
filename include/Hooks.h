#pragma once

#include "Distributor.h"

namespace Hooks
{
    void Install() noexcept;

    class InitItemImpl : public Singleton<InitItemImpl>
    {
    public:
        static void Thunk(RE::TESObjectREFR* a_ref) noexcept;

        inline static REL::Relocation<decltype(&Thunk)> func;

        static constexpr std::size_t idx{ 19 }; // 0x13
    };
} // namespace Hooks
