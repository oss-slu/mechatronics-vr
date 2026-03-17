// Fill out your copyright notice in the Description page of Project Settings.


#include "QuizLessonStep.h"

UQuizLessonStep::UQuizLessonStep() {}



void UQuizLessonStep::OnStarted() {}
void UQuizLessonStep::OnStopped() {}
void UQuizLessonStep::OnReset() {}


void UQuizLessonStep::SubmitAnswer(int32 ChoiceIndex) {}

FQuizQuestion UQuizLessonStep::GetCurrentQuestion() {return FQuizQuestion();}
FString UQuizLessonStep::GetProgressText() {return FString();}
FString UQuizLessonStep::GetScoreText() { return FString{};}
void UQuizLessonStep::EvaluateAnswerSubmission(int32 ChoiceIndex) {}
void UQuizLessonStep::FinishQuiz() {}

bool UQuizLessonStep::CheckCompletion_Implementation() const { return false; }


