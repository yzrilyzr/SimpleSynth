#include "SimpleSynthProject.h"
void CurrentProjectContext::ShowContextMenu(){
	// 检查是否点击右键
	if(ImGui::IsMouseClicked(ImGuiMouseButton_Right)){
		ImGui::OpenPopup("ContextMenu");
	}

	// 右键菜单
	if(ImGui::BeginPopup("ContextMenu")){
		ImVec2 mousePos=ImGui::GetIO().MousePos;
		bool hasSelection=false;
		for(auto * obj : objects){
			if(obj->isSelected){
				hasSelection=true;
				break;
			}
		}
		if(ImGui::MenuItem(LANG.getc("right_context_menu.copy"), "Ctrl+C", false, hasSelection)){
			copySelected();
		}
		if(ImGui::MenuItem(LANG.getc("right_context_menu.paste"), "Ctrl+V", false, !clipboardObjects.isEmpty())){
			pasteSelected();
		}
		if(ImGui::MenuItem(LANG.getc("right_context_menu.duplicate"), "Ctrl+D", false, hasSelection)){
			duplicateSelected();
		}
		if(ImGui::MenuItem(LANG.getc("right_context_menu.delete"), "Del", false, hasSelection)){
			deleteSelected();
		}
		ImGui::Separator();
		if(ImGui::MenuItem(LANG.getc("right_context_menu.save_as_sub"), "Ctrl+B", false, hasSelection)){
			saveAsSub(true);
		}
		ImGui::EndPopup();
	}
}