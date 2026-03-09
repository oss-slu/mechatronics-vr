// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LessonStep.h"
#include "QuizLessonStep.generated.h"


// Data Structures for creating quizzes
USTRUCT(BlueprintType)
struct FQuizChoice
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quiz")
	FString ChoiceText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quiz")
	bool bIsCorrect = false;
};

USTRUCT(BlueprintType)
struct FQuizQuestion
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quiz")
	FString QuestionText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quiz")
	TArray<FQuizChoice> Choices;
	
};

USTRUCT(BlueprintType)
struct FQuizQuestionResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Quiz")
	FString QuestionText;

	UPROPERTY(BlueprintReadOnly, Category = "Quiz")
	FString SelectedChoiceText;

	UPROPERTY(BlueprintReadOnly, Category = "Quiz")
	FString CorrectChoiceText;

	UPROPERTY(BlueprintReadOnly, Category = "Quiz")
	bool bAnsweredCorrectly = false;
};

UENUM(BlueprintType)
enum class EQuizState : uint8
{
	WaitingToStart  UMETA(DisplayName = "Waiting To Start"),
	ShowingQuestion UMETA(DisplayName = "Showing Question"),
	ShowingResults  UMETA(DisplayName = "Showing Results"),
	Completed       UMETA(DisplayName = "Completed")
};


UCLASS(Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced)
class MECHATRONICSVR_API UQuizLessonStep : public ULessonStep
{
	GENERATED_BODY()

public:
	UQuizLessonStep();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quiz|Content")
	TArray<FQuizQuestion> Questions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quiz|Rules")
	bool bRequirePassingScore = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quiz|Rules")
	float PassingScorePercent = 70.f;

	UPROPERTY(BlueprintReadOnly, Category = "Quiz|State")
	EQuizState QuizState = EQuizState::WaitingToStart;

	UPROPERTY(BlueprintReadOnly, Category = "Quiz|State")
	int32 CurrentQuestionIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Quiz|State")
	TArray<FQuizQuestionResult> QuestionResults;
	
	UPROPERTY(BlueprintReadOnly, Category = "Quiz|State")
	int32 Score = 0;


	// Blueprint functions

	UFUNCTION(BlueprintCallable, Category = "Quiz")
	void SubmitAnswer(int32 ChoiceIndex);

	UFUNCTION(BlueprintPure, Category = "Quiz")
	FQuizQuestion GetCurrentQuestion() const;

	UFUNCTION(BlueprintPure, Category = "Quiz")
	int32 GetTotalQuestions() const { return Questions.Num(); }

	UFUNCTION(BlueprintPure, Category = "Quiz")
	FString GetProgressText() const;

	UFUNCTION(BlueprintPure, Category = "Quiz")
	FString GetScoreText() const;

	UFUNCTION(BlueprintNativeEvent, Category = "Lesson|Step")
	bool CheckCompletion() const;
	virtual bool CheckCompletion_Implementation() const override;

protected:
	virtual void OnStarted() override;
	virtual void OnStopped() override;
	virtual void OnReset() override;

private:
	void EvaluateAnswerSubmission(int32 ChoiceIndex);
	void FinishQuiz();
};
