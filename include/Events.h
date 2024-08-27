#pragma once

namespace Events
{
    class LoadGameEventHandler : public EventSingleton<LoadGameEventHandler, RE::TESLoadGameEvent>
    {
    public:
        RE::BSEventNotifyControl ProcessEvent(const RE::TESLoadGameEvent* a_event, RE::BSTEventSource<RE::TESLoadGameEvent>* a_eventSource) noexcept override;
    };
} // namespace Events
