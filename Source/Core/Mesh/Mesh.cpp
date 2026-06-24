#include "Mesh.h"

#include "Core/Log/Log.h"

#include "Renderer/BufferLayout.h"
#include "Renderer/IndexBuffer.h"
#include "Renderer/VertexBuffer.h"

#include <algorithm>
#include <limits>
#include <string>

Mesh::Mesh(const std::vector<MeshVertex>& vertices, const std::vector<uint32_t>& indices)
	: m_Vertices(vertices), m_Indices(indices)
{
	if (!m_Indices.empty()) {
		m_SubMeshes.push_back({ 0, static_cast<uint32_t>(m_Indices.size()) });
	}

	RecalculateBounds();
	MarkDirty();
}

Ref<Mesh> Mesh::Create() {
	return CreateRef<Mesh>();
}

Ref<Mesh> Mesh::Create(const std::vector<MeshVertex>& vertices, const std::vector<uint32_t>& indices) {
	return CreateRef<Mesh>(vertices, indices);
}

void Mesh::Clear() {
	m_Vertices.clear();
	m_Indices.clear();
	m_SubMeshes.clear();
	m_Bounds = {};
	m_VertexArray.reset();
	MarkDirty();
}

void Mesh::SetVertices(const std::vector<glm::vec3>& vertices) {
	m_Vertices.resize(vertices.size());

	for (size_t i = 0; i < vertices.size(); ++i) {
		m_Vertices[i].Position = vertices[i];
	}

	RecalculateBounds();
	MarkDirty();
}

void Mesh::SetNormals(const std::vector<glm::vec3>& normals) {
	if (!ValidateOrInitializeVertexCount(normals.size(), "normals")) {
		return;
	}

	for (size_t i = 0; i < normals.size(); ++i) {
		const float normalLength = glm::length(normals[i]);
		if (normalLength > std::numeric_limits<float>::epsilon()) {
			m_Vertices[i].Normal = normals[i] / normalLength;
		}
		else {
			m_Vertices[i].Normal = glm::vec3(0.0f, 1.0f, 0.0f);
		}
	}

	MarkDirty();
}

void Mesh::SetUVs(const std::vector<glm::vec2>& uvs) {
	if (!ValidateOrInitializeVertexCount(uvs.size(), "uvs")) {
		return;
	}

	for (size_t i = 0; i < uvs.size(); ++i) {
		m_Vertices[i].UV = uvs[i];
	}

	MarkDirty();
}

void Mesh::SetUV2s(const std::vector<glm::vec2>& uv2s) {
	if (!ValidateOrInitializeVertexCount(uv2s.size(), "uv2s")) {
		return;
	}

	for (size_t i = 0; i < uv2s.size(); ++i) {
		m_Vertices[i].UV2 = uv2s[i];
	}

	MarkDirty();
}

void Mesh::SetColors(const std::vector<glm::vec4>& colors) {
	if (!ValidateOrInitializeVertexCount(colors.size(), "colors")) {
		return;
	}

	for (size_t i = 0; i < colors.size(); ++i) {
		m_Vertices[i].Color = colors[i];
	}

	MarkDirty();
}

void Mesh::SetTangents(const std::vector<glm::vec4>& tangents) {
	if (!ValidateOrInitializeVertexCount(tangents.size(), "tangents")) {
		return;
	}

	for (size_t i = 0; i < tangents.size(); ++i) {
		m_Vertices[i].Tangent = tangents[i];
	}

	MarkDirty();
}

void Mesh::SetTriangles(const std::vector<uint32_t>& indices, uint32_t subMeshIndex) {
	if (!ValidateIndices(indices)) {
		return;
	}

	if (m_SubMeshes.empty()) {
		if (!m_Indices.empty()) {
			m_SubMeshes.push_back({ 0, static_cast<uint32_t>(m_Indices.size()) });
		}
		else {
			m_SubMeshes.push_back({});
		}
	}

	if (subMeshIndex >= m_SubMeshes.size()) {
		m_SubMeshes.resize(static_cast<size_t>(subMeshIndex) + 1);
	}

	std::vector<std::vector<uint32_t>> chunks = ExtractSubMeshIndexChunks();
	if (subMeshIndex >= chunks.size()) {
		chunks.resize(static_cast<size_t>(subMeshIndex) + 1);
	}

	chunks[subMeshIndex] = indices;
	RebuildSubMeshesFromChunks(chunks);

	MarkDirty();
}

