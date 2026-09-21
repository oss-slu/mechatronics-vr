// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "CoreMinimal.h"
#include "OculusXRAnchorComponent.h"
#include "OculusXRAnchorTypes.h"
#include "OculusXRAnchorsRequests.h"

/**
 * Delegate called when spatial anchor creation completes.
 * @param Result The result of the anchor creation operation
 * @param Anchor The created anchor component, or nullptr if creation failed
 */
DECLARE_DELEGATE_TwoParams(FOculusXRSpatialAnchorCreateDelegate, EOculusXRAnchorResult::Type /*Result*/, UOculusXRAnchorComponent* /*Anchor*/);

/**
 * Delegate called when anchor erase operation completes.
 * @param Result The result of the erase operation
 * @param AnchorUUID The UUID of the erased anchor
 */
DECLARE_DELEGATE_TwoParams(FOculusXRAnchorEraseDelegate, EOculusXRAnchorResult::Type /*Result*/, FOculusXRUUID /*AnchorUUID*/);

/**
 * Delegate called when anchor component status change completes.
 * @param Result The result of the status change operation
 * @param AnchorHandle The handle of the affected anchor
 * @param ComponentType The type of component that was modified
 * @param Enabled Whether the component is now enabled
 */
DECLARE_DELEGATE_FourParams(FOculusXRAnchorSetComponentStatusDelegate, EOculusXRAnchorResult::Type /*Result*/, uint64 /*AnchorHandle*/, EOculusXRSpaceComponentType /*ComponentType*/, bool /*Enabled*/);

/**
 * Delegate called when single anchor save operation completes.
 * @param Result The result of the save operation
 * @param Anchor The anchor that was saved
 */
DECLARE_DELEGATE_TwoParams(FOculusXRAnchorSaveDelegate, EOculusXRAnchorResult::Type /*Result*/, UOculusXRAnchorComponent* /*Anchor*/);

/**
 * Delegate called when anchor list save operation completes.
 * @param Result The result of the save operation
 * @param SavedAnchors Array of anchors that were successfully saved
 */
DECLARE_DELEGATE_TwoParams(FOculusXRAnchorSaveListDelegate, EOculusXRAnchorResult::Type /*Result*/, const TArray<UOculusXRAnchorComponent*>& /*SavedAnchors*/);

/**
 * Delegate called when anchor query operation completes.
 * @param Result The result of the query operation
 * @param Results Array of query results containing found anchors
 */
DECLARE_DELEGATE_TwoParams(FOculusXRAnchorQueryDelegate, EOculusXRAnchorResult::Type /*Result*/, const TArray<FOculusXRSpaceQueryResult>& /*Results*/);

/**
 * Delegate called when anchor sharing operation completes.
 * @param Result The result of the sharing operation
 * @param Anchors Array of anchors that were shared
 * @param Users Array of user IDs the anchors were shared with
 */
DECLARE_DELEGATE_ThreeParams(FOculusXRAnchorShareDelegate, EOculusXRAnchorResult::Type /*Result*/, const TArray<UOculusXRAnchorComponent*>& /*Anchors*/, const TArray<uint64>& /*Users*/);

/**
 * Delegate called when batch anchor save operation completes.
 * @param Result The result of the save operation
 * @param SavedAnchors Array of anchors that were successfully saved
 */
DECLARE_DELEGATE_TwoParams(FOculusXRSaveAnchorsDelegate, EOculusXRAnchorResult::Type /*Result*/, const TArray<UOculusXRAnchorComponent*>& /*SavedAnchors*/);

/**
 * Delegate called when batch anchor erase operation completes.
 * @param Result The result of the erase operation
 * @param ErasedAnchors Array of anchor components that were erased
 * @param ErasedAnchorsHandles Array of erased anchor handles as UInt64
 * @param ErasedAnchorsUUIDs Array of erased anchor UUIDs
 */
