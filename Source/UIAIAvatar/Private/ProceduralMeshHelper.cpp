// Copyright 2018, Institute for Artificial Intelligence - University of Bremen

#include "ProceduralMeshHelper.h"


// Sets default values for this component's properties
UProceduralMeshHelper::UProceduralMeshHelper()
{
	// ...
}


// Called when the game starts
void UProceduralMeshHelper::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

UProceduralMeshComponent* UProceduralMeshHelper::ConvertStaticToProceduralMeshComponent(
	UStaticMeshComponent* StaticMeshComponent, TArray<FStaticMaterial> &StaticMaterials)
{
	// Needed so that the component can be cut/changed in runtime
	StaticMeshComponent->GetStaticMesh()->bAllowCPUAccess = true;

	UProceduralMeshComponent* ProceduralMeshComponent = NewObject<UProceduralMeshComponent>(StaticMeshComponent);
	ProceduralMeshComponent->bUseComplexAsSimpleCollision = false;
	CorrectProperties(ProceduralMeshComponent, StaticMeshComponent);

	// Copies the mesh, collision and currently used materials from the StaticMeshComponent
	UKismetProceduralMeshLibrary::CopyProceduralMeshFromStaticMeshComponent(
		StaticMeshComponent, 0, ProceduralMeshComponent, true);

	// Give out a copy of the static material from the original component, as procedural meshes do not have one
	// and therefore lose out on information about the materials
	StaticMaterials = StaticMeshComponent->GetStaticMesh()->StaticMaterials;

	// Remove the old static mesh
	StaticMeshComponent->DestroyComponent();

	return ProceduralMeshComponent;
}

UStaticMeshComponent* UProceduralMeshHelper::ConvertProceduralComponentToStaticMeshActor(
	UProceduralMeshComponent* ProceduralMeshComponent, const TArray<FStaticMaterial> &StaticMaterials, bool centerOrigin
	)
{
	// Generate the static mesh from the data scanned from the procedural mesh
	UStaticMesh* StaticMesh = GenerateStaticMesh(ProceduralMeshComponent, centerOrigin);
	// Set the static materials gotten from the old static mesh
	StaticMesh->StaticMaterials = StaticMaterials;

	AStaticMeshActor* StaticMeshActor = SpawnStaticMeshActor(ProceduralMeshComponent);

	// Edit the StaticMeshComponent to have the same properties as the old ProceduralMeshComponent
	UStaticMeshComponent* NewStaticMeshComponent = StaticMeshActor->GetStaticMeshComponent();
	NewStaticMeshComponent->SetStaticMesh(StaticMesh);
	CorrectProperties(NewStaticMeshComponent, ProceduralMeshComponent);

	if (centerOrigin) {
		StaticMeshActor->SetActorLocation(ProceduralMeshComponent->Bounds.Origin);
	}
	// Remove the old component
	ProceduralMeshComponent->DestroyComponent();

	return NewStaticMeshComponent;
}

template<class ComponentType>
ComponentType* UProceduralMeshHelper::GetSlicingComponent(UStaticMeshComponent* SlicingObject)
{
	TArray<USceneComponent*> SlicingComponents;
	SlicingObject->GetChildrenComponents(true, SlicingComponents);

	for (USceneComponent* Component : SlicingComponents)
	{
		if (ComponentType* TypedComponent = Cast<ComponentType>(Component))
		{
			// Only one slicing component of each type should exist
			return TypedComponent;
		}
	}
	return nullptr;
}

//* Sets the correct properties for the newly created component with the help of the old component
void UProceduralMeshHelper::CorrectProperties(UPrimitiveComponent* NewComponent, UPrimitiveComponent* OldComponent)
{
	NewComponent->SetRelativeTransform(OldComponent->GetRelativeTransform());
	NewComponent->RegisterComponent();
	NewComponent->SetCollisionProfileName(FName("PhysicsActor"));
	NewComponent->SetEnableGravity(true);
	NewComponent->SetSimulatePhysics(true);
//	NewComponent->bGenerateOverlapEvents = true;
	NewComponent->ComponentTags = OldComponent->ComponentTags;
}

