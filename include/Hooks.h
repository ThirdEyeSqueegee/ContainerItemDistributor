#pragma once

namespace Hooks
{
    void Install() noexcept;

    class SetFormEditorID : public Singleton<SetFormEditorID>
    {
    public:
        static bool Thunk(RE::TESForm* a_this, const char* a_str) noexcept;

        inline static REL::Relocation<decltype(&Thunk)> func;

        static constexpr std::size_t idx{ 51 }; // 0x33
    };
} // namespace Hooks
