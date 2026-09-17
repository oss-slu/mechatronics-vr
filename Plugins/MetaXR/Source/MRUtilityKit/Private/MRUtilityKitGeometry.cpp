// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "MRUtilityKitGeometry.h"

#include "MRUtilityKit.h"
#include "Generated/MRUtilityKitShared.h"

void MRUKTriangulatePolygon(const TArray<TArray<FVector2f>>& Polygons, TArray<FVector2D>& Vertices, TArray<int32>& Indices)
{
	Vertices.Empty();
	Indices.Empty();

	MRUKShared* SharedInstance = MRUKShared::GetInstance();
	if (!SharedInstance)
	{
		UE_LOG(LogMRUK, Error, TEXT("MRUK shared library is not available. To use this functionality make sure the library is included"));
		return;
	}

	TArray<MRUKShared::Polygon2f> ConvertedPolygons;
	ConvertedPolygons.Reserve(Polygons.Num());
	for (const TArray<FVector2f>& Polygon : Polygons)
	{
		ConvertedPolygons.Push({ Polygon.GetData(), static_cast<uint32_t>(Polygon.Num()) });
	}

	MRUKShared::Mesh2f Mesh = SharedInstance->TriangulatePolygon(ConvertedPolygons.GetData(), ConvertedPolygons.Num());

	Vertices.Reserve(Mesh.numVertices);
	Indices.Reserve(Mesh.numIndices);

	for (uint32 i = 0; i < Mesh.numVertices; ++i)
	{
		Vertices.Push(FVector2D(Mesh.vertices[i]));
	}
	for (uint32 i = 0; i < Mesh.numIndices; ++i)
	{
		Indices.Push(Mesh.indices[i]);
	}

	SharedInstance->FreeMesh(&Mesh);
}