UStaticMesh* UProceduralMeshHelper::GenerateStaticMesh(UProceduralMeshComponent* ProceduralMeshComponent, bool centerOrigin)
{
	///																   ///
	/// COPIED OVER FROM "ProceduralMeshComponentDetails.cpp l.118-212 ///
	///																   ///
	// Raw mesh data we are filling in
	FRawMesh RawMesh;
	// Materials to apply to new mesh
	TArray<UMaterialInterface*> MeshMaterials;

	FVector Origin = ProceduralMeshComponent->Bounds.Origin;
	FVector OriginLocal = ProceduralMeshComponent->GetComponentTransform().InverseTransformPosition(Origin);

	const int32 NumSections = ProceduralMeshComponent->GetNumSections();
	int32 VertexBase = 0;
	for (int32 SectionIdx = 0; SectionIdx < NumSections; SectionIdx++)
	{
		FProcMeshSection* ProcSection = ProceduralMeshComponent->GetProcMeshSection(SectionIdx);

		// Copy verts
		for (FProcMeshVertex& Vert : ProcSection->ProcVertexBuffer)
		{
			if (centerOrigin) {
				Vert.Position -= OriginLocal;
			}
			RawMesh.VertexPositions.Add(Vert.Position);
		}

		// Copy 'wedge' info
		int32 NumIndices = ProcSection->ProcIndexBuffer.Num();
		for (int32 IndexIdx = 0; IndexIdx < NumIndices; IndexIdx++)
		{
			int32 Index = ProcSection->ProcIndexBuffer[IndexIdx];

			RawMesh.WedgeIndices.Add(Index + VertexBase);

			FProcMeshVertex& ProcVertex = ProcSection->ProcVertexBuffer[Index];

			FVector TangentX = ProcVertex.Tangent.TangentX;
			FVector TangentZ = ProcVertex.Normal;
			FVector TangentY = (TangentX ^ TangentZ).GetSafeNormal() * (ProcVertex.Tangent.bFlipTangentY ? -1.f : 1.f);

			RawMesh.WedgeTangentX.Add(TangentX);
			RawMesh.WedgeTangentY.Add(TangentY);
			RawMesh.WedgeTangentZ.Add(TangentZ);

			RawMesh.WedgeTexCoords[0].Add(ProcVertex.UV0);
			RawMesh.WedgeColors.Add(ProcVertex.Color);
		}

		// copy face info
		int32 NumTris = NumIndices / 3;
		for (int32 TriIdx = 0; TriIdx < NumTris; TriIdx++)
		{
			RawMesh.FaceMaterialIndices.Add(SectionIdx);
			RawMesh.FaceSmoothingMasks.Add(0); // Assume this is ignored as bRecomputeNormals is false
		}

		// Remember material
		MeshMaterials.Add(ProceduralMeshComponent->GetMaterial(SectionIdx));

		// Update offset for creating one big index/vertex buffer
		VertexBase += ProcSection->ProcVertexBuffer.Num();
	}

	// The new StaticMesh that is going to be filled with the scanned info
	UStaticMesh* StaticMesh = NewObject<UStaticMesh>();

	// If we got some valid data.
	if (RawMesh.VertexPositions.Num() >= 3 && RawMesh.WedgeIndices.Num() >= 3)
	{
		StaticMesh->InitResources();

		StaticMesh->LightingGuid = FGuid::NewGuid();

		// Add source to new StaticMesh
		FStaticMeshSourceModel* SrcModel = new (StaticMesh->UStaticMesh::GetSourceModels()) FStaticMeshSourceModel();
		SrcModel->BuildSettings.bRecomputeNormals = false;
		SrcModel->BuildSettings.bRecomputeTangents = false;
		SrcModel->BuildSettings.bRemoveDegenerates = false;
		SrcModel->BuildSettings.bUseHighPrecisionTangentBasis = false;
		SrcModel->BuildSettings.bUseFullPrecisionUVs = false;
		SrcModel->BuildSettings.bGenerateLightmapUVs = true;
		SrcModel->BuildSettings.SrcLightmapIndex = 0;
		SrcModel->BuildSettings.DstLightmapIndex = 1;
		SrcModel->RawMeshBulkData->SaveRawMesh(RawMesh);

		// Copy materials to new mesh
		for (UMaterialInterface* Material : MeshMaterials)
		{
			StaticMesh->StaticMaterials.Add(FStaticMaterial(Material));
		}

		//Set the Imported version before calling the build
		StaticMesh->ImportVersion = EImportStaticMeshVersion::LastVersion;

		// Build mesh from source
		StaticMesh->Build();
		StaticMesh->PostEditChange();

		///
		/// END OF COPY
		///
		FKAggregateGeom CollGeom = ProceduralMeshComponent->GetBodySetup()->AggGeom;
		if (centerOrigin) {
			int32 totalElems = CollGeom.ConvexElems.Num();
			for (int32 idx = 0; idx < totalElems; idx++) {

				TArray<FVector> *vertexs = &CollGeom.ConvexElems[idx].VertexData;
				int32 totalVertexs = vertexs->Num();
				for (int32 v_idx = 0; v_idx < totalVertexs; v_idx++) {
					(*vertexs)[v_idx] -= OriginLocal;
				}
			}
		}
		StaticMesh->BodySetup->AddCollisionFrom(CollGeom);
		StaticMesh->Build();
		StaticMesh->PostEditChange();
	}

	return StaticMesh;
}

AStaticMeshActor* UProceduralMeshHelper::SpawnStaticMeshActor(UProceduralMeshComponent* ProceduralMeshComponent)
{
	FVector Location = ProceduralMeshComponent->GetAttachmentRootActor()->GetActorLocation();
	FRotator Rotation = ProceduralMeshComponent->GetAttachmentRootActor()->GetActorRotation();
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Template = ProceduralMeshComponent->GetAttachmentRootActor();
	AStaticMeshActor* StaticMeshActor =
		ProceduralMeshComponent->GetWorld()->SpawnActor<AStaticMeshActor>(Location, Rotation, SpawnInfo);

	return StaticMeshActor;
}