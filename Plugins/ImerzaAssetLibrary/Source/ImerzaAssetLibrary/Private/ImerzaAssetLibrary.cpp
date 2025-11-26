// Copyright Epic Games, Inc. All Rights Reserved.

#include "ImerzaAssetLibrary.h"
#include "ToolMenus.h"
#include "Widgets/SWindow.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/Application/SlateApplication.h"
#include "WebBrowserModule.h"
#include "IWebBrowserSingleton.h"
#include "IWebBrowserWindow.h"
#include "SWebBrowser.h"
#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"

#define LOCTEXT_NAMESPACE "FImerzaAssetLibraryModule"

// Tab identifier
static const FName ImerzaAssetLibraryTabName("ImerzaAssetLibraryTab");

// Path to the toolkit directory and Python executable
static const FString ToolkitDirectory = TEXT("G:/dev/Imerza/ImerzaLibraryToolkit");
static const FString PythonExePath = TEXT("G:/dev/Imerza/ImerzaLibraryToolkit/.venv/Scripts/python.exe");
static const FString ServerScript = TEXT("run_web_gui.py");
static const FString ServerArgs = TEXT("--no-browser");

void FImerzaAssetLibraryModule::StartupModule()
{
	// Start the web server
	StartWebServer();

	// Register the tab spawner
	RegisterTabSpawner();

	// Register the menu extension after ToolMenus is initialized
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FImerzaAssetLibraryModule::RegisterMenuExtension));
}

void FImerzaAssetLibraryModule::ShutdownModule()
{
	// Unregister the startup callback
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	// Unregister the tab spawner
	UnregisterTabSpawner();

	// Stop the web server
	StopWebServer();
}

void FImerzaAssetLibraryModule::RegisterTabSpawner()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(ImerzaAssetLibraryTabName, FOnSpawnTab::CreateRaw(this, &FImerzaAssetLibraryModule::SpawnTab))
		.SetDisplayName(LOCTEXT("ImerzaAssetLibraryTabTitle", "Imerza Asset Library"))
		.SetTooltipText(LOCTEXT("ImerzaAssetLibraryTabTooltip", "Open the Imerza Asset Library browser"));
}

void FImerzaAssetLibraryModule::UnregisterTabSpawner()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(ImerzaAssetLibraryTabName);
}

void FImerzaAssetLibraryModule::RegisterMenuExtension()
{
	// Register owner for cleanup
	FToolMenuOwnerScoped OwnerScoped(this);

	// Extend the Window menu
	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
	FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");

	Section.AddMenuEntry(
		"ImerzaAssetLibrary",
		LOCTEXT("ImerzaAssetLibraryMenuEntry", "Imerza Asset Library"),
		LOCTEXT("ImerzaAssetLibraryMenuEntryTooltip", "Open the Imerza Asset Library browser window"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([]()
		{
			FGlobalTabmanager::Get()->TryInvokeTab(ImerzaAssetLibraryTabName);
		}))
	);
}

TSharedRef<SDockTab> FImerzaAssetLibraryModule::SpawnTab(const FSpawnTabArgs& Args)
{
	// Ensure the web server is running before opening the window
	StartWebServer();

	// Ensure the WebBrowser module is loaded
	IWebBrowserModule::Get();

	// Create the web browser widget
	SAssignNew(WebBrowserWidget, SWebBrowser)
		.InitialURL(TEXT("http://localhost:8000/index.html"))
		.ShowControls(false)
		.ShowAddressBar(false)
		.ShowErrorMessage(true)
		.SupportsTransparency(false);

	// Create and return the dockable tab
	return SNew(SDockTab)
		.TabRole(NomadTab)
		.Label(LOCTEXT("ImerzaAssetLibraryTabLabel", "Imerza Asset Library"))
		[
			WebBrowserWidget.ToSharedRef()
		];
}

void FImerzaAssetLibraryModule::StartWebServer()
{
	// Check if Python executable exists
	if (!FPaths::FileExists(PythonExePath))
	{
		UE_LOG(LogTemp, Warning, TEXT("ImerzaAssetLibrary: Python executable not found at %s"), *PythonExePath);
		return;
	}

	// Check if server script exists
	FString FullScriptPath = FPaths::Combine(ToolkitDirectory, ServerScript);
	if (!FPaths::FileExists(FullScriptPath))
	{
		UE_LOG(LogTemp, Warning, TEXT("ImerzaAssetLibrary: Server script not found at %s"), *FullScriptPath);
		return;
	}

	// Check if we already have a running process
	if (ServerProcessHandle.IsValid() && FPlatformProcess::IsProcRunning(ServerProcessHandle))
	{
		UE_LOG(LogTemp, Log, TEXT("ImerzaAssetLibrary: Web server is already running"));
		return;
	}

	// Build the arguments: script path + arguments
	FString Args = FString::Printf(TEXT("\"%s\" %s"), *FullScriptPath, *ServerArgs);

	// Launch Python directly with the server script
	uint32 ProcessID = 0;
	ServerProcessHandle = FPlatformProcess::CreateProc(
		*PythonExePath,
		*Args,             // Params
		false,             // bLaunchDetached - false so we can track and terminate it
		true,              // bLaunchHidden - hide the console window
		true,              // bLaunchReallyHidden
		&ProcessID,        // OutProcessID
		0,                 // PriorityModifier
		*ToolkitDirectory, // OptionalWorkingDirectory
		nullptr            // PipeWriteChild
	);

	if (ServerProcessHandle.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("ImerzaAssetLibrary: Web server started successfully (PID: %u)"), ProcessID);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ImerzaAssetLibrary: Failed to start web server"));
	}
}

void FImerzaAssetLibraryModule::StopWebServer()
{
	if (ServerProcessHandle.IsValid())
	{
		if (FPlatformProcess::IsProcRunning(ServerProcessHandle))
		{
			FPlatformProcess::TerminateProc(ServerProcessHandle, true);
			UE_LOG(LogTemp, Log, TEXT("ImerzaAssetLibrary: Web server stopped"));
		}
		FPlatformProcess::CloseProc(ServerProcessHandle);
		ServerProcessHandle.Reset();
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FImerzaAssetLibraryModule, ImerzaAssetLibrary)
