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
	UFUNCTION(NetMulticast,Reliable)
	void MulticastRPCPaint(FVector Location,FVector PointNormal,FLinearColor Color);
protected:
	UFUNCTION(BlueprintImplementableEvent)
	void OnStaticMeshHitEvent(AActor* OtherActor);
private:
	UPROPERTY(EditAnywhere,Category = "PaintBall")
	TObjectPtr<class UMaterialInterface> TeamMaterial;

	UPROPERTY(EditAnywhere, Category = "PaintBall")
	TObjectPtr<class UTexture> SplatTexture;

	UPROPERTY(EditAnywhere, Category = "PaintBall")
	TObjectPtr<class UStaticMeshComponent> ProjectileSphere;

	UPROPERTY(EditAnywhere, Category = "PaintBall")
    TObjectPtr<class UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(EditAnywhere,Category = "PaintBall")
	float PaintSize = 100.f;

	
};
