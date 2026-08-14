# Assemblies
- Assemblies are made up of a few different blueprints that all interact with each other in order to allow a model to be split up and put together
- Each assembly is made up of multiple part actors

# Snap points/ Snap Validator
- Snap points are points where an assembly will connect if there exists a possible connection point
- These snap points are verified with based on two properties
    - The snap point ID
        - If snap points share the same ID, then they are able to connect to each other
        - Along with setting the ID of the snap point, the snap point should also have a compatible part and a compatible snap ID to associate with
    - The position of the snap point
        - The snap points also validate based on the coordinates of the snap point
        - These snap points are in a relative space with respect to each part actor, so the snap points should have the same relative offset
    - To ensure that the snap points connect correctly, ensure that the IDs are set correctly in the editor, and that their relative coordinates are the same as each other
    - To ensure nothing breaks with them, it is recommended to make sure each pair of snap points have the same properties as each other


# Assembly Actor
- Assembly actor is the base of an assembly. This blueprint has 2 parts to it, a base snap point and with a sphere collision shape as a child
- This snap point is the snap point where the assembly with start
- The assembly actor tracks what parts are currently connected, and if the assembly is active or completed

# Part Actor
- Park actor are what make up assemblies for the most part. 
- Each part actor contains mesh, a snap point, a grab component, and a snap validator
    - The mesh is the mesh of the actual part
    - The snap point is where the part will snap onto the assembly
    - the grab component allows the part actor to be grabbed
    - the snap validator validates the snap
- Part actors can also be motorized if they are a motor
    - The motor's speed, max rpm, and axis of rotation can all be changed in the details menu
- Each part actor contains a reference to the base assembly
    - This allows the part actor to know what assembly it should belong to