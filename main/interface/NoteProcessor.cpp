#include "NoteProcessor.h"
#include "events/Note.h"
#include "dsp/DSP.h"

using namespace yzrilyzr_array;
using namespace yzrilyzr_util;
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_lang;
using namespace yzrilyzr_simplesynth;

namespace yzrilyzr_util{
	namespace RegisterUtil{
		void registerParamPhaseSrc(ClassRegister & reg, const String & name, u_sp<PhaseSrc> * value){
			reg.registerParam(name, ParamType::PhaseSrc, value, nullptr, nullptr);
		}

		void registerParamSrc(ClassRegister & reg, const String & name, NoteProcPtr * value){
			reg.registerParam(name, ParamType::NoteSrc, value, nullptr, nullptr);
		}

		void registerParamOscSrc(ClassRegister & reg, const String & name, u_sp<Osc> * value){
			reg.registerParam(name, ParamType::OscSrc, value, nullptr, nullptr);
		}

		void registerParamSampleData(ClassRegister & reg, const String & name, u_sp<SampleProvider> * value){
			reg.registerParam(name, ParamType::SampleData, value, nullptr, nullptr);
		}
	}
}
namespace yzrilyzr_simplesynth{
	bool NoteProcessor::noMoreData(const Note & note){
		return note.closed(*note.cfg) || note.fclosed(*note.cfg);
	}

	/*void NoteProcessor::getAmpBlock(const Note * noteSnapshots, u_sample * output, u_index length){
		for(u_index i=0;i < length;i++){
			output[i]=getAmp(noteSnapshots[i]);
			if(noMoreData(const_cast<Note&>(noteSnapshots[i])))break;
		}
	}*/

	String NoteProcessor::toString() const{
		return "NoteProcessor";
	}
}