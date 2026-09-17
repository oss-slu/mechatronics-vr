// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "OculusXRQPL.h"
#include "OculusXRTelemetry.h"
#include "OculusXRTelemetryWrapper.h"

namespace OculusXRTelemetry
{
	namespace QPL
	{
		bool MarkerStart(const int MarkerId, const FTelemetryInstanceKey InstanceKey, const FTelemetryTimestamp Timestamp)
		{
			if (!OculusXRTelemetry::IsActive())
			{
				return false;
			}

			const auto result = OculusXR::EngineTelemetryPlugin::GetPlugin().QplMarkerStart(
				MarkerId,
				InstanceKey.GetValue(),
				Timestamp.GetTimestamp());

			return result == telemetryBool_True;
		}
		bool MarkerEnd(const int MarkerId, const EAction Action, const FTelemetryInstanceKey InstanceKey, const FTelemetryTimestamp Timestamp)
		{
			if (!OculusXRTelemetry::IsActive())
			{
				return false;
			}

			const auto result = OculusXR::EngineTelemetryPlugin::GetPlugin().QplMarkerEnd(
				MarkerId,
				static_cast<short>(Action),
				InstanceKey.GetValue(),
				Timestamp.GetTimestamp());

			return result == telemetryBool_True;
		}

		bool MarkerPoint(const int MarkerId, const char* Name, const FTelemetryInstanceKey InstanceKey, const FTelemetryTimestamp Timestamp)
		{
			if (!OculusXRTelemetry::IsActive())
			{
				return false;
			}

			const auto result = OculusXR::EngineTelemetryPlugin::GetPlugin().QplMarkerPoint(
				MarkerId,
				Name,
				InstanceKey.GetValue(),
				Timestamp.GetTimestamp());

			return result == telemetryBool_True;
		}

		bool MarkerPointCached(const int MarkerId, const int NameHandle, const FTelemetryInstanceKey InstanceKey, const FTelemetryTimestamp Timestamp)
		{
			if (!OculusXRTelemetry::IsActive())
			{
				return false;
			}

			const auto result = OculusXR::EngineTelemetryPlugin::GetPlugin().QplMarkerPointCached(
				MarkerId,
				NameHandle,
				InstanceKey.GetValue(),
				Timestamp.GetTimestamp());

			return result == telemetryBool_True;
		}

		bool MarkerAnnotation(const int MarkerId, const char* AnnotationKey, const char* AnnotationValue, const FTelemetryInstanceKey InstanceKey)
		{
			if (!OculusXRTelemetry::IsActive())
			{
				return false;
			}

			if (nullptr == AnnotationValue)
			{
				return false;
			}
			const auto result = OculusXR::EngineTelemetryPlugin::GetPlugin().QplMarkerAnnotation(
				MarkerId,
				AnnotationKey,
				AnnotationValue,
				InstanceKey.GetValue());

			return result == telemetryBool_True;
		}

		bool CreateMarkerHandle(const char* Name, int* NameHandle)
		{
			if (!OculusXRTelemetry::IsActive())
			{
				return false;
			}

			if (nullptr == NameHandle)
			{
				return false;
			}

			const auto result = OculusXR::EngineTelemetryPlugin::GetPlugin().QplCreateMarkerHandle(
				Name,
				NameHandle);

			return result == telemetryBool_True;
		}

		bool DestroyMarkerHandle(const int NameHandle)
		{
			if (!OculusXRTelemetry::IsActive())
			{
				return false;
			}

			const auto result = OculusXR::EngineTelemetryPlugin::GetPlugin().QplDestroyMarkerHandle(
				NameHandle);

			return result == telemetryBool_True;
		}

		bool OnEditorShutdown()
		{
			if (!OculusXRTelemetry::IsActive())
			{
				return false;
			}

			const auto result = OculusXR::EngineTelemetryPlugin::GetPlugin().OnEditorShutdown();

			return result == telemetryBool_True;
		}

	} // namespace QPL
	bool FQPLBackend::MarkerStart(const int MarkerId, const FTelemetryInstanceKey InstanceKey, const FTelemetryTimestamp Timestamp)
	{
		return QPL::MarkerStart(MarkerId, InstanceKey, Timestamp);
	}
	bool FQPLBackend::MarkerEnd(const int MarkerId, const EAction Action, const FTelemetryInstanceKey InstanceKey, const FTelemetryTimestamp Timestamp)
	{
		return QPL::MarkerEnd(MarkerId, Action, InstanceKey, Timestamp);
	};
	bool FQPLBackend::MarkerPoint(const int MarkerId, const char* Name, const FTelemetryInstanceKey InstanceKey, const FTelemetryTimestamp Timestamp)
	{
		return QPL::MarkerPoint(MarkerId, Name, InstanceKey, Timestamp);
	};
	bool FQPLBackend::MarkerPointCached(const int MarkerId, const int NameHandle, const FTelemetryInstanceKey InstanceKey, const FTelemetryTimestamp Timestamp)
	{
		return QPL::MarkerPointCached(MarkerId, NameHandle, InstanceKey, Timestamp);
	};
	bool FQPLBackend::MarkerAnnotation(const int MarkerId, const char* AnnotationKey, const char* AnnotationValue, const FTelemetryInstanceKey InstanceKey)
	{
		return QPL::MarkerAnnotation(MarkerId, AnnotationKey, AnnotationValue, InstanceKey);
	};
	bool FQPLBackend::CreateMarkerHandle(const char* Name, int* NameHandle)
	{
		return QPL::CreateMarkerHandle(Name, NameHandle);
	};
	bool FQPLBackend::DestroyMarkerHandle(const int NameHandle)
	{
		return QPL::DestroyMarkerHandle(NameHandle);
	};
	bool FQPLBackend::OnEditorShutdown()
	{
		return QPL::OnEditorShutdown();
	};
} // namespace OculusXRTelemetry
