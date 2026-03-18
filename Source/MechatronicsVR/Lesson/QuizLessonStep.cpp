// Fill out your copyright notice in the Description page of Project Settings.


#include "QuizLessonStep.h"
#include "Engine/Engine.h"

UQuizLessonStep::UQuizLessonStep()
{
	bWantsTickWhileActive = false;

	bRequirePassingScore = false;
	PassingScorePercent = 70.0f;
	CurrentQuestionIndex = 0;
	Score = 0;
	QuizState = EQuizState::WaitingToStart;

	StepType = ELessonStepType::Quiz;
}



void UQuizLessonStep::OnStarted()
{
	if (Questions.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Quiz step does not have any questions!"));
		return;
	}
	// Resetting just in case the step needs to be restarted
	CurrentQuestionIndex = 0;
	Score = 0;
	QuizState = EQuizState::ShowingQuestion;
	UE_LOG(LogTemp, Log, TEXT("Quiz has started"));
}
void UQuizLessonStep::OnStopped()
{
	return;
}
void UQuizLessonStep::OnReset()
{
	CurrentQuestionIndex = 0;
	Score = 0;
	QuizState = EQuizState::WaitingToStart;
	QuestionResults.Empty();
}


void UQuizLessonStep::SubmitAnswer(int32 ChoiceIndex)
{
	if (!bIsActive || QuizState != EQuizState::ShowingQuestion)
	{
		UE_LOG(LogTemp, Warning, TEXT("Answer is not ready to be submitted"));
		return;
	}
	if (!Questions.IsValidIndex(ChoiceIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("Questions is not valid"));
	}
	EvaluateAnswerSubmission(ChoiceIndex);
}

void UQuizLessonStep::EvaluateAnswerSubmission(int32 ChoiceIndex)
{
	const FQuizQuestion& CurrentQuestion  = Questions[CurrentQuestionIndex];

	if (!CurrentQuestion.Choices.IsValidIndex(ChoiceIndex))
	{
		UE_LOG (LogTemp, Warning, TEXT("Questions is not valid"));
		return;
	}

	FString CorrectChoiceText;

	for (const FQuizChoice& Choice : CurrentQuestion.Choices)
	{
		if (Choice.bIsCorrect)
		{
			CorrectChoiceText = Choice.ChoiceText;
			break;
		}
	}

	FQuizQuestionResult Result;
	Result.QuestionText = CorrectChoiceText;
	Result.SelectedChoiceText =	CurrentQuestion.Choices[ChoiceIndex].ChoiceText;
	Result.CorrectChoiceText = CorrectChoiceText;
	Result.bAnsweredCorrectly = CurrentQuestion.Choices[ChoiceIndex].bIsCorrect;

	if (Result.bAnsweredCorrectly)
	{
		++Score;
	}

	const bool BLastQuestion = CurrentQuestionIndex >= Questions.Num() - 1;
	if (BLastQuestion)
	{
		FinishQuiz();
	}
	else
	{
		++CurrentQuestionIndex;
		NotifyUpdated();
	}
}
void UQuizLessonStep::FinishQuiz()
{
	QuizState = EQuizState::ShowingResults;

	NotifyUpdated();
	if (CheckCompletion_Implementation())
	{
		CompleteStep();
	}
}

FQuizQuestion UQuizLessonStep::GetCurrentQuestion()
{
	if (Questions.IsValidIndex(CurrentQuestionIndex))
	{
		return Questions[CurrentQuestionIndex];
	}
	UE_LOG(LogTemp, Warning, TEXT("Questions is not valid"));

	return FQuizQuestion{};
}
FString UQuizLessonStep::GetProgressText()
{
	return FString::Printf(TEXT("Question %d of %d"), CurrentQuestionIndex + 1, Questions.Num());
}
FString UQuizLessonStep::GetScoreText()
{
	return FString::Printf(TEXT("%d / %d Correct"), Score, Questions.Num());
}


bool UQuizLessonStep::CheckCompletion_Implementation() const
{
	if (QuizState != EQuizState::ShowingResults && QuizState != EQuizState::ShowingQuestion)
	{
		return false;
	}

	// Maybe add check to see if the person did not get a passing score?
	
	return true;
}


