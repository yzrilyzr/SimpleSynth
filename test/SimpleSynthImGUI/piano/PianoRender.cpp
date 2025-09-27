#include "PianoRender.h"
#include "imgui.h"
#include "ast/ASTTokenizer.h"
#include "ast/TokenReader.h"
#include "util/Random.h"
#include "interface/IChannel.h"
#include "interface/IMixer.h"
#include "../SimpleSynthProject.h"
#include "synth/generators/physic/PianoSrc.h"
using namespace yzrilyzr_simplesynth;
using namespace yzrilyzr_util;
void pianoRenderFunc(CurrentProjectContext & ctx, ProjectObject & obj){
	std::shared_ptr<PianoSrc> paramRegPtr=std::static_pointer_cast<PianoSrc, ParamRegister>(obj.paramRegPtr);
	PianoSoundBoardParameters & sb=paramRegPtr->soundboardParameters;
	ImGui::Text("Soundboard");
	static double FREQ_MIN=10, FREQ_MAX=20000;
	bool c=false;
	c=ImGui::SliderScalar("ReverbLP", ImGuiDataType_Double, &sb.c1, &FREQ_MIN, &FREQ_MAX) || c;
	c=ImGui::SliderScalar("ReverbHP", ImGuiDataType_Double, &sb.c3, &FREQ_MIN, &FREQ_MAX) || c;
	c=ImGui::SliderScalar("LOW", ImGuiDataType_Double, &sb.eq1, &FREQ_MIN, &FREQ_MAX) || c;
	c=ImGui::SliderScalar("MID", ImGuiDataType_Double, &sb.eq2, &FREQ_MIN, &FREQ_MAX) || c;
	c=ImGui::SliderScalar("HIGH", ImGuiDataType_Double, &sb.eq3, &FREQ_MIN, &FREQ_MAX) || c;
	c=ImGui::SliderScalar("Hi-Decay", ImGuiDataType_Double, &sb.eq4, &FREQ_MIN, &FREQ_MAX) || c;
	c=ImGui::SliderScalar("LowPass", ImGuiDataType_Double, &sb.eq5, &FREQ_MIN, &FREQ_MAX) || c;
	ctx.paramChange=ctx.paramChange || c;
	bool d=false;
	ImGui::Text("String");
	static double minR=0.34, maxR=2, R_MIN=0.01, R_MAX=4;
	static double minL=0.07, maxL=1.39, L_MIN=0.01, L_MAX=4;
	static double weight=4, WEIGHT_MIN=0.01, WEIGHT_MAX=40;
	static double density=1, modulus=1, bridge=1, mult_impedance_hammer=0, hammer_mass=1, stiffness=1, force=1, hysteresis=1, radius_core=1, lossFilter=1;
	static double detune=0.00025, DETUNE_MIN=0.0, DETUNE_MAX=1;
	static double pos=0.135, POS_MIN=0.01, POS_MAX=0.99;
	static double  STD_MIN=0.0001, STD_MAX=10;
	/*kp.ampLl=-4;
	kp.ampLr=4;
	kp.amprl=4;
	kp.amprr=8;*/

	d=ImGui::SliderScalar("minR", ImGuiDataType_Double, &minR, &R_MIN, &R_MAX) || d;
	d=ImGui::SliderScalar("maxR", ImGuiDataType_Double, &maxR, &R_MIN, &R_MAX) || d;
	d=ImGui::SliderScalar("Core R", ImGuiDataType_Double, &radius_core, &STD_MIN, &STD_MAX) || d;

	d=ImGui::SliderScalar("minL", ImGuiDataType_Double, &minL, &L_MIN, &L_MAX) || d;
	d=ImGui::SliderScalar("maxL", ImGuiDataType_Double, &maxL, &L_MIN, &L_MAX) || d;

	d=ImGui::SliderScalar("Density", ImGuiDataType_Double, &density, &STD_MIN, &STD_MAX) || d;
	d=ImGui::SliderScalar("Young Modulus", ImGuiDataType_Double, &modulus, &STD_MIN, &STD_MAX) || d;
	d=ImGui::SliderScalar("Bridge Impedance", ImGuiDataType_Double, &bridge, &STD_MIN, &STD_MAX) || d;
	d=ImGui::SliderScalar("Detune", ImGuiDataType_Double, &detune, &DETUNE_MIN, &DETUNE_MAX) || d;
	d=ImGui::SliderScalar("Loss Filter", ImGuiDataType_Double, &lossFilter, &STD_MIN, &STD_MAX) || d;

	ImGui::Text("Hammer");
	d=ImGui::SliderScalar("Vel Weight", ImGuiDataType_Double, &weight, &WEIGHT_MIN, &WEIGHT_MAX) || d;
	d=ImGui::SliderScalar("Hammer Impedance", ImGuiDataType_Double, &mult_impedance_hammer, &STD_MIN, &STD_MAX) || d;
	d=ImGui::SliderScalar("Hammer Mass", ImGuiDataType_Double, &hammer_mass, &STD_MIN, &STD_MAX) || d;
	d=ImGui::SliderScalar("Hammer Stiffness", ImGuiDataType_Double, &stiffness, &STD_MIN, &STD_MAX) || d;
	d=ImGui::SliderScalar("Hammer Force", ImGuiDataType_Double, &force, &STD_MIN, &STD_MAX) || d;
	d=ImGui::SliderScalar("Hammer Hysteresis", ImGuiDataType_Double, &hysteresis, &STD_MIN, &STD_MAX) || d;
	d=ImGui::SliderScalar("Hammer Pos", ImGuiDataType_Double, &pos, &POS_MIN, &POS_MAX) || d;
	if(d){
		for(int i=0;i < CHANNEL_MAX_NOTE_ID;i++){
			PianoKeyParameters & kp=paramRegPtr->keyParams[i];
			kp.minr=minR;
			kp.maxr=maxR;
			kp.minL=minL;
			kp.maxL=maxL;
			//kp.hammer_type=2;
			kp.weight=weight;
			kp.ampLl=-4;
			kp.ampLr=4;
			kp.amprl=4;
			kp.amprr=8;
			kp.mult_density_string=density;
			kp.mult_modulus_string=modulus;
			kp.mult_impedance_bridge=bridge;
			kp.mult_impedance_hammer=mult_impedance_hammer;
			kp.mult_mass_hammer=hammer_mass;
			kp.mult_force_hammer=force;
			kp.mult_hysteresis_hammer=hysteresis;
			kp.mult_stiffness_exponent_hammer=stiffness;
			kp.position_hammer=pos;
			kp.mult_loss_filter=lossFilter;
			kp.detune=detune;
			kp.mult_radius_core_string=radius_core;
		}
	}
	ctx.paramChange=ctx.paramChange || d;
}