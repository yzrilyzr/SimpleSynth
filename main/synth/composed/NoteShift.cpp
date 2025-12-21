#include "NoteShift.h"
#include "events/Note.h"
#include "events/ChannelConfig.h"
#include "interface/NoteTuning.h"
using namespace yzrilyzr_util;
namespace yzrilyzr_simplesynth{
	NoteShift::NoteShift(NoteProcPtr a, int shift) : AmpUnaryComposition(a){
		this->shift=shift;
	}
	NoteShift::NoteShift() :NoteShift(nullptr, 0){
		static int32_t min=-128, max=128;
		registerParam("Shift", ParamType::Double, &shift, &min, &max);
	}
	u_sample NoteShift::getAmp(Note & note){
		s_phase t=note.phaseSynth;
		u_freq freq=note.cfg->tuning->getFrequencyByID(note.id + shift);
		NoteShiftKeyData * data=getData(note);
		data->freqTimeSynth+=freq * note.cfg->deltaTime;
		note.phaseSynth=data->freqTimeSynth;
		u_sample y=a->getAmp(note);
		note.phaseSynth=t;
		return y;
	}
	NoteProcPtr NoteShift::clone(){
		return mksp<NoteShift>(a, shift);
	}
	NoteShiftKeyData * NoteShift::init(NoteShiftKeyData * data, Note & note){
		if(data == nullptr) data=new NoteShiftKeyData();
		data->freqTimeSynth=0;
		return data;
	}
}