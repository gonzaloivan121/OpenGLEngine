#pragma once

#include "ECS/Entity.h"

#include "ECS/Components/NameComponent.h"
#include "ECS/Components/TransformComponent.h"

#include <algorithm>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

class Scene {
public:
	Entity CreateEntity(const UUID id) {
		m_Entities.push_back(id);

		Entity entity;
		entity.ID = id;
		entity.SetScene(this);
		entity.AddComponent<NameComponent>();
		entity.AddComponent<TransformComponent>();

		return entity;
	}

	Entity CreateEntity(const std::string& name = "Entity") {
		const UUID id;

		m_Entities.push_back(id);

		Entity entity;
		entity.ID = id;
		entity.SetScene(this);
		auto& nameComponent = entity.AddComponent<NameComponent>();
		nameComponent.Name = name;
		entity.AddComponent<TransformComponent>();

		return entity;
	}

	void DestroyEntity(UUID id) {
		m_Entities.erase(std::remove(m_Entities.begin(), m_Entities.end(), id), m_Entities.end());

		for (auto& [_, pool] : m_ComponentPools) {
			pool->Remove(id);
		}
	}

	void DestroyAllEntities() {
		m_Entities.clear();
		m_ComponentPools.clear();
	}

	bool IsValidEntity(UUID id) const {
		return std::find(m_Entities.begin(), m_Entities.end(), id) != m_Entities.end();
	}

	Entity GetEntity(UUID id) {
		if (!IsValidEntity(id)) {
			return Entity();
		}

		Entity entity;
		entity.ID = id;
		entity.SetScene(this);
		return entity;
	}

	std::vector<UUID>& GetEntityIDs() {
		return m_Entities;
	}

	const std::vector<UUID>& GetEntityIDs() const {
		return m_Entities;
	}

	template<typename T, typename... TArgs>
	T& AddComponent(UUID id, TArgs&&... args) {
		auto& pool = GetOrCreatePool<T>();
		auto [it, inserted] = pool.Components.try_emplace(id, std::forward<TArgs>(args)...);

		if (!inserted) {
			it->second = T(std::forward<TArgs>(args)...);
		}

		return it->second;
	}

	template<typename T>
	bool HasComponent(UUID id) const {
		auto it = m_ComponentPools.find(std::type_index(typeid(T)));
		if (it == m_ComponentPools.end()) {
			return false;
		}

		auto* pool = static_cast<const ComponentPool<T>*>(it->second.get());
		return pool->Components.find(id) != pool->Components.end();
	}

	template<typename T>
	T* GetComponent(UUID id) {
		auto it = m_ComponentPools.find(std::type_index(typeid(T)));
		if (it == m_ComponentPools.end()) {
			return nullptr;
		}

		auto* pool = static_cast<ComponentPool<T>*>(it->second.get());
		auto componentIt = pool->Components.find(id);
		if (componentIt == pool->Components.end()) {
			return nullptr;
		}

		return &componentIt->second;
	}

	template<typename T>
	const T* GetComponent(UUID id) const {
		auto it = m_ComponentPools.find(std::type_index(typeid(T)));
		if (it == m_ComponentPools.end()) {
			return nullptr;
		}

		auto* pool = static_cast<const ComponentPool<T>*>(it->second.get());
		auto componentIt = pool->Components.find(id);
		if (componentIt == pool->Components.end()) {
			return nullptr;
		}

		return &componentIt->second;
	}

	template<typename T>
	void RemoveComponent(UUID id) {
		auto it = m_ComponentPools.find(std::type_index(typeid(T)));
		if (it == m_ComponentPools.end()) {
			return;
		}

		auto* pool = static_cast<ComponentPool<T>*>(it->second.get());
		pool->Components.erase(id);
	}
private:
	struct IComponentPool {
		virtual ~IComponentPool() = default;
		virtual void Remove(UUID id) = 0;
	};

	template<typename T>
	struct ComponentPool : IComponentPool {
		std::unordered_map<UUID, T> Components;

		virtual void Remove(UUID id) override {
			Components.erase(id);
		}
	};

	template<typename T>
	ComponentPool<T>& GetOrCreatePool() {
		const auto key = std::type_index(typeid(T));
		auto it = m_ComponentPools.find(key);

		if (it == m_ComponentPools.end()) {
			auto pool = std::make_unique<ComponentPool<T>>();
			auto* ptr = pool.get();
			m_ComponentPools.emplace(key, std::move(pool));
			return *ptr;
		}

		return *static_cast<ComponentPool<T>*>(it->second.get());
	}
private:
	std::vector<UUID> m_Entities;
	std::unordered_map<std::type_index, std::unique_ptr<IComponentPool>> m_ComponentPools;
};

inline bool Entity::IsValid() const {
	return m_Scene != nullptr && m_Scene->IsValidEntity(ID);
}

template<typename T>
inline T* Entity::GetComponent() {
	if (m_Scene == nullptr) {
		return nullptr;
	}

	return m_Scene->GetComponent<T>(ID);
}

template<typename T>
inline const T* Entity::GetComponent() const {
	if (m_Scene == nullptr) {
		return nullptr;
	}

	return m_Scene->GetComponent<T>(ID);
}

template<typename T, typename... TArgs>
inline T& Entity::AddComponent(TArgs&&... args) {
	return m_Scene->AddComponent<T>(ID, std::forward<TArgs>(args)...);
}

template<typename T>
inline bool Entity::HasComponent() const {
	if (m_Scene == nullptr) {
		return false;
	}

	return m_Scene->HasComponent<T>(ID);
}

template<typename T>
inline void Entity::RemoveComponent() {
	if (m_Scene == nullptr) {
		return;
	}

	m_Scene->RemoveComponent<T>(ID);
}