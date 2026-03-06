#pragma once
#include "interface/NoteProcessor.h"
#include "SimpleSynth.h"
#include "events/Note.h"
#include "AmpBuilder.h"

namespace yzrilyzr_simplesynth{
	/**
	* 控制指令[2] 参数长度[1] 参数[参数长度]
	*/
	ECLASS(AmpBuilderProcessor, public NoteProcessor){
	private:
	AmpBuilder * amp=nullptr;
	uint8_t stack[128]={0};
	u_index stackPointer=0;
	u_index stackLength=0;
	int stackType=0;
	int regControl=0;
	u_index regArgLen=0;
	static constexpr int const TYPE_CONTROL=0;
	static constexpr int const TYPE_ARGLEN=1;
	static constexpr int const TYPE_ARG=2;
	void pushStack(const Note & id);
	void pushStack(uint8_t id);
	bool isStackFull();
	int32_t readShort();
	uint8_t readByte();
	float readFloat();
	void setStackType(int type, u_index len);
	void processData(IMixer * mixer, IChannel * channel);
	NoteProcPtr parseSrc();
	public:
	~AmpBuilderProcessor(){
		if(amp != nullptr)delete amp;
	}
	void init(ChannelConfig & cfg) override;
	u_sample postProcess(u_sample output)override{
		return 0;
	}
	bool noMoreData(const Note & note)override{
		return true;
	}
	u_sample getAmp(const Note & note)override;
	void cc(ChannelConfig & cfg, ChannelControl & cc) override;
	NoteProcPtr clone()override{
		return mksp<AmpBuilderProcessor>();
	}
	};
}