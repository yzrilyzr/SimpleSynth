#include "Matrix6x6Modulation.h"
#include "synth/generators/sine/SineWave.h"
using namespace yzrilyzr_util;

namespace yzrilyzr_simplesynth{
	FMOp::FMOp(){
		registerParamSrc("Src", &src);
		static double minFreqMul=0, maxFreqMul=64;
		static double minFreqOff=-10000.0, maxFreqOff=10000.0;
		static double minInitPhase=0.0, maxInitPhase=1.0;
		static double minOutput=-1.0, maxOutput=1.0;
		registerParam("FreqMul", ParamType::Double, &freqMul, &minFreqMul, &maxFreqMul);
		registerParam("FreqOff", ParamType::Double, &freqOff, &minFreqOff, &maxFreqOff);
		registerParam("InitPhase", ParamType::Double, &initPhase, &minInitPhase, &maxInitPhase);
		registerParam("Input", ParamType::Double, &input, &minOutput, &maxOutput);
		registerParam("Output", ParamType::Double, &output, &minOutput, &maxOutput);
	}
	void FMOp::init(ChannelConfig & cfg){
		if(src == nullptr)src=std::make_shared<SineWave>();
		if(src != nullptr)src->init(cfg);
	}
	u_sample FMOp::getAmp(Note & note){
		if(src == nullptr)return 0;
		return src->getAmp(note);
	}
	bool FMOp::noMoreData(Note & note){
		if(src == nullptr)return true;
		return src->noMoreData(note);
	}
	MatrixOpKeyData * FMOp::init(MatrixOpKeyData * data, Note & note){
		if(data == nullptr){
			data=new MatrixOpKeyData();
		}
		data->origPhaseSynth=initPhase;
		data->phaseSynth=initPhase;
		data->lastOutput=0;
		data->oscFreq=0;
		return data;
	}
	Matrix6x6Modulation::Matrix6x6Modulation(){
		for(size_t i=0;i < MATRIX_SIZE;i++){
			registerSub("OP" + std::to_string(i + 1), &op[i]);
		}
	}
	void Matrix6x6Modulation::init(ChannelConfig & cfg){
		for(size_t i=0;i < MATRIX_SIZE;i++){
			op[i].init(cfg);
		}
	}
	u_sample Matrix6x6Modulation::getAmp(Note & note){
		u_freq origFreq=note.freqSynth;
		s_phase origPhas=note.phaseSynth;
		u_sample sum=0;
		for(size_t to=0;to < MATRIX_SIZE;to++){
			u_sample fmDepth=0;
			u_sample rmDepth=1;
			FMOp & top=op[to];
			MatrixOpKeyData & data=*top.getData(note);
			data.oscFreq=origFreq * top.freqMul + top.freqOff;
			for(size_t from=0;from < MATRIX_SIZE;from++){
				FMOp & fop=op[from];
				MatrixOpKeyData & data1=*fop.getData(note);
				fmDepth+=data1.oscFreq * data1.lastOutput * fmMatrix[from][to];
				rmDepth*=1 + data1.lastOutput * rmMatrix[from][to] - std::abs(rmMatrix[from][to]);
			}
			note.freqSynth=data.oscFreq + fmDepth;
			data.phaseSynth+=note.freqSynth * note.cfg->deltaTime;
			note.phaseSynth=data.phaseSynth;
			u_sample out=top.getAmp(note) * rmDepth;
			data.lastOutput=out;
			sum+=out * top.output;
		}
		note.freqSynth=origFreq;
		note.phaseSynth=origPhas;
		return sum;
	}
}