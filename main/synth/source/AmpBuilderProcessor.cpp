#include "AmpBuilderProcessor.h"
#include "synth/generators/sine/SineWave.h"
#include "synth/generators/pulse/TriWave.h"
#include "synth/generators/pulse/SawWave.h"
#include "synth/generators/pulse/SquareWave.h"
#include "synth/source/AmpBuilder.h"
#include "interface/IChannel.h"
#include "interface/IMixer.h"
namespace yzrilyzr_simplesynth{
	uint8_t AmpBuilderProcessor::readByte(){
		return stack[stackPointer++] & 0xff;
	}
	float AmpBuilderProcessor::readFloat(){
		float value=0;
		memcpy(&value, stack + stackPointer, 4);
		stackPointer+=4;
		std::cout << value << std::endl;
		return value;
	}

	int32_t AmpBuilderProcessor::readShort(){
		int32_t s=readByte();
		s<<=8;
		s|=readByte();
		return static_cast<int16_t>(s);
	}
	bool AmpBuilderProcessor::isStackFull(){
		return stackPointer >= stackLength;
	}
	void AmpBuilderProcessor::setStackType(int type, u_index len){
		stackType=type;
		stackLength=len;
		stackPointer=0;
	}
	void AmpBuilderProcessor::pushStack(Note & id){
		pushStack(id.id);
	}
	void AmpBuilderProcessor::pushStack(uint8_t id){
		stack[stackPointer]=id;
		stackPointer+=1;
	}
	void AmpBuilderProcessor::init(ChannelConfig & cfg){
		if(amp != nullptr)delete amp;
		amp=new AmpBuilder();
		setStackType(TYPE_CONTROL, 2);
		std::cout << "BuilderStart" << std::endl;
	}
	void AmpBuilderProcessor::cc(ChannelConfig & cfg, ChannelControl & cc){}
	u_sample AmpBuilderProcessor::getAmp(Note & note){
		pushStack(note);
		switch(stackType){
			case TYPE_CONTROL:
				if(isStackFull()){
					stackPointer=0;
					regControl=readShort();
					std::cout << "Control:" << (int)regControl;
					setStackType(TYPE_ARGLEN, 1);
				}
				break;
			case TYPE_ARGLEN:
				if(isStackFull()){
					stackPointer=0;
					regArgLen=readByte();
					std::cout << " ArgLen:" << (int)regArgLen << std::endl;
					if(regArgLen == 0)goto lbProcData;
					else setStackType(TYPE_ARG, regArgLen);
				}
				break;
			case TYPE_ARG:
				if(isStackFull()){
				lbProcData:
					stackPointer=0;
					std::cout << "ProcessingData..." << std::endl;
					try{
						//processData(note.mixer, note.channel);
						std::cout << "Success" << std::endl;
					} catch(...){
						std::cout << "Error" << std::endl;
					}
					setStackType(TYPE_CONTROL, 2);
				}
				break;
		}
		return 0;
	}
	NoteProcPtr AmpBuilderProcessor::parseSrc(){
		const uint8_t type=readByte();
		switch(type){
			case 0://Sine
				return mksp<SineWave>();
			case 1://Tri
				return mksp<TriWave>();
			case 2://Saw
				return mksp<SawWave>();
			case 3://Square
				return mksp<SquareWave>();
			default:
				return nullptr;
				break;
		}
	}
	void AmpBuilderProcessor::processData(IMixer * mixer, IChannel * channel){
		//switch(regControl){
		//	case 0://reset builder
		//		if(amp != nullptr)delete amp;
		//		amp=new AmpBuilder();
		//		break;
		//	case 1://src
		//		amp->src(parseSrc());
		//		break;
		//	case 2://buildAndSet
		//	{
		//		/*int channelID=readByte();
		//		u_sp<Channel> channel=mixer->getMIDIChannel(channelID);
		//		if(channel == nullptr){
		//			std::cout << "Target channel == nullptr" << std::endl;
		//		} else{
		//			channel->setNoteProcessor(amp->build());
		//			amp=nullptr;
		//		}*/
		//	}
		//	break;
		//	case 3://pm
		//		amp->pm(spdc<Osc>(parseSrc()), readFloat(), readFloat());
		//		break;
		//	default:
		//		std::cout << "Unknown command:" << regControl << std::endl;
		//		break;
		//}
	}
}