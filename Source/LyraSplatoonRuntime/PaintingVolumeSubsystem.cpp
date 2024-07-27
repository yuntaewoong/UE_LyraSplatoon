// Fill out your copyright notice in the Description page of Project Settings.


#include "PaintingVolumeSubsystem.h"
#include "PaintingVolume.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"

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
