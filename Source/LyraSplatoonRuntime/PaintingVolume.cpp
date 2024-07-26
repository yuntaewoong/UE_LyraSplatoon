// Fill out your copyright notice in the Description page of Project Settings.


#include "PaintingVolume.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Engine/OverlapResult.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Canvas.h"
#include "Components/PostProcessComponent.h"
#include "Kismet/GameplayStatics.h"

TArray<APaintingVolume*> APaintingVolume::PaintingVolumeInstances;

// Sets default values
APaintingVolume::APaintingVolume()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	VolumeBox = CreateDefaultSubobject<UBoxComponent>("VolumeBox");
	RootComponent = VolumeBox;
	VolumeBox->SetBoxExtent(FVector(500.f, 500.f, 500.f));
}

// Called when the game starts or when spawned
void APaintingVolume::BeginPlay()
{
	Super::BeginPlay();

    //자기 자신을 인스턴스목록에 추가합니다
    PaintingVolumeInstances.Add(this);

    //색칠정보를 기록할 RenderTarget을 동적생성합니다
	PaintingRenderTarget = UKismetRenderingLibrary::CreateRenderTarget2D(this, RTWidth, RTHeight, RTF_RGBA16f);
	UKismetRenderingLibrary::ClearRenderTarget2D(this, PaintingRenderTarget, FLinearColor::Black);


    UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(PostProcessMaterial, this);
	DynamicMaterial->SetTextureParameterValue(FName(TEXT("RenderTarget")), PaintingRenderTarget);
    FVector BoxCenter = VolumeBox->GetComponentLocation();
    FVector BoxExtent = VolumeBox->GetScaledBoxExtent();

    FVector BoxMin = BoxCenter - BoxExtent;
    FVector BoxMax = BoxCenter + BoxExtent;
    DynamicMaterial->SetScalarParameterValue(FName(TEXT("BoxMaxX")), BoxMax.X);
    DynamicMaterial->SetScalarParameterValue(FName(TEXT("BoxMaxY")), BoxMax.Y);
    DynamicMaterial->SetScalarParameterValue(FName(TEXT("BoxMinX")), BoxMin.X);
    DynamicMaterial->SetScalarParameterValue(FName(TEXT("BoxMinY")), BoxMin.Y);



    APostProcessVolume* PostProcessVolume = Cast<APostProcessVolume>(
        UGameplayStatics::GetActorOfClass(GetWorld(), APostProcessVolume::StaticClass())
    );
    if (PostProcessVolume)
    {
        PostProcessVolume->AddOrUpdateBlendable(DynamicMaterial);
    }


	TArray<UStaticMeshComponent*> StaticMeshes;
	if (FindAllStaticMeshesInVolume(StaticMeshes))
	{
		for (int32 i = 0; i < StaticMeshes.Num(); i++)
		{
			SetMeshCanBePainted(StaticMeshes[i]);
		}
	}

}

// Called every frame
void APaintingVolume::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

bool APaintingVolume::IsPaintableMesh(UStaticMeshComponent* StaticMesh)
{
    return StaticMesh->CustomDepthStencilValue == CUSTOM_DEPTH_STENICL_VALUE;//인풋으로 주어진 StaticMesh가 Painting이 가능한 Mesh면 true
}

void APaintingVolume::Paint(FVector Location)
{
    UCanvas* Canvas;
	FVector2D Size;
	FDrawToRenderTargetContext Context;
	UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(this, PaintingRenderTarget, Canvas, Size, Context);
	
	Canvas->K2_DrawBox(WorldPositionToUV(Location)*Size, FVector2D(15,15));
	UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(this, Context);
}

APaintingVolume* APaintingVolume::GetInstance(FVector Location)
{
    for (int32 i = 0; i < PaintingVolumeInstances.Num(); i++)
    {
        FVector BoxCenter = PaintingVolumeInstances[i]->VolumeBox->GetComponentLocation();
        FVector BoxExtent = PaintingVolumeInstances[i]->VolumeBox->GetScaledBoxExtent();

        // Calculate the minimum and maximum bounds of the box
        FVector BoxMin = BoxCenter - BoxExtent;
        FVector BoxMax = BoxCenter + BoxExtent;

        if ((Location.X >= BoxMin.X && Location.X <= BoxMax.X &&
            Location.Y >= BoxMin.Y && Location.Y <= BoxMax.Y &&
            Location.Z >= BoxMin.Z && Location.Z <= BoxMax.Z))
        {
            return PaintingVolumeInstances[i];
        }
    }
    return nullptr;
}

bool APaintingVolume::FindAllStaticMeshesInVolume(TArray<UStaticMeshComponent*>& OutStaticMeshes)
{
	// 박스의 중심과 반경을 가져옵니다.
    FVector Origin = VolumeBox->Bounds.Origin;
    FVector BoxExtent = VolumeBox->Bounds.BoxExtent;

    // 충돌 쿼리 파라미터 설정
    TArray<FOverlapResult> OverlapResults;
    FCollisionQueryParams CollisionParams;
    CollisionParams.AddIgnoredActor(this); // 자신을 무시하도록 설정

    // 충돌 쿼리 수행
    bool bHasOverlap = GetWorld()->OverlapMultiByChannel(
        OverlapResults,
        Origin,
        FQuat::Identity,
        ECC_WorldStatic,
        FCollisionShape::MakeBox(BoxExtent),
        CollisionParams
    );

    // 충돌한 결과를 처리
    if (bHasOverlap)
    {
        for (const FOverlapResult& Result : OverlapResults)
        {
            UStaticMeshComponent* StaticMeshComp = Cast<UStaticMeshComponent>(Result.GetComponent());
            if (StaticMeshComp)
            {
                OutStaticMeshes.Add(StaticMeshComp);
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No StaticMeshComponents found in the box."));
        return false;
    }

    DrawDebugBox(GetWorld(), Origin, BoxExtent, FColor::Green, false, 5.f);
	return true;
}

void APaintingVolume::SetMeshCanBePainted(UStaticMeshComponent* MeshComponent)
{
    MeshComponent->SetRenderCustomDepth(true);//CustomDepth를 기록하도록 합니다
	MeshComponent->SetCustomDepthStencilValue(CUSTOM_DEPTH_STENICL_VALUE);//후처리를 위해서 CustomStencil값을 GBuffer에 렌더링합니다
}

FVector2D APaintingVolume::WorldPositionToUV(FVector Location)
{//렌더타겟은 2차원이므로 3차원인 Location을 2차원 UV좌표로 변환하는 로직(이 로직은 항상 M_PaintPostProcess머티리얼에서 사용되는 로직과 동일해야함)

    FVector BoxCenter = VolumeBox->GetComponentLocation();
    FVector BoxExtent = VolumeBox->GetScaledBoxExtent();

    FVector BoxMin = BoxCenter - BoxExtent;
    FVector BoxMax = BoxCenter + BoxExtent;

    FVector BoxMin2BoxMax = BoxMax - BoxMin;
    return FVector2D((Location.X - BoxMin.X)/BoxMin2BoxMax.X ,
        (Location.Y - BoxMin.Y)/BoxMin2BoxMax.Y);
    //일단 x,y만 사용(이러면 x,y좌표는 같지만 z좌표는 다른 mesh지점에도 painting효과가 적용됨)
}

