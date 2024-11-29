#include "Events.h"

#include "Map.h"

namespace Events
{
    RE::BSEventNotifyControl LoadGameEventHandler::ProcessEvent(const RE::TESLoadGameEvent* a_event, RE::BSTEventSource<RE::TESLoadGameEvent>* a_eventSource) noexcept
    {
        logger::debug("LoadGameEventHandler: Clearing processed_containers and added_objects");

        Map::processed_containers.clear();
        Map::added_objects.clear();

        return RE::BSEventNotifyControl::kContinue;
    }
} // namespace Events
