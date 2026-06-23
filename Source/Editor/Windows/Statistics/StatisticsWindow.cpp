#include "StatisticsWindow.h"

#include "Core/Log/Log.h"

void StatisticsWindow::OnCreate() {
	Log::Trace("StatisticsWindow::OnCreate - Creating Statistics Window");
}

void StatisticsWindow::OnAttach() {
	Log::Trace("StatisticsWindow::OnAttach - Attaching Statistics Window");
}

void StatisticsWindow::OnDetach() {
	Log::Trace("StatisticsWindow::OnDetach - Detaching Statistics Window");
}

void StatisticsWindow::OnUpdate(Timestep ts) {}

void StatisticsWindow::OnUIRender() {
	if (!m_IsOpen) {
		return;
	}

	ImGui::Begin("Statistics", &m_IsOpen);

	ImGuiIO& io = ImGui::GetIO();
	ImGui::Text("WantCaptureMouse: %d", io.WantCaptureMouse);
	ImGui::Text("WantCaptureKeyboard: %d", io.WantCaptureKeyboard);
	ImGui::Text("Mouse Position: (%.1f, %.1f)", io.MousePos.x, io.MousePos.y);

	ImGui::End();
}
