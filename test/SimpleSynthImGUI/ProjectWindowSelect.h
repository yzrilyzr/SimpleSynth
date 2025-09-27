#pragma once
#include "SimpleSynthProject.h"
#include "imgui.h"

bool IsMouseOverAnyWindowTitlebar(yzrilyzr_collection::ArrayList<ProjectObject *> & objects);
bool IsMouseOverWindowTitlebar(const ProjectObject * obj);
void DrawSelectionBox(SelectionBox & selection, yzrilyzr_collection::ArrayList<ProjectObject *> & objects);
void HandleSelection(SelectionBox & selection, yzrilyzr_collection::ArrayList<ProjectObject *> & objects);
void HandleSelectedWindows(yzrilyzr_collection::ArrayList<ProjectObject *> & objects);
void AlignSelectedWindows(yzrilyzr_collection::ArrayList<ProjectObject *> & objects);