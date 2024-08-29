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
	friend class UPaintingVolumeSubsystem;
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void Paint(FVector Location,float PaintSize,FLinearColor PaintColor,UTexture* PaintTexture);
	class UTextureRenderTarget2D* GetPaintingRenderTarget() const { return PaintingRenderTarget; }
private:
	bool FindAllStaticMeshesInVolume(TArray<class UStaticMeshComponent*>& OutStaticMeshes);//볼륨안에 존재하는 모든 StaticMeshComponent를 검색합니다
	FVector2D WorldPositionToUV(FVector Location);
private:
	UPROPERTY(EditAnywhere, Category = "Components")
	TObjectPtr<class UBoxComponent> VolumeBox;//PaintingVolume의 영역을 정의합니다
	UPROPERTY(EditAnywhere, Category = "Components")
	TObjectPtr<class UMaterialInterface> PostProcessMaterial;

	



	UPROPERTY()
	TObjectPtr<class UTextureRenderTarget2D> PaintingRenderTarget;//색칠결과가 업데이트되는 렌더타겟 텍스처입니다

	UPROPERTY(EditAnywhere,Category = "Components")
	int32 RTWidth = 4096;


	UPROPERTY(EditAnywhere,Category = "Components")
	int32 RTHeight = 4096;


	
};
