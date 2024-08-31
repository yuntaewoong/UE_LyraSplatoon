// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PaintingVolume.generated.h"


//PaintingVolume의 6가지 텍스쳐의 종류를 정의합니다
UENUM()
enum class ETextureNormalDirection : uint8  // 
{
    UP = 0 UMETA(DisplayName = "UP"),
    DOWN = 1 UMETA(DisplayName = "DOWN"),
    FRONT = 2 UMETA(DisplayName = "FRONT"),
    BACK= 3 UMETA(DisplayName = "BACK"),
	LEFT=4 UMETA(DisplayName = "LEFT"),
	RIGHT=5 UMETA(DisplayName = "RIGHT"),
	MAX =6 UMETA(Hidden) 
};





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
	void Paint(FVector Location,FVector PointNormal,float PaintSize,FLinearColor PaintColor,UTexture* PaintTexture);
	class UTextureRenderTarget2D* GetPaintingRenderTarget(ETextureNormalDirection Direction) const
		{ return PaintingRenderTarget[static_cast<int8>(Direction)]; }
private:
	bool FindAllStaticMeshesInVolume(TArray<class UStaticMeshComponent*>& OutStaticMeshes);//볼륨안에 존재하는 모든 StaticMeshComponent를 검색합니다
	FVector2D GetUV(FVector Location,ETextureNormalDirection TextureDirection) const;
	static const ETextureNormalDirection GetTextureNormalDirection(FVector Normal);//입력된 법선벡터에 따라 텍스쳐의 방향을 반환합니다
private:
	UPROPERTY(EditAnywhere, Category = "Components")
	TObjectPtr<class UBoxComponent> VolumeBox;//PaintingVolume의 영역을 정의합니다
	UPROPERTY(EditAnywhere, Category = "Components")
	TObjectPtr<class UMaterialInterface> PostProcessMaterial;

	



	//색칠결과가 업데이트되는 렌더타겟 텍스처입니다. 상단,하단,앞,뒤,왼쪽,오른쪽 6방향으로 정의됩니다
	UPROPERTY()
	TObjectPtr<class UTextureRenderTarget2D> PaintingRenderTarget[static_cast<int8>(ETextureNormalDirection::MAX)];

	UPROPERTY(EditAnywhere,Category = "Components")
	float RTVividness;//렌더타겟의 선명도를 정의합니다.(이 값이 높을수록 선명해집니다)


	const FString TextureNames[static_cast<int8>(ETextureNormalDirection::MAX)] = 
	{
		TEXT("RenderTargetUP"),
		TEXT("RenderTargetDOWN"),
		TEXT("RenderTargetFRONT"),
		TEXT("RenderTargetBACK"),
		TEXT("RenderTargetLEFT"),
		TEXT("RenderTargetRIGHT")
	};
	
};
