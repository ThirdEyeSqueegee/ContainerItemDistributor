#include "Hooks.h"

namespace Hooks // Lifted shamelessly from the great powerofthree
{
    void Install() noexcept
    {
        stl::write_vfunc<RE::Actor, SetFormEditorID>();
        stl::write_vfunc<RE::ActorValueInfo, SetFormEditorID>();
        stl::write_vfunc<RE::AlchemyItem, SetFormEditorID>();
        stl::write_vfunc<RE::ArrowProjectile, SetFormEditorID>();
        stl::write_vfunc<RE::BarrierProjectile, SetFormEditorID>();
        stl::write_vfunc<RE::BeamProjectile, SetFormEditorID>();
        stl::write_vfunc<RE::BGSAcousticSpace, SetFormEditorID>();
        stl::write_vfunc<RE::BGSAddonNode, SetFormEditorID>();
        stl::write_vfunc<RE::BGSApparatus, SetFormEditorID>();
        stl::write_vfunc<RE::BGSArtObject, SetFormEditorID>();
        stl::write_vfunc<RE::BGSAssociationType, SetFormEditorID>();
        stl::write_vfunc<RE::BGSBodyPartData, SetFormEditorID>();
        stl::write_vfunc<RE::BGSCameraPath, SetFormEditorID>();
        stl::write_vfunc<RE::BGSCameraShot, SetFormEditorID>();
        stl::write_vfunc<RE::BGSCollisionLayer, SetFormEditorID>();
        stl::write_vfunc<RE::BGSColorForm, SetFormEditorID>();
        stl::write_vfunc<RE::BGSConstructibleObject, SetFormEditorID>();
        stl::write_vfunc<RE::BGSDebris, SetFormEditorID>();
        stl::write_vfunc<RE::BGSDialogueBranch, SetFormEditorID>();
        stl::write_vfunc<RE::BGSDualCastData, SetFormEditorID>();
        stl::write_vfunc<RE::BGSEncounterZone, SetFormEditorID>();
        stl::write_vfunc<RE::BGSEquipSlot, SetFormEditorID>();
        stl::write_vfunc<RE::BGSExplosion, SetFormEditorID>();
        stl::write_vfunc<RE::BGSFootstep, SetFormEditorID>();
        stl::write_vfunc<RE::BGSFootstepSet, SetFormEditorID>();
        stl::write_vfunc<RE::BGSHazard, SetFormEditorID>();
        stl::write_vfunc<RE::BGSIdleMarker, SetFormEditorID>();
        stl::write_vfunc<RE::BGSImpactData, SetFormEditorID>();
        stl::write_vfunc<RE::BGSImpactDataSet, SetFormEditorID>();
        stl::write_vfunc<RE::BGSLensFlare, SetFormEditorID>();
        stl::write_vfunc<RE::BGSLightingTemplate, SetFormEditorID>();
        stl::write_vfunc<RE::BGSListForm, SetFormEditorID>();
        stl::write_vfunc<RE::BGSLocation, SetFormEditorID>();
        stl::write_vfunc<RE::BGSMaterialObject, SetFormEditorID>();
        stl::write_vfunc<RE::BGSMaterialType, SetFormEditorID>();
        stl::write_vfunc<RE::BGSMessage, SetFormEditorID>();
        stl::write_vfunc<RE::BGSMovementType, SetFormEditorID>();
        stl::write_vfunc<RE::BGSMusicTrackFormWrapper, SetFormEditorID>();
        stl::write_vfunc<RE::BGSNote, SetFormEditorID>();
        stl::write_vfunc<RE::BGSOutfit, SetFormEditorID>();
        stl::write_vfunc<RE::BGSPerk, SetFormEditorID>();
        stl::write_vfunc<RE::BGSProjectile, SetFormEditorID>();
        stl::write_vfunc<RE::BGSReferenceEffect, SetFormEditorID>();
        stl::write_vfunc<RE::BGSRelationship, SetFormEditorID>();
        stl::write_vfunc<RE::BGSReverbParameters, SetFormEditorID>();
        stl::write_vfunc<RE::BGSScene, SetFormEditorID>();
        stl::write_vfunc<RE::BGSShaderParticleGeometryData, SetFormEditorID>();
        stl::write_vfunc<RE::BGSSoundCategory, SetFormEditorID>();
        stl::write_vfunc<RE::BGSSoundOutput, SetFormEditorID>();
        stl::write_vfunc<RE::BGSStaticCollection, SetFormEditorID>();
        stl::write_vfunc<RE::BGSTalkingActivator, SetFormEditorID>();
        stl::write_vfunc<RE::BGSTextureSet, SetFormEditorID>();
        stl::write_vfunc<RE::Character, SetFormEditorID>();
        stl::write_vfunc<RE::ConeProjectile, SetFormEditorID>();
        stl::write_vfunc<RE::DialoguePackage, SetFormEditorID>();
        stl::write_vfunc<RE::EffectSetting, SetFormEditorID>();
        stl::write_vfunc<RE::EnchantmentItem, SetFormEditorID>();
        stl::write_vfunc<RE::FlameProjectile, SetFormEditorID>();
        stl::write_vfunc<RE::GrenadeProjectile, SetFormEditorID>();
        stl::write_vfunc<RE::Hazard, SetFormEditorID>();
        stl::write_vfunc<RE::IngredientItem, SetFormEditorID>();
        stl::write_vfunc<RE::MissileProjectile, SetFormEditorID>();
        stl::write_vfunc<RE::PlayerCharacter, SetFormEditorID>();
        stl::write_vfunc<RE::ScrollItem, SetFormEditorID>();
        stl::write_vfunc<RE::SpellItem, SetFormEditorID>();
        stl::write_vfunc<RE::TESAmmo, SetFormEditorID>();
        stl::write_vfunc<RE::TESClass, SetFormEditorID>();
        stl::write_vfunc<RE::TESClimate, SetFormEditorID>();
        stl::write_vfunc<RE::TESCombatStyle, SetFormEditorID>();
        stl::write_vfunc<RE::TESEffectShader, SetFormEditorID>();
        stl::write_vfunc<RE::TESEyes, SetFormEditorID>();
        stl::write_vfunc<RE::TESFaction, SetFormEditorID>();
        stl::write_vfunc<RE::TESFlora, SetFormEditorID>();
        stl::write_vfunc<RE::TESFurniture, SetFormEditorID>();
        stl::write_vfunc<RE::TESGrass, SetFormEditorID>();
        stl::write_vfunc<RE::TESImageSpace, SetFormEditorID>();
        stl::write_vfunc<RE::TESKey, SetFormEditorID>();
        stl::write_vfunc<RE::TESLandTexture, SetFormEditorID>();
        stl::write_vfunc<RE::TESLevCharacter, SetFormEditorID>();
        stl::write_vfunc<RE::TESLevItem, SetFormEditorID>();
        stl::write_vfunc<RE::TESLevSpell, SetFormEditorID>();
        stl::write_vfunc<RE::TESLoadScreen, SetFormEditorID>();
        stl::write_vfunc<RE::TESNPC, SetFormEditorID>();
        stl::write_vfunc<RE::TESObjectACTI, SetFormEditorID>();
        stl::write_vfunc<RE::TESObjectARMA, SetFormEditorID>();
        stl::write_vfunc<RE::TESObjectARMO, SetFormEditorID>();
        stl::write_vfunc<RE::TESObjectBOOK, SetFormEditorID>();
        stl::write_vfunc<RE::TESObjectCONT, SetFormEditorID>();
        stl::write_vfunc<RE::TESObjectDOOR, SetFormEditorID>();
        stl::write_vfunc<RE::TESObjectLIGH, SetFormEditorID>();
        stl::write_vfunc<RE::TESObjectMISC, SetFormEditorID>();
        stl::write_vfunc<RE::TESObjectREFR, SetFormEditorID>();
        stl::write_vfunc<RE::TESObjectSTAT, SetFormEditorID>();
        stl::write_vfunc<RE::TESObjectTREE, SetFormEditorID>();
        stl::write_vfunc<RE::TESObjectWEAP, SetFormEditorID>();
        stl::write_vfunc<RE::TESPackage, SetFormEditorID>();
        stl::write_vfunc<RE::TESRegion, SetFormEditorID>();
        stl::write_vfunc<RE::TESShout, SetFormEditorID>();
        stl::write_vfunc<RE::TESSoulGem, SetFormEditorID>();
        stl::write_vfunc<RE::TESTopicInfo, SetFormEditorID>();
        stl::write_vfunc<RE::TESWaterForm, SetFormEditorID>();
        stl::write_vfunc<RE::TESWeather, SetFormEditorID>();
        stl::write_vfunc<RE::TESWordOfPower, SetFormEditorID>();
        stl::write_vfunc<SetFormEditorID>(RE::VTABLE_BGSMovableStatic[2]);

        logger::info("Installed SetFormEditorID hooks");
    }

    bool SetFormEditorID::Thunk(RE::TESForm* a_this, const char* a_str) noexcept
    {
        if (!std::string{ a_str }.empty() && !a_this->IsDynamicForm())
        {
            if (const auto& [map, lock] = RE::TESForm::GetAllFormsByEditorID(); map)
            {
                const RE::BSWriteLockGuard guard{ lock };
                map->emplace(a_str, a_this);
            }
        }

        return func(a_this, a_str);
    }
} // namespace Hooks
