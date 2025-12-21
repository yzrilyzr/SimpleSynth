音源是音频合成的"音色库"，`SimpleSynth`支持多种加载方式，满足不同场景需求：

## 加载 SF2 文件

SoundFont 2 (SF2) 是一种广泛使用的音色库格式。

```cpp
#include "instrument/SF2FormatInstrument.h"
#include "io/FileInputStream.h"

auto instr = std::make_shared<yzrilyzr_simplesynth::SF2FormatInstrument>(
    yzrilyzr_io::FileInputStream("path_to_sf2.sf2")
);
mixer->setInstrumentProvider(instr);
```

## 加载 DLS 文件

Downloadable Sounds (DLS) 是另一种常见的音色库格式，尤其在Windows系统中。

```cpp
#include "instrument/DLSFormatInstrument.h"

auto instr = std::make_shared<yzrilyzr_simplesynth::DLSFormatInstrument>(
    yzrilyzr_io::FileInputStream("path_to_dls.dls")
);
mixer->setInstrumentProvider(instr);
```

## 使用内置音源

库中提供了一些简单的内置音源，方便快速测试和使用。

```cpp
// 使用简单的MIDI乐器
auto instr = std::make_shared<yzrilyzr_simplesynth::SimpleMidiInstrument>();
mixer->setInstrumentProvider(instr);

// 使用808鼓组
#include "instrument/TR808DrumSet.h"
#include "instrument/ReplaceableInstrument.h"

auto simpleInstr = std::make_shared<yzrilyzr_simplesynth::SimpleMidiInstrument>();
auto replaceableInstr = std::make_shared<yzrilyzr_simplesynth::ReplaceableInstrument>(simpleInstr);
//替换默认鼓组（SimpleMidiInstrument内包含默认鼓组）
//replaceableInstr->setDrumSet(std::make_shared<yzrilyzr_simplesynth::TR808DrumSet>());
mixer->setInstrumentProvider(replaceableInstr);
```

## 自定义音源

如果内置和标准格式的音源无法满足需求，您可以通过继承 `InstrumentProvider` 类来创建自己的音源。

```cpp
class MyCustomInstrumentProvider : public yzrilyzr_simplesynth::InstrumentProvider {
    // 重写必要的方法...
    NoteProcPtr get(s_bank_id bank, s_program_id program, u_sample_rate sampleRate) override;
    NoteProcPtr getDrumSet(s_bank_id bank, u_sample_rate sampleRate) override;
};

auto myInstr = std::make_shared<MyCustomInstrumentProvider>();
mixer->setInstrumentProvider(myInstr);
```

---