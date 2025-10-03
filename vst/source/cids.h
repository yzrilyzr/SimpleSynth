//------------------------------------------------------------------------
// Copyright(c) 2025 yzrilyzr.
//------------------------------------------------------------------------

#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"

namespace yzrilyzr_simplesynth_vst {
//------------------------------------------------------------------------
static const Steinberg::FUID kSimpleSynthProcessorUID (0x4FEB6B95, 0xAF135EA9, 0x97BC114B, 0xBF65C357);
static const Steinberg::FUID kSimpleSynthControllerUID (0x5060B6EF, 0x9EA5549E, 0x83E38138, 0x33C25987);

#define SimpleSynthVST3Category "Instrument|Synth|MIDI"

//------------------------------------------------------------------------
} // namespace yzrilyzr_simplesynth_vst
