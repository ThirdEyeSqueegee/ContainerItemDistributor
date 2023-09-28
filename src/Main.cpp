#include "Conflicts.h"
#include "Distributor.h"
#include "Logging.h"
#include "Settings.h"

void Listener(SKSE::MessagingInterface::Message* message) noexcept
{
    if (message->type <=> SKSE::MessagingInterface::kDataLoaded == 0)
    {
        if (!GetModuleHandle(L"po3_Tweaks"))
        {
            logger::error("ERROR: powerofthree's Tweaks not found");
            return;
        }
        Settings::LoadSettings();
        Conflicts::PrepareDistribution();
        Distributor::Distribute();
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
    InitializeLogging();

    const auto plugin{ SKSE::PluginDeclaration::GetSingleton() };
    const auto version{ plugin->GetVersion() };

    logger::info("{} {} is loading...", plugin->GetName(), version);

    Init(skse);

    if (const auto messaging{ SKSE::GetMessagingInterface() }; !messaging->RegisterListener(Listener))
        return false;

    logger::info("{} has finished loading.", plugin->GetName());
    logger::info("");

    return true;
}
