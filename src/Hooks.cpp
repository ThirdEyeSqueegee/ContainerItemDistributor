#include "Hooks.h"

#include "Distributor.h"
#include "Map.h"

namespace Hooks
{
    void Install() noexcept
    {
        stl::write_vfunc<RE::TESObjectREFR, Load3D>();
        logger::info("Installed TESObjectREFR::Load3D hook");

        stl::write_vfunc<RE::TESObjectREFR, ResetInventory>();
        logger::info("Installed TESObjectREFR::ResetInventory hook");
        logger::info("");
    }

    RE::NiAVObject* Load3D::Thunk(RE::TESObjectREFR* a_this, bool a_backgroundLoading) noexcept
    {
        if (a_this && a_this->HasContainer() && !Map::processed_refs.contains(a_this->GetFormID())) {
            Distributor::Distribute(a_this);
            if (const auto cont{ a_this->As<RE::TESObjectCONT>() }) {
                if (cont->data.flags & RE::CONT_DATA::Flag::kRespawn) {
                    Map::respawn_containers.insert(a_this->GetFormID());
                }
            }
        }

        return func(a_this, a_backgroundLoading);
    }

    void ResetInventory::Thunk(RE::TESObjectREFR* a_this, bool a_leveledOnly) noexcept
    {
        func(a_this, a_leveledOnly);

        if (a_this && Map::respawn_containers.contains(a_this->GetFormID())) {
            Distributor::Distribute(a_this, true);
        }
    }
} // namespace Hooks
