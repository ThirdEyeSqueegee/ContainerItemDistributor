#include "Events.h"
#include "Hooks.h"
#include "Parser.h"
#include "Settings.h"

void Listener(SKSE::MessagingInterface::Message* message) noexcept
{
    if (message->type == SKSE::MessagingInterface::kDataLoaded) {
        if (!REX::W32::GetModuleHandleW(L"po3_Tweaks")) {
            logger::error("ERROR: powerofthree's Tweaks not found");
            stl::report_and_fail("ERROR [ContainerItemDistributor.dll]: powerofthree's Tweaks not found");
        }
        Settings::LoadSettings();
        Parser::ParseINIs();
        Hooks::Install();
        Events::LoadGameEventHandler::Register();
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
    const auto plugin{ SKSE::PluginDeclaration::GetSingleton() };
    const auto name{ plugin->GetName() };
    const auto version{ plugin->GetVersion() };

    logger::init();

    logger::info("{} {} is loading...", name, version);

    Init(skse);

    if (const auto messaging{ SKSE::GetMessagingInterface() }; !messaging->RegisterListener(Listener)) {
        return false;
    }

    logger::info("{} has finished loading.", name);
    logger::info("");

    return true;
}
