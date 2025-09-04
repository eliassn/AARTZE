#pragma once

#include "Coordinator.hpp"

// === Core components
#include "components/AIComponent.hpp"
#include "components/AimingComponent.hpp"
#include "components/AnimationComponent.hpp"
#include "components/AnimationStateComponent.hpp"
#include "components/AudioSourceComponent.hpp"
#include "components/BreathComponent.hpp"
#include "components/CollisionComponent.hpp"
#include "components/CutsceneComponent.hpp"
#include "components/DecalComponent.hpp"
#include "components/DrivingComponent.hpp"
#include "components/FatigueComponent.hpp"
#include "components/HealthComponent.hpp"
#include "components/InputComponent.hpp"
#include "components/InventoryComponent.hpp"
#include "components/MaterialComponent.hpp"
#include "components/NameComponent.hpp"
#include "components/PositionComponent.hpp"
#include "components/RenderableComponent.hpp"
#include "components/SaveTagComponent.hpp"
#include "components/ScriptComponent.hpp"
#include "components/SkeletalMeshComponent.hpp"
#include "components/SkeletonComponent.hpp"
#include "components/SneakingComponent.hpp"
#include "components/TrailEffectComponent.hpp"
#include "components/TransformComponent.hpp"
#include "components/VelocityComponent.hpp"
#include "components/WeaponComponent.hpp"

// === Gameplay components
#include "AnimationEventComponent.hpp"
#include "BehaviorComponent.hpp"
#include "CameraComponent.hpp"
#include "CameraPathComponent.hpp"
#include "CombatCooldownComponent.hpp"
#include "CombatStanceComponent.hpp"
#include "DialogueComponent.hpp"
#include "DialogueCoolDownComponent.hpp"
#include "DialogueMoodComponent.hpp"
#include "EmotionComponent.hpp"
#include "HitboxComponent.hpp"
#include "InteractionComponent.hpp"
#include "LockOnComponent.hpp"
#include "MemoryComponent.hpp"
#include "MoneyComponent.hpp"
#include "NavigationComponent.hpp"
#include "NeedsComponent.hpp"
#include "PhoneComponent.hpp"
#include "ProfessionComponent.hpp"
#include "ProjectileComponent.hpp"
#include "ScheduleComponent.hpp"
#include "StaminaComponent.hpp"
#include "StateComponent.hpp"
#include "SuspicionComponent.hpp"
#include "WantedLevelComponent.hpp"
#include "profiles/NPCProfile.hpp"

// === Missions & Quests
#include "ActiveMissionTrackerComponent.hpp"
#include "MissionComponent.hpp"
#include "QuestDialogueComponent.hpp"
#include "SideMissionComponent.hpp"
#include "SideMissionOfferComponent.hpp"

// === Time & Weather
#include "GameEraComponent.hpp"
#include "TimeEventComponent.hpp"
#include "WeatherEffectComponent.hpp"
#include "WeatherStateComponent.hpp"
#include "WorldTimeComponent.hpp"

// === ClothingClothesTagsComponent.hpp
#include "ClothingComponent.hpp"
#include "ClothingTypeComponent.hpp"
#include "components/ClothesTags/ClothesTagsComponent.hpp"

// === Physics & vehicle
#include "BoundingSphereComponent.hpp"
#include "VehiculeTypeComponent.hpp"
#include "components/BuildingComponent.hpp"
#include "components/PhysicsTags/PhysicsTagComponents.hpp"
#include "components/VehiculeTags/VehiculeTagsComponents.hpp"
#include "components/environment/PuddleComponent.hpp"
#include "components/environment/RainAudioComponent.hpp"
#include "components/environment/RainComponent.hpp"
#include "components/environment/RainOcclusionComponent.hpp"
#include "components/environment/RainRipplesComponent.hpp"
#include "components/environment/ScreenWetnessEffectComponent.hpp"
#include "components/environment/TireSplashComponent.hpp"
#include "components/environment/WetSurfaceComponent.hpp"

// === Store
#include "components/environment/stores/StoreAndWorkSiteComponent.hpp"
#include "components/storeAndWorkPlaceTags/StoreAndWorkSiteTagComponent.hpp"
// === Tag Components
#include "TagComponents.hpp"

