unreal.DatasmithAreaLightActor â Unreal Python 5.4 (Experimental) documentation

### Navigation

* [index](../genindex.html?application_version=5.4 "General Index")
* [next](DatasmithAssetImportData.html?application_version=5.4 "unreal.DatasmithAssetImportData") |
* [previous](DatasmithAdditionalData.html?application_version=5.4 "unreal.DatasmithAdditionalData") |

# `unreal.DatasmithAreaLightActor`

*class* unreal.DatasmithAreaLightActor(*outer: [Object](Object.html?application_version=5.4#unreal.Object "unreal.Object") | [None](https://docs.python.org/2/library/constants.html#None "(in Python v2.7)") = None*, *name: [Name](Name.html?application_version=5.4#unreal.Name "unreal.Name") | [str](https://docs.python.org/2/library/functions.html#str "(in Python v2.7)") = 'None'*)
:   Bases: [`Actor`](Actor.html?application_version=5.4#unreal.Actor "unreal.Actor")

    Datasmith Area Light Actor

    **C++ Source:**

    * **Plugin**: DatasmithContent
    * **Module**: DatasmithContent
    * **File**: DatasmithAreaLightActor.h

    **Editor Properties:** (see get\_editor\_property/set\_editor\_property)

    * `actor_guid` (Guid): [Read-Write] The GUID for this actor; this guid will be the same for actors from instanced streaming levels.
      see: ActorInstanceGuid, FActorInstanceGuidMapper
      note: Donât use VisibleAnywhere here to avoid getting the CPF\_Edit flag and get this property reset when resetting to defaults. See FActorDetails::AddActorCategory and EditorUtilities::CopySingleProperty for details.
    * `actor_instance_guid` (Guid): [Read-Write] The instance GUID for this actor; this guid will be unique for actors from instanced streaming levels.
      see: ActorGuid
      note: This is not guaranteed to be valid during PostLoad, but safe to access from RegisterAllComponents.
    * `allow_tick_before_begin_play` (bool): [Read-Write] Whether we allow this Actor to tick before it receives the BeginPlay event.
      Normally we donât tick actors until after BeginPlay; this setting allows this behavior to be overridden.
      This Actor must be able to tick for this setting to be relevant.
    * `always_relevant` (bool): [Read-Write] Always relevant for network (overrides bOnlyRelevantToOwner).
    * `async_physics_tick_enabled` (bool): [Read-Write] Whether to use use the async physics tick with this actor.
    * `attenuation_radius` (float): [Read-Write]
    * `auto_destroy_when_finished` (bool): [Read-Write] If true then destroy self when âfinishedâ, meaning all relevant components report that they are done and no timelines or timers are in flight.
    * `auto_receive_input` (AutoReceiveInput): [Read-Write] Automatically registers this actor to receive input from a player.
    * `block_input` (bool): [Read-Write] If true, all input on the stack below this actor will not be considered
    * `call_pre_replication` (bool): [Read-Write]
    * `call_pre_replication_for_replay` (bool): [Read-Write]
    * `can_be_damaged` (bool): [Read-Write] Whether this actor can take damage. Must be true for damage events (e.g. ReceiveDamage()) to be called.
      see: <https://www.unrealengine.com/blog/damage-in-ue4>
      see: TakeDamage(), ReceiveDamage()
    * `can_be_in_cluster` (bool): [Read-Write] If true, this actor can be put inside of a GC Cluster to improve Garbage Collection performance
    * `color` (LinearColor): [Read-Write]
    * `content_bundle_guid` (Guid): [Read-Write] The GUID for this actorâs content bundle.
    * `custom_time_dilation` (float): [Read-Write] Allow each actor to run at a different time speed. The DeltaTime for a frame is multiplied by the global TimeDilation (in WorldSettings) and this CustomTimeDilation for this actorâs tick.
    * `data_layer_assets` (Array[DataLayerAsset]): [Read-Write]
    * `data_layers` (Array[ActorDataLayer]): [Read-Only] DataLayers the actor belongs to.
    * `default_update_overlaps_method_during_level_streaming` (ActorUpdateOverlapsMethod): [Read-Only] Default value taken from config file for this class when âUseConfigDefaultâ is chosen for
      âUpdateOverlapsMethodDuringLevelStreamingâ. This allows a default to be chosen per class in the matching config.
      For example, for Actor it could be specified in DefaultEngine.ini as:

      [/Script/Engine.Actor]
      DefaultUpdateOverlapsMethodDuringLevelStreaming = OnlyUpdateMovable

      Another subclass could set their default to something different, such as:

      [/Script/Engine.BlockingVolume]
      DefaultUpdateOverlapsMethodDuringLevelStreaming = NeverUpdate
      see: UpdateOverlapsMethodDuringLevelStreaming
    * `dimensions` (Vector2D): [Read-Write]
    * `enable_auto_lod_generation` (bool): [Read-Write] Whether this actor should be considered or not during HLOD generation.
    * `external_data_layer_asset` (ExternalDataLayerAsset): [Read-Only]
    * `find_camera_component_when_view_target` (bool): [Read-Write] If true, this actor should search for an owned camera component to view through when used as a view target.
    * `generate_overlap_events_during_level_streaming` (bool): [Read-Write] If true, this actor will generate overlap Begin/End events when spawned as part of level streaming, which includes initial level load.
      You might enable this is in the case where a streaming level loads around an actor and you want Begin/End overlap events to trigger.
      see: UpdateOverlapsMethodDuringLevelStreaming
    * `hidden` (bool): [Read-Write] Allows us to only see this Actor in the Editor, and not in the actual game.
      see: SetActorHiddenInGame()
    * `hlod_layer` (HLODLayer): [Read-Write] The UHLODLayer in which this actor should be included.
    * `ies_brightness_scale` (float): [Read-Write]
    * `ies_texture` (TextureLightProfile): [Read-Write]
    * `ignores_origin_shifting` (bool): [Read-Write] Whether this actor should not be affected by world origin shifting.
    * `initial_life_span` (float): [Read-Write] How long this Actor lives before dying, 0=forever. Note this is the INITIAL value and should not be modified once play has begun.
    * `input_priority` (int32): [Read-Write] The priority of this input component when pushed in to the stack.
    * `instigator` (Pawn): [Read-Write] Pawn responsible for damage and other gameplay events caused by this actor.
    * `intensity` (float): [Read-Write]
    * `intensity_units` (LightUnits): [Read-Write]
    * `is_editor_only_actor` (bool): [Read-Write] Whether this actor is editor-only. Use with care, as if this actor is referenced by anything else that reference will be NULL in cooked builds
    * `is_main_world_only` (bool): [Read-Write] If checked, this Actor will only get loaded in a main world (persistent level), it will not be loaded through Level Instances.
    * `is_spatially_loaded` (bool): [Read-Write] Determine if this actor is spatially loaded when placed in a partitioned world.
      :   If true, this actor will be loaded when in the range of any streaming sources and if (1) in no data layers, or (2) one or more of its data layers are enabled.
          If false, this actor will be loaded if (1) in no data layers, or (2) one or more of its data layers are enabled.
    * `layers` (Array[Name]): [Read-Write] Layers the actor belongs to. This is outside of the editoronly data to allow hiding of LD-specified layers at runtime for profiling.
    * `light_shape` (DatasmithAreaLightActorShape): [Read-Write]
    * `light_type` (DatasmithAreaLightActorType): [Read-Write]
    * `min_net_update_frequency` (float): [Read-Write] Used to determine what rate to throttle down to when replicated properties are changing infrequently
    * `mobility` (ComponentMobility): [Read-Write]
    * `net_cull_distance_squared` (float): [Read-Write] Square of the max distance from the clientâs viewpoint that this actor is relevant and will be replicated.
    * `net_dormancy` (NetDormancy): [Read-Write] Dormancy setting for actor to take itself off of the replication list without being destroyed on clients.
    * `net_load_on_client` (bool): [Read-Write] This actor will be loaded on network clients during map load
    * `net_priority` (float): [Read-Write] Priority for this actor when checking for replication in a low bandwidth or saturated situation, higher priority means it is more likely to replicate
    * `net_update_frequency` (float): [Read-Write] How often (per second) this actor will be considered for replication, used to determine NetUpdateTime
    * `net_use_owner_relevancy` (bool): [Read-Write] If actor has valid Owner, call Ownerâs IsNetRelevantFor and GetNetPriority
    * `on_actor_begin_overlap` (ActorBeginOverlapSignature): [Read-Write] Called when another actor begins to overlap this actor, for example a player walking into a trigger.
      For events when objects have a blocking collision, for example a player hitting a wall, see âHitâ events.
      note: Components on both this and the other Actor must have bGenerateOverlapEvents set to true to generate overlap events.
    * `on_actor_end_overlap` (ActorEndOverlapSignature): [Read-Write] Called when another actor stops overlapping this actor.
      note: Components on both this and the other Actor must have bGenerateOverlapEvents set to true to generate overlap events.
    * `on_actor_hit` (ActorHitSignature): [Read-Write] Called when this Actor hits (or is hit by) something solid. This could happen due to things like Character movement, using Set Location with âsweepâ enabled, or physics simulation.
      For events when objects overlap (e.g. walking into a trigger) see the âOverlapâ event.
      note: For collisions during physics simulation to generate hit events, âSimulation Generates Hit Eventsâ must be enabled.
    * `on_begin_cursor_over` (ActorBeginCursorOverSignature): [Read-Write] Called when the mouse cursor is moved over this actor if mouse over events are enabled in the player controller.
    * `on_clicked` (ActorOnClickedSignature): [Read-Write] Called when the left mouse button is clicked while the mouse is over this actor and click events are enabled in the player controller.
    * `on_destroyed` (ActorDestroyedSignature): [Read-Write] Event triggered when the actor has been explicitly destroyed.
    * `on_end_cursor_over` (ActorEndCursorOverSignature): [Read-Write] Called when the mouse cursor is moved off this actor if mouse over events are enabled in the player controller.
    * `on_end_play` (ActorEndPlaySignature): [Read-Write] Event triggered when the actor is being deleted or removed from a level.
    * `on_input_touch_begin` (ActorOnInputTouchBeginSignature): [Read-Write] Called when a touch input is received over this actor when touch events are enabled in the player controller.
    * `on_input_touch_end` (ActorOnInputTouchEndSignature): [Read-Write] Called when a touch input is received over this component when touch events are enabled in the player controller.
    * `on_input_touch_enter` (ActorBeginTouchOverSignature): [Read-Write] Called when a finger is moved over this actor when touch over events are enabled in the player controller.
    * `on_input_touch_leave` (ActorEndTouchOverSignature): [Read-Write] Called when a finger is moved off this actor when touch over events are enabled in the player controller.
    * `on_released` (ActorOnReleasedSignature): [Read-Write] Called when the left mouse button is released while the mouse is over this actor and click events are enabled in the player controller.
    * `on_take_any_damage` (TakeAnyDamageSignature): [Read-Write] Called when the actor is damaged in any way.
    * `on_take_point_damage` (TakePointDamageSignature): [Read-Write] Called when the actor is damaged by point damage.
    * `on_take_radial_damage` (TakeRadialDamageSignature): [Read-Write] Called when the actor is damaged by radial damage.
    * `only_relevant_to_owner` (bool): [Read-Write] If true, this actor is only relevant to its owner. If this flag is changed during play, all non-owner channels would need to be explicitly closed.
    * `optimize_bp_component_data` (bool): [Read-Write] Whether to cook additional data to speed up spawn events at runtime for any Blueprint classes based on this Actor. This option may slightly increase memory usage in a cooked build.
    * `physics_replication_mode` (PhysicsReplicationMode): [Read-Write] Which mode to replicate physics through for this actor. Only relevant if the actor replicates movement and has a component that simulate physics.
    * `pivot_offset` (Vector): [Read-Write] Local space pivot offset for the actor, only used in the editor
    * `primary_actor_tick` (ActorTickFunction): [Read-Write] Primary Actor tick function, which calls TickActor().
      Tick functions can be configured to control whether ticking is enabled, at what time during a frame the update occurs, and to set up tick dependencies.
      see: <https://docs.unrealengine.com/API/Runtime/Engine/Engine/FTickFunction>
      see: AddTickPrerequisiteActor(), AddTickPrerequisiteComponent()
    * `relevant_for_level_bounds` (bool): [Read-Write] If true, this actorâs componentâs bounds will be included in the levelâs
      bounding box unless the Actorâs class has overridden IsLevelBoundsRelevant
    * `remote_role` (NetRole): [Read-Only] Describes how much control the remote machine has over the actor.
    * `replay_rewindable` (bool): [Read-Write] If true, this actor will only be destroyed during scrubbing if the replay is set to a time before the actor existed.
      Otherwise, RewindForReplay will be called if we detect the actor needs to be reset.
      Note, this Actor must not be destroyed by gamecode, and RollbackViaDeletion may not be used.
    * `replicate_movement` (bool): [Read-Write] If true, replicate movement/location related properties.
      Actor must also be set to replicate.
      see: SetReplicates()
      see: <https://docs.unrealengine.com/InteractiveExperiences/Networking/Actors>
    * `replicate_using_registered_sub_object_list` (bool): [Read-Write] When true the replication system will only replicate the registered subobjects and the replicated actor components list
      When false the replication system will instead call the virtual ReplicateSubobjects() function where the subobjects and actor components need to be manually replicated.
    * `replicated_movement` (RepMovement): [Read-Write] Used for replication of our RootComponentâs position and velocity
    * `replicates` (bool): [Read-Write] If true, this actor will replicate to remote machines
      see: SetReplicates()
    * `role` (NetRole): [Read-Only] Describes how much control the local machine has over the actor.
    * `root_component` (SceneComponent): [Read-Write] The component that defines the transform (location, rotation, scale) of this Actor in the world, all other components must be attached to this one somehow
    * `rotation` (Rotator): [Read-Write]
    * `runtime_grid` (Name): [Read-Write] Determine in which partition grid this actor will be placed in the partition (if the world is partitioned).
      If None, the decision will be left to the partition.
    * `source_length` (float): [Read-Write]
    * `source_radius` (float): [Read-Write]
    * `spawn_collision_handling_method` (SpawnActorCollisionHandlingMethod): [Read-Write] Controls how to handle spawning this actor in a situation where itâs colliding with something else. âDefaultâ means AlwaysSpawn here.
    * `spotlight_inner_angle` (float): [Read-Write]
    * `spotlight_outer_angle` (float): [Read-Write]
    * `sprite_scale` (float): [Read-Write] The scale to apply to any billboard components in editor builds (happens in any WITH\_EDITOR build, including non-cooked games).
    * `tags` (Array[Name]): [Read-Write] Array of tags that can be used for grouping and categorizing.
    * `temperature` (float): [Read-Write]
    * `update_overlaps_method_during_level_streaming` (ActorUpdateOverlapsMethod): [Read-Write] Condition for calling UpdateOverlaps() to initialize overlap state when loaded in during level streaming.
      If set to âUseConfigDefaultâ, the default specified in ini (displayed in âDefaultUpdateOverlapsMethodDuringLevelStreamingâ) will be used.
      If overlaps are not initialized, this actor and attached components will not have an initial state of what objects are touching it,
      and overlap events may only come in once one of those objects update overlaps themselves (for example when moving).
      However if an object touching it *does* initialize state, both objects will know about their touching state with each other.
      This can be a potentially large performance savings during level loading and streaming, and is safe if the object and others initially
      overlapping it do not need the overlap state because they will not trigger overlap notifications.

      Note that if âbGenerateOverlapEventsDuringLevelStreamingâ is true, overlaps are always updated in this case, but that flag
      determines whether the Begin/End overlap events are triggered.
      see: bGenerateOverlapEventsDuringLevelStreaming, DefaultUpdateOverlapsMethodDuringLevelStreaming, GetUpdateOverlapsMethodDuringLevelStreaming()
    * `use_ies_brightness` (bool): [Read-Write]

    *property* attenuation\_radius*: [float](https://docs.python.org/2/library/functions.html#float "(in Python v2.7)")*
    :   [Read-Write]

        Type:
        :   ([float](https://docs.python.org/2/library/functions.html#float "(in Python v2.7)"))

    *property* color*: [LinearColor](LinearColor.html?application_version=5.4#unreal.LinearColor "unreal.LinearColor")*
    :   [Read-Write]

        Type:
        :   ([LinearColor](LinearColor.html?application_version=5.4#unreal.LinearColor "unreal.LinearColor"))

    *property* dimensions*: [Vector2D](Vector2D.html?application_version=5.4#unreal.Vector2D "unreal.Vector2D")*
    :   [Read-Write]

        Type:
        :   ([Vector2D](Vector2D.html?application_version=5.4#unreal.Vector2D "unreal.Vector2D"))

    *property* ies\_brightness\_scale*: [float](https://docs.python.org/2/library/functions.html#float "(in Python v2.7)")*
    :   [Read-Write]

        Type:
        :   ([float](https://docs.python.org/2/library/functions.html#float "(in Python v2.7)"))

    *property* ies\_texture*: [TextureLightProfile](TextureLightProfile.html?application_version=5.4#unreal.TextureLightProfile "unreal.TextureLightProfile")*
    :   [Read-Write]

        Type:
        :   ([TextureLightProfile](TextureLightProfile.html?application_version=5.4#unreal.TextureLightProfile "unreal.TextureLightProfile"))

    *property* intensity*: [float](https://docs.python.org/2/library/functions.html#float "(in Python v2.7)")*
    :   [Read-Write]

        Type:
        :   ([float](https://docs.python.org/2/library/functions.html#float "(in Python v2.7)"))

    *property* intensity\_units*: [LightUnits](LightUnits.html?application_version=5.4#unreal.LightUnits "unreal.LightUnits")*
    :   [Read-Write]

        Type:
        :   ([LightUnits](LightUnits.html?application_version=5.4#unreal.LightUnits "unreal.LightUnits"))

    *property* light\_shape*: [DatasmithAreaLightActorShape](DatasmithAreaLightActorShape.html?application_version=5.4#unreal.DatasmithAreaLightActorShape "unreal.DatasmithAreaLightActorShape")*
    :   [Read-Write]

        Type:
        :   ([DatasmithAreaLightActorShape](DatasmithAreaLightActorShape.html?application_version=5.4#unreal.DatasmithAreaLightActorShape "unreal.DatasmithAreaLightActorShape"))

    *property* light\_type*: [DatasmithAreaLightActorType](DatasmithAreaLightActorType.html?application_version=5.4#unreal.DatasmithAreaLightActorType "unreal.DatasmithAreaLightActorType")*
    :   [Read-Write]

        Type:
        :   ([DatasmithAreaLightActorType](DatasmithAreaLightActorType.html?application_version=5.4#unreal.DatasmithAreaLightActorType "unreal.DatasmithAreaLightActorType"))

    *property* mobility*: [ComponentMobility](ComponentMobility.html?application_version=5.4#unreal.ComponentMobility "unreal.ComponentMobility")*
    :   [Read-Only]

        Type:
        :   ([ComponentMobility](ComponentMobility.html?application_version=5.4#unreal.ComponentMobility "unreal.ComponentMobility"))

    *property* rotation*: [Rotator](Rotator.html?application_version=5.4#unreal.Rotator "unreal.Rotator")*
    :   [Read-Write]

        Type:
        :   ([Rotator](Rotator.html?application_version=5.4#unreal.Rotator "unreal.Rotator"))

    *property* source\_length*: [float](https://docs.python.org/2/library/functions.html#float "(in Python v2.7)")*
    :   [Read-Write]

        Type:
        :   ([float](https://docs.python.org/2/library/functions.html#float "(in Python v2.7)"))

    *property* source\_radius*: [float](https://docs.python.org/2/library/functions.html#float "(in Python v2.7)")*
    :   [Read-Write]

        Type:
        :   ([float](https://docs.python.org/2/library/functions.html#float "(in Python v2.7)"))

    *property* spotlight\_inner\_angle*: [float](https://docs.python.org/2/library/functions.html#float "(in Python v2.7)")*
    :   [Read-Write]

        Type:
        :   ([float](https://docs.python.org/2/library/functions.html#float "(in Python v2.7)"))

    *property* spotlight\_outer\_angle*: [float](https://docs.python.org/2/library/functions.html#float "(in Python v2.7)")*
    :   [Read-Write]

        Type:
        :   ([float](https://docs.python.org/2/library/functions.html#float "(in Python v2.7)"))

    *property* temperature*: [float](https://docs.python.org/2/library/functions.html#float "(in Python v2.7)")*
    :   [Read-Write]

        Type:
        :   ([float](https://docs.python.org/2/library/functions.html#float "(in Python v2.7)"))

    *property* use\_ies\_brightness*: [bool](https://docs.python.org/2/library/functions.html#bool "(in Python v2.7)")*
    :   [Read-Write]

        Type:
        :   ([bool](https://docs.python.org/2/library/functions.html#bool "(in Python v2.7)"))

### Navigation

* [index](../genindex.html?application_version=5.4 "General Index")
* [next](DatasmithAssetImportData.html?application_version=5.4 "unreal.DatasmithAssetImportData") |
* [previous](DatasmithAdditionalData.html?application_version=5.4 "unreal.DatasmithAdditionalData") |