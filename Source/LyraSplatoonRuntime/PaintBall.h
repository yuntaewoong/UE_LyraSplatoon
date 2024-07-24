// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PaintBall.generated.h"




UCLASS(Abstract)
class LYRASPLATOONRUNTIME_API APaintBall : public AActor
{
	GENERATED_BODY()
	
public:	
	APaintBall();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

private:
	UFUNCTION()
    void OnStaticMeshHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit
	);

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void OnStaticMeshHitEvent(AActor* OtherActor);
private:

	UPROPERTY(VisibleAnywhere, Category = "PaintBall")
	TObjectPtr<class UStaticMeshComponent> ProjectileSphere;

	UPROPERTY(VisibleAnywhere, Category = "PaintBall")
    TObjectPtr<class UProjectileMovementComponent> ProjectileMovement;

	
};
