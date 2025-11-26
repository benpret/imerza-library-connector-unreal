// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetActions/MyAssetActionUtility.h"

void UMyAssetActionUtility::TestFunc()
{
	DebugHeader::Print(TEXT("Working"), FColor::Cyan);
	DebugHeader::PrintLog(TEXT("Working"));
}