DECLARE_DELEGATE_FourParams(FOculusXREraseAnchorsDelegate, EOculusXRAnchorResult::Type /*Result*/, const TArray<UOculusXRAnchorComponent*>& /*ErasedAnchors*/, const TArray<FOculusXRUInt64>& /*ErasedAnchorsHandles*/, const TArray<FOculusXRUUID>& /*ErasedAnchorsUUIDs*/);

/**
 * Delegate called when anchor discovery finds results.
 * @param DiscoveredSpace Array of discovered anchor results
 */
DECLARE_DELEGATE_OneParam(FOculusXRDiscoverAnchorsResultsDelegate, const TArray<FOculusXRAnchorsDiscoverResult>& /*DiscoveredSpace*/);

/**
 * Delegate called when anchor discovery operation completes.
 * @param Result The result of the discovery operation
 */
DECLARE_DELEGATE_OneParam(FOculusXRDiscoverAnchorsCompleteDelegate, EOculusXRAnchorResult::Type /*Result*/);

/**
 * Delegate called when get shared anchors operation completes.
 * @param Result The result of the operation
 * @param Results Array of shared anchor discovery results
 */
DECLARE_DELEGATE_TwoParams(FOculusXRGetSharedAnchorsDelegate, EOculusXRAnchorResult::Type /*Result*/, const TArray<FOculusXRAnchorsDiscoverResult>& /*Results*/);

/**
 * Contains functionality for managing OculusXR spatial anchors.
 * Provides APIs for creating, saving, querying, sharing, and managing spatial anchors
 * in the OculusXR ecosystem. Anchors allow virtual content to be persistently positioned
 * in the real world across sessions.
 */
namespace OculusXRAnchors
{
	/**
	 * Main interface for OculusXR anchor operations.
	 * This struct provides static methods for all anchor-related functionality including
	 * creation, persistence, querying, sharing, and lifecycle management.
	 * Uses the singleton pattern for internal state management.
	 */
	struct OCULUSXRANCHORS_API FOculusXRAnchors
	{
		/**
		 * Initializes the anchor system and sets up internal state.
		 * Must be called before using any other anchor functionality.
		 */
		void Initialize();

		/**
		 * Cleans up the anchor system and releases resources.
		 * Should be called when anchor functionality is no longer needed.
		 */
		void Teardown();

		/**
		 * Gets the singleton instance of the anchor system.
		 * @return Pointer to the singleton instance
		 */
		static FOculusXRAnchors* GetInstance();

		/**
		 * Creates a new spatial anchor at the specified transform.
		 * @param InTransform The world transform where the anchor should be created
		 * @param TargetActor The actor to attach the anchor component to
		 * @param ResultCallback Delegate called when the operation completes
		 * @param OutResult Immediate result of the operation request
		 * @return True if the request was successfully submitted
		 */
		static bool CreateSpatialAnchor(const FTransform& InTransform, AActor* TargetActor, const FOculusXRSpatialAnchorCreateDelegate& ResultCallback, EOculusXRAnchorResult::Type& OutResult);

		/**
		 * Erases an anchor from storage.
		 * @param Anchor The anchor component to erase
		 * @param ResultCallback Delegate called when the operation completes
		 * @param OutResult Immediate result of the operation request
		 * @return True if the request was successfully submitted
		 */
		static bool EraseAnchor(UOculusXRAnchorComponent* Anchor, const FOculusXRAnchorEraseDelegate& ResultCallback, EOculusXRAnchorResult::Type& OutResult);

		/**
		 * Destroys an anchor by handle, removing it from the current session.
		 * @param AnchorHandle Handle of the anchor to destroy
		 * @param OutResult Result of the destroy operation
		 * @return True if the anchor was successfully destroyed
		 */
		static bool DestroyAnchor(uint64 AnchorHandle, EOculusXRAnchorResult::Type& OutResult);