void Mesh::SetSubMeshes(const std::vector<MeshSubMesh>& subMeshes) {
	for (const MeshSubMesh& subMesh : subMeshes) {
		const uint64_t indexEnd = static_cast<uint64_t>(subMesh.IndexStart) + static_cast<uint64_t>(subMesh.IndexCount);
		if (indexEnd > m_Indices.size()) {
			Log::Error("Mesh::SetSubMeshes - One or more submesh ranges are out of bounds.");
			return;
		}

		if ((subMesh.IndexCount % 3) != 0) {
			Log::Warning("Mesh::SetSubMeshes - Submesh index count is not a multiple of 3.");
		}
	}

	m_SubMeshes = subMeshes;
	MarkDirty();
}

void Mesh::ClearSubMeshes() {
	m_SubMeshes.clear();
	if (!m_Indices.empty()) {
		m_SubMeshes.push_back({ 0, static_cast<uint32_t>(m_Indices.size()) });
	}

	MarkDirty();
}

void Mesh::RecalculateNormals() {
	if (m_Vertices.empty()) {
		Log::Warning("Mesh::RecalculateNormals - Cannot recalculate normals on an empty mesh.");
		return;
	}

	for (auto& vertex : m_Vertices) {
		vertex.Normal = glm::vec3(0.0f);
	}

	if (!m_Indices.empty()) {
		const size_t triangleIndexCount = m_Indices.size() - (m_Indices.size() % 3);
		for (size_t i = 0; i < triangleIndexCount; i += 3) {
			const uint32_t i0 = m_Indices[i + 0];
			const uint32_t i1 = m_Indices[i + 1];
			const uint32_t i2 = m_Indices[i + 2];

			const glm::vec3& p0 = m_Vertices[i0].Position;
			const glm::vec3& p1 = m_Vertices[i1].Position;
			const glm::vec3& p2 = m_Vertices[i2].Position;

			const glm::vec3 faceNormal = glm::cross(p1 - p0, p2 - p0);

			m_Vertices[i0].Normal += faceNormal;
			m_Vertices[i1].Normal += faceNormal;
			m_Vertices[i2].Normal += faceNormal;
		}
	}
	else {
		const size_t triangleVertexCount = m_Vertices.size() - (m_Vertices.size() % 3);
		for (size_t i = 0; i < triangleVertexCount; i += 3) {
			const glm::vec3& p0 = m_Vertices[i + 0].Position;
			const glm::vec3& p1 = m_Vertices[i + 1].Position;
			const glm::vec3& p2 = m_Vertices[i + 2].Position;

			const glm::vec3 faceNormal = glm::cross(p1 - p0, p2 - p0);

			m_Vertices[i + 0].Normal += faceNormal;
			m_Vertices[i + 1].Normal += faceNormal;
			m_Vertices[i + 2].Normal += faceNormal;
		}
	}

	for (auto& vertex : m_Vertices) {
		const float normalLength = glm::length(vertex.Normal);
		if (normalLength > std::numeric_limits<float>::epsilon()) {
			vertex.Normal /= normalLength;
		}
		else {
			vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
		}
	}

	MarkDirty();
}

void Mesh::RecalculateBounds() {
	if (m_Vertices.empty()) {
		m_Bounds = {};
		return;
	}

	glm::vec3 minPoint = m_Vertices.front().Position;
	glm::vec3 maxPoint = m_Vertices.front().Position;

	for (const MeshVertex& vertex : m_Vertices) {
		minPoint = glm::min(minPoint, vertex.Position);
		maxPoint = glm::max(maxPoint, vertex.Position);
	}

	m_Bounds.Min = minPoint;
	m_Bounds.Max = maxPoint;
	m_Bounds.Size = maxPoint - minPoint;
	m_Bounds.Center = minPoint + (m_Bounds.Size * 0.5f);
}

