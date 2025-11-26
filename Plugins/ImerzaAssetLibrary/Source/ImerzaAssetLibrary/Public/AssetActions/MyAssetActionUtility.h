// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AssetActionUtility.h"
#include "DebugHeader.h"
#include "MyAssetActionUtility.generated.h"


/**
 * 
 */
UCLASS()
class IMERZAASSETLIBRARY_API UMyAssetActionUtility : public UAssetActionUtility
{
	GENERATED_BODY()

public:
	UFUNCTION(CallInEditor)
	void TestFunc();
	
};