inline void RegisterAllComponents()
{
    // Core Systems
    gCoordinator.RegisterComponent<PositionComponent>();
    gCoordinator.RegisterComponent<TransformComponent>();
    gCoordinator.RegisterComponent<VelocityComponent>();
    gCoordinator.RegisterComponent<RenderableComponent>();
    gCoordinator.RegisterComponent<SkeletonComponent>();
    gCoordinator.RegisterComponent<SkeletalMeshComponent>();
    gCoordinator.RegisterComponent<AnimationComponent>();
    gCoordinator.RegisterComponent<MaterialComponent>();
    gCoordinator.RegisterComponent<CollisionComponent>();
    gCoordinator.RegisterComponent<HealthComponent>();
    gCoordinator.RegisterComponent<AIComponent>();
    gCoordinator.RegisterComponent<InputComponent>();
    gCoordinator.RegisterComponent<InventoryComponent>();
    gCoordinator.RegisterComponent<NameComponent>();
    gCoordinator.RegisterComponent<BreathComponent>();
    gCoordinator.RegisterComponent<FatigueComponent>();
    gCoordinator.RegisterComponent<AnimationStateComponent>();
    gCoordinator.RegisterComponent<AudioSourceComponent>();
    gCoordinator.RegisterComponent<WeaponComponent>();
    gCoordinator.RegisterComponent<SaveTagComponent>();
    gCoordinator.RegisterComponent<NavigationComponent>();
    gCoordinator.RegisterComponent<AnimationEventComponent>();
    gCoordinator.RegisterComponent<CameraFollowComponent>();
    gCoordinator.RegisterComponent<CameraStateComponent>();
    gCoordinator.RegisterComponent<CameraPathComponent>();
    gCoordinator.RegisterComponent<LockOnComponent>();
    gCoordinator.RegisterComponent<PhoneComponent>();
    gCoordinator.RegisterComponent<StateComponent>();
    gCoordinator.RegisterComponent<AimingComponent>();
    gCoordinator.RegisterComponent<DecalComponent>();
    gCoordinator.RegisterComponent<DrivingComponent>();
    gCoordinator.RegisterComponent<SneakingComponent>();
    gCoordinator.RegisterComponent<TrailEffectComponent>();

    // Gameplay Components
    gCoordinator.RegisterComponent<StaminaComponent>();
    gCoordinator.RegisterComponent<NeedsComponent>();
    gCoordinator.RegisterComponent<EmotionComponent>();
    gCoordinator.RegisterComponent<BehaviorComponent>();
    gCoordinator.RegisterComponent<SuspicionComponent>();
    gCoordinator.RegisterComponent<WantedLevelComponent>();
    gCoordinator.RegisterComponent<MoneyComponent>();
    gCoordinator.RegisterComponent<DialogueMoodComponent>();
    gCoordinator.RegisterComponent<DialogueComponent>();
    gCoordinator.RegisterComponent<DialogueCooldownComponent>();
    gCoordinator.RegisterComponent<ScheduleComponent>();
    gCoordinator.RegisterComponent<ProfessionComponent>();
    gCoordinator.RegisterComponent<NPCProfile>();
    gCoordinator.RegisterComponent<CombatCooldownComponent>();
    gCoordinator.RegisterComponent<HitboxComponent>();
    gCoordinator.RegisterComponent<ProjectileComponent>();
    gCoordinator.RegisterComponent<CombatStanceComponent>();
    gCoordinator.RegisterComponent<MemoryComponent>();
    gCoordinator.RegisterComponent<InteractionComponent>();
    gCoordinator.RegisterComponent<ScriptComponent>();
    gCoordinator.RegisterComponent<CutsceneComponent>();

    // Mission & Quest
    gCoordinator.RegisterComponent<ActiveMissionTrackerComponent>();
    gCoordinator.RegisterComponent<MissionComponent>();
    gCoordinator.RegisterComponent<SideMissionComponent>();
    gCoordinator.RegisterComponent<SideMissionOfferComponent>();
    gCoordinator.RegisterComponent<QuestDialogueComponent>();

    // Time & Weather
    gCoordinator.RegisterComponent<WorldTimeComponent>();
    gCoordinator.RegisterComponent<TimeEventComponent>();
    gCoordinator.RegisterComponent<GameEraComponent>();
    gCoordinator.RegisterComponent<WeatherStateComponent>();
    gCoordinator.RegisterComponent<WeatherReactiveComponent>();
    gCoordinator.RegisterComponent<WetnessComponent>();
    gCoordinator.RegisterComponent<WindEffectComponent>();
    gCoordinator.RegisterComponent<RainComponent>();
    gCoordinator.RegisterComponent<RainAudioComponent>();
    gCoordinator.RegisterComponent<RainOcclusionComponent>();
    gCoordinator.RegisterComponent<RainRipplesComponent>();
    gCoordinator.RegisterComponent<ScreenWetnessEffectComponent>();
    gCoordinator.RegisterComponent<PuddleComponent>();
    gCoordinator.RegisterComponent<TireSplashComponent>();
    gCoordinator.RegisterComponent<WetSurfaceComponent>();
    gCoordinator.RegisterComponent<BuildingComponent>();

    // Clothing
    gCoordinator.RegisterComponent<ClothingComponent>();
    gCoordinator.RegisterComponent<ClothingTypeComponent>();

    // Physics
    gCoordinator.RegisterComponent<BoundingSphereComponent>();
    gCoordinator.RegisterComponent<VehiculeTypeComponent>();
    gCoordinator.RegisterComponent<IsCarDamaged>();
    gCoordinator.RegisterComponent<IsCarTireFlat>();

    // Store
    gCoordinator.RegisterComponent<StoreComponent>();

    // Tags (all)
    gCoordinator.RegisterComponent<IsNPC>();
    gCoordinator.RegisterComponent<IsPlayer>();
    gCoordinator.RegisterComponent<IsFemale>();
    gCoordinator.RegisterComponent<IsPedestrian>();
    gCoordinator.RegisterComponent<IsNeighbour>();
    gCoordinator.RegisterComponent<IsFedUp>();
    gCoordinator.RegisterComponent<IsNpcSittingInPublic>();
    gCoordinator.RegisterComponent<IsMother>();
    gCoordinator.RegisterComponent<IsFather>();
    gCoordinator.RegisterComponent<IsBrother>();
    gCoordinator.RegisterComponent<IsFriend>();
    gCoordinator.RegisterComponent<IsOg>();
    gCoordinator.RegisterComponent<IsRude>();
    gCoordinator.RegisterComponent<IsTough>();
    gCoordinator.RegisterComponent<IsHighOrDrunk>();
    gCoordinator.RegisterComponent<IsHomeless>();
    gCoordinator.RegisterComponent<IsTherapist>();
    gCoordinator.RegisterComponent<IsMissionEntity>();
    gCoordinator.RegisterComponent<IsGlobalTimeEntity>();
    gCoordinator.RegisterComponent<IsGlobalWeatherEntity>();
    gCoordinator.RegisterComponent<IsPublicPhone>();
    gCoordinator.RegisterComponent<IsPolice>();
    gCoordinator.RegisterComponent<IsGangMember>();
    gCoordinator.RegisterComponent<IsBusDriver>();
    gCoordinator.RegisterComponent<IsSubwayRider>();
    gCoordinator.RegisterComponent<IsMechanic>();
    gCoordinator.RegisterComponent<IsCarPainter>();
    gCoordinator.RegisterComponent<IsCarTuner>();
    gCoordinator.RegisterComponent<IsShopKeeper>();
    gCoordinator.RegisterComponent<IsGunStoreKeeper>();
    gCoordinator.RegisterComponent<IsButcher>();
    gCoordinator.RegisterComponent<IsPharmacist>();
    gCoordinator.RegisterComponent<IsDoctor>();
    gCoordinator.RegisterComponent<IsNurse>();
    gCoordinator.RegisterComponent<IsEmergencyStaffNpc>();
    gCoordinator.RegisterComponent<IsFireFighter>();
    gCoordinator.RegisterComponent<IsConstructionWorker>();
    gCoordinator.RegisterComponent<IsConstructionSiteSecurity>();
    gCoordinator.RegisterComponent<IsConstructionSiteArchitect>();
    gCoordinator.RegisterComponent<IsSecurityAgent>();
    gCoordinator.RegisterComponent<IsClothingStoreKeeper>();
    gCoordinator.RegisterComponent<IsBrandClothingStoreKeeper>();
    gCoordinator.RegisterComponent<IsPawnShopKeeper>();
    gCoordinator.RegisterComponent<IsGasStationEmployee>();
    gCoordinator.RegisterComponent<IsRestaurantServer>();
    gCoordinator.RegisterComponent<IsRestaurantChef>();
    gCoordinator.RegisterComponent<IsBrandRestaurantServer>();
    gCoordinator.RegisterComponent<IsSuperMarketCashier>();
    gCoordinator.RegisterComponent<IsElectronicsStoreKeeper>();
    gCoordinator.RegisterComponent<IsGroceryStoreKeeper>();
    gCoordinator.RegisterComponent<IsHotelReceptionAgent>();
    gCoordinator.RegisterComponent<IsCarRepairShop>();
    gCoordinator.RegisterComponent<IsCarPaintingShop>();
    gCoordinator.RegisterComponent<IsCarTuningShop>();
    gCoordinator.RegisterComponent<IsTaxiDriver>();
    gCoordinator.RegisterComponent<IsDeliveryWorker>();
    gCoordinator.RegisterComponent<IsWaiter>();
    gCoordinator.RegisterComponent<IsBartender>();
    gCoordinator.RegisterComponent<IsDJ>();
    gCoordinator.RegisterComponent<IsTeacher>();
    gCoordinator.RegisterComponent<IsStreetPerformer>();
    gCoordinator.RegisterComponent<IsGraffitiArtist>();
    gCoordinator.RegisterComponent<IsInfluencer>();
    gCoordinator.RegisterComponent<IsStreetVendor>();
    gCoordinator.RegisterComponent<IsPickpocket>();
    gCoordinator.RegisterComponent<IsDrugDealer>();
    gCoordinator.RegisterComponent<IsFence>();
    gCoordinator.RegisterComponent<IsConArtist>();
    gCoordinator.RegisterComponent<IsFixer>();
    gCoordinator.RegisterComponent<IsInformant>();
    gCoordinator.RegisterComponent<IsPriest>();
    gCoordinator.RegisterComponent<IsSocialWorker>();
    gCoordinator.RegisterComponent<IsVolunteer>();
    gCoordinator.RegisterComponent<IsDog>();
    gCoordinator.RegisterComponent<IsCat>();
    gCoordinator.RegisterComponent<IsRat>();
    gCoordinator.RegisterComponent<IsCarDriver>();
    gCoordinator.RegisterComponent<IsMotorcycleDriver>();
    gCoordinator.RegisterComponent<IsBicycleDriver>();
    gCoordinator.RegisterComponent<IsElectricScooterDriver>();
    gCoordinator.RegisterComponent<IsSkateBoardSurfer>();
    gCoordinator.RegisterComponent<IsElectricSkateBoardSurfer>();

    // Vehicle Tags
    gCoordinator.RegisterComponent<Is1990sVehicle>();
    gCoordinator.RegisterComponent<Is2020sVehicle>();
    gCoordinator.RegisterComponent<IsTaxiCab>();
    gCoordinator.RegisterComponent<IsTouristBus>();
    gCoordinator.RegisterComponent<IsPoliceCar>();
    gCoordinator.RegisterComponent<IsSportsCar>();
    gCoordinator.RegisterComponent<IsLowrider>();
    gCoordinator.RegisterComponent<IsPickupTruck>();
    gCoordinator.RegisterComponent<IsSUV>();
    gCoordinator.RegisterComponent<IsClassicCar>();
    gCoordinator.RegisterComponent<IsElectricVehicle>();
    gCoordinator.RegisterComponent<IsHybridVehicle>();
    gCoordinator.RegisterComponent<IsDelivaryScooter>();
    gCoordinator.RegisterComponent<IsDelivaryTruck>();
    gCoordinator.RegisterComponent<IsCargoVan>();
    gCoordinator.RegisterComponent<IsDeliveryTruck>();
    gCoordinator.RegisterComponent<IsMuscleCar>();
    gCoordinator.RegisterComponent<IsProfessionalMoversTruck>();
    gCoordinator.RegisterComponent<IsBankTruck>();
    gCoordinator.RegisterComponent<IsTrashTruck>();
    gCoordinator.RegisterComponent<IsPoliceSwatTruck>();
    gCoordinator.RegisterComponent<IsMilitaryTruck>();
    gCoordinator.RegisterComponent<IsCivilianHelicopter>();
    gCoordinator.RegisterComponent<IsMilitaryHelicopter>();
    gCoordinator.RegisterComponent<IsPoliceHelicopter>();
    gCoordinator.RegisterComponent<IsMilitaryBoat>();
    gCoordinator.RegisterComponent<IsGangBoat>();
    gCoordinator.RegisterComponent<IsCivilianBus>();
    gCoordinator.RegisterComponent<IsPoliceBus>();
    gCoordinator.RegisterComponent<IsCargoTrain>();
    gCoordinator.RegisterComponent<IsCivilianTrain>();
    gCoordinator.RegisterComponent<IsSubway>();
    gCoordinator.RegisterComponent<IsCivilianPlane>();
    gCoordinator.RegisterComponent<IsPrivateJet>();
    gCoordinator.RegisterComponent<IsMilitaryJet>();
    gCoordinator.RegisterComponent<IsCivilianJeep>();
    gCoordinator.RegisterComponent<IsMilitaryJeep>();
    gCoordinator.RegisterComponent<IsMilitaryTank>();
    gCoordinator.RegisterComponent<IsSkateBoard>();
    gCoordinator.RegisterComponent<IsElectricSkateBoard>();

    // Clothing Tags
    gCoordinator.RegisterComponent<Is1990sClothing>();
    gCoordinator.RegisterComponent<Is2020sClothing>();
    gCoordinator.RegisterComponent<IsWearingBaggyJeans>();
    gCoordinator.RegisterComponent<IsWearingOversizedTee>();
    gCoordinator.RegisterComponent<IsWearingTimberlands>();
    gCoordinator.RegisterComponent<IsWearingLeatherJacket>();
    gCoordinator.RegisterComponent<IsWearingFubuJersey>();
    gCoordinator.RegisterComponent<IsWearingStarterJacket>();
    gCoordinator.RegisterComponent<IsWearingSlimFitJeans>();
    gCoordinator.RegisterComponent<IsWearingHoodie>();
    gCoordinator.RegisterComponent<IsWearingChinos>();
    gCoordinator.RegisterComponent<IsWearingSneakersModern>();
    gCoordinator.RegisterComponent<IsWearingTechwear>();
    gCoordinator.RegisterComponent<IsWearingCropPuffer>();
    gCoordinator.RegisterComponent<IsWearingCap>();
    gCoordinator.RegisterComponent<IsWearingGoldChain>();

    // Store Tags
    gCoordinator.RegisterComponent<IsPharmacy>();
    gCoordinator.RegisterComponent<IsDrugStore>();
    gCoordinator.RegisterComponent<IsBrandClothingStore>();
    gCoordinator.RegisterComponent<IsClothingStore>();
    gCoordinator.RegisterComponent<IsGunStore>();
    gCoordinator.RegisterComponent<IsGroceryStore>();
    gCoordinator.RegisterComponent<IsButcherStore>();
    gCoordinator.RegisterComponent<IsSuperMarket>();
    gCoordinator.RegisterComponent<IsElectronicsStore>();
    gCoordinator.RegisterComponent<IsGasStation>();
    gCoordinator.RegisterComponent<IsRestaurant>();
    gCoordinator.RegisterComponent<IsBrandRestaurant>();
    gCoordinator.RegisterComponent<IsPawnShop>();
    gCoordinator.RegisterComponent<IsHotel>();
    gCoordinator.RegisterComponent<IsConstructionSite>();
}
