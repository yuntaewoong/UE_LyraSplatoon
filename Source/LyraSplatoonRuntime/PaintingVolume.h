// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PaintingVolume.generated.h"

UCLASS(Abstract)
class LYRASPLATOONRUNTIME_API APaintingVolume : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APaintingVolume();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	static bool IsPaintableMesh(class UStaticMeshComponent* StaticMesh);
	void Paint(FVector Location);
	static APaintingVolume* GetInstance(FVector Location);//Location? ???? APaintingVolume??????? ?????(??? ????? ??? nullptr??)
private:
	bool FindAllStaticMeshesInVolume(TArray<class UStaticMeshComponent*>& OutStaticMeshes);//???? ?? StaticMesh????? ????
	void SetMeshCanBePainted(class UStaticMeshComponent* MeshComponent);//?? Mesh? Painting??? Mesh? ?????
	FVector2D WorldPositionToUV(FVector Location);
private:
	const static int32 CUSTOM_DEPTH_STENICL_VALUE = 11;//???? ?? CustomDepthStencil????
	//(%Material Editor? ?? ????%)

	static TArray<APaintingVolume*> PaintingVolumeInstances;//APaintingVolume??? ????? ???? ?????

	UPROPERTY(EditAnywhere, Category = "Components")
	TObjectPtr<class UBoxComponent> VolumeBox;//????? ???? ??? ?????
	UPROPERTY(EditAnywhere, Category = "Components")
	TObjectPtr<class UMaterialInterface> PostProcessMaterial;

	UPROPERTY()
	TObjectPtr<class UTextureRenderTarget2D> PaintingRenderTarget;//?? ???? ??? ???

	UPROPERTY(EditAnywhere,Category = "Components")
	int32 RTWidth = 4096;


	UPROPERTY(EditAnywhere,Category = "Components")
	int32 RTHeight = 4096;


	
};
