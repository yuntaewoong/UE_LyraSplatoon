// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PaintableComponent.generated.h"


UCLASS(Abstract,Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LYRASPLATOONRUNTIME_API UPaintableComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPaintableComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void PaintCircle(FVector2D UV,int Radius);

private:
	UPROPERTY(EditAnywhere)
	int32 Width = 100;

	UPROPERTY(EditAnywhere)
	int32 Height = 100;

	UPROPERTY(EditAnywhere)
	TObjectPtr<class UMaterialInterface> PaintedMaterial;//기존 Material을 대체할 Material클래스

	UPROPERTY()
	TObjectPtr<class UTextureRenderTarget2D> PaintingRenderTarget;//색칠 데이터가 저장될 텍스처
		
};
