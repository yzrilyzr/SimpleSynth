#define SDL_MAIN_HANDLED
#define IMGUI_ENABLE_FREETYPE
#include "ImGuiFileDialog.h"
#include "Mixer2.h"
#include "SDL.h"
#include "SynthUtil.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_sdl2.h"
#include "imgui.h"
#include "implot.h"
#include "imnodes.h"
#include "instrument/ReplaceableInstrument.h"
#include "instrument/SimpleMIDIInstrument.h"
#include "io/File.h"
#include "lang/Runtime.h"
#include "lang/System.h"
#include "lang/Thread.h"
#include "stdio.h"
#include "synth/generators/physic/TwoStringResonator.h"
#include "util/Lang.h"
#include "util/Locale.h"
#include "util/Util.h"
#include "windows.h"
#include "yzrutil.h"
#include <cstddef>
#include <iostream>
#include <string>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include "SDL_opengles2.h"
#else
#include "SDL_opengl.h"
#endif

#pragma comment(lib,"winmm.lib")
#if defined(_WIN32) && defined (_DEBUG)
#endif
#include "MenuRegister.hpp"
#include "RegisterAllModules.h"
#include "nlohmann/json.hpp"
using json=nlohmann::json;

#include "SimpleSynthWindow.h"
#include "SimpleSynthProject.h"

using namespace yzrilyzr_simplesynth;
using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;
using namespace yzrilyzr_io;

int floatBufferLen=256;
IMixer * mixer;

HMIDIIN hMidiIn;
MIDIHDR midiHdr;
#define SYSEX_BUFFER_SIZE 1024
BYTE sysexBuffer[SYSEX_BUFFER_SIZE]={0};

MenuRegister allNoteProcessor;
MenuRegister allDSP;
MenuRegister allInterpolator;
MenuRegister allPhaseSrc;
MenuRegister allSubModule;

CurrentProjectContext ctx;
void saveFileDlg(){
	IGFD::FileDialogConfig cfg;
	cfg.fileName=ctx.LANG.get("save_file_dialog.default_file_name").tostring();
	cfg.flags=ImGuiFileDialogFlags_Modal;
	ImGuiFileDialog::Instance()->OpenDialog("SaveFileDlgKey", ctx.LANG.get("save_file_dialog.title").tostring(UTF8), ".ssp", cfg);
}
void newProject(){
	ctx.file="";
	ctx.objects.clear();
	ctx.dragPayloadType=nullptr;
	ctx.finalProcessor=nullptr;
}
void openFileDlg(){
	 // 创建对话框配置
	IGFD::FileDialogConfig config;
	config.countSelectionMax=1;   // 单选文件
	config.flags=ImGuiFileDialogFlags_Modal;  // 模态对话框
	// 打开文件对话框
	ImGuiFileDialog::Instance()->OpenDialog(
		"OpenFileDlgKey",    // 对话框唯一标识key
		ctx.LANG.get("open_file_dialog.title").tostring(UTF8),         // 对话框标题
		".ssp",      // 文件过滤器
		config              // 配置参数
	);
}
MenuRegister::MenuRegisterObject notfound;
MenuRegister::MenuRegisterObject findObjectAt(const String & category, const String & name, MenuRegister & at1){
	for(auto & obj : at1.regObjects[category]){
		if(obj.name == name){
			return obj;
		}
	}
	for(auto & obj : at1.allRegObjects){
		if(obj.name == name){
			return obj;
		}
	}
	notfound.name="##/NOT_FOUND/##";
	return notfound;
}
void name2obj(const String & category, const String & name, String * name1, String * category1, String * showName, u_sp<ClassRegister> * obj, MenuRegister::RenderFunc * rfunc, bool * enableOriginalRender){
	auto mobj=findObjectAt(category, name, allNoteProcessor);
	if(mobj.name == notfound.name)mobj=findObjectAt(category, name, allDSP);
	if(mobj.name == notfound.name)mobj=findObjectAt(category, name, allInterpolator);
	if(mobj.name == notfound.name)mobj=findObjectAt(category, name, allPhaseSrc);
	if(mobj.name == notfound.name)mobj=findObjectAt(category, name, allSubModule);
	if(mobj.name != notfound.name){
		*name1=mobj.name;
		*category1=mobj.category;
		*showName=mobj.showName;
		*obj=mobj.cfunc();
		(*obj)->registerParams();
		*rfunc=mobj.rfunc;
		*enableOriginalRender=mobj.enableOriginalRender;
	}
}
void closeAndExit(){
	midiInStop(hMidiIn);
	midiInClose(hMidiIn);
	SDL_CloseAudio();
	delete mixer;
	SynthUtil::deleteStatic();
	SDL_Quit();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImPlot::DestroyContext();
	ImGui::DestroyContext();
}

