#include "SakuraRender.h"
#include "imgui.h"
#include "ast/ASTTokenizer.h"
#include "ast/TokenReader.h"
#include "util/Random.h"
#include "interface/IMixer.h"
#include "../SimpleSynthProject.h"
#include "imgui-knobs.h"
#include "../ParamHelper.h"
using namespace yzrilyzr_ast;
using namespace yzrilyzr_simplesynth;
using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;
static int fMulI1=4;
static int fMulI2=4;
void sakuraRenderFunc(CurrentProjectContext & ctx, ProjectObject & obj){
	u_sp<Sakura> paramRegPtr=spsc<Sakura, ClassRegister>(obj.paramRegPtr);
	int renderPar=0;
	for(auto & reg : paramRegPtr->RegisteredParams){
		if(renderPar < 6){
			renderOneParam(ctx, reg);
		} else break;
		renderPar++;
	}
	ImGuiKnobFlags knobFlags=ImGuiKnobFlags_NoInput | ImGuiKnobFlags_ValueTooltip;
	ImGuiKnobVariant knobvariant=ImGuiKnobVariant_Tick;//ImGuiKnobFlags_Logarithmic
	float knobSize=60;
	//
	bool change=false;
	ImGui::BeginChild("Exciter", ImVec2(210, 500));
	//change=ImGuiKnobs::Knob("NoiseMix", &paramRegPtr->noiseMixRatio, 0.0f, 1.0f, 0.01f, "%.2f", knobvariant, knobSize, knobFlags) || change;
	//ImGui::SameLine();
	//change=ImGuiKnobs::Knob("NoiseRate", &paramRegPtr->noiseRate, 0.0f, 1.0f, 0.01f, "%.2f", knobvariant, knobSize, knobFlags) || change;
	float temp=paramRegPtr->exciterHiCutFreq;
	change=ImGuiKnobs::Knob("HiCutFreq", &temp, 0.0f, 1.0f, 0.01f, "%.2f", knobvariant, knobSize, knobFlags) || change;
	paramRegPtr->exciterHiCutFreq=temp;
	//
	ImGui::SameLine();
	change=ImGuiKnobs::Knob("HiCutQ", &paramRegPtr->exciterHiCutQ, 0.1f, 10.0f, 0.01f, "%.2f", knobvariant, knobSize, knobFlags) || change;
	//
	ImGui::Text("Hi Cut Env");
	static double timeMin=0;
	static double timeMax=1;
	static double sustainMin=0.0;
	static double suatainMax=1.0;
	static double freqMin=20.0;
	static double freqMax=20000.0;
	static double n_1=-1;
	static double n0=0;
	static double n1=1;
	change=ImGui::VSliderScalar("A##Env", ImVec2(18, 160), ImGuiDataType_Double, &paramRegPtr->exciterHiCutEnv.attackTime, &timeMin, &timeMax, "", 0) || change;
	ImGui::SameLine();
	change=ImGui::VSliderScalar("H##Env", ImVec2(18, 160), ImGuiDataType_Double, &paramRegPtr->exciterHiCutEnv.holdTime, &timeMin, &timeMax, "", 0) || change;
	ImGui::SameLine();
	change=ImGui::VSliderScalar("D##Env", ImVec2(18, 160), ImGuiDataType_Double, &paramRegPtr->exciterHiCutEnv.decayTime, &timeMin, &timeMax, "", 0) || change;
	ImGui::SameLine();
	change=ImGui::VSliderScalar("S##Env", ImVec2(18, 160), ImGuiDataType_Double, &paramRegPtr->exciterHiCutEnv.sustainVolume, &sustainMin, &suatainMax, "", 0) || change;
	//
	float temp2=paramRegPtr->exciterLowCutFreq;
	change=ImGuiKnobs::Knob("LoCutFreq", &temp2, 0.0f, 1.0f, 0.01f, "%.2f", knobvariant, knobSize, knobFlags) || change;
	paramRegPtr->exciterLowCutFreq=temp2;
	ImGui::SameLine();
	change=ImGuiKnobs::Knob("LoCutQ", &paramRegPtr->exciterLowCutQ, 0.1f, 10.0f, 0.01f, "%.2f", knobvariant, knobSize, knobFlags) || change;
	ImGui::EndChild();
	//
	ImGui::SameLine();
	//
	ImGui::BeginChild("Strings", ImVec2(250, 720));
	ImGui::Text("String1");
	const char * elems_names[9]={"1/5", "1/4", "1/3", "1/2", "1", "2", "3", "4", "5"};
	const double elems_value[9]={1.0 / 5.0, 1.0 / 4.0, 1.0 / 3.0, 1.0 / 2.0, 1.0, 2.0, 3.0, 4.0, 5.0};
	paramRegPtr->stringFMul1=elems_value[fMulI1];
	change=ImGui::SliderInt("F Mul##1", &fMulI1, 0, 8, elems_names[fMulI1]) || change;
	change=ImGui::SliderScalar("V Feedback##1", ImGuiDataType_Double, &paramRegPtr->stringVFeedback1, &n_1, &n1) || change;
	change=ImGui::SliderScalar("V Alpha##1", ImGuiDataType_Double, &paramRegPtr->stringVAlpha1, &n0, &n1) || change;

	ImGui::Text("String2");
	paramRegPtr->stringFMul2=elems_value[fMulI2];
	change=ImGui::SliderInt("F Mul##2", &fMulI2, 0, 8, elems_names[fMulI2]) || change;
	change=ImGui::SliderScalar("V Feedback##2", ImGuiDataType_Double, &paramRegPtr->stringVFeedback2, &n_1, &n1) || change;
	change=ImGui::SliderScalar("V Alpha##2", ImGuiDataType_Double, &paramRegPtr->stringVAlpha2, &n0, &n1) || change;

	ImGui::Text("Comb");
	change=ImGui::SliderScalar("Position##3", ImGuiDataType_Double, &paramRegPtr->combPosition, &n0, &n1) || change;
	change=ImGui::SliderScalar("V Feedback1##c3", ImGuiDataType_Double, &paramRegPtr->combFeedback1, &n_1, &n1) || change;
	change=ImGui::SliderScalar("V Feedback2##c3", ImGuiDataType_Double, &paramRegPtr->combFeedback2, &n_1, &n1) || change;
	change=ImGui::SliderFloat("OutputLevel##comb", &paramRegPtr->combOutputLevel, 0.0f, 1.0f) || change;

	ImGui::Text("Mix");
	change=ImGui::SliderScalar("StringMix##2", ImGuiDataType_Double, &paramRegPtr->stringMix, &n_1, &n1) || change;

	ImGui::Text("Env");
	change=ImGui::VSliderScalar("A##Env", ImVec2(18, 160), ImGuiDataType_Double, &paramRegPtr->stringEnv.attackTime, &timeMin, &timeMax, "", 0) || change;
	ImGui::SameLine();
	change=ImGui::VSliderScalar("H##Env", ImVec2(18, 160), ImGuiDataType_Double, &paramRegPtr->stringEnv.holdTime, &timeMin, &timeMax, "", 0) || change;
	ImGui::SameLine();
	change=ImGui::VSliderScalar("D##Env", ImVec2(18, 160), ImGuiDataType_Double, &paramRegPtr->stringEnv.decayTime, &timeMin, &timeMax, "", 0) || change;
	ImGui::SameLine();
	change=ImGui::Checkbox("S", &paramRegPtr->stringEnv.canSustain) || change;
	ImGui::SameLine();
	change=ImGui::VSliderScalar("S##Env", ImVec2(18, 160), ImGuiDataType_Double, &paramRegPtr->stringEnv.sustainVolume, &sustainMin, &suatainMax, "", 0) || change;
	ImGui::SameLine();
	change=ImGui::VSliderScalar("R##Env", ImVec2(18, 160), ImGuiDataType_Double, &paramRegPtr->stringEnv.releaseTime, &timeMin, &timeMax, "", 0) || change;

	ImGui::Text("String Output Level");
	change=ImGui::SliderFloat("OutputLevel##string", &paramRegPtr->stringOutputLevel, 0.0f, 1.0f) || change;

	ImGui::EndChild();
	//
	ImGui::SameLine();
	//
	ImGui::BeginChild("Resonator", ImVec2(250, 600));
	//ImGui::PushItemWidth(300);
	for(u_index i=0;i < 8;i++){
		ImGui::PushID(i);
		change=ImGui::SliderScalar("Freq##ExciterHiCutFreq", ImGuiDataType_Double, &paramRegPtr->resonatorFreq[i], &freqMin, &freqMax, "%.1f Hz", ImGuiSliderFlags_Logarithmic) || change;
		change=ImGui::SliderScalar("Fb", ImGuiDataType_Double, &paramRegPtr->resonatorFeedback[i], &n_1, &n1) || change;
		ImGui::SameLine();
		change=ImGui::Checkbox("En", &paramRegPtr->resonatorEnabled[i]) || change;
		ImGui::PopID();
	}
	//ImGui::PopItemWidth();
	ImGui::Text("Resonator Output Level");
	change=ImGui::SliderFloat("OutputLevel", &paramRegPtr->resonatorOutputLevel, 0.0f, 1.0f) || change;
	ImGui::EndChild();
	//SakuraEditWindow(*ctx.mixer, paramRegPtr);
	ctx.paramChange=ctx.paramChange || change;
}

