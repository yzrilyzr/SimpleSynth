#include "Matrix6x6Modulation.h"
#include "synth/osc/sine/SineWave.h"
#include "dsp/DSP.h"
using namespace yzrilyzr_util;

namespace yzrilyzr_simplesynth{
	void FMOp::onRegisterParam(){
		RegisterUtil::registerParamSrc(*this, "Src", &src);
		static double minFreqMul=0, maxFreqMul=64;
		static double minFreqOff=-10000.0, maxFreqOff=10000.0;
		static double minInitPhase=0.0, maxInitPhase=1.0;
		static double minOutput=-1.0, maxOutput=1.0;
		registerParam("FreqMul", ParamType::Double, &freqMul, &minFreqMul, &maxFreqMul);
		registerParam("FreqOff", ParamType::Double, &freqOff, &minFreqOff, &maxFreqOff);
		registerParam("InitPhase", ParamType::Double, &initPhase, &minInitPhase, &maxInitPhase);
		RegisterUtil::registerParamGain(*this, "Input", &input);
		RegisterUtil::registerParamGain(*this, "Output", &output);
	}
	void FMOp::init(ChannelConfig & cfg){
		if(src == nullptr)src=mksp<SineWave>();
		if(src != nullptr)src->init(cfg);
	}
	u_sample FMOp::getAmp(const Note & note){
		if(src == nullptr)return 0;
		return src->getAmp(note);
	}
	bool FMOp::noMoreData(const Note & note)const{
		if(src == nullptr)return true;
		return src->noMoreData(note);
	}
	MatrixOpKeyData * FMOp::init(MatrixOpKeyData * data, const Note & note){
		if(data == nullptr){
			data=new MatrixOpKeyData();
		}
		data->origPhaseSynth=initPhase;
		data->phaseSynth=initPhase;
		data->lastOutput=0;
		data->oscFreq=0;
		return data;
	}
	void Matrix6x6Modulation::onRegisterParam(){
		for(u_index i=0;i < MATRIX_SIZE;i++){
			registerSub("OP" + std::to_string(i + 1), &op[i]);
		}
	}
	bool Matrix6x6Modulation::noMoreData(const Note & note)const{
		for(u_index i=0;i < MATRIX_SIZE;i++){
			if(!op[i].noMoreData(note))return false;
		}
		return true;
	}

	Matrix6x6Modulation::Matrix6x6Modulation(){}
	void Matrix6x6Modulation::init(ChannelConfig & cfg){
		for(u_index i=0;i < MATRIX_SIZE;i++){
			op[i].init(cfg);
		}
	}
	u_sample Matrix6x6Modulation::getAmp(const Note & note){
		static thread_local Note myNote;
		myNote.set(note);
		myNote.uniqueID=note.uniqueID;
		u_freq origFreq=note.freqSynth;
		u_sample sum=0;
		for(u_index to=0;to < MATRIX_SIZE;to++){
			u_sample fmDepth=0;
			u_sample rmDepth=1;
			FMOp & top=op[to];
			MatrixOpKeyData & data=*top.getData(myNote);
			data.oscFreq=origFreq * top.freqMul + top.freqOff;
			for(u_index from=0;from < MATRIX_SIZE;from++){
				FMOp & fop=op[from];
				MatrixOpKeyData & data1=*fop.getData(myNote);
				fmDepth+=data1.oscFreq * data1.lastOutput * fmMatrix[from][to];
				rmDepth*=1 + data1.lastOutput * rmMatrix[from][to] - std::abs(rmMatrix[from][to]);
			}
			myNote.freqSynth=data.oscFreq + fmDepth;
			data.phaseSynth+=myNote.freqSynth * myNote.cfg->deltaTime;
			myNote.phaseSynth=data.phaseSynth;
			u_sample out=top.getAmp(myNote) * rmDepth;
			data.lastOutput=out;
			sum+=out * top.output;
		}

		return sum;
	}
}