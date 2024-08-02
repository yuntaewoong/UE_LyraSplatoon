// Fill out your copyright notice in the Description page of Project Settings.


#include "PaintingVolume.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Engine/OverlapResult.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture.h"
#include "Engine/Canvas.h"
#include "Components/PostProcessComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PaintingVolumeSubsystem.h"

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
    UPaintingVolumeSubsystem* PaintingVolumeSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UPaintingVolumeSubsystem>();
    check(PaintingVolumeSubsystem);//PaintingVolumeSubsystem이 만들어져야 합니다, 생성이 안되었을시 런타임에러 강제발생
    
    PaintingVolumeSubsystem->AddInstance(this);

    //색칠정보를 기록할 RenderTarget을 동적생성합니다
	PaintingRenderTarget = UKismetRenderingLibrary::CreateRenderTarget2D(this, RTWidth, RTHeight, RTF_RGBA16f);
    ensure(PaintingRenderTarget);
    PaintingRenderTarget->SRGB = true;
	UKismetRenderingLibrary::ClearRenderTarget2D(this, PaintingRenderTarget, FLinearColor::Black);
    
    check(PostProcessMaterial);
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
			PaintingVolumeSubsystem->SetMeshCanBePainted(StaticMeshes[i]);
		}
	}

}

// Called every frame
void APaintingVolume::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}


void APaintingVolume::Paint(FVector Location,float PaintSize,FLinearColor PaintColor,UTexture* PaintTexture)
{
    UCanvas* Canvas;
	FVector2D Size;
	FDrawToRenderTargetContext Context;
	UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(this, PaintingRenderTarget, Canvas, Size, Context);
    check(PaintTexture);
    Canvas->K2_DrawTexture(
        PaintTexture, WorldPositionToUV(Location) * Size - FVector2D(PaintSize/2,PaintSize/2),
        FVector2D(PaintSize, PaintSize), FVector2D(0, 0),FVector2D::UnitVector,PaintColor
    );
	UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(this, Context);
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

