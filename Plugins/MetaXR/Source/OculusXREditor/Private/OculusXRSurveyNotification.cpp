// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "OculusXRSurveyNotification.h"

#include <regex>

#include "OculusXRHMDModule.h"
#include "OculusXRSurveySettings.h"
#include "Interfaces/IMainFrameModule.h"
#include "Widgets/Text/SRichTextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Images/SImage.h"
#include "Framework/Application/SlateApplication.h"

#define LOCTEXT_NAMESPACE "OculusXRSurveyNotification"

namespace OculusXRSurvey
{
	namespace
	{
		const FString SurveyURL = TEXT("https://www.meta.com/survey/?config_id=783964270991893");

		void OnBrowserLinkClicked(const FSlateHyperlinkRun::FMetadata& Metadata)
		{
			const FString* Url = Metadata.Find(TEXT("href"));
			if (Url)
			{
				FPlatformProcess::LaunchURL(**Url, nullptr, nullptr);
			}
		}

		void OnSurveyLinkClicked()
		{
			FPlatformProcess::LaunchURL(*SurveyURL, nullptr, nullptr);
		}

		std::string MarkdownToRTF(const std::string& markdown)
		{
			const std::regex boldText(R"(\*\*(.*?)\*\*)");
			const std::string boldReplacement = "<NormalText.Important>$1</>";

			std::string rtf = "";
			rtf = std::regex_replace(markdown, boldText, boldReplacement);

			const std::regex linkRegex(R"(\[(.*?)\]\((.*?)\))");
			const std::string linkReplacement = "<a id=\"browser\" style=\"Hyperlink\" href=\"$2\">$1</>";
			rtf = std::regex_replace(rtf, linkRegex, linkReplacement);
			return rtf;
		}

		FString GetSurveyTitle()
		{
			return TEXT("Please tell us what you think about the Meta XR Plugin!");
		}

		FString GetSurveyText()
		{
			return TEXT("Please take a moment to complete a short survey.");
		}

		class SOculusSurveyWindow : public SCompoundWidget
		{
			SLATE_BEGIN_ARGS(SOculusSurveyWindow) {}
			SLATE_ARGUMENT(std::string, SurveyText);
			SLATE_END_ARGS()

			void Construct(const FArguments& InArgs)
			{
				TSharedPtr<SVerticalBox> RootContainer = SNew(SVerticalBox);
				const TSharedPtr<SHorizontalBox> ButtonsWidget = SNew(SHorizontalBox);

				ButtonsWidget->AddSlot()
					[SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
							[SNew(SButton)
									.HAlign(HAlign_Center)
									.VAlign(VAlign_Center)
									.ContentPadding(5)
									.OnClicked(this, &SOculusSurveyWindow::OnNoThanksClicked)
									.Text(LOCTEXT("NoThanks", "No Thanks"))]];

				ButtonsWidget->AddSlot()
					[SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
							[SNew(SButton)
									.HAlign(HAlign_Center)
									.VAlign(VAlign_Center)
									.ContentPadding(5)
									.OnClicked(this, &SOculusSurveyWindow::OnTakeSurveyClicked)
									.ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("PrimaryButton"))
									.Text(LOCTEXT("TakeSurvey", "Take Survey"))]];

				const TSharedPtr<SVerticalBox> TextWidget = SNew(SVerticalBox);

				TextWidget->AddSlot()
					.AutoHeight()
						[SNew(SRichTextBlock)
								.AutoWrapText(true)
								.Text(FText::FromString(MarkdownToRTF(InArgs._SurveyText).data()))
								.DecoratorStyleSet(&FAppStyle::Get())
							+ SRichTextBlock::HyperlinkDecorator(TEXT("browser"), FSlateHyperlinkRun::FOnClick::CreateStatic(&OnBrowserLinkClicked))];

				RootContainer->AddSlot()
					.FillHeight(1.0f)
					.Padding(20, 20, 20, 10)
					.VAlign(VAlign_Center)
						[TextWidget.ToSharedRef()];

				RootContainer->AddSlot()
					.AutoHeight()
					.Padding(20, 10, 20, 20)
					.VAlign(VAlign_Bottom)
						[ButtonsWidget.ToSharedRef()];

				ChildSlot
					[SNew(SBox)
							.WidthOverride(640)
							.HeightOverride(150)
								[RootContainer.ToSharedRef()]];
			}

		private:
			FReply OnTakeSurveyClicked()
			{
				OnSurveyLinkClicked();
				FSlateApplication::Get().FindWidgetWindow(AsShared())->RequestDestroyWindow();
				return FReply::Handled();
			}

			FReply OnNoThanksClicked()
			{
				FSlateApplication::Get().FindWidgetWindow(AsShared())->RequestDestroyWindow();
				return FReply::Handled();
			}
		};

		void SpawnFullSurveyWindow()
		{
			if (FSlateApplication::Get().IsRenderingOffScreen())
			{
				return;
			}

			FString SurveyWindowTitle = GetSurveyTitle();
			FString SurveyText = GetSurveyText();

			IMainFrameModule::Get().OnMainFrameCreationFinished().AddLambda([SurveyWindowTitle, SurveyText](const TSharedPtr<SWindow>& RootWindow, bool /*bIsRunningStartupDialog*/) {
				const TSharedRef<SWindow> Window = SNew(SWindow)
													   .Title(FText::FromString(SurveyWindowTitle))
													   .SizingRule(ESizingRule::Autosized)
													   .SupportsMaximize(false)
													   .SupportsMinimize(false)[SNew(SOculusSurveyWindow).SurveyText(std::string(TCHAR_TO_ANSI(*SurveyText)))];

				FSlateApplication::Get().AddModalWindow(Window, RootWindow);
			});
		}
	} // namespace

	void MaybeSpawnSurveyRequest()
	{
		if (!FModuleManager::Get().IsModuleLoaded("MainFrame"))
		{
			return;
		}

		// Get the survey settings object
		UOculusXRSurveySettings* SurveySettings = GetMutableDefault<UOculusXRSurveySettings>();
		if (!SurveySettings)
		{
			return;
		}

		// Record this launch
		SurveySettings->RecordLaunch();

		// Check if we should show the survey
		if (SurveySettings->ShouldShowSurvey())
		{
			SpawnFullSurveyWindow();

			// Mark survey as shown
			SurveySettings->MarkSurveyShown();
		}
	}
} // namespace OculusXRSurvey

#undef LOCTEXT_NAMESPACE
