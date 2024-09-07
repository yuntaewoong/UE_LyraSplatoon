// Fill out your copyright notice in the Description page of Project Settings.


#include "PaintingVolume.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Engine/OverlapResult.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/TextureRenderTarget2DArray.h"
#include "Engine/Texture.h"
#include "Engine/Canvas.h"
#include "TextureRenderTarget2DArrayResource.h"
#include "Components/PostProcessComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PaintingVolumeSubsystem.h"
#include "RHI.h"

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

    
    // 시스템에서 지원하는 최대 렌더타겟 해상도를 가져옵니다(대부분의 GPU는 16384x16384까지 지원합니다)
    
    int32 MaxTextureWidth = 16384;
    int32 MaxTextureHeight = MaxTextureWidth;
    
    //상,하,앞,뒤,좌,우 6방향에 대응하는 렌더타겟 텍스쳐를 생성합니다
    for (int32 i = 0; i < static_cast<int8>(ETextureNormalDirection::MAX); i++)
    {
        int32 WidthResolution = 0;
        int32 HeightResolution = 0;
        ETextureRenderTargetFormat PaintingRenderTargetFormat = RTF_RGBA8;
        FLinearColor ClearColor = FLinearColor::Black;
        switch (static_cast<ETextureNormalDirection>(i))
        {
        case ETextureNormalDirection::UP:
        case ETextureNormalDirection::DOWN://xy평면 사용
            WidthResolution = static_cast<int32>(BoxMin2BoxMax.X * RTVividness * NumSliceXY);
            HeightResolution = static_cast<int32>(BoxMin2BoxMax.Y * RTVividness);
            checkf(WidthResolution <= MaxTextureWidth, TEXT("%d Exceeded Max Texture XY Width Resolution, Please Adjust RTVividness Value"),WidthResolution);
            checkf(HeightResolution <= MaxTextureHeight, TEXT("%d Exceeded Max Texture XY Height Resolution, Please Adjust RTVividness Value"),HeightResolution);
            break;
        case ETextureNormalDirection::FRONT:
        case ETextureNormalDirection::BACK://xz평면 사용
            WidthResolution = static_cast<int32>(BoxMin2BoxMax.X * RTVividness * NumSliceXZ);
            HeightResolution = static_cast<int32>(BoxMin2BoxMax.Z * RTVividness);
            checkf(WidthResolution <= MaxTextureWidth, TEXT("%d Exceeded Max Texture XY Width Resolution, Please Adjust RTVividness Value"),WidthResolution);
            checkf(HeightResolution <= MaxTextureHeight, TEXT("%d Exceeded Max Texture XY Height Resolution, Please Adjust RTVividness Value"),HeightResolution);
            break;
        case ETextureNormalDirection::LEFT:
        case ETextureNormalDirection::RIGHT://yz평면 사용
            WidthResolution = static_cast<int32>(BoxMin2BoxMax.Y * RTVividness * NumSliceYZ);
            HeightResolution = static_cast<int32>(BoxMin2BoxMax.Z * RTVividness);
            checkf(WidthResolution <= MaxTextureWidth, TEXT("%d Exceeded Max Texture XY Width Resolution, Please Adjust RTVividness Value"),WidthResolution);
            checkf(HeightResolution <= MaxTextureHeight, TEXT("%d Exceeded Max Texture XY Height Resolution, Please Adjust RTVividness Value"),HeightResolution);
            break;
        }
        
        PaintingRenderTargets[i] = UKismetRenderingLibrary::CreateRenderTarget2D(
            this, WidthResolution, HeightResolution,PaintingRenderTargetFormat,ClearColor
        );

        ensure(PaintingRenderTargets[i]);
        //Material Instance 텍스쳐 파라미터 지정
        DynamicMaterial->SetTextureParameterValue(FName(TextureNames[i]), PaintingRenderTargets[i]);
    }

    DynamicMaterial->SetScalarParameterValue(FName(TEXT("NumSliceXY")), NumSliceXY);
    DynamicMaterial->SetScalarParameterValue(FName(TEXT("NumSliceYZ")), NumSliceYZ);
    DynamicMaterial->SetScalarParameterValue(FName(TEXT("NumSliceXZ")), NumSliceXZ);

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
	{//Painting Volume안에 있는 StaticMesh들을 색칠할 수 있도록 설정합니다
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

#if WITH_EDITOR
    if (bDebugDrawSliceArea)
        DebugDrawSlicedPaintingArea();
#endif
}