		/**
		 * Sets the status of a specific component on an anchor.
		 * @param Anchor The anchor to modify
		 * @param SpaceComponentType Type of component to enable/disable
		 * @param Enable Whether to enable or disable the component
		 * @param Timeout Timeout for the operation in seconds
		 * @param ResultCallback Delegate called when the operation completes
		 * @param OutResult Immediate result of the operation request
		 * @return True if the request was successfully submitted
		 */
		static bool SetAnchorComponentStatus(UOculusXRAnchorComponent* Anchor, EOculusXRSpaceComponentType SpaceComponentType, bool Enable, float Timeout, const FOculusXRAnchorSetComponentStatusDelegate& ResultCallback, EOculusXRAnchorResult::Type& OutResult);

		/**
		 * Gets the current status of a specific component on an anchor.
		 * @param Anchor The anchor to query
		 * @param SpaceComponentType Type of component to check
		 * @param OutEnabled Whether the component is currently enabled
		 * @param OutChangePending Whether a status change is pending
		 * @param OutResult Result of the query operation
		 * @return True if the query was successful
		 */
		static bool GetAnchorComponentStatus(UOculusXRAnchorComponent* Anchor, EOculusXRSpaceComponentType SpaceComponentType, bool& OutEnabled, bool& OutChangePending, EOculusXRAnchorResult::Type& OutResult);

		/**
		 * Gets all supported component types for an anchor.
		 * @param Anchor The anchor to query
		 * @param OutSupportedComponents Array of supported component types
		 * @param OutResult Result of the query operation
		 * @return True if the query was successful
		 */
		static bool GetAnchorSupportedComponents(UOculusXRAnchorComponent* Anchor, TArray<EOculusXRSpaceComponentType>& OutSupportedComponents, EOculusXRAnchorResult::Type& OutResult);

		/**
		 * Sets the status of a specific component on a space by handle.
		 * @param Space Handle of the space to modify
		 * @param SpaceComponentType Type of component to enable/disable
		 * @param Enable Whether to enable or disable the component
		 * @param Timeout Timeout for the operation in seconds
		 * @param ResultCallback Delegate called when the operation completes
		 * @param OutResult Immediate result of the operation request
		 * @return True if the request was successfully submitted
		 */
		static bool SetComponentStatus(uint64 Space, EOculusXRSpaceComponentType SpaceComponentType, bool Enable, float Timeout, const FOculusXRAnchorSetComponentStatusDelegate& ResultCallback, EOculusXRAnchorResult::Type& OutResult);

		/**
		 * Gets the current status of a specific component on a space by handle.
		 * @param AnchorHandle Handle of the anchor to query
		 * @param SpaceComponentType Type of component to check
		 * @param OutEnabled Whether the component is currently enabled
		 * @param OutChangePending Whether a status change is pending
		 * @param OutResult Result of the query operation
		 * @return True if the query was successful
		 */
		static bool GetComponentStatus(uint64 AnchorHandle, EOculusXRSpaceComponentType SpaceComponentType, bool& OutEnabled, bool& OutChangePending, EOculusXRAnchorResult::Type& OutResult);

		/**
		 * Gets all supported component types for a space by handle.
		 * @param AnchorHandle Handle of the anchor to query
		 * @param OutSupportedComponents Array of supported component types
		 * @param OutResult Result of the query operation
		 * @return True if the query was successful
		 */
		static bool GetSupportedComponents(uint64 AnchorHandle, TArray<EOculusXRSpaceComponentType>& OutSupportedComponents, EOculusXRAnchorResult::Type& OutResult);

		/**
		 * Saves a single anchor to the specified storage location.
		 * @param Anchor The anchor to save
		 * @param StorageLocation Where to save the anchor (local or cloud)
		 * @param ResultCallback Delegate called when the operation completes
		 * @param OutResult Immediate result of the operation request
		 * @return True if the request was successfully submitted
		 */
		static bool SaveAnchor(UOculusXRAnchorComponent* Anchor, EOculusXRSpaceStorageLocation StorageLocation, const FOculusXRAnchorSaveDelegate& ResultCallback, EOculusXRAnchorResult::Type& OutResult);

