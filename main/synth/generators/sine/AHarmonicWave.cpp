#include "AHarmonicWave.h"
using namespace yzrilyzr_array;
namespace yzrilyzr_simplesynth{
	AHarmonicWave::AHarmonicWave(std::shared_ptr<PhaseSrc> freq): SineHarmonicWave(freq,
																				  new DoubleArray(new double[12]{
																						  0.0066f,
																						  0.1894f,
																						  0.1407f,
																						  0.1355f,
																						  0.0309f,
																						  0.01f,
																						  0.0113f,
																						  0.0214f,
																						  0.0222f,
																						  0.018f,
																						  0.0184f,
																						  0.0121f
																				  }, 12)){
	}
}