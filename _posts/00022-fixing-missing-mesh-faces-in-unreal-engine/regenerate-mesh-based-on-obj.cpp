UE::Geometry::FDynamicMesh3 AMyActor::RegenerateMeshBasedOnObj() {

  UE::Geometry::FDynamicMesh3 mesh;

  mesh.EnableAttributes();

  UE::Geometry::FDynamicMeshNormalOverlay *normalsOverlay =
      mesh.Attributes()->PrimaryNormals();

  const std::array vertices{
      FVector3d(1.000000, 1.000000, -1.000000),
      FVector3d(1.000000, -1.000000, -1.000000),
      FVector3d(1.000000, 1.000000, 1.000000),
      FVector3d(1.000000, -1.000000, 1.000000),
      FVector3d(-1.000000, 1.000000, -1.000000),
      FVector3d(-1.000000, -1.000000, -1.000000),
      FVector3d(-1.000000, 1.000000, 1.000000),
      FVector3d(-1.000000, -1.000000, 1.000000),
  };

  const std::array normals{
      FVector3f(-0.0000, 1.0000, -0.0000),
      FVector3f(-0.0000, -0.0000, 1.0000),
      FVector3f(-1.0000, -0.0000, -0.0000),
      FVector3f(-0.0000, -1.0000, -0.0000),
      FVector3f(1.0000, -0.0000, -0.0000),
      FVector3f(-0.0000, -0.0000, -1.0000),
  };

  for (auto vertex : vertices) {
    mesh.AppendVertex(vertex * 100);
  }

  for (auto normal : normals) {
    normalsOverlay->AppendElement(normal);
    normalsOverlay->AppendElement(normal);
    normalsOverlay->AppendElement(normal);
  }

  struct FaceIndices {
    int vertex;
    int textCoord;
    int normal;
  };

  struct Face {
    FaceIndices a, b, c, d;
  };

  struct Triangle {
    FaceIndices a, b, c;
  };

  const std::array faces{
      Face{FaceIndices{1, 1, 1}, FaceIndices{5, 2, 1}, FaceIndices{7, 3, 1},
           FaceIndices{3, 4, 1}},
      Face{FaceIndices{4, 5, 2}, FaceIndices{3, 4, 2}, FaceIndices{7, 6, 2},
           FaceIndices{8, 7, 2}},
      Face{FaceIndices{8, 8, 3}, FaceIndices{7, 9, 3}, FaceIndices{5, 10, 3},
           FaceIndices{6, 11, 3}},
      Face{FaceIndices{6, 12, 4}, FaceIndices{2, 13, 4}, FaceIndices{4, 5, 4},
           FaceIndices{8, 14, 4}},
      Face{FaceIndices{2, 13, 5}, FaceIndices{1, 1, 5}, FaceIndices{3, 4, 5},
           FaceIndices{4, 5, 5}},
      Face{FaceIndices{6, 11, 6}, FaceIndices{5, 10, 6}, FaceIndices{1, 1, 6},
           FaceIndices{2, 13, 6}},
  };

  for (Face face : faces) {
    Triangle triOne{face.a, face.b, face.c};
    Triangle triTwo{face.a, face.c, face.d};

    int32 tidOne = mesh.AppendTriangle(triOne.a.vertex - 1, triOne.b.vertex - 1,
                                       triOne.c.vertex - 1);
    int32 tidTwo = mesh.AppendTriangle(triTwo.a.vertex - 1, triTwo.b.vertex - 1,
                                       triTwo.c.vertex - 1);

    auto result = normalsOverlay->SetTriangle(
        tidOne, UE::Geometry::FIndex3i(3 * (triOne.a.normal - 1),
                                       3 * (triOne.b.normal - 1) + 1,
                                       3 * (triOne.c.normal - 1) + 2));
    result = normalsOverlay->SetTriangle(
        tidOne, UE::Geometry::FIndex3i(3 * (triTwo.a.normal - 1),
                                       3 * (triTwo.b.normal - 1) + 1,
                                       3 * (triTwo.c.normal - 1) + 2));
  }

  const auto validityResult =
      mesh.CheckValidity({}, UE::Geometry::EValidityCheckFailMode::Ensure);

  return mesh;
}