		/**
		 * Saves multiple anchors to the specified storage location.
		 * @param Anchors Array of anchors to save
		 * @param StorageLocation Where to save the anchors (local or cloud)
		 * @param ResultCallback Delegate called when the operation completes
		 * @param OutResult Immediate result of the operation request
		 * @return True if the request was successfully submitted
		 */
		static bool SaveAnchorList(const TArray<UOculusXRAnchorComponent*>& Anchors, EOculusXRSpaceStorageLocation StorageLocation, const FOculusXRAnchorSaveListDelegate& ResultCallback, EOculusXRAnchorResult::Type& OutResult);

		/**
		 * Queries for anchors by their UUIDs from the specified storage location.
		 * @param AnchorUUIDs Array of anchor UUIDs to search for
		 * @param Location Storage location to search in
		 * @param ResultCallback Delegate called when the operation completes
		 * @param OutResult Immediate result of the operation request
		 * @return True if the request was successfully submitted
		 */
		static bool QueryAnchors(const TArray<FOculusXRUUID>& AnchorUUIDs, EOculusXRSpaceStorageLocation Location, const FOculusXRAnchorQueryDelegate& ResultCallback, EOculusXRAnchorResult::Type& OutResult);

		/**
		 * Performs an advanced anchor query with custom search parameters.
		 * @param QueryInfo Advanced query parameters and filters
		 * @param ResultCallback Delegate called when the operation completes
		 * @param OutResult Immediate result of the operation request
		 * @return True if the request was successfully submitted
		 */
		static bool QueryAnchorsAdvanced(const FOculusXRSpaceQueryInfo& QueryInfo, const FOculusXRAnchorQueryDelegate& ResultCallback, EOculusXRAnchorResult::Type& OutResult);

		/**
		 * Shares anchors with specified users by anchor components.
		 * @param Anchors Array of anchor components to share
		 * @param OculusUserIDs Array of Oculus user IDs to share with
		 * @param ResultCallback Delegate called when the operation completes
		 * @param OutResult Immediate result of the operation request
		 * @return True if the request was successfully submitted
		 */
		static bool ShareAnchors(const TArray<UOculusXRAnchorComponent*>& Anchors, const TArray<uint64>& OculusUserIDs, const FOculusXRAnchorShareDelegate& ResultCallback, EOculusXRAnchorResult::Type& OutResult);

		/**
		 * Shares anchors with specified users by anchor handles.
		 * @param AnchorHandles Array of anchor handles to share
		 * @param OculusUserIDs Array of Oculus user IDs to share with
		 * @param ResultCallback Delegate called when the operation completes
		 * @param OutResult Immediate result of the operation request
		 * @return True if the request was successfully submitted
		 */
		static bool ShareAnchors(const TArray<uint64>& AnchorHandles, const TArray<uint64>& OculusUserIDs, const FOculusXRAnchorShareDelegate& ResultCallback, EOculusXRAnchorResult::Type& OutResult);

		/**
		 * Gets the UUIDs of all spaces contained within a space container.
		 * @param Space Handle of the container space
		 * @param OutUUIDs Array to receive the contained space UUIDs
		 * @param OutResult Result of the operation
		 * @return True if the operation was successful
		 */
		static bool GetSpaceContainerUUIDs(uint64 Space, TArray<FOculusXRUUID>& OutUUIDs, EOculusXRAnchorResult::Type& OutResult);

		/**
		 * Saves multiple anchors using batch operation.
		 * @param Anchors Array of anchor components to save
		 * @param ResultCallback Delegate called when the operation completes
		 * @param OutResult Immediate result of the operation request
		 * @return True if the request was successfully submitted
		 */
		static bool SaveAnchors(const TArray<UOculusXRAnchorComponent*>& Anchors, const FOculusXRSaveAnchorsDelegate& ResultCallback, EOculusXRAnchorResult::Type& OutResult);

