#include "AmplitudeSources.h"
#include "AmpBuilder.h"

namespace yzrilyzr_simplesynth{
	u_sp<PhaseSrc> ConstPhase(u_freq hz){
		return mksp<_ConstPhase>(hz);
	}
	NoteProcPtr ConstAmp(u_sample value){
		return mksp<_ConstAmp>(value);
	}
	NoteProcPtr SineW(){
		return mksp<SineWave>();
	}
	NoteProcPtr SawW(){
		return mksp<SawWave>();
	}
	NoteProcPtr SquareW(){
		return mksp<SquareWave>();
	}
	NoteProcPtr TriW(){
		return mksp<TriWave>();
	}
	NoteProcPtr SineAmp(u_freq hz){
		return ConstFreq(mksp<SineWave>(), hz);
	}
	NoteProcPtr ConstFreq(NoteProcPtr src, u_freq hz){
		return AmpBuilder(src).freqSrc(hz).build();
	}
	NoteProcPtr ConstFreq(NoteProcPtr src, u_sp<PhaseSrc> freq){
		return AmpBuilder(src).freqSrc(freq).build();
	}
	u_sp<AmpAdder> operator+(NoteProcPtr a, NoteProcPtr b){
		return mksp<AmpAdder>(a, b);
	}
	u_sp<AmpAdder> operator+(NoteProcPtr a, u_sample b){
		return mksp<AmpAdder>(a, ConstAmp(b));
	}
	u_sp<AmpMultiplier> operator*(NoteProcPtr a, NoteProcPtr b){
		return mksp<AmpMultiplier>(a, b);
	}
	u_sp<AmpMultiplier> operator*(NoteProcPtr a, u_sample b){
		return mksp<AmpMultiplier>(a, ConstAmp(b));
	}

	u_sp<PhaseSrc> operator+(u_sp<PhaseSrc> a, u_sp<PhaseSrc> b){
		return mksp<_AddPhase>(a, b);
	}
	u_sp<PhaseSrc> operator*(u_sp<PhaseSrc> a, u_sample b){
		return mksp<_MulPhase>(a, b);
	}
}