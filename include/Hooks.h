#pragma once
#include "Distributor.h"

namespace Hooks
{

    struct InitItemImpl
    {
        static void Thunk(RE::TESObjectREFR* a_ref)
        {
            func(a_ref);

            if (a_ref->HasContainer()) {
                Distributor::RuntimeDistribute(a_ref);
            }
        }

        inline static REL::Relocation<decltype(Thunk)> func;
        inline static constexpr std::size_t            idx{ 0x13 };

        static void Install()
        {
            stl::write_vfunc<RE::TESObjectREFR, InitItemImpl>();
            logger::info("Installed Object Reference hook");
        }
    };

    void Install();
} // namespace Hooks
