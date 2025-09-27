#pragma once
#include "MenuRegister.hpp"
#include "util/Lang.h"

void registerAllNoteProcessor(yzrilyzr_util::Lang & lang, MenuRegister & reg);
void registerAllDSP(yzrilyzr_util::Lang & lang, MenuRegister & reg);
void registerAllInterpolator(yzrilyzr_util::Lang & lang, MenuRegister & reg);
void registerAllPhaseSrc(yzrilyzr_util::Lang & lang, MenuRegister & reg);