#include "ASineHarmonicWaveTable.h"
using namespace yzrilyzr_array;
namespace yzrilyzr_simplesynth{
	ASineHarmonicWaveTable::ASineHarmonicWaveTable() : SineHarmonicWaveTable(std::make_shared<Array<DoubleArray *>>(new DoubleArray * [8]{
		dBToAmp(4, new DoubleArray(new double[9]{0, 261, 2051, -25.7f, -26.2f, -21.7f, -21.2f, -15.6f, -07.6f}, 9)),
			dBToAmp(4, new DoubleArray(new double[9]{0, 261, 2051, -33.4f, -31.7f, -19.6f, -26.8f, -35.4f, -49.6f}, 9)),
			dBToAmp(4, new DoubleArray(new double[9]{0, 261, 2051, -34.3f, -22.3f, -15.4f, -39.5f, -48.3f, -45.2f}, 9)),
			dBToAmp(4, new DoubleArray(new double[9]{0, 261, 2051, -25.5f, -23.4f, -36.6f, -56.8f, -42.7f, -58.3f}, 9)),
			dBToAmp(4, new DoubleArray(new double[9]{0, 261, 2051, -25.7f, -36.2f, -45.8f, -39.7f, -48.8f, -58.6f}, 9)),
			dBToAmp(4, new DoubleArray(new double[9]{0, 261, 2051, -31.2f, -41.1f, -53.8f, -53.1f, -57.9f, -65.9f}, 9)),
			dBToAmp(4, new DoubleArray(new double[9]{0, 261, 2051, -37.2f, -51.5f, -40.4f, -55.4f, -63.1f, -61.0f}, 9)),
			dBToAmp(4, new DoubleArray(new double[9]{0, 261, 2051, -51.4f, -50.8f, -48.2f, -61.4f, -61.4f, -57.1f}, 9))
	}, 8)){}
}