		/**
		 * Erases multiple anchors using batch operation by components.
		 * @param Anchors Array of anchor components to erase
		 * @param ResultCallback Delegate called when the operation completes
		 * @param OutResult Immediate result of the operation request
		 * @return True if the request was successfully submitted
		 */
		static bool EraseAnchors(const TArray<UOculusXRAnchorComponent*>& Anchors, const FOculusXREraseAnchorsDelegate& ResultCallback, EOculusXRAnchorResult::Type& OutResult);

		/**
		 * Erases multiple anchors using batch operation by handles and UUIDs.
		 * @param AnchorHandles Array of anchor handles to erase
		 * @param AnchorUUIDs Array of anchor UUIDs to erase
		 * @param ResultCallback Delegate called when the operation completes
		 * @param OutResult Immediate result of the operation request
		 * @return True if the request was successfully submitted
		 */
		static bool EraseAnchors(const TArray<FOculusXRUInt64>& AnchorHandles, const TArray<FOculusXRUUID>& AnchorUUIDs, const FOculusXREraseAnchorsDelegate& ResultCallback, EOculusXRAnchorResult::Type& OutResult);

		/**
		 * Discovers anchors based on discovery criteria.
		 * @param DiscoveryInfo Parameters for the discovery operation
		 * @param DiscoveryResultsCallback Delegate called when results are found
		 * @param DiscoveryCompleteCallback Delegate called when discovery completes
		 * @param OutResult Immediate result of the operation request
		 * @return True if the request was successfully submitted
		 */
		static bool DiscoverAnchors(const FOculusXRSpaceDiscoveryInfo& DiscoveryInfo, const FOculusXRDiscoverAnchorsResultsDelegate& DiscoveryResultsCallback, const FOculusXRDiscoverAnchorsCompleteDelegate& DiscoveryCompleteCallback, EOculusXRAnchorResult::Type& OutResult);

		/**
		 * Gets anchors that have been shared with the current user.
		 * @param AnchorUUIDs Array of anchor UUIDs to retrieve
		 * @param ResultCallback Delegate called when the operation completes
		 * @param OutResult Immediate result of the operation request
		 * @return True if the request was successfully submitted
		 */
		static bool GetSharedAnchors(const TArray<FOculusXRUUID>& AnchorUUIDs, const FOculusXRGetSharedAnchorsDelegate& ResultCallback, EOculusXRAnchorResult::Type& OutResult);

		/**
		 * Asynchronously shares anchors with groups.
		 * @param AnchorHandles Array of anchor handles to share
		 * @param Groups Array of group UUIDs to share with
		 * @param OnComplete Delegate called when the operation completes
		 * @return Shared pointer to the async operation for tracking
		 */
		static TSharedPtr<FShareAnchorsWithGroups> ShareAnchorsAsync(const TArray<FOculusXRUInt64>& AnchorHandles, const TArray<FOculusXRUUID>& Groups, const FShareAnchorsWithGroups::FCompleteDelegate& OnComplete);

		/**
		 * Asynchronously gets anchors shared with a specific group.
		 * @param Group UUID of the group to query
		 * @param WantedAnchors Array of specific anchor UUIDs to retrieve
		 * @param OnComplete Delegate called when the operation completes
		 * @return Shared pointer to the async operation for tracking
		 */
		static TSharedPtr<FGetAnchorsSharedWithGroup> GetSharedAnchorsAsync(const FOculusXRUUID& Group, const TArray<FOculusXRUUID>& WantedAnchors, const FGetAnchorsSharedWithGroup::FCompleteDelegate& OnComplete);

		// Request ID validation methods
		bool HasAnchorQueryBinding(uint64 RequestId) const;
		bool HasDiscoveryBinding(uint64 RequestId) const;

	private:
		struct AnchorQueryBinding;
		struct GetSharedAnchorsBinding;

