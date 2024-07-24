// Fill out your copyright notice in the Description page of Project Settings.


#include "PaintableComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Canvas.h"
#include "Kismet/KismetRenderingLibrary.h"
UPaintableComponent::UPaintableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UPaintableComponent::BeginPlay()
{
	Super::BeginPlay();
	AActor* Owner = GetOwner();
	if (UStaticMeshComponent* StaticMeshComponent = Owner->GetComponentByClass<UStaticMeshComponent>())
	{
		//색칠정보를 기록할 RenderTarget을 동적생성합니다
		PaintingRenderTarget = UKismetRenderingLibrary::CreateRenderTarget2D(this, Width, Height, RTF_RGBA16f);
		UKismetRenderingLibrary::ClearRenderTarget2D(this, PaintingRenderTarget, FLinearColor::Black);

		//Mesh의 Material을 기본설정된 Material로 교체합니다
		//ToDo: 기존 material을 사용하면서 paintting할수 있게 구현
		UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(PaintedMaterial, this);
		DynamicMaterial->SetTextureParameterValue(TEXT("PaintedRenderTarget"), PaintingRenderTarget);
		StaticMeshComponent->SetMaterial(0, DynamicMaterial);
		
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Paintable Component Should have Static Mesh Component"));
	}
	
}


void UPaintableComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UPaintableComponent::PaintCircle(FVector2D UV, int Radius)
{//WorldPosition좌표를 중심으로 radius반지름만큼의 원을 RenderTarget에 그립니다

	UCanvas* Canvas;
	FVector2D Size;
	FDrawToRenderTargetContext Context;
	UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(this, PaintingRenderTarget, Canvas, Size, Context);
	
	Canvas->K2_DrawBox(UV*Size, FVector2D(10,10), 5.f);
	UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(this, Context);
}

