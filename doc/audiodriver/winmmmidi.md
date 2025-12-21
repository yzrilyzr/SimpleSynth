WinMM (Windows Multimedia) API 可以用来接收和发送MIDI消息。

```cpp
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

HMIDIIN hMidiIn;

// MIDI输入回调函数
void CALLBACK MidiInProc(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
    if (wMsg == MIM_DATA) {
        DWORD midiMessage = dwParam1;
        BYTE status = midiMessage & 0xFF;
        BYTE data1 = (midiMessage >> 8) & 0xFF;
        BYTE data2 = (midiMessage >> 16) & 0xFF;
        
        // 将接收到的MIDI消息立即发送给mixer
        yzrilyzr_simplesynth::SynthUtil::sendMIDIBytes(mixer.get(), status, data1, data2, "WinMM_MIDI");
    }
}

// 在主函数中打开MIDI输入设备
void openMidiInput() {
    UINT numDevices = midiInGetNumDevs();
    for (UINT i = 0; i < numDevices; ++i) {
        MIDIINCAPS mic;
        if (midiInGetDevCaps(i, &mic, sizeof(MIDIINCAPS)) == MMSYSERR_NOERROR) {
            // 可以根据设备名进行筛选
            if (midiInOpen(&hMidiIn, i, reinterpret_cast<DWORD_PTR>(MidiInProc), 0, CALLBACK_FUNCTION) == MMSYSERR_NOERROR) {
                midiInStart(hMidiIn);
                std::cout << "MIDI input device opened: " << mic.szPname << std::endl;
                return;
            }
        }
    }
    std::cerr << "Failed to open MIDI input device." << std::endl;
}
```