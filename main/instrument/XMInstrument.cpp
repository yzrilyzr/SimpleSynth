#include "XMInstrument.h"
#include "synth/source/XMNoteProcessor.h"
using namespace yzrilyzr_util;
namespace yzrilyzr_simplesynth{
	XMInstrument::XMInstrument(std::shared_ptr<XMFile::Module> mod) :mod(mod){
		for(int i=0;i < mod->instruments.size();i++){
			insts.add(std::make_shared<XMNoteProcessor>(mod, &mod->instruments[i], i));
		}
	}
}