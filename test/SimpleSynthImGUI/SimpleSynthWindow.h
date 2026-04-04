#pragma once
#include "MenuRegister.hpp"
#include "nlohmann/json.hpp"
#include "dsp/RingBuffer.h"

using json=nlohmann::json;

extern yzrilyzr_dsp::RingBuffer<float> cpuLoadHistory;

void mainMenuBar(const char * name, MenuRegister & np, CurrentProjectContext & ctx);
void mixerSettingWindow(CurrentProjectContext & ctx);
void channelSettingWindow(CurrentProjectContext & ctx);
void fileOpenWindow(CurrentProjectContext & ctx);
void instrumentSourceWindow(CurrentProjectContext & ctx);
void oscilloscopeWindow(CurrentProjectContext & ctx);
void eqWindow(CurrentProjectContext & ctx);
void objectToStringWindow(CurrentProjectContext & ctx);
void importFromBankWindow(CurrentProjectContext & ctx);
