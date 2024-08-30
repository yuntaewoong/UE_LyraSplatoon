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


    check(PostProcessMaterial);
    UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(PostProcessMaterial, this);


    FVector BoxCenter = VolumeBox->GetComponentLocation();
    FVector BoxExtent = VolumeBox->GetScaledBoxExtent();

    FVector BoxMin = BoxCenter - BoxExtent;
    FVector BoxMax = BoxCenter + BoxExtent;
    FVector BoxMin2BoxMax = BoxMax - BoxMin;
    //색칠정보를 기록할 RenderTarget을 동적생성합니다.(6방향)
    for (int32 i = 0; i < static_cast<int8>(ETextureNormalDirection::MAX); i++)
    {


        //렌더타겟 텍스쳐 생성
        switch (static_cast<ETextureNormalDirection>(i))
        {
        case ETextureNormalDirection::UP:
        case ETextureNormalDirection::DOWN://xy평면 사용
            PaintingRenderTarget[i] = UKismetRenderingLibrary::CreateRenderTarget2D(
                this, BoxMin2BoxMax.X * RTVividness, BoxMin2BoxMax.Y * RTVividness, RTF_RGBA8);
            break;
        case ETextureNormalDirection::LEFT:
        case ETextureNormalDirection::RIGHT://yz평면 사용
            PaintingRenderTarget[i] = UKismetRenderingLibrary::CreateRenderTarget2D(
                this, BoxMin2BoxMax.Y * RTVividness, BoxMin2BoxMax.Z * RTVividness, RTF_RGBA8);
            break;
        case ETextureNormalDirection::FRONT:
        case ETextureNormalDirection::BACK://xz평면 사용
            PaintingRenderTarget[i] = UKismetRenderingLibrary::CreateRenderTarget2D(
                this, BoxMin2BoxMax.X * RTVividness, BoxMin2BoxMax.Z * RTVividness, RTF_RGBA8);
            break;
        }

        
        ensure(PaintingRenderTarget[i]);
        UKismetRenderingLibrary::ClearRenderTarget2D(this, PaintingRenderTarget[i], FLinearColor::Black);

        //Material Instance 텍스쳐 파라미터 지정
        DynamicMaterial->SetTextureParameterValue(FName(TextureNames[i]), PaintingRenderTarget[i]);
    }
	
    
    
    
    
    DynamicMaterial->SetScalarParameterValue(FName(TEXT("BoxMaxX")), BoxMax.X);
    DynamicMaterial->SetScalarParameterValue(FName(TEXT("BoxMaxY")), BoxMax.Y);
    DynamicMaterial->SetScalarParameterValue(FName(TEXT("BoxMaxZ")), BoxMax.Z);
    DynamicMaterial->SetScalarParameterValue(FName(TEXT("BoxMinX")), BoxMin.X);
    DynamicMaterial->SetScalarParameterValue(FName(TEXT("BoxMinY")), BoxMin.Y);
    DynamicMaterial->SetScalarParameterValue(FName(TEXT("BoxMinZ")), BoxMin.Z);



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


