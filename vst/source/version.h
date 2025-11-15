//------------------------------------------------------------------------
// Copyright(c) 2025 yzrilyzr.
//------------------------------------------------------------------------

#pragma once

#include "pluginterfaces/base/fplatform.h"
#ifdef _DEBUG
	#define stringOriginalFilename	"SimpleSynth_Debug.vst3"
	#if SMTG_PLATFORM_64
		#define stringFileDescription	"SimpleSynth_Debug VST3 (64Bit)"
	#else
		#define stringFileDescription	"SimpleSynth_Debug VST3"
#endif
#else
	#define stringOriginalFilename	"SimpleSynth.vst3"
	#if SMTG_PLATFORM_64
		#define stringFileDescription	"SimpleSynth VST3 (64Bit)"
	#else
		#define stringFileDescription	"SimpleSynth VST3"
	#endif
#endif
#define stringCompanyName		"yzrilyzr"
#define stringLegalCopyright	"Copyright(c) 2024-2025 yzrilyzr."
#define stringLegalTrademarks	"VST is a trademark of Steinberg Media Technologies GmbH"

#ifdef _DEBUG
	#define FULL_VERSION_STR "0.0.0.1"
#else
	#define FULL_VERSION_STR "1.0.0.0"
#endif

#define MAJOR_VERSION_INT 1
#define SUB_VERSION_INT 0
#define RELEASE_NUMBER_INT 0
#define BUILD_NUMBER_INT 0

