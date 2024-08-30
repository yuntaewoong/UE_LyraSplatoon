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
    int32 Result = 0;
    for (int8 i = 0; i < static_cast<int8>(ETextureNormalDirection::MAX); i++)
    {
        Result += PaintedRate[TeamID][i];
        //UE_LOG(LogTemp, Warning, TEXT("%d %f"), i, PaintedRate[TeamID][i]);
    }
    return Result;
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
        
        for (int8 i = 0; i < static_cast<int8>(ETextureNormalDirection::MAX); i++)
        {//6면에 대해서 각각 컴퓨트셰이더 연산을 수행합니다
            ////컴퓨트 쉐이더 파라미터
            FNumPixelPaintedComputeShaderDispatchParams Params;
            Params.InputTexture = PaintingVolumes[0]->GetPaintingRenderTarget(static_cast<ETextureNormalDirection>(i));
            Params.TargetColor1 = *RedTeamDisplayAsset->ColorParameters.Find(FName(TEXT("TeamColor")));
            Params.TargetColor2 = *BlueTeamDisplayAsset->ColorParameters.Find(FName(TEXT("TeamColor")));
            Params.TextureSize = FIntPoint(
                PaintingVolumes[0]->GetPaintingRenderTarget(static_cast<ETextureNormalDirection>(i))->SizeX,
                PaintingVolumes[0]->GetPaintingRenderTarget(static_cast<ETextureNormalDirection>(i))->SizeY
            );
            //컴퓨트 셰이더 호출 후 람다함수를 이용해서 결과값을 저장합니다
            FNumPixelPaintedComputeShaderInterface::Dispatch(Params, [this,i](int OutputVal1,int OutputVal2) {
                this->PaintedRate[1][i] = (static_cast<float>(OutputVal1) / (1024.0 * 1024.0)) * 100.0;
                this->PaintedRate[2][i] = (static_cast<float>(OutputVal2) / (1024.0 * 1024.0)) * 100.0;
            });

            //    // Render target resource를 가져옵니다.
            //FTextureRenderTargetResource* RenderTargetResource = PaintingVolumes[0]->GetPaintingRenderTarget()->GameThread_GetRenderTargetResource();
            //if (!RenderTargetResource)
            //{
            //    UE_LOG(LogTemp, Warning, TEXT("Failed to get RenderTargetResource!"));
            //    return;
            //}

            //// 텍스처의 크기 가져오기
            //int32 TextureWidth = PaintingVolumes[0]->GetPaintingRenderTarget()->SizeX;
            //int32 TextureHeight = PaintingVolumes[0]->GetPaintingRenderTarget()->SizeY;

            //// 텍스처 데이터를 저장할 메모리 할당 (RGBA8 형식으로 가정)
            //TArray<FColor> PixelData;
            //PixelData.SetNumUninitialized(TextureWidth * TextureHeight);

            //// 텍스처 데이터를 CPU 메모리로 복사
            //RenderTargetResource->ReadPixels(PixelData);

            //// 특정 색상에 해당하는 픽셀 수를 세는 변수
            //int32 SpecificColorPixelCount = 0;
            //FLinearColor SpecificColor = *RedTeamDisplayAsset->ColorParameters.Find(FName(TEXT("TeamColor")));; 

            //// 모든 픽셀을 순회하며 특정 색상인지 확인
            //for (int32 Y = 0; Y < TextureHeight; Y++)
            //{
            //    for (int32 X = 0; X < TextureWidth; X++)
            //    {
            //        // 픽셀 색상 가져오기
            //        FLinearColor PixelColor = PixelData[Y * TextureWidth + X];

            //        // 특정 색상과 비교
            //        if (PixelColor == SpecificColor)
            //        {
            //            SpecificColorPixelCount++;
            //        }
            //    }
            //}
        }
        
        


        
    }
}


