#include "imgui.h"
#include "SimpleSynthProject.h"
void CurrentProjectContext::HandleShortcuts(){
    ImGuiIO & io=ImGui::GetIO();
    // Ctrl+S: Save
    if(io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false)){
        saveFile();
    }
    // Ctrl+C: 复制
    if(io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_C)){
        copySelected();
    }
    // Ctrl+V: 粘贴
    if(io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_V)){
        pasteSelected();
    }
    // Ctrl+D: Duplicate
    if(io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_D)){
        duplicateSelected();
    }
    // Ctrl+B: Save as sub
    if(io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_B)){
        saveAsSub(true);
    }
    // Delete: 删除
    if(ImGui::IsKeyPressed(ImGuiKey_Delete)){
        deleteSelected();
    }

}