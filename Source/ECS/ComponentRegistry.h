#pragma once

#include "ECS/Entity.h"

#include <functional>
#include <string>
#include <typeindex>
#include <vector>

struct ComponentRegistryEntry {
	std::string Name;
	std::type_index Type;
	std::function<bool(const Entity&)> Has;
	std::function<void(Entity&)> Add;
	std::function<void(Entity&)> Remove;
};

class ComponentRegistry {
public:
	template<typename T>
	static void Register(const std::string& name) {
		auto& entries = GetMutableEntries();

		for (const auto& entry : entries) {
			if (entry.Type == std::type_index(typeid(T))) {
				return;
			}
		}

		entries.push_back({
			name,
			std::type_index(typeid(T)),
			[](const Entity& entity) {
				return entity.HasComponent<T>();
			},
			[](Entity& entity) {
				if (!entity.HasComponent<T>()) {
					entity.AddComponent<T>();
				}
			},
			[](Entity& entity) {
				entity.RemoveComponent<T>();
			}
		});
	}

	static const std::vector<ComponentRegistryEntry>& Entries() {
		return GetMutableEntries();
	}
private:
	static std::vector<ComponentRegistryEntry>& GetMutableEntries() {
		static std::vector<ComponentRegistryEntry> s_Entries;
		return s_Entries;
	}
};
