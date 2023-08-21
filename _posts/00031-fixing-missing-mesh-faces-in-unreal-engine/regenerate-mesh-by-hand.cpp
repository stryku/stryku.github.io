UE::Geometry::FDynamicMesh3 AMyActor::RegenerateMeshByHand2() {

  UE::Geometry::FDynamicMesh3 mesh;
  mesh.EnableAttributes();

  UE::Geometry::FDynamicMeshNormalOverlay *normalsOverlay =
      mesh.Attributes()->PrimaryNormals();

  const std::array vertices{FVector3d(-1, -1, -1), FVector3d(1, -1, -1),
                            FVector3d(-1, -1, 1),  FVector3d(1, -1, 1),
                            FVector3d(-1, 1, -1),  FVector3d(1, 1, -1),
                            FVector3d(-1, 1, 1),   FVector3d(1, 1, 1)};

  const std::array normals{FVector3f(0, -1, 0), FVector3f(0, 1, 0),
                           FVector3f(-1, 0, 0), FVector3f(1, 0, 0),
                           FVector3f(0, 0, -1), FVector3f(0, 0, 1)};

  [[maybe_unused]] int32 id;
  [[maybe_unused]] UE::Geometry::EMeshResult result;

  for (auto vertex : vertices) {
    mesh.AppendVertex(vertex * 100);
  }

  for (auto normal : normals) {
    normalsOverlay->AppendElement(normal);
    normalsOverlay->AppendElement(normal);
    normalsOverlay->AppendElement(normal);
  }

  // Face 0, 1. Normal 0
  id = mesh.AppendTriangle(0, 2, 3);
  result = normalsOverlay->SetTriangle(id, {0, 1, 2});

  id = mesh.AppendTriangle(0, 3, 1);
  result = normalsOverlay->SetTriangle(id, {3, 4, 5});

  // Face 2, 3. Normal 1
  id = mesh.AppendTriangle(4, 6, 7);
  result = normalsOverlay->SetTriangle(id, {1 * 3, 1 * 3 + 1, 1 * 3 + 2});

  id = mesh.AppendTriangle(4, 7, 5);
  result = normalsOverlay->SetTriangle(id, {1 * 3, 1 * 3 + 1, 1 * 3 + 2});

  // Face 4, 5. Normal 2
  id = mesh.AppendTriangle(0, 2, 6);
  result = normalsOverlay->SetTriangle(id, {2 * 3, 2 * 3 + 1, 2 * 3 + 2});

  id = mesh.AppendTriangle(0, 6, 4);
  result = normalsOverlay->SetTriangle(id, {2 * 3, 2 * 3 + 1, 2 * 3 + 2});

  // Face 6, 7. Normal 3
  id = mesh.AppendTriangle(1, 3, 7);
  result = normalsOverlay->SetTriangle(id, {3 * 3, 3 * 3 + 1, 3 * 3 + 2});

  id = mesh.AppendTriangle(1, 7, 5);
  result = normalsOverlay->SetTriangle(id, {3 * 3, 3 * 3 + 1, 3 * 3 + 2});

  // Face 8, 9. Normal 4
  id = mesh.AppendTriangle(0, 4, 5);
  result = normalsOverlay->SetTriangle(id, {4 * 3, 4 * 3 + 1, 4 * 3 + 2});

  id = mesh.AppendTriangle(0, 5, 1);
  result = normalsOverlay->SetTriangle(id, {4 * 3, 4 * 3 + 1, 4 * 3 + 2});

  // Face 10, 11. Normal 5
  id = mesh.AppendTriangle(2, 6, 7);
  result = normalsOverlay->SetTriangle(id, {5 * 3, 5 * 3 + 1, 5 * 3 + 2});

  id = mesh.AppendTriangle(2, 7, 3);
  result = normalsOverlay->SetTriangle(id, {5 * 3, 5 * 3 + 1, 5 * 3 + 2});

  return mesh;
}