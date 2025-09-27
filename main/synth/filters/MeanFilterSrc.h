#pragma once
#include "interface/NoteProcessor.h"
#include "events/Note.h"
#include "events/NoteData.hpp"
#include <string>
#include "yzrutil.h"

namespace yzrilyzr_simplesynth{
	EBCLASS(MeanFilterSrcKeyData){
	public:
	u_sample last=0;
	};
	ECLASS(MeanFilterSrc, public NoteProcessor), NoteData<MeanFilterSrcKeyData>{
	private:
	NoteProcPtr src;
	NoteProcPtr env;
	double envMulti;
	/**
	 * env越大，滤波效果越强
	 */
	public:
	MeanFilterSrc();
	MeanFilterSrc(NoteProcPtr src, NoteProcPtr env, double envMulti);
	u_sample getAmp(Note & note) override;
	bool noMoreData(Note & note) override;
	NoteProcPtr clone() override;
	MeanFilterSrcKeyData * init(MeanFilterSrcKeyData * data, Note & note) override;
	std::string toString() const override;
	};
}