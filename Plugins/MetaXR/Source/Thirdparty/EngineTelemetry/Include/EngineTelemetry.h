// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#pragma once

#ifndef ENGINE_TELEMETRY_API
#ifdef _WIN32
#define ENGINE_TELEMETRY_API __declspec(dllexport)
#elif defined(__ANDROID__) || defined(__APPLE__)
#define ENGINE_TELEMETRY_API __attribute__((visibility("default")))
#else
#define ENGINE_TELEMETRY_API
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum {
  telemetryBool_False = 0,
  telemetryBool_True = 1,
};
typedef int telemetryBool;

enum {
  optionalTelemetryBool_False = 0,
  optionalTelemetryBool_True = 1,
  optionalTelemetryBool_NotSet = 2,
};
typedef int optionalTelemetryBool;

typedef short telemetryInt16;
typedef long long telemetryInt64;

typedef enum {
  telemetryQplVariantType_None = 0,
  telemetryQplVariantType_String = 1,
  telemetryQplVariantType_Int = 2,
  telemetryQplVariantType_Double = 3,
  telemetryQplVariantType_Bool = 4,
  telemetryQplVariantType_StringArray = 5,
  telemetryQplVariantType_IntArray = 6,
  telemetryQplVariantType_DoubleArray = 7,
  telemetryQplVariantType_BoolArray = 8,
  telemetryQplVariantType_Max = 0x7fffffff,
} telemetryQplVariantType;

typedef struct telemetryQplVariant_ {
  telemetryQplVariantType Type;
  int ValueCount;
  union {
    const char* StringValue;
    telemetryInt64 IntValue;
    double DoubleValue;
    telemetryBool BoolValue;
    const char** StringValues;
    telemetryInt64* IntValues;
    double* DoubleValues;
    telemetryBool* BoolValues;
  };
} telemetryQplVariant;

typedef struct telemetryQplAnnotation_ {
  const char* Key;
  telemetryQplVariant Value;
} telemetryQplAnnotation;

typedef enum {
  telemetryOptionalBool_False = 0,
  telemetryOptionalBool_True = 1,
  telemetryOptionalBool_Unknown = 2
} telemetryOptionalBool;

ENGINE_TELEMETRY_API void engineTelemetry_Init();

ENGINE_TELEMETRY_API void engineTelemetry_ShutDown();

ENGINE_TELEMETRY_API telemetryBool
engineTelemetry_SetDeveloperTelemetryConsent(telemetryBool consent);

ENGINE_TELEMETRY_API telemetryBool
engineTelemetry_SendEvent(const char* eventName, const char* param, const char* source);

ENGINE_TELEMETRY_API telemetryBool engineTelemetry_SendUnifiedEvent(
    telemetryBool isEssential,
    const char* productType,
    const char* eventName,
    const char* event_metadata_json,
    const char* project_name,
    const char* event_entrypoint,
    const char* project_guid,
    const char* event_type,
    const char* event_target,
    const char* error_msg,
    optionalTelemetryBool is_internal_build,
    optionalTelemetryBool batch_mode,
    unsigned long long machine_oculus_user_id,
    optionalTelemetryBool is_runtime,
    const char* event_status);

ENGINE_TELEMETRY_API telemetryBool
engineTelemetry_AddCustomMetadata(const char* metadataName, const char* metadataParam);

ENGINE_TELEMETRY_API telemetryBool engineTelemetry_OnEditorShutdown();

ENGINE_TELEMETRY_API telemetryBool
engineTelemetry_SaveUnifiedConsent(int toolId, telemetryBool consentValue);

ENGINE_TELEMETRY_API telemetryBool engineTelemetry_SaveUnifiedConsentWithOlderVersion(
    int toolId,
    telemetryBool consentValue,
    int consentVersion);

ENGINE_TELEMETRY_API telemetryOptionalBool engineTelemetry_GetUnifiedConsent(int toolId);

ENGINE_TELEMETRY_API telemetryBool engineTelemetry_ShouldShowTelemetryConsentWindow(int toolId);

ENGINE_TELEMETRY_API telemetryBool engineTelemetry_IsConsentSettingsChangeEnabled(int toolId);

