#pragma once
#include "yzrutil.h"
#include "../MenuRegister.hpp"
#include "synth/generators/physic/Sakura.h"



void sakuraRenderFunc(CurrentProjectContext & ctx, ProjectObject & obj);
void SakuraExciterWindow(std::shared_ptr<yzrilyzr_simplesynth::Sakura> & paramRegPtr);
void SakuraStringWindow(std::shared_ptr<yzrilyzr_simplesynth::Sakura> & paramRegPtr);
void SakuraResonatorWindow(std::shared_ptr<yzrilyzr_simplesynth::Sakura> & paramRegPtr);
void SakuraEditWindow(yzrilyzr_simplesynth::IMixer & mixer, std::shared_ptr<yzrilyzr_simplesynth::Sakura> & paramRegPtr);
