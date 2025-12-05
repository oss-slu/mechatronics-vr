// Fill out your copyright notice in the Description page of Project Settings.


#include "MechatronicsVR/Public/SnapValidatorComponent.h"
#include "SnapPointComponent.h"
#include "PartActor.h"
#include "AssemblyActor.h"
#include "MechatronicsGameMode.h"
#include "MechatronicsVR/Lesson/LessonManagerComponent.h"
#include "MechatronicsVR/Lesson/AssembleStep.h"
#include "MechatronicsVR/Lesson/LessonStep.h"

// Sets default values for this component's properties
USnapValidatorComponent::USnapValidatorComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

bool USnapValidatorComponent::isSnapValid(USnapPointComponent* SnapPoint, const USnapPointComponent* TargetSnapPoint) const
{
	if (!SnapPoint->CanAcceptPoint(TargetSnapPoint) || !TargetSnapPoint->CanAcceptPoint(SnapPoint))
	{
		return false;
	}
	
	UWorld* World = GetWorld();
	if (!World) return true;

	// checks current gamemode, if a lesson component exists, and if there is currently an assemble step
	const AMechatronicsGameMode* GM = World->GetAuthGameMode<AMechatronicsGameMode>();
	if (!GM) return true;

	const ULessonManagerComponent* LessonManager = GM->FindComponentByClass<ULessonManagerComponent>();
	if (!LessonManager || !LessonManager->CurrentStep) return true;

	const UAssembleStep* AssembleStep = Cast<UAssembleStep>(LessonManager->CurrentStep);
	if (!AssembleStep) return true;

	if (AssembleStep->TargetPartClasses.Num() == 0)
	{
		return true;
	}

	const AActor* OwnerA = SnapPoint->GetOwner();
	const AActor* OwnerB = TargetSnapPoint->GetOwner();

	const bool bAIsBase = OwnerA && OwnerA->IsA<AAssemblyActor>();
	const bool bBIsBase = OwnerB && OwnerB->IsA<AAssemblyActor>();

	const APartActor* PartA = Cast<APartActor>(OwnerA);
	const APartActor* PartB = Cast<APartActor>(OwnerB);

	// Lambda function to see if a part's class matches one in the current step
	auto IsAllowedByStep  = [AssembleStep](const APartActor* Part)->bool
	{
		if (!Part) return false;

		for (const TSubclassOf<APartActor>& Allowed : AssembleStep->TargetPartClasses)
		{
			if (*Allowed && Part->IsA(Allowed))
			{
				return true;
			}
		}
		return false;
	};
	if (bAIsBase || bBIsBase)
	{
		const APartActor* ThePart = bAIsBase? PartB : PartA;
		return IsAllowedByStep(ThePart);
	}
	return IsAllowedByStep(PartA) && IsAllowedByStep(PartB);
}
void USnapValidatorComponent::OnSnapCompleted(USnapPointComponent* SnapPoint, USnapPointComponent* TargetSnapPoint)
{
}

bool USnapValidatorComponent::CanBeDisassembled(USnapPointComponent* SnapPoint) const
{
	//default
	if (!SnapPoint) return false;
	return true;
}


// Called when the game starts
void USnapValidatorComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void USnapValidatorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

