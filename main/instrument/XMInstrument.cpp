#include "XMInstrument.h"
#include "synth/source/XMNoteProcessor.h"
using namespace yzrilyzr_util;
namespace yzrilyzr_simplesynth{
	XMInstrument::XMInstrument(u_sp<XMFile::Module> mod) :mod(mod){
		for(u_index i=0;i < mod->instruments.size();i++){
			insts.add(mksp<XMNoteProcessor>(mod, &mod->instruments[i], i));
		}
	}
}