int toFMulI(double x){
	const double elems_value[9]={1.0 / 5.0, 1.0 / 4.0, 1.0 / 3.0, 1.0 / 2.0, 1.0, 2.0, 3.0, 4.0, 5.0};
	for(u_index i=0;i < 9;i++){
		if(abs(x - elems_value[i]) < 0.001)return i;
	}
	return 4;
}
void SakuraEditWindow(IMixer & mixer, u_sp<Sakura> & paramRegPtr){
	ImGui::Begin("SakuraEdit");
	static char text[1024 * 16]="";
	ImGui::InputTextMultiline("##source", text, IM_ARRAYSIZE(text), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), ImGuiInputTextFlags_AllowTabInput);
	if(ImGui::Button("Import")){
		try{
			u_sp<TokenReader> tokens=ASTTokenizer::tokenize(std::string(text));
			bool resOn[8]={false};
			while(tokens->has()){
				Token & to=tokens->getToken();
				if(to.type == TokenType::ID){
					/*if(to.token == "exciter"){
						tokens->skipTo(TokenType::Number);
						paramRegPtr->noiseMixRatio=tokens->getFloat();
						tokens->next();
						tokens->skipTo(TokenType::Number);
						paramRegPtr->noiseRate=tokens->getFloat();
						tokens->next();
					} else */
					if(to.token == "exciterHiCut"){
						tokens->skipTo(TokenType::Number);
						paramRegPtr->exciterHiCutFreq=tokens->getFloat();
						tokens->next();
						tokens->skipTo(TokenType::Number);
						paramRegPtr->exciterHiCutQ=tokens->getFloat();
						tokens->next();
					} else if(to.token == "exciterLowCut"){
						tokens->skipTo(TokenType::Number);
						paramRegPtr->exciterLowCutFreq=tokens->getFloat();
						tokens->next();
						tokens->skipTo(TokenType::Number);
						paramRegPtr->exciterLowCutQ=tokens->getFloat();
						tokens->next();
					} else if(to.token == "exciterHiCutEnv"){
						tokens->skipTo(TokenType::Number);
						paramRegPtr->exciterHiCutEnv.attackTime=tokens->getFloat() / 1000.0;
						tokens->next();
						tokens->skipTo(TokenType::Number);
						paramRegPtr->exciterHiCutEnv.holdTime=tokens->getFloat() / 1000.0;
						tokens->next();
						tokens->skipTo(TokenType::Number);
						paramRegPtr->exciterHiCutEnv.decayTime=tokens->getFloat() / 1000.0;
						tokens->next();
						tokens->skipTo(TokenType::Number);
						paramRegPtr->exciterHiCutEnv.sustainVolume=tokens->getFloat();
						tokens->next();
					} else if(to.token == "string1"){
						tokens->skipTo(TokenType::Number);
						fMulI1=toFMulI(tokens->getFloat());
						tokens->next();
						tokens->skipTo(TokenType::Number);
						paramRegPtr->stringVFeedback1=tokens->getFloat();
						tokens->next();
						tokens->skipTo(TokenType::Number);
						paramRegPtr->stringVAlpha1=tokens->getFloat();
						tokens->next();
					} else if(to.token == "string2"){
						tokens->skipTo(TokenType::Number);
						fMulI2=toFMulI(tokens->getFloat());
						tokens->next();
						tokens->skipTo(TokenType::Number);
						paramRegPtr->stringVFeedback2=tokens->getFloat();
						tokens->next();
						tokens->skipTo(TokenType::Number);
						paramRegPtr->stringVAlpha2=tokens->getFloat();
						tokens->next();
					} else if(to.token == "stringMix"){
						tokens->skipTo(TokenType::Number);
						paramRegPtr->stringMix=tokens->getFloat();
						tokens->next();
					} else if(to.token == "stringLevel"){
						tokens->skipTo(TokenType::Number);
						paramRegPtr->stringOutputLevel=tokens->getFloat();
						tokens->next();
					} else if(to.token == "stringEnv"){
						tokens->skipTo(TokenType::Number);
						paramRegPtr->stringEnv.attackTime=tokens->getFloat() / 1000.0;
						tokens->next();
						tokens->skipTo(TokenType::Number);
						paramRegPtr->stringEnv.holdTime=tokens->getFloat() / 1000.0;
						tokens->next();
						tokens->skipTo(TokenType::Number);
						paramRegPtr->stringEnv.decayTime=tokens->getFloat() / 1000.0;
						tokens->next();
						tokens->skipTo(TokenType::ID);
						paramRegPtr->stringEnv.canSustain=tokens->getBoolean();
						tokens->next();
						tokens->skipTo(TokenType::Number);
						paramRegPtr->stringEnv.sustainVolume=tokens->getFloat();
						tokens->next();
						tokens->skipTo(TokenType::Number);
						paramRegPtr->stringEnv.releaseTime=tokens->getFloat() / 1000.0;
						tokens->next();
					} else if(to.token == "comb"){
						tokens->skipTo(TokenType::Number);
						paramRegPtr->combPosition=tokens->getFloat();
						tokens->next();
						tokens->skipTo(TokenType::Number);
						paramRegPtr->combFeedback1=tokens->getFloat();
						tokens->next();
						tokens->skipTo(TokenType::Number);
						paramRegPtr->combFeedback2=tokens->getFloat();
						tokens->next();
						tokens->skipTo(TokenType::Number);
						paramRegPtr->combOutputLevel=tokens->getFloat();
						tokens->next();
					} else if(to.token == "resonator"){
						tokens->skipTo(TokenType::Number);
						int which=tokens->getInt();
						paramRegPtr->resonatorEnabled[which]=true;
						resOn[which]=true;
						tokens->next();
						tokens->skipTo(TokenType::Number);
						paramRegPtr->resonatorFreq[which]=tokens->getFloat();
						tokens->next();
						tokens->skipTo(TokenType::Number);
						paramRegPtr->resonatorFeedback[which]=tokens->getFloat();
						tokens->next();
					} else if(to.token == "resonatorLevel"){
						tokens->skipTo(TokenType::Number);
						paramRegPtr->resonatorOutputLevel=tokens->getFloat();
						tokens->next();
					}
				}
				std::cout << to.toString().c_str(UTF8) << std::endl;
				tokens->next();
			}
			for(u_index i=0;i < 8;i++){
				if(!resOn[i])paramRegPtr->resonatorEnabled[i]=false;
			}
		} catch(...){
		}
	}
	ImGui::SameLine();
	if(ImGui::Button("Export")){
		std::stringstream ss;
		ss << "SakuraBuilder()" << std::endl;
		//ss << ".exciter(" <<
			//paramRegPtr->noiseMixRatio << ", " <<
			//paramRegPtr->noiseRate << ")" << std::endl;
		ss << ".exciterHiCut(" << paramRegPtr->exciterHiCutFreq << ", " << paramRegPtr->exciterHiCutQ << ")" << std::endl;
		ss << ".exciterHiCutEnv(" <<
			paramRegPtr->exciterHiCutEnv.attackTime * 1000.0 << ", " <<
			paramRegPtr->exciterHiCutEnv.holdTime * 1000.0 << ", " <<
			paramRegPtr->exciterHiCutEnv.decayTime * 1000.0 << ", " <<
			paramRegPtr->exciterHiCutEnv.sustainVolume <<
			")" << std::endl;
		ss << ".exciterLowCut(" << paramRegPtr->exciterLowCutFreq << ", " << paramRegPtr->exciterLowCutQ << ")" << std::endl;
		ss << ".string1(" << paramRegPtr->stringFMul1 << ", " << paramRegPtr->stringVFeedback1 << ", " << paramRegPtr->stringVAlpha1 << ")" << std::endl;
		ss << ".string2(" << paramRegPtr->stringFMul2 << ", " << paramRegPtr->stringVFeedback2 << ", " << paramRegPtr->stringVAlpha2 << ")" << std::endl;
		ss << ".stringMix(" << paramRegPtr->stringMix << ")" << std::endl;
		ss << ".stringEnv(" <<
			paramRegPtr->stringEnv.attackTime * 1000.0 << ", " <<
			paramRegPtr->stringEnv.holdTime * 1000.0 << ", " <<
			paramRegPtr->stringEnv.decayTime * 1000.0 << ", " <<
			(paramRegPtr->stringEnv.canSustain?"true":"false") << ", " <<
			paramRegPtr->stringEnv.sustainVolume << ", " <<
			paramRegPtr->stringEnv.releaseTime * 1000.0 <<
			")" << std::endl;
		ss << ".stringLevel(" << paramRegPtr->stringOutputLevel << ")" << std::endl;
		ss << ".comb(" << paramRegPtr->combPosition << ", " << paramRegPtr->combFeedback1 << ", " << paramRegPtr->combFeedback2 << ", " << paramRegPtr->combOutputLevel << ")" << std::endl;
		for(u_index i=0;i < 8;i++){
			bool enable=paramRegPtr->resonatorEnabled[i];
			if(!enable)continue;
			ss << ".resonator(" << i << ", " << paramRegPtr->resonatorFreq[i] << ", " << paramRegPtr->resonatorFeedback[i] << ")" << std::endl;
		}
		ss << ".resonatorLevel(" << paramRegPtr->resonatorOutputLevel << ")" << std::endl;
		ss << ".build()";
		std::string a=ss.str();
		std::memcpy(text, a.c_str(), a.length() + 1);
	}
	ImGui::SameLine();
	if(ImGui::Button("Random")){
		mixer.resetLimiter();
		Random random;
		//paramRegPtr->noiseMixRatio=random.nextFloat();
		//paramRegPtr->noiseRate=random.nextFloat();
		//
		paramRegPtr->exciterHiCutFreq=random.nextFloat();
		paramRegPtr->exciterHiCutQ=random.nextFloat() * 5.0;
		paramRegPtr->exciterHiCutEnv.attackTime=random.nextFloat();
		paramRegPtr->exciterHiCutEnv.holdTime=random.nextFloat();
		paramRegPtr->exciterHiCutEnv.decayTime=random.nextFloat();
		paramRegPtr->exciterHiCutEnv.sustainVolume=random.nextFloat();
		//
		paramRegPtr->exciterLowCutFreq=random.nextFloat();
		paramRegPtr->exciterLowCutQ=random.nextFloat() * 5.0;
		//
		fMulI1=random.nextInt(9);
		fMulI2=random.nextInt(9);
		paramRegPtr->stringVFeedback1=random.nextFloat() * 2.0 - 1.0;
		paramRegPtr->stringVAlpha1=random.nextFloat();
		paramRegPtr->stringVFeedback2=random.nextFloat() * 2.0 - 1.0;
		paramRegPtr->stringVAlpha2=random.nextFloat();
		//
		paramRegPtr->combFeedback1=random.nextFloat() * 2.0 - 1.0;
		paramRegPtr->combFeedback2=random.nextFloat() * 2.0 - 1.0;
		paramRegPtr->combOutputLevel=random.nextFloat();
		paramRegPtr->combPosition=random.nextFloat();
		//
		paramRegPtr->stringMix=random.nextFloat() * 2.0 - 1.0;
	}
}