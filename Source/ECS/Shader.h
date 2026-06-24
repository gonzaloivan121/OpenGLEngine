#pragma once

#include <filesystem>

struct Shader {
	std::filesystem::path VertexFilepath;
	std::filesystem::path FragmentFilepath;
};