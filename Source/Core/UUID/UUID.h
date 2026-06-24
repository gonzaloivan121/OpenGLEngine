#pragma once

#include <cstdint>
#include <random>
#include <functional>

class UUID {
public:
	UUID();
	explicit UUID(uint64_t uuid) : m_UUID(uuid) {}

	operator uint64_t() const { return m_UUID; }
	bool operator==(const UUID& other) const { return m_UUID == other.m_UUID; }
	bool operator!=(const UUID& other) const { return m_UUID != other.m_UUID; }
private:
	uint64_t m_UUID;
};

// Allow using UUID as a key in std::unordered_map
namespace std {
	template<>
	struct hash<UUID> {
		size_t operator()(const UUID& uuid) const noexcept {
			return hash<uint64_t>()(static_cast<uint64_t>(uuid));
		}
	};
}