// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTService_DetectPlayerCharacter.h"
#include "Controllers/NPCAIController.h"
#include "Characters/AOSCharacterBase.h"
#include "Characters/NonPlayerCharacterBase.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_DetectPlayerCharacter::UBTService_DetectPlayerCharacter()
{
	NodeName = TEXT("DetectPlayerCharacter");
	Interval = 0.3f;
}

void UBTService_DetectPlayerCharacter::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	ANPCAIController* AIC = Cast<ANPCAIController>(OwnerComp.GetAIOwner());
	if (::IsValid(AIC))
	{
		ANonPlayerCharacterBase* NPC = Cast<ANonPlayerCharacterBase>(AIC->GetPawn());
		if (::IsValid(NPC))
		{
			UWorld* World = NPC->GetWorld();
			if (::IsValid(World))
			{
				FVector CenterPosition = NPC->GetActorLocation();
				float DetectRadius = 500.f;

				TArray<FOverlapResult> OverlapResults;
				FCollisionQueryParams CollisionQueryParams(NAME_None, false, NPC);

				bool bResult = World->OverlapMultiByChannel(
					OverlapResults,
					CenterPosition,
					FQuat::Identity,
					ECollisionChannel::ECC_EngineTraceChannel4,
					FCollisionShape::MakeSphere(DetectRadius),
					CollisionQueryParams
				);

				if (bResult)
				{
					for (auto const& OverlapResult : OverlapResults)
					{
						AAOSCharacterBase* Player = Cast<AAOSCharacterBase>(OverlapResult.GetActor());
						if (::IsValid(Player))
						{
							if (Player->GetController()->IsPlayerController() == true)
							{
								OwnerComp.GetBlackboardComponent()->SetValueAsObject(ANPCAIController::TargetActorKey, Player);
								DrawDebugSphere(World, CenterPosition, DetectRadius, 16, FColor::Red, false, 0.5f);
							}
							else
							{
								OwnerComp.GetBlackboardComponent()->SetValueAsObject(ANPCAIController::TargetActorKey, nullptr);
								DrawDebugSphere(World, CenterPosition, DetectRadius, 16, FColor::Green, false, 0.5f);
								
							}
						}
					}
				}
				else
				{
					OwnerComp.GetBlackboardComponent()->SetValueAsObject(ANPCAIController::TargetActorKey, nullptr);
					DrawDebugSphere(World, CenterPosition, DetectRadius, 16, FColor::Green, false, 0.5f);
				}
			}
		}
	}
}