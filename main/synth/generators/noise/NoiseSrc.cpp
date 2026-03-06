#include "NoiseSrc.h"
#include "events/Note.h"
#include "dsp/RingBuffer.h"
#include "SynthUtil.h"
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	u_sample NoiseSrc::getAmp(const Note & note){
		int32_t time1=(int32_t)(note.passedTime * note.cfg->sampleRate);
		u_index time=RingBufferUtil::mod(time1, SynthUtil::NOISE->length);
		u_sample r=(*SynthUtil::NOISE)[time];
		return r * note.velocitySynth;
	}
	String NoiseSrc::toString()const{
		return StringFormat::object2string("NoiseSrc");
	}
}