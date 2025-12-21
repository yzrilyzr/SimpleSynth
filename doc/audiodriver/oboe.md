Oboe是Google推荐的Android低延迟音频API，适合移动端应用：

```cpp
#include <oboe/Oboe.h>
#include "Mixer2.h"
using namespace yzrilyzr_simplesynth;
using namespace oboe;

// 全局变量（根据实际项目调整为成员变量）
Mixer2* g_mixer;
std::shared_ptr<AudioStream> g_audioStream;
std::optional<std::future<void>> g_mixFuture;

// Oboe音频回调（处理音频合成与缓冲区填充）
class OboeAudioCallback : public AudioStreamCallback {
public:
    DataCallbackResult onAudioReady(AudioStream* stream, void* audioData, int32_t numFrames) override {
        // 等待上一次合成完成（避免线程冲突）
        if (g_mixFuture.has_value()) {
            try { g_mixFuture->get(); } 
            catch (const std::exception& e) { LOGI("合成失败：%s", e.what()); }
            g_mixFuture.reset();
        }

        // 检查缓冲区大小匹配（避免音频错位）
        if (numFrames != g_mixer->getBufferSize()) {
            LOGI("缓冲区大小不匹配：期望%d，实际%d", g_mixer->getBufferSize(), numFrames);
            return DataCallbackResult::Continue;
        }

        // 异步执行合成（避免阻塞音频线程）
        g_mixFuture = std::async(std::launch::async, []() { g_mixer->mix(); });

        // 复制合成结果到Oboe缓冲区（格式：Float → 适配高版本Android）
        float* outBuf = static_cast<float*>(audioData);
        u_sample* left = g_mixer->getOutput(0);
        u_sample* right = g_mixer->getOutput(1);

        for (int i = 0, j = 0; j < numFrames; ++j) {
            outBuf[i++] = left[j];  // 左声道
            outBuf[i++] = right[j]; // 右声道
        }

        return DataCallbackResult::Continue;
    }
};

// JNI方法：初始化Oboe与Mixer（供Android Java层调用）
extern "C" JNIEXPORT jint JNICALL
Java_com_yourpackage_AudioEngine_init(JNIEnv* env, jobject thiz, jint sdkVersion) {
    // 初始化Mixer
    g_mixer = new Mixer2(256);
    g_mixer->setSynthMode(IMixer::MODE_THREAD_POOL, 4); // 移动端建议限制线程数（避免耗电）
    g_mixer->setSampleRate(48000);
    g_mixer->setInstrumentProvider(std::make_shared<SimpleMidiInstrument>());

    // 配置Oboe音频流
    AudioStreamBuilder builder;
    auto callback = std::make_unique<OboeAudioCallback>();
    builder.setSharingMode(SharingMode::Exclusive)
           .setPerformanceMode(sdkVersion >= 26 ? PerformanceMode::LowLatency : PerformanceMode::PowerSaving)
           .setFormat(sdkVersion >= 23 ? AudioFormat::Float : AudioFormat::I16) // 低版本用16位整数
           .setChannelCount(ChannelCount::Stereo)
           .setSampleRate(g_mixer->getSampleRate())
           .setFramesPerDataCallback(g_mixer->getBufferSize())
           .setCallback(callback.release());

    // 打开音频流
    Result result = builder.openStream(g_audioStream);
    if (result != Result::OK || !g_audioStream) {
        LOGI("Oboe初始化失败：%s", convertToText(result));
        return -1;
    }

    // 开始播放
    g_audioStream->requestStart();
    return 0;
}

// JNI方法：发送MIDI事件（供Java层触发实时声音）
extern "C" JNIEXPORT void JNICALL
Java_com_yourpackage_AudioEngine_sendMIDIEvent(JNIEnv* env, jobject thiz, jint status, jint data1, jint data2) {
    if (g_mixer) {
        SynMixer::MODE_SINGLE_THREAD, 0);
```
