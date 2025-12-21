#pragma once
#include "SimpleSynth.h"
#include "FreqModAmp.h"
#include "events/NoteData.hpp"
#include "interface/NoteProcessor.h"
#include "AmpUnaryComposition.h"
#include "synth/envelopers/AHDSREnvelop.h"
#include <array>
#include <vector>
#include <memory>

namespace yzrilyzr_simplesynth{
	EBCLASS(MatrixOpKeyData){
	public:
	s_phase origPhaseSynth=0;
	s_phase phaseSynth=0;
	u_sample lastOutput=0.0;
	u_freq oscFreq=0;
	};
	ECLASS(FMOp, public NoteProcessor, public NoteData<MatrixOpKeyData>){
	public:
	NoteProcPtr src=nullptr;
	double freqMul=1.0;
	u_freq freqOff=0.0;
	s_phase initPhase=0.0;
	u_sample input=1.0;//[-1,1]
	u_sample output=0.0;//[-1,1]
	FMOp();
	~FMOp()=default;
	void init(ChannelConfig & cfg)override;
	u_sample getAmp(Note & note)override;
	bool noMoreData(Note & note)override;
	MatrixOpKeyData * init(MatrixOpKeyData * data, Note & note)override;
	};
	ECLASS(Matrix6x6Modulation, public NoteProcessor){
	public:
	static constexpr int MATRIX_SIZE=6;
	FMOp op[MATRIX_SIZE];
	u_sample fmMatrix[MATRIX_SIZE][MATRIX_SIZE]={0};
	u_sample rmMatrix[MATRIX_SIZE][MATRIX_SIZE]={0};
	Matrix6x6Modulation();
	~Matrix6x6Modulation()=default;
	void init(ChannelConfig & cfg)override;
	u_sample getAmp(Note & note)override;
	
	};
	EBCLASS(Matrix6x6ModulationBuilder){
		private:
		u_sp<Matrix6x6Modulation> ptr;
		public:
		static constexpr int TYPE_FM=0;
		static constexpr int TYPE_RM=1;
		Matrix6x6ModulationBuilder(){
			ptr=mksp<Matrix6x6Modulation>();
		}
		Matrix6x6ModulationBuilder & setOp(u_index index, NoteProcPtr osc,
									double freqMul=1.0, u_freq freqOff=0.0,
									s_phase initPhase=0.0, u_sample input=1.0,
									u_sample output=0.0){
			auto & op=ptr->op[index];
			op.src=osc;
			op.freqMul=freqMul;
			op.freqOff=freqOff;
			op.initPhase=initPhase;
			op.input=input;
			op.output=output;
			return *this;
		}
		Matrix6x6ModulationBuilder & setMatrix(int type, u_index from, u_index to, u_sample depth){
			if(type == TYPE_FM)ptr->fmMatrix[from][to]=depth;
			else if(type == TYPE_RM)ptr->rmMatrix[from][to]=depth;
			return *this;
		}
		NoteProcPtr build(){
			return ptr;
		}
	};
}