#include "ProjectWindowSelect.h"
#include "collection/ArrayList.hpp"
#include "imgui_internal.h"
using namespace yzrilyzr_collection;
// 检查鼠标是否在窗口标题栏上
bool IsMouseOverWindowTitlebar(const ProjectObject * obj){
	ImVec2 mousePos=ImGui::GetIO().MousePos;
	float titlebarHeight=ImGui::GetFrameHeight(); // 标题栏高度

	return (mousePos.x >= obj->windowPos.x &&
			mousePos.x <= obj->windowPos.x + obj->windowSize.x &&
			mousePos.y >= obj->windowPos.y &&
			mousePos.y <= obj->windowPos.y + titlebarHeight);
}
// 检查鼠标是否在任何窗口的标题栏上
bool IsMouseOverAnyWindowTitlebar(ArrayList<ProjectObject *> & objects){
	for(const auto * obj : objects){
		if(!obj->showWindow) continue;
		if(IsMouseOverWindowTitlebar(obj)){
			return true;
		}
	}
	return false;
}
void CurrentProjectContext::HandleSelectionAndDrag(){
	ImGuiIO & io=ImGui::GetIO();
	ImDrawList * drawList=ImGui::GetForegroundDrawList();

	static std::vector<ImVec2> originalPositions;
	static ProjectObject * draggedWindow=nullptr;

	// 检查是否有窗口正在调整大小
	bool isAnyWindowResizing=false;
	for(const auto * obj : objects){
		if(ImGui::IsItemActive() && ImGui::IsMouseDragging(0)){
			isAnyWindowResizing=true;
			break;
		}
	}

	// 如果正在调整窗口大小，禁用所有框选和拖动功能
	if(isAnyWindowResizing){
		selection.isSelecting=false;
		selection.isDragging=false;
		draggedWindow=nullptr;
		return;
	}

	// 处理点击
	if(ImGui::IsMouseClicked(ImGuiMouseButton_Left)){
		bool clickedTitlebar=false;
		draggedWindow=nullptr;

		// 检查是否点击了标题栏
		for(auto * obj : objects){
			if(!obj->showWindow) continue;

			if(IsMouseOverWindowTitlebar(obj)){
				clickedTitlebar=true;
				draggedWindow=obj;

				if(!obj->isSelected){
					if(!io.KeyCtrl){
						for(auto * other : objects){
							other->isSelected=false;
						}
					}
					obj->isSelected=true;
				}
				break;
			}
		}

		if(!clickedTitlebar && !ImGui::IsAnyItemHovered()){
			// 只有在没有任何ImGui元素被悬停时才开始框选
			selection.start=io.MousePos;
			selection.isSelecting=true;
			selection.isDragging=false;

			if(!io.KeyCtrl){
				for(auto * obj : objects){
					obj->isSelected=false;
				}
			}
		} else if(clickedTitlebar){
			selection.dragStartPos=io.MousePos;
			originalPositions.clear();

			for(const auto * obj : objects){
				if(obj->isSelected){
					originalPositions.push_back(obj->windowPos);
				}
			}
		}
	}

	// 处理拖动
	if(ImGui::IsMouseDown(ImGuiMouseButton_Left) && draggedWindow != nullptr){
		ImVec2 dragDelta=ImVec2(
			io.MousePos.x - selection.dragStartPos.x,
			io.MousePos.y - selection.dragStartPos.y
		);

		if(!selection.isDragging &&
		   (abs(dragDelta.x) > 3.0f || abs(dragDelta.y) > 3.0f)){
			selection.isDragging=true;
		}

		if(selection.isDragging){
			size_t posIndex=0;
			for(auto * obj : objects){
				if(obj->isSelected && posIndex < originalPositions.size()){
					obj->windowPos=ImVec2(
						originalPositions[posIndex].x + dragDelta.x,
						originalPositions[posIndex].y + dragDelta.y
					);
					posIndex++;
				}
			}
		}
	}

	// 处理框选
	if(selection.isSelecting && !ImGui::IsAnyItemActive()){
		selection.current=io.MousePos;

		selection.isLeftToRight=(selection.current.x >= selection.start.x &&
								 selection.current.y >= selection.start.y);

		ImVec2 min=ImVec2(ImMin(selection.start.x, selection.current.x),
						  ImMin(selection.start.y, selection.current.y));
		ImVec2 max=ImVec2(ImMax(selection.start.x, selection.current.x),
						  ImMax(selection.start.y, selection.current.y));

		ImU32 selectColor=selection.isLeftToRight?
			IM_COL32(0, 255, 0, 50):IM_COL32(255, 165, 0, 50);

		drawList->AddRectFilled(min, max, selectColor);
		drawList->AddRect(min, max, IM_COL32(255, 255, 255, 255));

		if(ImGui::IsMouseReleased(ImGuiMouseButton_Left)){
			selection.isSelecting=false;

			for(auto * obj : objects){
				if(!obj->showWindow) continue;

				ImVec2 winPos=obj->windowPos;
				ImVec2 winMax=ImVec2(winPos.x + obj->windowSize.x,
									 winPos.y + obj->windowSize.y);

				bool isSelected=false;
				if(selection.isLeftToRight){
					isSelected=
						winPos.x >= min.x && winPos.y >= min.y &&
						winMax.x <= max.x && winMax.y <= max.y;
				} else{
					isSelected=
						!(winPos.x > max.x || winMax.x < min.x ||
						  winPos.y > max.y || winMax.y < min.y);
				}

				if(isSelected){
					obj->isSelected=true;
				}
			}
		}
	}

	// 结束拖动
	if(ImGui::IsMouseReleased(ImGuiMouseButton_Left)){
		selection.isDragging=false;
		draggedWindow=nullptr;
		originalPositions.clear();
	}
}