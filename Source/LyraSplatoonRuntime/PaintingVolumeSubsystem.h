﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PaintingVolumeSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class LYRASPLATOONRUNTIME_API UPaintingVolumeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	class APaintingVolume* GetPaintingVolumeInstance(FVector Location);//Location에 속하는 Painting Volume인스턴스 리턴(속하는 볼륨이 없다면 nullptr리턴)
	void AddInstance(class APaintingVolume* VolumeInstance);//서브시스템에 PaintingVolume을 등록합니다
	void SetMeshCanBePainted(class UStaticMeshComponent* MeshComponent);//인풋으로 주어진 Mesh가 색칠이 가능하도록 세팅합니다
private:

	const int32 CUSTOM_DEPTH_STENICL_VALUE = 11;//색칠 post process에 사용되는 CUSTOMDEPTHSTENCIL값
	//Material Editor에서 동일한 값을 사용해야 정상동작함

	TArray<class APaintingVolume*> PaintingVolumes;//게임 중 존재하는 PaintingVolume의 배열

};
