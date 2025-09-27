#include "DSPRender.h"
#include "imgui.h"
#include "dsp/Compressor.h"
#include "../SimpleSynthProject.h"
using namespace yzrilyzr_util;
using namespace yzrilyzr_dsp;
void compressorRenderFunc(CurrentProjectContext & ctx, ProjectObject & obj){
	std::shared_ptr<Compressor> paramRegPtr=std::dynamic_pointer_cast<Compressor, ParamRegister>(obj.paramRegPtr);
	ImGui::ProgressBar((float)paramRegPtr->getFinalGain(), ImVec2(-1, 0));
}