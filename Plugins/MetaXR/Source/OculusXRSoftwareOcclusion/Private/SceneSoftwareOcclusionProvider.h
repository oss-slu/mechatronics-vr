// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#ifdef WITH_OCULUS_BRANCH

#include "ICustomOcclusionProvider.h"
#include "SceneSoftwareOcclusion.h"

class FViewInfo;
class FSceneSoftwareOcclusionProvider : public ICustomOcclusionProvider
{
	// Required for creating a TSharedPtr because this class has a private constructor.
	template <typename, ESPMode>
	friend class SharedPointerInternals::TIntrusiveReferenceController;

public:
	static TSharedPtr<FSceneSoftwareOcclusionProvider> Get();

	virtual void SetupCustomOcclusion(FViewInfo& View) override;

private:
	FSceneSoftwareOcclusionProvider();

	static TSharedPtr<FSceneSoftwareOcclusionProvider> Instance;
};
#endif // WITH_OCULUS_BRANCH
