#include "AmpMultiplier.h"
#include "lang/StringFormat.hpp"
#include "array/Array.hpp"
#include "dsp/DSP.h"
using namespace yzrilyzr_lang;
using namespace yzrilyzr_array;
namespace yzrilyzr_simplesynth{
	String AmpMultiplier::toString() const{
		return StringFormat::object2string("AmpMultiplier", a, b);
	}
	void AmpMultiplier::getAmpBlock(const Note * note, u_sample * output, u_index length){
		static thread_local SampleArray temp1;
		if(temp1 == nullptr || temp1.length < length)temp1=SampleArray(length);
		u_sample * data=temp1.data();
		a->getAmpBlock(note, output, length);
		b->getAmpBlock(note, data, length);
		for(u_index i=0;i < length;i++){
			output[i]*=data[i];
		}
	}
}