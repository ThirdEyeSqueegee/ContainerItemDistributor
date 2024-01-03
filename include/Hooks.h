#pragma once

namespace Hooks {

    struct InitItemImpl
    {
        static void Thunk(RE::TESObjectREFR* a_ref)
        {
            if (a_ref->HasContainer()) {
                func(a_ref);

                logger::info("processing container: {}", a_ref->GetFormID());

                auto inv = a_ref->GetInventory();
                for (const auto& [form, data] : inv) {
                    logger::info("contains: {}", form->GetFormEditorID());
                }

                auto gold = RE::TESDataHandler::GetSingleton()->LookupForm(0xF, "Skyrim.esm");

                a_ref->RemoveItem(gold->As<RE::TESBoundObject>(), 1000000, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
                a_ref->AddObjectToContainer(gold->As<RE::TESBoundObject>(), nullptr, 100, nullptr);

                return;
            }

            func(a_ref);
        }
        static inline REL::Relocation<decltype(Thunk)> func;
        static inline constexpr std::size_t            idx{ 0x13 };

        static void Install()
        {
            stl::write_vfunc<RE::TESObjectREFR, InitItemImpl>();
            logger::info("Installed Object Reference hook");
        }
    };

    void Install();
}
