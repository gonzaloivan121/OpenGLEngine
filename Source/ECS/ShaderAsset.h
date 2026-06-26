#pragma once

#include <filesystem>

struct ShaderAsset {
	std::filesystem::path VertexFilepath;
	std::filesystem::path FragmentFilepath;
};