void APaintingVolume::Paint(FVector Location,FVector PointNormal,float PaintSize,FLinearColor PaintColor,UTexture* PaintTexture)
{
    //PointNormal에 따라 그려야 할 렌더타겟을 선택합니다.
    ETextureNormalDirection TextureDirection =  GetTextureNormalDirection(PointNormal);
    //해당 렌더타겟에 텍스쳐를 그립니다
    UCanvas* Canvas;
	FVector2D Size;
	FDrawToRenderTargetContext Context;
	UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(
        this,
        PaintingRenderTarget[static_cast<int8>(TextureDirection)],
        Canvas,
        Size,
        Context
    );
    check(PaintTexture);
    UE_LOG(LogTemp,Warning,TEXT("Paint to %s"),*TextureNames[static_cast<int8>(TextureDirection)]);
    FVector2D AdjustedSize = FVector2D(PaintSize, PaintSize);

    UE_LOG(LogTemp, Warning, TEXT("Size : %s"), *Size.ToString());
    Canvas->K2_DrawTexture(
        PaintTexture, GetUV(Location,TextureDirection) * Size - AdjustedSize/2,
        AdjustedSize, FVector2D(0, 0),FVector2D::UnitVector,PaintColor
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


FVector2D APaintingVolume::GetUV(FVector Location,ETextureNormalDirection TextureDirection) const 
{
    //Location(절대 월드좌표)를 TextureDirection에 맞는 UV좌표로 변환합니다

    FVector BoxCenter = VolumeBox->GetComponentLocation();
    FVector BoxExtent = VolumeBox->GetScaledBoxExtent();

    FVector BoxMin = BoxCenter - BoxExtent;
    FVector BoxMax = BoxCenter + BoxExtent;

    FVector BoxMin2BoxMax = BoxMax - BoxMin;
    switch (TextureDirection)
    {
    case ETextureNormalDirection::UP:
    case ETextureNormalDirection::DOWN://xy평면
		return FVector2D((Location.X - BoxMin.X)/BoxMin2BoxMax.X ,
			(Location.Y - BoxMin.Y)/BoxMin2BoxMax.Y);
    case ETextureNormalDirection::FRONT:
    case ETextureNormalDirection::BACK://xz평면
        return FVector2D((Location.X - BoxMin.X) / BoxMin2BoxMax.X,
            (Location.Z - BoxMin.Z) / BoxMin2BoxMax.Z);
    case ETextureNormalDirection::LEFT:
    case ETextureNormalDirection::RIGHT://yz평면
        return FVector2D((Location.Y - BoxMin.Y) / BoxMin2BoxMax.Y,
            (Location.Z - BoxMin.Z) / BoxMin2BoxMax.Z);
    }
    ensure(true);//어느 경우에도 속하지 못한경우 에러메시지를 출력합니다
    return FVector2D::ZeroVector;
}

const ETextureNormalDirection APaintingVolume::GetTextureNormalDirection(FVector Normal)
{
    //6방향의 단위 벡터
    const FVector UpVector = FVector::UpVector;        // (0, 0, 1)
    const FVector DownVector = -FVector::UpVector;     // (0, 0, -1)
    const FVector ForwardVector = FVector::ForwardVector;  // (1, 0, 0)
    const FVector BackwardVector = -FVector::ForwardVector; // (-1, 0, 0)
    const FVector RightVector = FVector::RightVector;  // (0, 1, 0)
    const FVector LeftVector = -FVector::RightVector;  // (0, -1, 0)

    
    // 노멀 벡터와 단위 벡터 간의 내적 계산
    Normal.Normalize();
    const float DotUp = FVector::DotProduct(Normal, UpVector);
    const float DotDown = FVector::DotProduct(Normal, DownVector);
    const float DotForward = FVector::DotProduct(Normal, ForwardVector);
    const float DotBackward = FVector::DotProduct(Normal, BackwardVector);
    const float DotRight = FVector::DotProduct(Normal, RightVector);
    const float DotLeft = FVector::DotProduct(Normal, LeftVector);

    // 내적 결과 중 최대값을 찾아 해당 방향을 반환
    float MaxDot = FMath::Max3(DotUp, DotDown, DotForward);
    MaxDot = FMath::Max3(MaxDot, DotBackward, DotRight);
    MaxDot = FMath::Max(MaxDot, DotLeft);

    if (MaxDot == DotUp) return ETextureNormalDirection::UP;
    if (MaxDot == DotDown) return ETextureNormalDirection::DOWN;
    if (MaxDot == DotForward) return ETextureNormalDirection::FRONT;
    if (MaxDot == DotBackward) return ETextureNormalDirection::BACK;
    if (MaxDot == DotRight) return ETextureNormalDirection::RIGHT;
    if (MaxDot == DotLeft) return ETextureNormalDirection::LEFT;

    ensure(true);//어느 경우에도 속하지 못한경우 에러메시지를 출력합니다
    return ETextureNormalDirection::MAX;
}

