#include "FadeOutEnvelop.h"
#include "lang/StringFormat.hpp"

using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;

namespace yzrilyzr_simplesynth{

	FadeOutEnvelop::FadeOutEnvelop(float fade, int mode)
		: fade(fade), mode(mode){}

	u_sample FadeOutEnvelop::getAmp(const Note & note){
		// 若 fade == 0，永不衰减
		if(fade == 0.0f){
			return 1.0f;
		}

		u_time time;
		if(mode == OnNoteOn){
			time=note.passedTime;
		} else{ // OnNoteOff
			if(note.closed(*note.cfg)){
				time=note.closedPassedTime;
			} else{
				// 未关闭时保持为 1
				return 1.0f;
			}
		}

		// 线性衰减：amp = max(0, 1 - fade * time)
		u_sample amp=1.0f - fade * time;
		return (amp > 0.0f)?amp:0.0f;
	}

	bool FadeOutEnvelop::noMoreData(const Note & note)const{
		// 强制关闭直接结束
		if(note.fclosed(*note.cfg)) return true;

		// fade=0 时永不结束
		if(fade == 0.0f) return false;

		u_time time;
		if(mode == OnNoteOn){
			time=note.passedTime;
		} else{ // OnNoteOff
			if(note.closed(*note.cfg)){
				time=note.closedPassedTime;
			} else{
				// 未关闭，肯定未结束
				return false;
			}
		}

		// 当时间 >= 1/fade 时，振幅已为 0，包络结束
		const u_time threshold=1.0f / fade;
		return time >= threshold;
	}
	void FadeOutEnvelop::onRegisterParam(){
		static float min=0, max=2;
		registerParam("Fade", ParamType::Float, &fade, &min, &max);
		static const char * type_to_name[9]={"NoteOn", "NoteOff"};
		registerParamEnum("Mode", &mode, type_to_name, 2);
	}
	void FadeOutEnvelop::init(ChannelConfig & cfg){
		// 无需额外初始化
	}

	u_sp<NoteProcessor> FadeOutEnvelop::clone(){
		return mksp<FadeOutEnvelop>(fade, mode);
	}

	String FadeOutEnvelop::toString() const{
		return StringFormat::object2string("FadeOutEnvelop", fade, mode);
	}

} // namespace yzrilyzr_simplesynth