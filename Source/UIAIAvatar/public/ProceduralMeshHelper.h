// Copyright 2018, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "StaticMeshResources.h"
#include "ProceduralMeshComponent.h"
#include "KismetProceduralMeshLibrary.h"
#include "RawMesh.h"

#include "PhysicsEngine/BodySetup.h"
#include "PhysicsEngine/ConvexElem.h"

#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "ProceduralMeshHelper.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UIAIAVATAR_API UProceduralMeshHelper : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UProceduralMeshHelper();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	/**
	 * Convert a static mesh component to a procedural mesh component, while retaining the previous static materials
	 *
	 * @param StaticMeshComponent - The staic mesh component that will be used to create the procedural mesh
	 * @param StaticMaterials - The static materials that get extracted from the original static mesh
	 * @return A reference to the newly created procedural mesh
	 */
	UFUNCTION(BlueprintCallable)
	static UProceduralMeshComponent* ConvertStaticToProceduralMeshComponent(
		UStaticMeshComponent* StaticMeshComponent, TArray<FStaticMaterial> &StaticMaterials
	);

	/**
	* Convert a procedural mesh component to a static mesh component
	*
	* @param ProceduralMeshComponent - The procedural mesh component that will be used to create the static mesh
	* @param StaticMaterials - The old static materials used to properly recreate the static mesh
	*/
	UFUNCTION(BlueprintCallable)
	static UStaticMeshComponent* ConvertProceduralComponentToStaticMeshActor(
		UProceduralMeshComponent* ProceduralMeshComponent, const TArray<FStaticMaterial> &StaticMaterials, bool centerOrigin = false
	);

	/**
	 * Get a specific slicing component from a static mesh component
	 *
	 * @param SlicingObject - The static mesh component on which the slicing components are attached to
	 * @return The first found slicing component of the specified type
	 */
	template<class ComponentType>
	static FORCEINLINE ComponentType* GetSlicingComponent(UStaticMeshComponent* SlicingObject);

private:
	static void CorrectProperties(UPrimitiveComponent* NewComponent, UPrimitiveComponent* OldComponent);
	static UStaticMesh* GenerateStaticMesh(UProceduralMeshComponent* ProceduralMeshComponent, bool centerOrigin);
	static AStaticMeshActor* SpawnStaticMeshActor(UProceduralMeshComponent* ProceduralMeshComponent);

};
