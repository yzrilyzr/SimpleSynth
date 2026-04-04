#include "PianoSrc.h"
#include "piano/PianoModel.h"
#include "events/NoteUpdater.h"
using namespace yzrilyzr_util;
using namespace yzrilyzr_interpolator;
using namespace yzrilyzr_array;
namespace yzrilyzr_simplesynth{
	PianoSrc::PianoSrc(){
		soundboardParameters.eq1=200;
		soundboardParameters.eq2=500;
		soundboardParameters.eq3=3000;
		soundboardParameters.eq4=5000;
		soundboardParameters.eq5=8000;
		soundboardParameters.c1=9;
		soundboardParameters.c3=30;
		GraphInterpolator vol(DoubleArray({//
			36, 1,//
				48, 1,//
				60, 1,//
				66, 2,//
				72, 3,//
				78, 4,//
				83, 4,//
				96, 18,//
				120, 40//
		}));
		GraphInterpolator pos(DoubleArray({//
			36, 0.4,//
				48, 0.4,//
				60, 0.3,//
				72, 0.2,//
				84, 0.1,//
				120, 0.05//
		}));
		GraphInterpolator weight(DoubleArray({//
			36, 8,//
				42, 5,//
				48, 4,//
				54, 6,//
				60, 5,//
				72, 3,//
				78, 3,//
				84, 3,//
				90, 2,//
				120, 2//
		}));
		GraphInterpolator detune(DoubleArray({//
			0, 0.00015,  // 低音区更小的detune
				36, 0.00015, // 低音区
				48, 0.00020, // 低音到中音过渡
				60, 0.00040, // 中音区
				72, 0.00080, // 中音到高音过渡
				84, 0.00120, // 高音区
				96, 0.00180, // 高音区
				108, 0.00200, // 极高音区
				128, 0.00250  // 极高音区
		}));
		for(u_index i=0;i < CHANNEL_MAX_NOTE_ID;i++){
			PianoKeyParameters & kp=keyParams[i];
			kp.noteID=i;
			kp.minr=0.34;
			kp.maxr=2;
			kp.minL=0.07;
			kp.maxL=1.39;
			kp.hammer_type=2;
			kp.weight=weight.y(i) * 4;
			kp.ampLl=-4;
			kp.ampLr=4;
			kp.amprl=4;
			kp.amprr=8;
			kp.mult_density_string=1;
			kp.mult_modulus_string=1;
			kp.mult_impedance_bridge=1;
			kp.mult_impedance_hammer=0;
			kp.mult_mass_hammer=1;
			kp.mult_force_hammer=1;
			kp.mult_hysteresis_hammer=1;
			kp.mult_stiffness_exponent_hammer=1;
			kp.position_hammer=pos.y(i);
			kp.mult_loss_filter=1;
			kp.detune=detune.y(i);
			kp.mult_radius_core_string=1;
			kp.outputVolume=vol.y(i) * 15;
		}
	}

	u_sample PianoSrc::getAmp(const Note & note){
		int id=note.id;
		if(id < 21 || id>108)return 0;
		PianoKey & pianoKey=*getData(note);
		if(pianoKey.string.empty())return 0;
		PianoKeyParameters & kp=keyParams[id];
		pianoKey.onTimePassed+=note.cfg->deltaTime;
		if(pianoKey.onTimePassed > 4.9){
			pianoKey.offTimePassed+=note.cfg->deltaTime;
			PianoModel::PianoKeyDamper(pianoKey);
		}
		double output=PianoModel::PianoKeyGo(pianoKey);
		output*=kp.outputVolume;
		return output;
	}
	void PianoSrc::init(ChannelConfig & cfg){
		u_sample_rate sampleRate=cfg.sampleRate;
		soundboard.setParam(soundboardParameters);
		soundboard.init(sampleRate);
		soundboard.resetMemory();
	}
	void PianoSrc::noteOn(ChannelConfig & cfg, s_note_id_i id, s_note_vel vel){
		if(id < 21 || id>108)return;
		//PianoModel::PianoKeyTrigger(pianoKey, tmpNote.velocitySynth);*/
	}
	void PianoSrc::noteOff(ChannelConfig & cfg, s_note_id_i id, s_note_vel vel){
		//PianoModel::PianoKeyDamper(pianoKey);
	}
	void PianoSrc::postProcess(u_sample * input, u_index length){
		soundboard.procBlock(input, input,length);
	}
	bool PianoSrc::noMoreData(const Note & note)const{
		int id=note.id;
		if(id < 21 || id>108)return true;
		PianoKey & pianoKey=*getDataConst(note);
		if(note.closed(*note.cfg) && pianoKey.onTimePassed < 4.9){
			pianoKey.onTimePassed=4.9;
		}
		return pianoKey.onTimePassed > 5 || pianoKey.offTimePassed >= 0.1 || note.fclosed(*note.cfg);
	}
	NoteProcPtr PianoSrc::clone(){
		return mksp<PianoSrc>();
	}
	void PianoSrc::cc(ChannelConfig & cfg, ChannelControl & cc){}
	PianoKey * PianoSrc::init(PianoKey * data, const Note & note){
		if(data == nullptr){
			data=new PianoKey();
		}
		PianoKey & pianoKey=*data;
		pianoKey.loadState=0;
		pianoKey.onTimePassed=0;
		pianoKey.offTimePassed=0;
		PianoKeyParameters & kp=keyParams[note.id];
		if(kp.sampleRate != note.cfg->sampleRate || kp.frequency != note.freqSynth){
			kp.sampleRate=note.cfg->sampleRate;
			kp.frequency=note.freqSynth;
		}
		PianoModel::PianoKeyInitialize(pianoKey, kp);
		PianoModel::PianoKeyTrigger(pianoKey, note.velocitySynth * (note.cfg->SoftPedal?0.5:1.0));
		return data;
	}
}