void fill_audio_pcm(void * userdata, Uint8 * stream, int len){
	u_time t=(u_time)System::nanoTime();
	/*try{
		std::unique_lock<std::shared_mutex> lock(ctx.mixerLock);
		mixer->commitChannels();
		mixer->waitForChannels();
		mixer->mixChannels();
	} catch(Exception e){
		std::cout << e.what() << std::endl;
		closeAndExit();
	}*/
	mixer->mix();
	for(uint32_t sample=0, j=0, buf=mixer->getBufferSize(), chc=mixer->getOutputChannelCount(); sample < buf; sample++){
		for(uint32_t ch=0; ch < chc; ch++){
			double f1=mixer->getOutput(ch)[sample];
			f1=Util::clamp(f1, -1.0, 1.0);
			int32_t c1=(int32_t)(f1 * Integer_MAX_VALUE);
			stream[j++]=(Uint8)(c1 & 0xff);
			stream[j++]=(Uint8)((c1 >> 8) & 0xff);
			stream[j++]=(Uint8)((c1 >> 16) & 0xff);
			stream[j++]=(Uint8)((c1 >> 24) & 0xff);
		}
	}
	ctx.processTime=(u_time_f)((u_time_f)(System::nanoTime() - t) / 1000000000.0);
	//cout << (float)(mixer->getCurrentSampleIndex()/ mixer->getSampleRate())<< endl;
}
void CALLBACK MidiInProc(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2){
	switch(wMsg){
		case MIM_DATA:
		{
			DWORD_PTR midiMessage=dwParam1;
			BYTE status=midiMessage & 0xFF;
			BYTE data1=(midiMessage >> 8) & 0xFF;
			BYTE data2=(midiMessage >> 16) & 0xFF;
			SynthUtil::sendMIDIBytes(mixer, status, data1, data2, "WM_MIDI_Instant");
		}
		break;
		case MIM_LONGDATA:
		{
			LPMIDIHDR phdr=(LPMIDIHDR)dwParam1;
			//String sysex(phdr->lpData, 0, phdr->dwBytesRecorded);
			//std::cout << sysex.c_str() << std::endl;
			//SynthUtil::sendSysexMessage(sysex);
		}
		break;
		case MIM_OPEN:
			std::cout << "MIDI Open" << std::endl;
			break;
		case MIM_CLOSE:
			std::cout << "MIDI Close" << std::endl;
			break;
		case MIM_ERROR:
			std::cout << "MIDI Error" << std::endl;
			break;
		default:
			break;
	}
}
int openMIDIDevice(){
	UINT numDevices=midiInGetNumDevs();
	bool deviceFound=false;

	// 枚举所有 MIDI 输入设备
	for(UINT i=0; i < numDevices; ++i){
		MIDIINCAPS mic;
		if(midiInGetDevCaps(i, &mic, sizeof(MIDIINCAPS)) == MMSYSERR_NOERROR){
			String deviceName(mic.szPname);
			if(deviceName.contains("SimpleSynthIn")){
				// 找到 SimpleSynth 设备
				MMRESULT result=midiInOpen(&hMidiIn, i, reinterpret_cast<DWORD_PTR>(MidiInProc), 0, CALLBACK_FUNCTION);
				if(result != MMSYSERR_NOERROR){
					std::cerr << ctx.LANG.get("wm_midi_device.open_failed") << std::endl;
					return 1;
				}
				deviceFound=true;
				break;
			}
		}
	}

	if(!deviceFound){
		std::cerr << ctx.LANG.get("wm_midi_device.not_found") << std::endl;
		return 1;
	}

	// 启动 MIDI 输入
	ZeroMemory(&midiHdr, sizeof(MIDIHDR));
	midiHdr.lpData=(LPSTR)sysexBuffer;
	midiHdr.dwBufferLength=SYSEX_BUFFER_SIZE;
	midiInPrepareHeader(hMidiIn, &midiHdr, sizeof(MIDIHDR));
	midiInAddBuffer(hMidiIn, &midiHdr, sizeof(MIDIHDR));
	midiInStart(hMidiIn);
	return 0;
}
int main(int argc, char * argv[]){
	ctx.sampleRate=48000;
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO) != 0){
		fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
		return -1;
	}
	mixer=new Mixer2(floatBufferLen);
	mixer->setSynthMode(Mixer2::MODE_THREAD_POOL, -1);
	mixer->setSampleRate(ctx.sampleRate);
	u_sp<SimpleMIDIInstrument> simple=mksp<SimpleMIDIInstrument>();
	u_sp <ReplaceableInstrument> rin=mksp<ReplaceableInstrument>(simple);
	mixer->getGlobalConfig().setInstrumentProvider(rin);
	ctx.setMixer(mixer);
	SDL_AudioSpec spec;
	spec.freq=ctx.sampleRate;
	spec.format=AUDIO_S32SYS;
	spec.channels=2;
	spec.silence=0;
	spec.samples=floatBufferLen;
	spec.callback=fill_audio_pcm;
	spec.userdata=nullptr;
	if(SDL_OpenAudio(&spec, nullptr)){
		fprintf(stderr, "Failed to open audio device, %s\n", SDL_GetError());
		return -1;
	}
	SDL_PauseAudio(0);
	openMIDIDevice();
	mixer->setUseLimiter(false);
	for(File & f : File(File("lang"),"SimpleSynth").listFiles()){
		String name=f.getName();
		if(!name.endsWith(".properties"))continue;
		name=name.substring(0, name.length() - 11);
		auto sp=name.split("_");
		String l1=sp[0];
		String l2="";
		if(sp.length == 2)l2=sp[1];
		ctx.LANG.add(mksp<Locale>(l1, l2), mksp<Properties>(f));
	}
	registerAllNoteProcessor(ctx.LANG, allNoteProcessor);
	registerAllDSP(ctx.LANG, allDSP);
	registerAllInterpolator(ctx.LANG, allInterpolator);
	registerAllPhaseSrc(ctx.LANG, allPhaseSrc);

// Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
	// GL ES 2.0 + GLSL 100
	const char * glsl_version="#version 100";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
	// GL 3.2 Core + GLSL 150
	const char * glsl_version="#version 150";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
	// GL 3.0 + GLSL 130
	const char * glsl_version="#version 130";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

// From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
	SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

// Create window with graphics context
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_WindowFlags window_flags=(SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	SDL_Window * window=SDL_CreateWindow(ctx.LANG.getc("app.name"), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
	if(window == nullptr){
		printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
		return -1;
	}

	SDL_GLContext gl_context=SDL_GL_CreateContext(window);
	SDL_GL_MakeCurrent(window, gl_context);
	SDL_GL_SetSwapInterval(1); // Enable vsync

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();

	ImGui::CreateContext();	
	ImPlot::CreateContext();
	ImNodes::CreateContext();

	ImGuiIO & io=ImGui::GetIO();
	io.ConfigFlags|=ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags|=ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	//ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Load Fonts
	// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
	// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
	// - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
	// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
	// - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
	// - Read 'docs/FONTS.md' for more instructions and details.
	// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
	// - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
	//io.Fonts->AddFontDefault();

	for(auto f : File("fonts").listFiles()){
		if(!f.getName().endsWith(".ttf"))continue;
		ImFontConfig config;
		config.GlyphExtraSpacing=ImVec2(0, 0);
		config.GlyphOffset=ImVec2(0, 0);
		io.Fonts->AddFontFromFileTTF(f.getAbsolutePath().c_str(), 25.0, &config, io.Fonts->GetGlyphRangesChineseFull());
	}
	io.Fonts->Build();
	io.Fonts->AddFontDefault();

	// Our state
	ImVec4 clear_color=ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	static bool showEQWindow=false;
	static bool showChannelWindow=true;
	static bool showImportBankWindow=false;
	static bool showMixerWindow=true;
	static bool showInstSourceWindow=false;
	static bool showOscilloscopeWindow=false;
	static bool showToSourceWindow=false;
	static bool showPlayFileWindow=false;
	static bool showDemoWindow=false;
	ctx.openFile("test.ssp");
	// Main loop
	bool done=false;
#ifdef __EMSCRIPTEN__
// For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
// You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
	io.IniFilename=nullptr;
	EMSCRIPTEN_MAINLOOP_BEGIN
	#else
	while(!done)
	#endif
	{
		// Poll and handle events (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		SDL_Event event;
		while(SDL_PollEvent(&event)){
			ImGui_ImplSDL2_ProcessEvent(&event);
			if(event.type == SDL_QUIT)
				done=true;
			if(event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
				done=true;
		}
		if(SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED){
			SDL_Delay(500);
			continue;
		}

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();
		if(ImGui::BeginMainMenuBar()){
			if(ImGui::BeginMenu(ctx.LANG.getc("menu.project"))){
				if(ImGui::MenuItem(ctx.LANG.getc("menu.project.new"))){
					ctx.newProject();
				}
				if(ImGui::MenuItem(ctx.LANG.getc("menu.project.open"))){
					openFileDlg();
				}
				if(ImGui::MenuItem(ctx.LANG.getc("menu.project.save"))){
					if(ctx.file == ""){
						saveFileDlg();
					} else ctx.saveFile();
				}
				if(ImGui::MenuItem(ctx.LANG.getc("menu.project.save_as"))){
					saveFileDlg();
				}
				if(ImGui::MenuItem(ctx.LANG.getc("menu.project.save_as_submodule"))){
					ctx.saveAsSub(false);
				}
				ImGui::EndMenu();
			}

			if(ImGui::BeginMenu(ctx.LANG.getc("menu.windows"))){
				ImGui::MenuItem(ctx.LANG.getc("menu.windows.eq"), nullptr, &showEQWindow);
				ImGui::MenuItem(ctx.LANG.getc("menu.windows.channel"), nullptr, &showChannelWindow);
				ImGui::MenuItem(ctx.LANG.getc("menu.windows.import_bank"), nullptr, &showImportBankWindow);
				ImGui::MenuItem(ctx.LANG.getc("menu.windows.mixer"), nullptr, &showMixerWindow);
				ImGui::MenuItem(ctx.LANG.getc("menu.windows.inst_source"), nullptr, &showInstSourceWindow);
				ImGui::MenuItem(ctx.LANG.getc("menu.windows.oscilloscope"), nullptr, &showOscilloscopeWindow);
				ImGui::MenuItem(ctx.LANG.getc("menu.windows.objtostring"), nullptr, &showToSourceWindow);
				ImGui::MenuItem(ctx.LANG.getc("menu.windows.play_file"), nullptr, &showPlayFileWindow);
				ImGui::MenuItem(ctx.LANG.getc("menu.windows.demo"), nullptr, &showDemoWindow);
				ImGui::EndMenu();
			}
			if(ImGui::BeginMenu(ctx.LANG.getc("menu.lang_select"))){
				for(auto & pp : ctx.LANG.getMap()){
					auto sec=pp.second;
					bool select=pp.first == ctx.LANG.getCurrentLocale();
					if(ImGui::MenuItem(sec->get("lang").c_str(UTF8), nullptr, &select)){
						ctx.LANG.setCurrentLocale(pp.first);
						allNoteProcessor.clear();
						allDSP.clear();
						allInterpolator.clear();
						allPhaseSrc.clear();
						registerAllNoteProcessor(ctx.LANG, allNoteProcessor);
						registerAllDSP(ctx.LANG, allDSP);
						registerAllInterpolator(ctx.LANG, allInterpolator);
						registerAllPhaseSrc(ctx.LANG, allPhaseSrc);
					}
				}
				ImGui::EndMenu();
			}
			ImGui::Separator();
			mainMenuBar(ctx.LANG.getc("menu.notesrc"), allNoteProcessor, ctx);
			mainMenuBar(ctx.LANG.getc("menu.dsp"), allDSP, ctx);
			mainMenuBar(ctx.LANG.getc("menu.interpolator"), allInterpolator, ctx);
			mainMenuBar(ctx.LANG.getc("menu.phasesrc"), allPhaseSrc, ctx);
			mainMenuBar(ctx.LANG.getc("menu.submodule"), allSubModule, ctx);

			ImGui::EndMainMenuBar();
		}
		if(ImGuiFileDialog::Instance()->Display("SaveFileDlgKey")){
			if(ImGuiFileDialog::Instance()->IsOk()){
				ctx.file=ImGuiFileDialog::Instance()->GetFilePathName();
				ctx.saveFile();
			}
			ImGuiFileDialog::Instance()->Close();
		}
		if(ImGuiFileDialog::Instance()->Display("OpenFileDlgKey")){
			if(ImGuiFileDialog::Instance()->IsOk()){
				try{
					ctx.openFile(ImGuiFileDialog::Instance()->GetFilePathName());
				} catch(Exception & e){
					std::cout << e.what() << std::endl;
				}
			}
			ImGuiFileDialog::Instance()->Close();
		}
		ctx.renderCurrentProjectWindow();
		if(showMixerWindow)mixerSettingWindow(ctx);
		if(showChannelWindow)channelSettingWindow(ctx);
		if(showImportBankWindow)importFromBankWindow(ctx);
		if(showPlayFileWindow)fileOpenWindow(ctx);
		if(showInstSourceWindow)instrumentSourceWindow(ctx);
		if(showOscilloscopeWindow)oscilloscopeWindow(ctx);
		if(showEQWindow)eqWindow(ctx);
		if(showToSourceWindow)objectToStringWindow(ctx);
		if(showDemoWindow)ImGui::ShowDemoWindow(&showDemoWindow);
		ctx.notificationManager.Draw();
		ImGui::Render();
		glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
		glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		SDL_GL_SwapWindow(window);
	}

	SDL_GL_DeleteContext(gl_context);
	SDL_DestroyWindow(window);
	closeAndExit();

	return 0;
}