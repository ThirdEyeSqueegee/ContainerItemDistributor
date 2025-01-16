#include "Hooks.h"

#include "Distributor.h"
#include "Map.h"

namespace Hooks
{
    void Install() noexcept
    {
        stl::write_vfunc<RE::TESObjectREFR, Load3D>();
        logger::info("Installed TESObjectREFR::Load3D hook");

        stl::write_vfunc<RE::Character, Load3DCharacter>();
        logger::info("Installed Character::Load3DCharacter hook");

        stl::write_vfunc<RE::TESObjectREFR, ResetInventory>();
        logger::info("Installed TESObjectREFR::ResetInventory hook");

        stl::write_vfunc<RE::TESObjectREFR, SaveGame>();
        logger::info("Installed TESObjectREFR::SaveGame hook");
        logger::info("");
    }

    RE::NiAVObject* Load3D::Thunk(RE::TESObjectREFR* a_this, bool a_backgroundLoading) noexcept
    {
        if (a_this && a_this->HasContainer()) {
            if (const auto cont{ a_this->GetBaseObject()->As<RE::TESObjectCONT>() }) {
                if (cont->data.flags & RE::CONT_DATA::Flag::kRespawn) {
                    Map::respawn_containers.insert(a_this->GetFormID());
                }
            }
            Distributor::Distribute(a_this);
        }
        return func(a_this, a_backgroundLoading);
    }

    RE::NiAVObject* Load3DCharacter::Thunk(RE::Character* a_this, bool a_backgroundLoading) noexcept
    {
        if (a_this && a_this->HasContainer()) {
            Distributor::Distribute(a_this);
        }

        return func(a_this, a_backgroundLoading);
    }

    void ResetInventory::Thunk(RE::TESObjectREFR* a_this, bool a_leveledOnly) noexcept
    {
        func(a_this, a_leveledOnly);

        if (a_this && Map::respawn_containers.contains(a_this->GetFormID())) {
            Map::processed_containers.erase(a_this->GetFormID());
            Distributor::Distribute(a_this);
        }
    }

    void SaveGame::Thunk(RE::TESObjectREFR* a_this, RE::BGSSaveFormBuffer* a_buf) noexcept
    {
        if (!Map::processed_containers.contains(a_this->GetFormID()) || !Map::added_objects.contains(a_this)) {
            func(a_this, a_buf);
            return;
        }

        logger::debug("Removing added objects from {}", a_this);

        for (const auto& [obj, count] : Map::added_objects[a_this]) {
            a_this->RemoveItem(obj, count, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
            logger::debug("\tRemoved {} ({})", obj, count);
        }

        func(a_this, a_buf);
    }

} // namespace Hooks
