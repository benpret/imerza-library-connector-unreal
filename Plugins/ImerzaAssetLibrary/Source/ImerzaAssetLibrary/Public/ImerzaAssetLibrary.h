// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"
#include "Widgets/Docking/SDockTab.h"

class SWebBrowser;

class FImerzaAssetLibraryModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	/** Registers the tab spawner */
	void RegisterTabSpawner();

	/** Unregisters the tab spawner */
	void UnregisterTabSpawner();

	/** Registers the menu extension */
	void RegisterMenuExtension();

	/** Spawns the dockable tab */
	TSharedRef<SDockTab> SpawnTab(const FSpawnTabArgs& Args);

	/** Starts the web server if not already running */
	void StartWebServer();

	/** Stops the web server */
	void StopWebServer();

	/** Handle to the web browser widget */
	TSharedPtr<SWebBrowser> WebBrowserWidget;

	/** Handle to the server process */
	FProcHandle ServerProcessHandle;
};