		void HandleSpatialAnchorCreateComplete(FOculusXRUInt64 RequestId, EOculusXRAnchorResult::Type Result, FOculusXRUInt64 Space, FOculusXRUUID UUID);
		void HandleAnchorEraseComplete(FOculusXRUInt64 RequestId, EOculusXRAnchorResult::Type Result, FOculusXRUUID UUID, EOculusXRSpaceStorageLocation Location);

		void HandleSetComponentStatusComplete(FOculusXRUInt64 RequestId, EOculusXRAnchorResult::Type Result, FOculusXRUInt64 Space, FOculusXRUUID UUID, EOculusXRSpaceComponentType ComponentType, bool Enabled);

		void HandleAnchorSaveComplete(FOculusXRUInt64 RequestId, FOculusXRUInt64 Space, bool Success, EOculusXRAnchorResult::Type Result, FOculusXRUUID UUID);
		void HandleAnchorSaveListComplete(FOculusXRUInt64 RequestId, EOculusXRAnchorResult::Type Result);

		void HandleAnchorQueryResultElement(FOculusXRUInt64 RequestId, FOculusXRUInt64 Space, FOculusXRUUID UUID);
		void UpdateQuerySpacesBinding(AnchorQueryBinding* Binding, FOculusXRUInt64 RequestId, FOculusXRUInt64 Space, FOculusXRUUID UUID);
		void UpdateGetSharedAnchorsBinding(GetSharedAnchorsBinding* Binding, FOculusXRUInt64 RequestId, FOculusXRUInt64 Space, FOculusXRUUID UUID);

		void HandleAnchorQueryComplete(FOculusXRUInt64 RequestId, EOculusXRAnchorResult::Type Result);
		void QuerySpacesComplete(AnchorQueryBinding* Binding, FOculusXRUInt64 RequestId, EOculusXRAnchorResult::Type Result);
		void GetSharedAnchorsComplete(GetSharedAnchorsBinding* Binding, FOculusXRUInt64 RequestId, EOculusXRAnchorResult::Type Result);

		void HandleAnchorSharingComplete(FOculusXRUInt64 RequestId, EOculusXRAnchorResult::Type Result);

		void HandleAnchorsSaveComplete(FOculusXRUInt64 RequestId, EOculusXRAnchorResult::Type Result);
		void HandleAnchorsEraseComplete(FOculusXRUInt64 RequestId, EOculusXRAnchorResult::Type Result);
		void HandleAnchorsEraseByComponentsComplete(FOculusXRUInt64 RequestId, EOculusXRAnchorResult::Type Result);
		void HandleAnchorsEraseByHandleAndUUIDComplete(FOculusXRUInt64 RequestId, EOculusXRAnchorResult::Type Result);

		void HandleAnchorsDiscoverResults(FOculusXRUInt64 RequestId, const TArray<FOculusXRAnchorsDiscoverResult>& Results);
		void HandleAnchorsDiscoverComplete(FOculusXRUInt64 RequestId, EOculusXRAnchorResult::Type Result);

		struct EraseAnchorBinding
		{
			FOculusXRUInt64 RequestId;
			FOculusXRAnchorEraseDelegate Binding;
			TWeakObjectPtr<UOculusXRAnchorComponent> Anchor;
		};

		struct SetComponentStatusBinding
		{
			FOculusXRUInt64 RequestId;
			FOculusXRAnchorSetComponentStatusDelegate Binding;
			uint64 AnchorHandle;
		};

		struct CreateAnchorBinding
		{
			FOculusXRUInt64 RequestId;
			FOculusXRSpatialAnchorCreateDelegate Binding;
			TWeakObjectPtr<AActor> Actor;
		};

		struct SaveAnchorBinding
		{
			FOculusXRUInt64 RequestId;
			FOculusXRAnchorSaveDelegate Binding;
			EOculusXRSpaceStorageLocation Location;
			TWeakObjectPtr<UOculusXRAnchorComponent> Anchor;
		};

