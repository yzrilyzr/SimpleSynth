#include "DSPRender.h"
#include "imgui.h"
#include "dsp/Compressor.h"
#include "../SimpleSynthProject.h"
using namespace yzrilyzr_util;
using namespace yzrilyzr_dsp;
void compressorRenderFunc(CurrentProjectContext & ctx, ProjectObject & obj){
	u_sp<Compressor> paramRegPtr=std::dynamic_pointer_cast<Compressor, ClassRegister>(obj.paramRegPtr);
	ImGui::PushItemWidth(200);
	ImGui::ProgressBar((float)paramRegPtr->getFinalGain(), ImVec2(200, 0));
	ImGui::PopItemWidth();
}