void Mesh::Upload() {
	if (m_Vertices.empty()) {
		Log::Warning("Mesh::Upload - Mesh has no vertices. Upload skipped.");
		m_VertexArray.reset();
		m_IsDirty = false;
		return;
	}

	std::vector<uint32_t> uploadIndices = m_Indices;
	if (uploadIndices.empty()) {
		uploadIndices.resize(m_Vertices.size());
		for (uint32_t i = 0; i < uploadIndices.size(); ++i) {
			uploadIndices[i] = i;
		}

		m_SubMeshes.clear();
		m_SubMeshes.push_back({ 0, static_cast<uint32_t>(uploadIndices.size()) });
	}
	else if (m_SubMeshes.empty()) {
		m_SubMeshes.push_back({ 0, static_cast<uint32_t>(uploadIndices.size()) });
	}

	std::vector<float> interleaved;
	interleaved.reserve(m_Vertices.size() * 18);

	for (const MeshVertex& vertex : m_Vertices) {
		interleaved.push_back(vertex.Position.x);
		interleaved.push_back(vertex.Position.y);
		interleaved.push_back(vertex.Position.z);

		interleaved.push_back(vertex.UV.x);
		interleaved.push_back(vertex.UV.y);

		interleaved.push_back(vertex.Normal.x);
		interleaved.push_back(vertex.Normal.y);
		interleaved.push_back(vertex.Normal.z);

		interleaved.push_back(vertex.UV2.x);
		interleaved.push_back(vertex.UV2.y);

		interleaved.push_back(vertex.Color.r);
		interleaved.push_back(vertex.Color.g);
		interleaved.push_back(vertex.Color.b);
		interleaved.push_back(vertex.Color.a);

		interleaved.push_back(vertex.Tangent.x);
		interleaved.push_back(vertex.Tangent.y);
		interleaved.push_back(vertex.Tangent.z);
		interleaved.push_back(vertex.Tangent.w);
	}

	Ref<VertexArray> vertexArray = VertexArray::Create();
	Ref<VertexBuffer> vertexBuffer = VertexBuffer::Create(interleaved.data(), static_cast<uint32_t>(interleaved.size() * sizeof(float)));
	vertexBuffer->SetLayout({
		{ ShaderDataType::Float3, "a_Position" },
		{ ShaderDataType::Float2, "a_TexCoord" },
		{ ShaderDataType::Float3, "a_Normal" },
		{ ShaderDataType::Float2, "a_TexCoord2" },
		{ ShaderDataType::Float4, "a_Color" },
		{ ShaderDataType::Float4, "a_Tangent" }
	});

	Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(uploadIndices.data(), static_cast<uint32_t>(uploadIndices.size()));

	vertexArray->AddVertexBuffer(vertexBuffer);
	vertexArray->SetIndexBuffer(indexBuffer);

	m_VertexArray = vertexArray;
	m_IsDirty = false;
}

const Ref<VertexArray>& Mesh::GetVertexArray() {
	if (m_IsDirty) {
		Upload();
	}

	return m_VertexArray;
}

bool Mesh::ValidateOrInitializeVertexCount(size_t count, const char* attributeName) {
	if (count == 0) {
		Log::Warning(std::string("Mesh::Set") + attributeName + " - Empty attribute array provided.");
		return false;
	}

	if (m_Vertices.empty()) {
		m_Vertices.resize(count);
		RecalculateBounds();
		return true;
	}

	if (count != m_Vertices.size()) {
		Log::Error(
			std::string("Mesh::Set") + attributeName + " - Attribute count mismatch. Expected " +
			std::to_string(m_Vertices.size()) + ", got " + std::to_string(count) + "."
		);
		return false;
	}

	return true;
}

bool Mesh::ValidateIndices(const std::vector<uint32_t>& indices) const {
	if (indices.size() % 3 != 0) {
		Log::Warning("Mesh::SetTriangles - Index count is not a multiple of 3. Extra indices will be ignored by triangle rendering.");
	}

	for (uint32_t index : indices) {
		if (index >= m_Vertices.size()) {
			Log::Error(
				"Mesh::SetTriangles - Index " + std::to_string(index) +
				" is out of range for " + std::to_string(m_Vertices.size()) + " vertices."
			);
			return false;
		}
	}

	return true;
}

std::vector<std::vector<uint32_t>> Mesh::ExtractSubMeshIndexChunks() const {
	std::vector<std::vector<uint32_t>> chunks;
	chunks.reserve(m_SubMeshes.size());

	for (const MeshSubMesh& subMesh : m_SubMeshes) {
		std::vector<uint32_t> indices;
		if (subMesh.IndexCount > 0 && (subMesh.IndexStart + subMesh.IndexCount) <= m_Indices.size()) {
			indices.insert(
				indices.end(),
				m_Indices.begin() + subMesh.IndexStart,
				m_Indices.begin() + subMesh.IndexStart + subMesh.IndexCount
			);
		}

		chunks.push_back(std::move(indices));
	}

	return chunks;
}

void Mesh::RebuildSubMeshesFromChunks(const std::vector<std::vector<uint32_t>>& chunks) {
	m_Indices.clear();
	m_SubMeshes.clear();
	m_SubMeshes.reserve(chunks.size());

	for (const auto& chunk : chunks) {
		MeshSubMesh subMesh;
		subMesh.IndexStart = static_cast<uint32_t>(m_Indices.size());
		subMesh.IndexCount = static_cast<uint32_t>(chunk.size());

		m_Indices.insert(m_Indices.end(), chunk.begin(), chunk.end());
		m_SubMeshes.push_back(subMesh);
	}
}

void Mesh::MarkDirty() {
	m_IsDirty = true;
}
