#include "Events.h"

#include "Map.h"

namespace Events
{
    RE::BSEventNotifyControl LoadGameEventHandler::ProcessEvent(const RE::TESLoadGameEvent* a_event, RE::BSTEventSource<RE::TESLoadGameEvent>* a_eventSource) noexcept
    {
        Map::processed_containers.clear();

        return RE::BSEventNotifyControl::kContinue;
    }
} // namespace Events
