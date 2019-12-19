// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: 

#include "UIAIAvatarModule.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogAvatarROS);
DEFINE_LOG_CATEGORY(LogAvatarCharacter);

#define LOCTEXT_NAMESPACE "FUIAIAvatarModule"

void FUIAIAvatarModule::StartupModule()
{
	// Empty as of right now
}

void FUIAIAvatarModule::ShutdownModule()
{
	// Empty as of right now
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FUIAIAvatarModule, UIAIAvatar)