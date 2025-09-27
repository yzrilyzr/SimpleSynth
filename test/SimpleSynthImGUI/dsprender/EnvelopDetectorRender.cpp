#include "DSPRender.h"
#include "imgui.h"
#include "dsp/EnvelopDetector.h"
#include "../SimpleSynthProject.h"
using namespace yzrilyzr_util;
using namespace yzrilyzr_dsp;

void envelopDetectorRenderFunc(CurrentProjectContext & ctx, ProjectObject & obj){
	std::shared_ptr<EnvelopDetector> paramRegPtr=std::dynamic_pointer_cast<EnvelopDetector, ParamRegister>(obj.paramRegPtr);
	ImGui::ProgressBar((float)paramRegPtr->getEnvValue(), ImVec2(-1, 0));
}