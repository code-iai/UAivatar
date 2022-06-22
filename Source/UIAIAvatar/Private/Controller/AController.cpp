// Copyright 2019, Institute for Artificial Intelligence - University of Bremen


#include "Controller/AController.h"


void UAController::CancelAction()
{
	if (bCancel)
	{
		if (bActive)
		{
			GoalStatusList.Last().Status = 6;
			bActive = false;
		}
		else
		{
			GoalStatusList.Last().Status = 2;
			bActive = false;
		}
		bPublishResult = true;
	}
}