		struct SaveAnchorListBinding
		{
			FOculusXRUInt64 RequestId;
			FOculusXRAnchorSaveListDelegate Binding;
			EOculusXRSpaceStorageLocation Location;
			TArray<TWeakObjectPtr<UOculusXRAnchorComponent>> SavedAnchors;
		};

		struct AnchorQueryBinding
		{
			FOculusXRUInt64 RequestId;
			FOculusXRAnchorQueryDelegate Binding;
			EOculusXRSpaceStorageLocation Location;
			TArray<FOculusXRSpaceQueryResult> Results;
		};

		struct ShareAnchorsBinding
		{
			FOculusXRUInt64 RequestId;
			FOculusXRAnchorShareDelegate Binding;
			TArray<TWeakObjectPtr<UOculusXRAnchorComponent>> SharedAnchors;
			TArray<uint64> OculusUserIds;
		};

		struct SaveAnchorsBinding
		{
			FOculusXRUInt64 RequestId;
			FOculusXRSaveAnchorsDelegate Binding;
			TArray<TWeakObjectPtr<UOculusXRAnchorComponent>> SavedAnchors;
		};

		struct EraseAnchorsBinding
		{
			FOculusXRUInt64 RequestId;
			FOculusXREraseAnchorsDelegate Binding;
			TArray<TWeakObjectPtr<UOculusXRAnchorComponent>> ErasedAnchors;
			TArray<FOculusXRUInt64> ErasedAnchorsHandles;
			TArray<FOculusXRUUID> ErasedAnchorsUUIDs;
		};

		struct AnchorDiscoveryBinding
		{
			FOculusXRUInt64 RequestId;
			FOculusXRDiscoverAnchorsResultsDelegate ResultBinding;
			FOculusXRDiscoverAnchorsCompleteDelegate CompleteBinding;
		};

		struct GetSharedAnchorsBinding
		{
			FOculusXRUInt64 RequestId;
			FOculusXRGetSharedAnchorsDelegate ResultBinding;
			TArray<FOculusXRAnchorsDiscoverResult> Results;
		};

		// Delegate bindings
		TMap<uint64, CreateAnchorBinding> CreateSpatialAnchorBindings;
		TMap<uint64, EraseAnchorBinding> EraseAnchorBindings;
		TMap<uint64, SetComponentStatusBinding> SetComponentStatusBindings;
		TMap<uint64, SaveAnchorBinding> AnchorSaveBindings;
		TMap<uint64, SaveAnchorListBinding> AnchorSaveListBindings;
		TMap<uint64, AnchorQueryBinding> AnchorQueryBindings;
		TMap<uint64, ShareAnchorsBinding> ShareAnchorsBindings;
		TMap<uint64, SaveAnchorsBinding> SaveAnchorsBindings;
		TMap<uint64, EraseAnchorsBinding> EraseAnchorsBindings;
		TMap<uint64, AnchorDiscoveryBinding> AnchorDiscoveryBindings;
		TMap<uint64, GetSharedAnchorsBinding> GetSharedAnchorsBindings;

		// Delegate handles
		FDelegateHandle DelegateHandleAnchorCreate;
		FDelegateHandle DelegateHandleAnchorErase;
		FDelegateHandle DelegateHandleSetComponentStatus;
		FDelegateHandle DelegateHandleAnchorSave;
		FDelegateHandle DelegateHandleAnchorSaveList;
		FDelegateHandle DelegateHandleQueryResultsBegin;
		FDelegateHandle DelegateHandleQueryResultElement;
		FDelegateHandle DelegateHandleQueryComplete;
		FDelegateHandle DelegateHandleAnchorShare;
		FDelegateHandle DelegateHandleAnchorsSave;
		FDelegateHandle DelegateHandleAnchorsErase;
		FDelegateHandle DelegateHandleAnchorsDiscoverResults;
		FDelegateHandle DelegateHandleAnchorsDiscoverComplete;
	};

} // namespace OculusXRAnchors
