#pragma once

#include "Core/Core.h"

#include "Renderer/VertexArray.h"

#include <cstdint>
#include <vector>

#include <glm/glm.hpp>

struct MeshVertex {
	glm::vec3 Position = glm::vec3(0.0f);
	glm::vec3 Normal = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec2 UV = glm::vec2(0.0f);
	glm::vec2 UV2 = glm::vec2(0.0f);
	glm::vec4 Color = glm::vec4(1.0f);
	glm::vec4 Tangent = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
};

struct MeshSubMesh {
	uint32_t IndexStart = 0;
	uint32_t IndexCount = 0;
};

struct MeshBounds {
	glm::vec3 Center = glm::vec3(0.0f);
	glm::vec3 Size = glm::vec3(0.0f);
	glm::vec3 Min = glm::vec3(0.0f);
	glm::vec3 Max = glm::vec3(0.0f);
};

class Mesh {
public:
	Mesh() = default;
	Mesh(const std::vector<MeshVertex>& vertices, const std::vector<uint32_t>& indices = {});

	static Ref<Mesh> Create();
	static Ref<Mesh> Create(const std::vector<MeshVertex>& vertices, const std::vector<uint32_t>& indices = {});

	void Clear();

	void SetVertices(const std::vector<glm::vec3>& vertices);
	void SetNormals(const std::vector<glm::vec3>& normals);
	void SetUVs(const std::vector<glm::vec2>& uvs);
	void SetUV2s(const std::vector<glm::vec2>& uv2s);
	void SetColors(const std::vector<glm::vec4>& colors);
	void SetTangents(const std::vector<glm::vec4>& tangents);
	void SetTriangles(const std::vector<uint32_t>& indices, uint32_t subMeshIndex = 0);

	void SetSubMeshes(const std::vector<MeshSubMesh>& subMeshes);
	void ClearSubMeshes();

	const std::vector<MeshVertex>& GetVertices() const { return m_Vertices; }
	const std::vector<uint32_t>& GetTriangles() const { return m_Indices; }
	const std::vector<MeshSubMesh>& GetSubMeshes() const { return m_SubMeshes; }

	uint32_t GetVertexCount() const { return static_cast<uint32_t>(m_Vertices.size()); }
	uint32_t GetTriangleIndexCount() const { return static_cast<uint32_t>(m_Indices.size()); }
	uint32_t GetTriangleCount() const { return static_cast<uint32_t>(m_Indices.size() / 3); }
	uint32_t GetSubMeshCount() const { return static_cast<uint32_t>(m_SubMeshes.size()); }

	bool IsDirty() const { return m_IsDirty; }
	bool HasVertexData() const { return !m_Vertices.empty(); }
	bool HasIndexData() const { return !m_Indices.empty(); }

	void RecalculateNormals();
	void RecalculateBounds();
	void Upload();

	const MeshBounds& GetBounds() const { return m_Bounds; }
	const Ref<VertexArray>& GetVertexArray();

private:
	bool ValidateIndices(const std::vector<uint32_t>& indices) const;
	bool ValidateOrInitializeVertexCount(size_t count, const char* attributeName);
	std::vector<std::vector<uint32_t>> ExtractSubMeshIndexChunks() const;
	void RebuildSubMeshesFromChunks(const std::vector<std::vector<uint32_t>>& chunks);
	void MarkDirty();

private:
	std::vector<MeshVertex> m_Vertices;
	std::vector<uint32_t> m_Indices;
	std::vector<MeshSubMesh> m_SubMeshes;
	MeshBounds m_Bounds;
	Ref<VertexArray> m_VertexArray = nullptr;
	bool m_IsDirty = true;
};
