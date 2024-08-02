#include "Hooks.h"

namespace Hooks
{
    void Install() noexcept
    {
        stl::write_vfunc<RE::TESObjectREFR, InitItemImpl>();
        logger::info("Installed TESObjectREFR::InitItemImpl hook");
    }

    void InitItemImpl::Thunk(RE::TESObjectREFR* a_ref) noexcept
    {
        func(a_ref);

        if (a_ref && a_ref->HasContainer() && !Distributor::processed_refs.contains(a_ref->GetFormID())) {
            Distributor::processed_refs.insert(a_ref->GetFormID());
            Distributor::RuntimeDistribute(a_ref);
        }
    }
} // namespace Hooks
