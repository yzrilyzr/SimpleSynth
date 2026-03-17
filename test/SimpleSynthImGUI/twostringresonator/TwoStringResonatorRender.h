#pragma once
#include "yzrutil.h"
#include "../MenuRegister.hpp"
#include "synth/generators/physic/TwoStringResonator.h"



void twostringresonatorRenderFunc(CurrentProjectContext & ctx, ProjectObject & obj);
void TwoStringResonatorExciterWindow(u_sp<yzrilyzr_simplesynth::TwoStringResonator> & paramRegPtr);
void TwoStringResonatorStringWindow(u_sp<yzrilyzr_simplesynth::TwoStringResonator> & paramRegPtr);
void TwoStringResonatorResonatorWindow(u_sp<yzrilyzr_simplesynth::TwoStringResonator> & paramRegPtr);
void TwoStringResonatorEditWindow(yzrilyzr_simplesynth::IMixer & mixer, u_sp<yzrilyzr_simplesynth::TwoStringResonator> & paramRegPtr);
