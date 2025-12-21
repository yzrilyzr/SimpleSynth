SDL是跨平台多媒体库，适合快速实现 Windows/macOS/Linux 的音频播放：
```cpp
#include "SDL.h"
#include "Mixer2.h"
using namespace yzrilyzr_simplesynth;

Mixer2* g_mixer; // 全局Mixer实例（供回调函数访问）

// SDL音频回调函数（系统自动调用，填充音频缓冲区）
void sdl_audio_callback(void* userdata, Uint8* stream, int len) {
    // 1. 执行音频合成
    g_mixer->mix();

    // 2. 复制合成结果到SDL缓冲区（格式：AUDIO_S32SYS → 32位有符号整数）
    int numSamples = g_mixer->getBufferSize();
    u_sample* left = g_mixer->getOutput(0);
    u_sample* right = g_mixer->getOutput(1);
    int32_t* outBuf = reinterpret_cast<int32_t*>(stream);

    for (int i = 0; i < numSamples; ++i) {
        // 音频数据归一化（u_sample→int32_t，避免溢出）
        outBuf[i * 2] = static_cast<int32_t>(left[i] * 0x7fffffff);  // 左声道
        outBuf[i * 2 + 1] = static_cast<int32_t>(right[i] * 0x7fffffff); // 右声道
    }
}

int main() {
    // 初始化Mixer
    g_mixer = new Mixer2(256);
    g_mixer->setSynthMode(IMixer::MODE_THREAD_POOL, -1);
    g_mixer->setSampleRate(48000);
    g_mixer->setInstrumentProvider(std::make_shared<SimpleMidiInstrument>());

    // 初始化SDL音频
    SDL_Init(SDL_INIT_AUDIO);
    SDL_AudioSpec spec{};
    spec.freq = g_mixer->getSampleRate();
    spec.format = AUDIO_S32SYS;
    spec.channels = 2; // 立体声
    spec.samples = g_mixer->getBufferSize();
    spec.callback = sdl_audio_callback;

    if (SDL_OpenAudio(&spec, nullptr) != 0) {
        fprintf(stderr, "SDL音频初始化失败：%s\n", SDL_GetError());
        return -1;
    }

    // 开始播放（发送一个测试事件）
    SDL_PauseAudio(0);
    g_mixer->sendInstantEvent(new NoteOn(0, 60, 100));

    // 等待5秒后退出
    std::this_thread::sleep_for(std::chrono::seconds(5));

    // 资源释放
    SDL_CloseAudio();
    SDL_Quit();
    delete g_mixer;
    return 0;
}
```