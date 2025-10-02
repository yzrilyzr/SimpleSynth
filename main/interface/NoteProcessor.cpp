#include "NoteProcessor.h"
#include "events/Note.h"
#include "dsp/DSP.h"

using namespace yzrilyzr_array;
using namespace yzrilyzr_util;
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	bool NoteProcessor::noMoreData(Note & note){
		return note.closed(*note.cfg) || note.fclosed(*note.cfg);
	}

	void NoteProcessor::registerParamPhaseSrc(const String & name,
											  std::shared_ptr<PhaseSrc> * value){
		registerParam(name, ParamType::PhaseSrc, value, nullptr, nullptr);
	}

	void NoteProcessor::registerParamSrc(const String & name, NoteProcPtr * value){
		registerParam(name, ParamType::NoteSrc, value, nullptr, nullptr);
	}

	void NoteProcessor::registerParamOscSrc(const String & name, std::shared_ptr<Osc> * value){
		registerParam(name, ParamType::OscSrc, value, nullptr, nullptr);
	}

	void NoteProcessor::registerParamDSP(const String & name, std::shared_ptr<DSP> * value){
		registerParam(name, ParamType::DSP, value, nullptr, nullptr);
	}

	void NoteProcessor::registerParamSample(const String & name,
											std::shared_ptr<SampleProvider> * value){
		registerParam(name, ParamType::SampleData, value, nullptr, nullptr);
	}

	String NoteProcessor::toString() const{
		return "NoteProcessor";
	}
}