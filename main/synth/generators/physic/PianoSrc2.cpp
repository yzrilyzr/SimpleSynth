#include "PianoSrc2.h"
#include "piano/PianoModel.h"
#include "events/NoteUpdater.h"
using namespace yzrilyzr_util;
using namespace yzrilyzr_interpolator;
using namespace yzrilyzr_array;
namespace yzrilyzr_simplesynth{
	PianoSrc2::PianoSrc2(){
		soundboardParameters.eq1=200;
		soundboardParameters.eq2=500;
		soundboardParameters.eq3=3000;
		soundboardParameters.eq4=5000;
		soundboardParameters.eq5=8000;
		soundboardParameters.c1=12;
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
				78, 4,//
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
			kp.weight=weight.y(i) * 2;
			kp.ampLl=-4;
			kp.ampLr=4;
			kp.amprl=4;
			kp.amprr=8;
			kp.mult_density_string=1;
			kp.mult_modulus_string=1;
			kp.mult_impedance_bridge=1;
			kp.mult_impedance_hammer=0;
			kp.mult_mass_hammer=1;
			kp.mult_force_hammer=2;
			kp.mult_hysteresis_hammer=1;
			kp.mult_stiffness_exponent_hammer=1;
			kp.position_hammer=pos.y(i);
			kp.mult_loss_filter=1;
			kp.detune=detune.y(i);
			kp.mult_radius_core_string=1;
			kp.outputVolume=vol.y(i) * 40;
		}
	}

	u_sample PianoSrc2::getAmp(Note & note){
		int id=note.id;
		if(id < 21 || id>108)return 0;
		PianoKey & pianoKey=pianoKeys[id];
		if(pianoKey.string.empty())return 0;
		PianoKeyParameters & kp=keyParams[id];
		//防止piano key重复合成
		if(isInSynth[id])return 0;
		pianoKey.onTimePassed+=note.cfg->deltaTime;
		double output=PianoModel::PianoKeyGo(pianoKey);
		output*=kp.outputVolume;
		if(onState[id] && pianoKey.onTimePassed < 0.001){
			double y=Util::clamp01((pianoKey.onTimePassed) * 1000.0);
			output*=y;
		}
		if(!onState[id] || pianoKey.onTimePassed > 4.9){
			pianoKey.offTimePassed+=note.cfg->deltaTime;
			PianoModel::PianoKeyDamper(pianoKey);
		}
		isInSynth[id]=true;
		return output;
	}
	void PianoSrc2::init(ChannelConfig & cfg){
		u_sample_rate sampleRate=cfg.sampleRate;
		soundboard.setParam(soundboardParameters);
		soundboard.init(sampleRate);
	}
	void PianoSrc2::noteOn(ChannelConfig & cfg, s_note_id_i id, s_note_vel vel){
		if(id < 21 || id>108)return;
		PianoKey & pianoKey=pianoKeys[id];
		pianoKey.loadState=0;
		pianoKey.onTimePassed=0;
		pianoKey.offTimePassed=0;
		onState[id]=true;
		Note tmpNote(0);
		tmpNote.id=id;
		tmpNote.velocity=(cfg.SoftPedal?0.5:1) * vel;
		NoteUpdater::preUpdateNote(tmpNote, cfg);
		NoteUpdater::postUpdateNote(tmpNote, cfg);
		PianoKeyParameters & kp=keyParams[id];
		if(kp.sampleRate != cfg.sampleRate || kp.frequency != tmpNote.freqSynth){
			kp.sampleRate=cfg.sampleRate;
			kp.frequency=tmpNote.freqSynth;
			PianoModel::PianoKeyInitialize(pianoKey, kp);
		}
		PianoModel::PianoKeyTrigger(pianoKey, tmpNote.velocitySynth);
	}
	void PianoSrc2::noteOff(ChannelConfig & cfg, s_note_id_i id, s_note_vel vel){
		if(id < 21 || id>108)return;
		onState[id]=false;
	}
	u_sample PianoSrc2::postProcess(u_sample output){
		output=soundboard.procDsp(output);
		//重置合成重复状态
		u_index siz=CHANNEL_MAX_NOTE_ID * sizeof(false);
		memset(isInSynth, false, siz);
		return output;
	}
	bool PianoSrc2::noMoreData(Note & note){
		int id=note.id;
		if(id < 21 || id>108)return true;
		PianoKey & pianoKey=pianoKeys[id];
		return onState[id] && pianoKey.onTimePassed > 5 || !onState[id] && pianoKey.offTimePassed >= 0.1 || note.fclosed(*note.cfg);
	}
	NoteProcPtr PianoSrc2::clone(){
		return std::make_shared<PianoSrc2>();
	}
	void PianoSrc2::cc(ChannelConfig & cfg, ChannelControl & cc){}
}