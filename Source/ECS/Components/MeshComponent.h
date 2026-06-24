#pragma once

#include "ECS/Component.h"

#include "Core/Core.h"

#include <filesystem>

class Mesh;

struct MeshComponent : Component {
	Ref<Mesh> Mesh;
	std::filesystem::path MeshFilepath;
	
	bool CastShadows = true;
	bool ReceiveShadows = true;
};