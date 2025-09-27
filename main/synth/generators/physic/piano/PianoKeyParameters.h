#pragma once
#include "SimpleSynth.h"

namespace yzrilyzr_simplesynth{
	EBCLASS(PianoKeyParameters){
	public:
	s_note_id noteID=0;
	u_freq frequency=0.0;
	u_sample_rate sampleRate=0;
	u_sample outputVolume=0.0;//输出音量
	double weight=0.0;
	double minr=0.0;
	double maxr=0.0;
	double amprl=0.0;
	double amprr=0.0;
	double minL=0.0;
	double maxL=0.0;
	double ampLl=0.0;
	double ampLr=0.0;
	double mult_radius_core_string;//核心弦的半径
	double mult_density_string;//弦密度
	double mult_modulus_string;// 弦杨氏模量
	double mult_impedance_bridge;//琴桥阻抗
	double mult_impedance_hammer=0.0;//琴锤阻抗
	double mult_mass_hammer=0.0;//琴锤质量
	double mult_force_hammer=0.0;//锤击力度
	double mult_hysteresis_hammer=0.0;//琴锤滞留
	double mult_stiffness_exponent_hammer=0.0;//琴锤刚度
	double position_hammer=0.0;//琴锤位置
	double mult_loss_filter=0.0;
	double detune=0.0;// 弦之间的音高差
	/**
	 * 1 {@link  StulovHammer}
	 * 2 {@link  BanksHammer}
	 */
	int hammer_type=0;
	};
}
