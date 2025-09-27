#include "NoteProcessor.h"
#include "events/Note.h"
#include "dsp/DSP.h"

using namespace yzrilyzr_array;
using namespace yzrilyzr_util;
using namespace yzrilyzr_dsp;
namespace yzrilyzr_simplesynth {
    bool NoteProcessor::noMoreData(Note &note) {
        return note.closed(*note.cfg) || note.fclosed(*note.cfg);
    }

    void NoteProcessor::registerParamPhaseSrc(const std::string &name,
                                              std::shared_ptr<PhaseSrc> *value) {
        registerParam(name, ParamType::PhaseSrc, value, nullptr, nullptr);
    }

    void NoteProcessor::registerParamSrc(const std::string &name, NoteProcPtr *value) {
        registerParam(name, ParamType::NoteSrc, value, nullptr, nullptr);
    }

    void NoteProcessor::registerParamOscSrc(const std::string &name, std::shared_ptr<Osc> *value) {
        registerParam(name, ParamType::OscSrc, value, nullptr, nullptr);
    }

    void NoteProcessor::registerParamDSP(const std::string &name, std::shared_ptr<DSP> *value) {
        registerParam(name, ParamType::DSP, value, nullptr, nullptr);
    }

    void NoteProcessor::registerParamSample(const std::string &name,
                                            std::shared_ptr<SampleProvider> *value) {
        registerParam(name, ParamType::SampleData, value, nullptr, nullptr);
    }

    std::string NoteProcessor::toString() const {
        return "NoteProcessor";
    }
}