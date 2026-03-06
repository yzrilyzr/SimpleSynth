#include "DSPRender.h"
#include "imgui.h"
#include "dsp/EnvelopDetector.h"
#include "../SimpleSynthProject.h"
using namespace yzrilyzr_util;
using namespace yzrilyzr_dsp;

void envelopDetectorRenderFunc(CurrentProjectContext & ctx, ProjectObject & obj){
	u_sp<EnvelopDetector> paramRegPtr=std::dynamic_pointer_cast<EnvelopDetector, ClassRegister>(obj.paramRegPtr);
	ImGui::PushItemWidth(200);
	ImGui::ProgressBar((float)paramRegPtr->getEnvValue(), ImVec2(200, 0));
	ImGui::PopItemWidth();
}