// Fill out your copyright notice in the Description page of Project Settings.


#include "PaintingVolumeSubsystem.h"
#include "PaintingVolume.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "ComputeShader/Public/NumPixelPaintedComputeShader/NumPixelPaintedComputeShader.h"
#include "Teams/LyraTeamSubsystem.h"
#include "Teams/LyraTeamDisplayAsset.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UPaintingVolumeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    // 게임 시작 후 0.3초에 한 번씩 색칠정보 계산을 위한 Compute Shader를 비동기 실행합니다
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UPaintingVolumeSubsystem::ComputePaintRate, 0.3f, true,5.f);
}

void UPaintingVolumeSubsystem::Deinitialize()
{
    GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
    Super::Deinitialize();
}

APaintingVolume* UPaintingVolumeSubsystem::GetPaintingVolumeInstance(FVector Location)
{   
    for (int32 i = 0; i < PaintingVolumes.Num(); i++)
    {
        FVector BoxCenter = PaintingVolumes[i]->VolumeBox->GetComponentLocation();
        FVector BoxExtent = PaintingVolumes[i]->VolumeBox->GetScaledBoxExtent();

        // Calculate the minimum and maximum bounds of the box
        FVector BoxMin = BoxCenter - BoxExtent;
        FVector BoxMax = BoxCenter + BoxExtent;

        if ((Location.X >= BoxMin.X && Location.X <= BoxMax.X &&
            Location.Y >= BoxMin.Y && Location.Y <= BoxMax.Y &&
            Location.Z >= BoxMin.Z && Location.Z <= BoxMax.Z))
        {
            return PaintingVolumes[i];
        }
    }
    return nullptr;
}

void UPaintingVolumeSubsystem::AddInstance(APaintingVolume* VolumeInstance)
{
    PaintingVolumes.Add(VolumeInstance);
}

void UPaintingVolumeSubsystem::SetMeshCanBePainted(UStaticMeshComponent* MeshComponent)
{
    MeshComponent->SetRenderCustomDepth(true);//CustomDepth를 기록하도록 합니다
	MeshComponent->SetCustomDepthStencilValue(CUSTOM_DEPTH_STENICL_VALUE);//후처리를 위해서 CustomStencil값을 GBuffer에 렌더링합니다
}

int32 UPaintingVolumeSubsystem::GetPaintRate(int32 TeamID)
{
    return PaintedRate[TeamID];
}

void UPaintingVolumeSubsystem::ComputePaintRate()
{//Compute Shader를 이용해서 색칠된 비율을 계산합니다.(Timer에 의해서 설정된 시간에 한번씩 호출됩니다)
    ULyraTeamSubsystem* LyraTeamSubsystem = GetWorld()->GetSubsystem<ULyraTeamSubsystem>();
    if (LyraTeamSubsystem)
    {//Lyra 팀 서브시스템을 가져옵니다
        ULyraTeamDisplayAsset* RedTeamDisplayAsset =
            LyraTeamSubsystem->GetTeamDisplayAsset(1,1);
        ULyraTeamDisplayAsset* BlueTeamDisplayAsset =
            LyraTeamSubsystem->GetTeamDisplayAsset(2,2);
        check(RedTeamDisplayAsset);
        check(BlueTeamDisplayAsset);
        
        //컴퓨트 쉐이더 파라미터
        FNumPixelPaintedComputeShaderDispatchParams Params;
        Params.InputTexture = PaintingVolumes[0]->GetPaintingRenderTarget();
        Params.TargetColor1 = *RedTeamDisplayAsset->ColorParameters.Find(FName(TEXT("TeamColor")));
        Params.TargetColor2 = *BlueTeamDisplayAsset->ColorParameters.Find(FName(TEXT("TeamColor")));
        Params.TextureSize = FIntPoint(
            PaintingVolumes[0]->GetPaintingRenderTarget()->SizeX,
            PaintingVolumes[0]->GetPaintingRenderTarget()->SizeY
        );
        //컴퓨트 셰이더 호출 후 람다함수를 이용해서 결과값을 저장합니다
        FNumPixelPaintedComputeShaderInterface::Dispatch(Params, [this](int OutputVal1,int OutputVal2) {
            this->PaintedRate[1] = (static_cast<float>(OutputVal1) / (4096.0*4096.0)) * 100.0;
            this->PaintedRate[2] = (static_cast<float>(OutputVal2) / (4096.0*4096.0)) * 100.0;
        });
        
        
    }
}


