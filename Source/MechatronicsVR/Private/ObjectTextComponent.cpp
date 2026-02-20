// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectTextComponent.h"

#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

UObjectTextComponent::UObjectTextComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UObjectTextComponent::OnRegister()
{
    Super::OnRegister();

    AActor* Owner = GetOwner();
    if (!Owner) return;
    
    if (!WidgetComponent)
    {
        WidgetComponent = NewObject<UWidgetComponent>(Owner, TEXT("ObjectTextWidget"));
        if (!WidgetComponent) return;
        
        Owner->AddInstanceComponent(WidgetComponent);
        WidgetComponent->RegisterComponent();
        
        WidgetComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);

        // Basic defaults
        WidgetComponent->SetWidgetSpace(EWidgetSpace::World);
        WidgetComponent->SetTwoSided(true);
        WidgetComponent->SetDrawAtDesiredSize(false);
    }
    
    ApplyAllSettings();
}

void UObjectTextComponent::OnUnregister()
{
    if (WidgetComponent)
    {
        WidgetComponent->DestroyComponent();
        WidgetComponent = nullptr;
    }

    Super::OnUnregister();
}

void UObjectTextComponent::BeginPlay()
{
    Super::BeginPlay();

    ApplyAllSettings();
}

void UObjectTextComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bFacePlayer || !WidgetComponent || !WidgetComponent->IsVisible())
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (!PC || !PC->PlayerCameraManager) return;

    const FVector CamLoc = PC->PlayerCameraManager->GetCameraLocation();
    const FVector MyLoc  = WidgetComponent->GetComponentLocation();

    FRotator LookRot = (CamLoc - MyLoc).Rotation();

    if (bLockPitch)
    {
        LookRot.Pitch = 0.f;
        LookRot.Roll  = 0.f;
    }

    WidgetComponent->SetWorldRotation(LookRot);
}

void UObjectTextComponent::SetLabelText(const FText& NewText)
{
    LabelText = NewText;
    UpdateWidgetText();
}

void UObjectTextComponent::SetLabelVisible(bool bInVisible)
{
    bLabelVisible = bInVisible;
    UpdateWidgetVisibility();
}

void UObjectTextComponent::ShowLabel()
{
    SetLabelVisible(true);
}

void UObjectTextComponent::HideLabel()
{
    SetLabelVisible(false);
}

void UObjectTextComponent::ApplyAllSettings()
{
    if (!WidgetComponent)
    {
        return;
    }

    // Apply class
    if (LabelWidgetClass)
    {
        WidgetComponent->SetWidgetClass(LabelWidgetClass);
    }

    WidgetComponent->SetDrawSize(DrawSize);
    WidgetComponent->SetPivot(Pivot);
    WidgetComponent->SetWorldScale3D(FVector(WorldScale));

    UpdateWidgetTransform();
    UpdateWidgetVisibility();
    UpdateWidgetText();
}

void UObjectTextComponent::UpdateWidgetTransform()
{
    if (!WidgetComponent)
    {
        return;
    }

    // Put widget above the owning component/actor.
    WidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, HeightOffset));
}

void UObjectTextComponent::UpdateWidgetVisibility()
{
    if (!WidgetComponent)
    {
        return;
    }

    WidgetComponent->SetVisibility(bLabelVisible, true);
    
    const bool bShouldTick = bFacePlayer && bLabelVisible;
    SetComponentTickEnabled(bShouldTick);
}

void UObjectTextComponent::UpdateWidgetText()
{
    if (!WidgetComponent)
    {
        return;
    }

    UUserWidget* Widget = WidgetComponent->GetUserWidgetObject();
    if (!Widget)
    {
        return;
    }
    
    static const FName FuncName(TEXT("SetText"));
    UFunction* Func = Widget->FindFunction(FuncName);

    if (!Func)
    {
        return;
    }

    struct FSetLabelText_Params
    {
        FText InText;
    };

    FSetLabelText_Params Params;
    Params.InText = LabelText;

    Widget->ProcessEvent(Func, &Params);
}