#pragma once
#include "yzrutil.h"
#include "../MenuRegister.hpp"
#include "synth/generators/physic/Sakura.h"



void sakuraRenderFunc(CurrentProjectContext & ctx, ProjectObject & obj);
void SakuraExciterWindow(u_sp<yzrilyzr_simplesynth::Sakura> & paramRegPtr);
void SakuraStringWindow(u_sp<yzrilyzr_simplesynth::Sakura> & paramRegPtr);
void SakuraResonatorWindow(u_sp<yzrilyzr_simplesynth::Sakura> & paramRegPtr);
void SakuraEditWindow(yzrilyzr_simplesynth::IMixer & mixer, u_sp<yzrilyzr_simplesynth::Sakura> & paramRegPtr);
