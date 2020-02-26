#pragma once

#include "ROSBridgeMsg.h"

#include "QnAActionGoal.h"
#include "QnAActionResult.h"
#include "QnAActionFeedback.h"

namespace iai_avatar_msgs
{
	class QnAAction : public FROSBridgeMsg
	{
		QnAActionGoal ActionGoal;
		QnAActionResult ActionResult;
		QnAActionFeedback ActionFeedback;
	public:
		QnAAction()
		{
			MsgType = "iai_avatar_msgs/QnAAction";
		}

		QnAAction
		(
			QnAActionGoal InActionGoal,
			QnAActionResult InActionResult,
			QnAActionFeedback InActionFeedback
		):
			ActionGoal(InActionGoal),
			ActionResult(InActionResult),
			ActionFeedback(InActionFeedback)
		{
			MsgType = "iai_avatar_msgs/QnAAction";
		}

		~QnAAction() override {}

		QnAActionGoal GetActionGoal() const
		}
			return ActionGoal;
		}

		QnAActionResult GetActionResult() const
		}
			return ActionResult;
		}

		QnAActionFeedback GetActionFeedback() const
		}
			return ActionFeedback;
		}

		void SetActionGoal(QnAActionGoal InActionGoal)
		}
			ActionGoal = InActionGoal;
		}

		void SetActionResult(QnAActionResult InActionResult)
		}
			ActionResult = InActionResult;
		}

		void SetActionFeedback(QnAActionFeedback InActionFeedback)
		}
			ActionFeedback = InActionFeedback;
		}

		virtual void FromJson(TSharedPtr<FJsonObject> JsonObject) override
		{
			ActionGoal = QnAActionGoal::GetFromJson(JsonObject->GetObjectField(TEXT("action_goal")));

			ActionResult = QnAActionResult::GetFromJson(JsonObject->GetObjectField(TEXT("action_result")));

			ActionFeedback = QnAActionFeedback::GetFromJson(JsonObject->GetObjectField(TEXT("action_feedback")));

		}

		static QnAAction GetFromJson(TSharedPtr<FJsonObject> JsonObject)
		{
			QnAAction Result;
			Result.FromJson(JsonObject);
			return Result;
		}

		virtual TSharedPtr<FJsonObject> ToJsonObject() const override
		{
			TSharedPtr<FJsonObject> Object = MakeShareable<FJsonObject>(new FJsonObject());

			Object->SetObjectField(TEXT("action_goal"), ActionGoal.ToJsonObject());
			Object->SetObjectField(TEXT("action_result"), ActionResult.ToJsonObject());
			Object->SetObjectField(TEXT("action_feedback"), ActionFeedback.ToJsonObject());
			return Object;
		}
		virtual FString ToYamlString() const override
		{
			FString OutputString;
			TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&OutputString);
			FJsonSerializer::Serialize(ToJsonObject().ToSharedRef(), Writer);
			return OutputString;
		}
	};
}