ENGINE_TELEMETRY_API telemetryBool engineTelemetry_ShouldShowTelemetryNotification(int toolId);

ENGINE_TELEMETRY_API telemetryBool
engineTelemetry_GetConsentSettingsChangeText(char* consentSettingsChangeText);

ENGINE_TELEMETRY_API telemetryBool engineTelemetry_SetNotificationShown(int toolId);

ENGINE_TELEMETRY_API telemetryBool engineTelemetry_GetConsentTitle(char* consentTitle);

ENGINE_TELEMETRY_API telemetryBool
engineTelemetry_GetConsentMarkdownText(char* consentMarkdownText);

ENGINE_TELEMETRY_API telemetryBool engineTelemetry_GetConsentNotificationMarkdownText(
    const char* consentChangeLocationMarkdown,
    char* consentNotificationMarkdownText);

ENGINE_TELEMETRY_API telemetryBool
engineTelemetry_QplMarkerStart(int markerId, int instanceKey, telemetryInt64 timestampMs);

ENGINE_TELEMETRY_API telemetryBool engineTelemetry_QplMarkerStartForJoin(
    int markerId,
    const char* joinId,
    telemetryBool cancelMarkerIfAppBackgrounded,
    int instanceKey,
    long long timestampMs);

ENGINE_TELEMETRY_API telemetryBool engineTelemetry_QplMarkerEnd(
    int markerId,
    telemetryInt16 actionId,
    int instanceKey,
    telemetryInt64 timestampMs);

ENGINE_TELEMETRY_API telemetryBool engineTelemetry_QplMarkerPoint(
    int markerId,
    const char* name,
    int instanceKey,
    telemetryInt64 timestampMs);

ENGINE_TELEMETRY_API telemetryBool engineTelemetry_QplMarkerPointCached(
    int markerId,
    int nameHandle,
    int instanceKey,
    telemetryInt64 timestampMs);

ENGINE_TELEMETRY_API telemetryBool engineTelemetry_QplMarkerPointData(
    int markerId,
    const char* name,
    const telemetryQplAnnotation* annotations,
    int annotationCount,
    int instanceKey,
    telemetryInt64 timestampMs);

ENGINE_TELEMETRY_API telemetryBool engineTelemetry_QplMarkerAnnotationVariant(
    int markerId,
    const char* annotationKey,
    const telemetryQplVariant* annotationValue,
    int instanceKey);

ENGINE_TELEMETRY_API telemetryBool engineTelemetry_QplMarkerAnnotation(
    int markerId,
    const char* annotationKey,
    const char* annotationValue,
    int instanceKey);

ENGINE_TELEMETRY_API telemetryBool
engineTelemetry_QplCreateMarkerHandle(const char* name, int* nameHandle);

ENGINE_TELEMETRY_API telemetryBool engineTelemetry_QplDestroyMarkerHandle(int nameHandle);

ENGINE_TELEMETRY_API telemetryBool engineTelemetry_QplSetConsent(telemetryBool qplConsent);

ENGINE_TELEMETRY_API telemetryBool engineTelemetry_GetMachineID(char* machineId);

#ifdef __cplusplus
}

#include <optional>
#include <string>

namespace EngineTelemetry {

int Telemetry_CreateMetadataHandle();
bool Telemetry_SetMetadata(const char* key, const char* value, int handle);
bool Telemetry_SetMetadataInt(const char* key, int value, int handle);
bool Telemetry_SetMetadataLong(const char* key, long long value, int handle);
bool Telemetry_SetMetadataFloat(const char* key, float value, int handle);
bool Telemetry_SetMetadataDouble(const char* key, double value, int handle);
bool Telemetry_SetMetadataBool(const char* key, bool value, int handle);
bool Telemetry_SetMetadataIntArray(const char* key, const int* values, int count, int handle);
bool Telemetry_SetMetadataLongArray(
    const char* key,
    const long long* values,
    int count,
    int handle);
bool Telemetry_SetMetadataDoubleArray(const char* key, const double* values, int count, int handle);
bool Telemetry_SetMetadataStringArray(const char* key, const char** values, int count, int handle);
std::optional<std::string> Telemetry_GetMetadata(int handle);
bool Telemetry_ClearMetadata(int handle);

} // namespace EngineTelemetry

#endif
