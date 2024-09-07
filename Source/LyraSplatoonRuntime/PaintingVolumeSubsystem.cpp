// Fill out your copyright notice in the Description page of Project Settings.


#include "PaintingVolumeSubsystem.h"
#include "PaintingVolume.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "ComputeShader/Public/NumPixelPaintedComputeShader/NumPixelPaintedComputeShader.h"
#include "Engine/TextureRenderTarget2DArray.h"
#include "Teams/LyraTeamSubsystem.h"
#include "Teams/LyraTeamDisplayAsset.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "PhysicsEngine/BodySetup.h"


void UPaintingVolumeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    // 게임 시작 후 일정주기마다 색칠정보 계산을 위한 Compute Shader를 비동기 실행합니다
    float StartDelay = 5.f;
    FTimerDelegate TimerDelegate;
    TimerDelegate.BindUFunction(this, FName("ComputePaintRate"),static_cast<ETextureNormalDirection>(ETextureNormalDirection::UP));
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, StartDelay,false,StartDelay);
    
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
    check(MeshComponent);
    MeshComponent->SetRenderCustomDepth(true);//CustomDepth를 기록하도록 합니다
    MeshComponent->SetCustomDepthStencilValue(CUSTOM_DEPTH_STENICL_VALUE);//후처리를 위해서 CustomStencil값을 GBuffer에 렌더링합니다

    UBodySetup* BodySetup = MeshComponent->GetStaticMesh()->GetBodySetup();
    if (BodySetup)
    {//ComplxeAsSimple을 사용하도록 재설정합니다
        BodySetup->CollisionTraceFlag = CTF_UseComplexAsSimple;
        MeshComponent->RecreatePhysicsState();
    }
}



int32 UPaintingVolumeSubsystem::GetPaintRate(int32 TeamID)
{
    int32 Result = 0;
    for (int8 i = 0; i < static_cast<int8>(ETextureNormalDirection::MAX); i++)
    {
        Result += PaintedRate[TeamID][i];
        //UE_LOG(LogTemp, Warning, TEXT("TeamID : %d dir : %d val : %f"),TeamID,i, PaintedRate[TeamID][i]);
    }
    return Result;
}

void UPaintingVolumeSubsystem::ComputePaintRate(ETextureNormalDirection TextureDir)
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
        
        ////컴퓨트 쉐이더 파라미터
        FNumPixelPaintedComputeShaderDispatchParams Params;
        Params.InputTexture = PaintingVolumes[0]->GetPaintingRenderTarget2DArray(TextureDir);
        Params.TargetColor1 = *RedTeamDisplayAsset->ColorParameters.Find(FName(TEXT("TeamColor")));
        Params.TargetColor2 = *BlueTeamDisplayAsset->ColorParameters.Find(FName(TEXT("TeamColor")));
        Params.TextureSize = FIntPoint(
            PaintingVolumes[0]->GetPaintingRenderTarget2DArray(TextureDir)->SizeX,
            PaintingVolumes[0]->GetPaintingRenderTarget2DArray(TextureDir)->SizeY
        );
        //컴퓨트 셰이더 호출 후 람다함수를 이용해서 결과값을 저장합니다
        FNumPixelPaintedComputeShaderInterface::Dispatch(Params,
            [this,iCopy = static_cast<int8>(TextureDir)](int OutputVal1,int OutputVal2)
            {
                //컴퓨트 셰이더 호출이 완료되고 실행되는 로직

                //1. GPU결과를 CPU메모리에 저장합니다
                this->PaintedRate[1][iCopy] = (static_cast<float>(OutputVal1) / (1024.0 * 1024.0)) * 100.0;
                this->PaintedRate[2][iCopy] = (static_cast<float>(OutputVal2) / (1024.0 * 1024.0)) * 100.0;

                // 일정 시간 뒤에 다시 ComputePaintRate를 호출합니다(한 프레임 중복 실행을 방지합니다)
                const float Delay = 0.5f; //호출 간격
                FTimerDelegate TimerDelegate;
                TimerDelegate.BindUFunction(this, FName("ComputePaintRate"),static_cast<ETextureNormalDirection>(
                    (iCopy + 1) % static_cast<int8>(ETextureNormalDirection::MAX)) //다음 텍스쳐 방향설정
                );
                GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, Delay,false,Delay);
            }
        );
    }
}


