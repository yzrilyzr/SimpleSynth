#include "NoiseSrc.h"
#include "events/Note.h"
#include "dsp/RingBuffer.h"
#include "SynthUtil.h"
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	u_sample NoiseSrc::getAmp(const Note & note){
		static thread_local FixedRandom random;
		u_sample r=random.next();
		return r * note.velocitySynth;
	}
	String NoiseSrc::toString()const{
		return StringFormat::object2string("NoiseSrc");
	}
}