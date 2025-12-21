库提供了将合成的音频导出为WAV文件的功能，支持导出为单个混合文件或分轨文件。

```cpp
#include "util/WAVWriter.h"

void exportToWav(const std::string& baseFileName, std::shared_ptr<yzrilyzr_simplesynth::MixerSequence> seq) {
    // 创建一个用于导出的临时mixer
    yzrilyzr_simplesynth::Mixer2 exportMixer(8192); // 使用更大的缓冲区提高导出速度
    exportMixer.setSampleRate(48000);
    exportMixer.setSynthMode(yzrilyzr_simplesynth::IMixer::MODE_THREAD_POOL, -1);
    exportMixer.setInstrumentProvider(mixer->getInstrumentProvider()); // 使用与实时播放相同的音源
    exportMixer.setUseLimiter(true); // 启用限制器防止削波

    // 将事件序列发送到导出mixer
    seq->postToMixer(&exportMixer, 0);

    // 创建WAV文件写入器
    yzrilyzr_simplesynth::WAVWriter writer(baseFileName + ".wav");
    writer.prepare(exportMixer.getSampleRate(), 32, yzrilyzr_simplesynth::WAVWriter::FORMAT_FLOAT, 2);

    // 循环合成并写入文件，直到所有事件处理完毕
    while (exportMixer.hasData()) {
        exportMixer.mix();
        for (uint32_t sample = 0; sample < exportMixer.getBufferSize(); ++sample) {
            writer.writeFloat((float)exportMixer.getOutput(0)[sample]); // 左声道
            writer.writeFloat((float)exportMixer.getOutput(1)[sample]); // 右声道
            writer.nextSample();
        }
    }
    
    writer.end(); // 完成文件写入
}
```
