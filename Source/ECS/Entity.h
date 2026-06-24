#pragma once

#include "Core/UUID/UUID.h"

class Scene;

struct Entity {
	UUID ID = UUID(0);

	Entity() = default;
	Entity(UUID id, Scene* scene) : ID(id), m_Scene(scene) {}

	void SetScene(Scene* scene) { m_Scene = scene; }

	bool IsValid() const;

	template<typename T>
	T* GetComponent();

	template<typename T>
	const T* GetComponent() const;

	template<typename T, typename... TArgs>
	T& AddComponent(TArgs&&... args);

	template<typename T>
	bool HasComponent() const;

	template<typename T>
	void RemoveComponent();

	operator bool() const { return IsValid(); }
private:
	Scene* m_Scene = nullptr;
};