void APaintingVolume::Paint(FVector Location,FVector PointNormal,float PaintSize,FLinearColor PaintColor,UTexture* PaintTexture)
{
    //PointNormal에 따라 Texture의 방향을 결정합니다
    ETextureNormalDirection TextureDirection =  GetTextureNormalDirection(PointNormal);
    
    
    UCanvas* Canvas;
	FVector2D Size;//렌더타겟 크기(X해상도 * Y해상도)
	FDrawToRenderTargetContext Context;
	UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(
        this,
        PaintingRenderTargets[static_cast<int8>(TextureDirection)],//
        Canvas,
        Size,
        Context
    );
    check(PaintTexture);
    FVector2D AdjustedSize = FVector2D(PaintSize * RTVividness, PaintSize * RTVividness);

    int32 CurrentSlice = GetCurrentSlice(Location, TextureDirection);//Slice번호[0,NumSlice-1]
    FVector2D UV = GetUV(Location, TextureDirection);//Slice내부 UV좌표
    FVector2D ScreenPos;
    switch (TextureDirection)
    {
    case ETextureNormalDirection::UP:
    case ETextureNormalDirection::DOWN://xy평면
        ScreenPos = UV * FVector2D(Size.X / NumSliceXY, Size.Y) + FVector2D(Size.X / NumSliceXY * CurrentSlice,0);
        break;
    case ETextureNormalDirection::FRONT:
    case ETextureNormalDirection::BACK://yz평면
        ScreenPos = UV * FVector2D(Size.X / NumSliceYZ, Size.Y) + FVector2D(Size.X / NumSliceYZ * CurrentSlice,0);
        break;
    case ETextureNormalDirection::LEFT:
    case ETextureNormalDirection::RIGHT://xz평면
        ScreenPos = UV * FVector2D(Size.X / NumSliceXZ, Size.Y) + FVector2D(Size.X / NumSliceXZ * CurrentSlice,0);
        break;
    }
    Canvas->K2_DrawTexture(
        PaintTexture, ScreenPos - AdjustedSize/2,
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
		return FVector2D(
            (Location.X - BoxMin.X)/ BoxMin2BoxMax.X ,
			(Location.Y - BoxMin.Y)/ BoxMin2BoxMax.Y
        );
    case ETextureNormalDirection::FRONT:
    case ETextureNormalDirection::BACK://yz평면
        return FVector2D(
            (Location.Y - BoxMin.Y) / BoxMin2BoxMax.Y,
            (Location.Z - BoxMin.Z) / BoxMin2BoxMax.Z
        );
    case ETextureNormalDirection::LEFT:
    case ETextureNormalDirection::RIGHT://xz평면
        return FVector2D(
            (Location.X - BoxMin.X) / BoxMin2BoxMax.X,
            (Location.Z - BoxMin.Z) / BoxMin2BoxMax.Z
        );
    }
    ensure(false);//어느 경우에도 속하지 못한경우 에러메시지를 출력합니다
    return FVector2D::ZeroVector;
}

int32 APaintingVolume::GetCurrentSlice(FVector Location, ETextureNormalDirection TextureDirection) const
{
    //Location에 맞는 Slice번호를 반환합니다[0,NumSlice-1]
    FVector BoxCenter = VolumeBox->GetComponentLocation();
    FVector BoxExtent = VolumeBox->GetScaledBoxExtent();

    FVector BoxMin = BoxCenter - BoxExtent;
    FVector BoxMax = BoxCenter + BoxExtent;
    FVector BoxMin2BoxMax = BoxMax - BoxMin;

    float LenSliceXY = BoxMin2BoxMax.Z / NumSliceXY;//xy평면의 Slice를 결정하는 단위 길이
    float LenSliceYZ = BoxMin2BoxMax.X / NumSliceYZ;//yz평면의 Slice를 결정하는 단위 길이
    float LenSliceXZ = BoxMin2BoxMax.Y / NumSliceXZ;//xz평면의 Slice를 결정하는 단위 길이
    switch (TextureDirection)
    {
    case ETextureNormalDirection::UP:
    case ETextureNormalDirection::DOWN://xy평면(Z값에 따라 Slice번호가 달라집니다)
		return static_cast<int32>((Location.Z - BoxMin.Z) / LenSliceXY);
    case ETextureNormalDirection::FRONT:
    case ETextureNormalDirection::BACK://yz평면(X값에 따라 Slice번호가 달라집니다)
        return static_cast<int32>((Location.X - BoxMin.X) / LenSliceYZ);
    case ETextureNormalDirection::LEFT:
    case ETextureNormalDirection::RIGHT://xz평면(Y값에 따라 Slice번호가 달라집니다)
        return static_cast<int32>((Location.Y - BoxMin.Y) / LenSliceXZ);
    }
    ensure(false);
    return 0;
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

    ensure(false);//어느 경우에도 속하지 못한경우 에러메시지를 출력합니다
    return ETextureNormalDirection::MAX;
}

void APaintingVolume::DebugDrawSlicedPaintingArea()
{//디버깅용, 나뉘어진 Painting영역을 시각화합니다
    FVector BoxExtent = VolumeBox->GetScaledBoxExtent();
    FVector BoxCenter = VolumeBox->GetComponentLocation();


    FVector BoxMin = BoxCenter - BoxExtent;
    FVector BoxMax = BoxCenter + BoxExtent;
    FVector BoxMin2BoxMax = BoxMax - BoxMin;


    FVector CellSize(BoxMin2BoxMax.X / NumSliceYZ, BoxMin2BoxMax.Y / NumSliceXZ, BoxMin2BoxMax.Z / NumSliceXY);

    for (int32 i = 0; i <= NumSliceYZ; i++)
    {
        for (int32 j = 0; j <= NumSliceXZ; j++)
        {
            for (int32 k = 0; k <= NumSliceXY; k++)
            {
                FVector Start = BoxMin + FVector(i * CellSize.X, j * CellSize.Y, k * CellSize.Z);
                FVector EndX = Start + FVector(CellSize.X, 0, 0);
                FVector EndY = Start + FVector(0, CellSize.Y, 0);
                FVector EndZ = Start + FVector(0, 0, CellSize.Z);

                DrawDebugLine(GetWorld(), Start, EndX, FColor::Green, false, -1.0f, 0, 1.0f);
                DrawDebugLine(GetWorld(), Start, EndY, FColor::Green, false, -1.0f, 0, 1.0f);
                DrawDebugLine(GetWorld(), Start, EndZ, FColor::Green, false, -1.0f, 0, 1.0f);
            }
        }
    }

}

