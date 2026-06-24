#include "UUID.h"

static std::random_device s_RandomDevice;
static std::mt19937_64    s_Engine(s_RandomDevice());
static std::uniform_int_distribution<uint64_t> s_UniformDist;

UUID::UUID() : m_UUID(s_UniformDist(s_Engine)) {}
