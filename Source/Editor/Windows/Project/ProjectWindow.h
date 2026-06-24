#pragma once

#include "Editor/Window.h"
#include "Editor/Payload.h"
#include "Editor/UI.h"

#include "Renderer/Texture.h"

#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <vector>

class ProjectWindow : public Window {
public:
	using SceneLoadCallback = std::function<void(const std::filesystem::path&)>;
	using SceneNewCallback = std::function<void(const std::filesystem::path&)>;

	using MaterialNewCallback = std::function<void(const std::filesystem::path&)>;

	ProjectWindow(bool& isOpen) : Window(isOpen) {}

	virtual void OnCreate() override;
	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnUpdate(Timestep ts) override;
	virtual void OnUIRender() override;

	void SetSceneLoadCallback(const SceneLoadCallback& callback) { m_SceneLoadCallback = callback; }
	void SetSceneNewCallback(const SceneNewCallback& callback) { m_SceneNewCallback = callback; }

	void SetMaterialNewCallback(const MaterialNewCallback& callback) { m_MaterialNewCallback = callback; }

private:
	struct DragDropSourceType {
		std::string Extension;
		Payload::Type PayloadType;
		const char* Label;
	};

	// Drawing
	void DrawFolderContent();
	void DrawBreadcrumbs();
	void DrawItem(const std::filesystem::directory_entry& entry);
	void DrawItemLabel(const std::filesystem::path& path);

	// Context menus & modals
	void DrawBackgroundContextMenu();
	void DrawItemContextMenu(const std::filesystem::path& path);
	void DrawDeleteModal();

	void DrawIconSizeSlider();

	// File operations
	void CreateFolder(const std::filesystem::path& directory);
	void CreateScene(const std::filesystem::path& directory);
	void CreateMaterial(const std::filesystem::path& directory);
	void DeleteItem(const std::filesystem::path& path);
	void RenameItem(const std::filesystem::path& oldPath, const std::string& newStem);
	void DuplicateItem(const std::filesystem::path& path);
	void MoveItem(const std::filesystem::path& source, const std::filesystem::path& targetDir);
	void OpenInExplorer(const std::filesystem::path& path);

	// Drag & Drop
	void HandleDragDropSource(const std::filesystem::directory_entry& entry);
	void HandleDragDropTarget(const std::filesystem::path& targetDir);
	void DrawDragDropSourcePreview(const std::filesystem::directory_entry& entry, const DragDropSourceType& type);
	void RegisterDragDropSourceType(std::string extension, Payload::Type payloadType, const char* label);
	const DragDropSourceType* ResolveDragDropSourceType(const std::filesystem::directory_entry& entry) const;

	// Helpers
	Ref<Texture2D> GetIconForPayloadType(Payload::Type payloadType) const;
	Ref<Texture2D> GetIconForEntry(const std::filesystem::directory_entry& entry) const;
	std::filesystem::path GenerateUniqueFilename(const std::filesystem::path& preferred) const;
	bool IsInsideAssetsDirectory(const std::filesystem::path& path) const;
	bool IsAncestorOf(const std::filesystem::path& ancestor, const std::filesystem::path& descendant) const;
	void BeginRenaming(const std::filesystem::path& path, const std::string& initialText);
	static bool IsValidFilename(const std::string& name);

private:
	const std::filesystem::path m_AssetsDirectory = "Assets";
	std::filesystem::path m_CurrentDirectory = m_AssetsDirectory;

	Ref<Texture2D> m_FullFolderIcon;
	Ref<Texture2D> m_EmptyFolderIcon;
	Ref<Texture2D> m_FileIcon;
	Ref<Texture2D> m_SceneIcon;
	Ref<Texture2D> m_MeshIcon;
	Ref<Texture2D> m_MaterialIcon;
	Ref<Texture2D> m_AudioIcon;
	Ref<Texture2D> m_ShaderIcon;
	std::vector<DragDropSourceType> m_DragDropSourceTypes;

	float m_Padding = 24.0f;
	float m_ThumbnailSize = 96.0f;

	// Rename state
	std::optional<std::filesystem::path> m_RenamingPath;
	char m_RenameBuffer[256] = {};
	bool m_FocusRenameInput = false;

	// Delete state
	std::optional<std::filesystem::path> m_PendingDeletePath;
	bool m_OpenDeleteModal = false;

	// Selection
	std::optional<std::filesystem::path> m_SelectedPath;

	SceneLoadCallback m_SceneLoadCallback;
	SceneNewCallback m_SceneNewCallback;

	MaterialNewCallback m_MaterialNewCallback;
};