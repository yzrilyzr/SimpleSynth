#include "vstgui/plugin-bindings/vst3editor.h"
#include "vstgui/lib/controls/cbuttons.h"
#include "vstgui/lib/cviewcontainer.h"
#include "util/Lang.h"

using namespace VSTGUI;

// 在控制器中创建自定义多列控件
class MultiColumnProgramSelector : public CViewContainer{
    public:
    MultiColumnProgramSelector(const CRect & size, IControlListener * listener, Steinberg::Vst::ParamID baseTag, yzrilyzr_util::Lang & langManager)
        : CViewContainer(size), listener(listener), baseTag(baseTag), langManager(langManager){
        setupColumns();
    }

    void setupColumns(){
        setBackgroundColor(CColor(60, 60, 60));

        const int columns=4;
        const int itemsPerColumn=32; // 128 / 4
        float columnWidth=getWidth() / columns;

        for(int col=0; col < columns; col++){
            CRect colRect(col * columnWidth, 0, (col + 1) * columnWidth, getHeight());
            auto columnContainer=new CViewContainer(colRect);

            // 为每列创建按钮
            float buttonHeight=20.0f;
            for(int row=0; row < itemsPerColumn; row++){
                int programIndex=col * itemsPerColumn + row;
                if(programIndex >= 128) break;

                CRect buttonRect(2, row * buttonHeight + 2, columnWidth - 2, (row + 1) * buttonHeight);
                auto button=new CTextButton(buttonRect, listener, baseTag + programIndex);

                // 获取程序名称
                button->setTitle(langManager.getc(yzrilyzr_lang::String("midi.program.")+programIndex));

                button->setRoundRadius(3.0f);
                button->setGradient(CGradient::create(0, 0, CColor(80, 80, 80), CColor(60, 60, 60)));
                button->setGradientHighlighted(CGradient::create(0, 0, CColor(100, 100, 150), CColor(80, 80, 120)));
                button->setFrameColor(CColor(120, 120, 120));

                columnContainer->addView(button);
            }

            addView(columnContainer);
        }
    }

    void setSelectedProgram(int program){
        // 使用安全的方式遍历子视图
        for(int32_t i=0; i < getNbViews(); i++){
            if(auto columnContainer=dynamic_cast<CViewContainer *>(getView(i))){
                for(int32_t j=0; j < columnContainer->getNbViews(); j++){
                    if(auto button=dynamic_cast<CTextButton *>(columnContainer->getView(j))){
                        int buttonProgram=button->getTag() - baseTag;
                        bool isSelected=(buttonProgram == program);

                        if(isSelected){
                            button->setGradient(CGradient::create(0, 0, CColor(100, 150, 100), CColor(80, 120, 80)));
                        } else{
                            button->setGradient(CGradient::create(0, 0, CColor(80, 80, 80), CColor(60, 60, 60)));
                        }
                    }
                }
            }
        }
    }

    private:
    IControlListener * listener;
    int32_t baseTag;
    yzrilyzr_util::Lang & langManager;
};