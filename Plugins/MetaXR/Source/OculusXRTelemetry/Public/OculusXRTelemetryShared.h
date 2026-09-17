// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "OculusXRQPL.h"

namespace OculusXRTelemetry
{
#ifndef TURN_OFF_META_TELEMETRY
	using FTelemetryBackend = FQPLBackend;
#else
	using FTelemetryBackend = FEmptyBackend;
#endif

	constexpr int UNREAL_TOOL_ID = 2;
	constexpr int CONSENT_TITLE_MAX_LENGTH = 256;
	constexpr int CONSENT_TEXT_MAX_LENGTH = 2048;
	constexpr int CONSENT_NOTIFICATION_MAX_LENGTH = 1024;

	namespace
	{
		const char* TelemetrySource = "UE5Integration";
	}
} // namespace OculusXRTelemetry
