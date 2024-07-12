// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/CharacterBase.h"
#include "Structs/EnumMinionType.h"
#include "MinionBase.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_AOS_API AMinionBase : public ACharacterBase
{
	GENERATED_BODY()
	
public:
	AMinionBase();

protected:
	virtual void BeginPlay() override;
	virtual void SetWidget(class UUserWidgetBase* InUserWidgetBase) override;
	virtual void PostInitializeComponents() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	void EnableRagdoll();
	void SetAnimMontages(const TMap<FString, UAnimMontage*>& MontageMap);

	UPROPERTY()
	EMinionType MinionType = EMinionType::None;

public:	
	// Getter �Լ� ����
	UFUNCTION(BlueprintCallable, Category = "Bounty")
	float GetExpBounty() const;

	UFUNCTION(BlueprintCallable, Category = "Bounty")
	int32 GetGoldBounty() const;

	// Setter �Լ� ����
	UFUNCTION(BlueprintCallable, Category = "Bounty")
	void SetExpBounty(float NewExpBounty);

	UFUNCTION(BlueprintCallable, Category = "Bounty")
	void SetGoldBounty(int32 NewGoldBounty);

	UFUNCTION(BlueprintCallable, Category = "Direction")
	int32 GetRelativeDirection(AActor* OtherActor) const;

protected:
	// ��Ÿ�� ���� �Լ�
	UFUNCTION(Server, Reliable)
	void StopAllMontages_Server(float BlendOut);

	UFUNCTION(NetMulticast, Reliable)
	void StopAllMontages_NetMulticast(float BlendOut);

	UFUNCTION(Server, Reliable)
	void PlayMontage_Server(UAnimMontage* Montage, float PlayRate);

	UFUNCTION(NetMulticast, Reliable)
	void PlayMontage_NetMulticast(UAnimMontage* Montage, float PlayRate);

	UFUNCTION(Server, Reliable)
	void MontageJumpToSection_Server(UAnimMontage* Montage, FName SectionName, float PlayRate);

	UFUNCTION(NetMulticast, Reliable)
	void MontageJumpToSection_NetMulticast(UAnimMontage* Montage, FName SectionName, float PlayRate);

	UFUNCTION()
	void OnRep_SkeletalMesh();

	UFUNCTION()
	virtual void OnCharacterDeath();

	UFUNCTION()
	virtual void MontageEnded(UAnimMontage* Montage, bool bInterrupted);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MinionBase")
	TObjectPtr<class UMinionAnimInstance> AnimInstance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MinionBase")
	TObjectPtr<class AMinionAIController> AIController;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MinionBase", Meta = (AllowPrivateAccess))
	TObjectPtr<class UCharacterWidgetComponent> WidgetComponent;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "MinionBase", Meta = (AllowPrivateAccess))
	TObjectPtr<class UUW_StateBar> StateBar;

public:
	UPROPERTY(ReplicatedUsing = OnRep_SkeletalMesh)
	USkeletalMesh* ReplicatedSkeletalMesh;

protected:
	/**	 Animation Montages **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, UAnimMontage*> Montages;

protected:
	// Bounty ���� ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounty", meta = (AllowPrivateAccess = "true"))
	float ExpBounty;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounty", meta = (AllowPrivateAccess = "true"))
	int32 GoldBounty;

	// ��� �ִϸ��̼� ������ ���� Ȯ���ϴ� ����
	FTimerHandle DeathMontageTimerHandle;

	// Ragdoll ���� Ÿ��
	float RagdollBlendTime = 0.2f;
};
