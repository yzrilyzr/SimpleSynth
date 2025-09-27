#ifdef USEJNI
#include "jni.h"
#ifdef __cplusplus
extern "C" {
#endif
	JNIEXPORT jlong JNICALL Java_yzrilyzr_simplesynth_synth_source_AmpBuilderN_constructor(JNIEnv*, jobject);
	JNIEXPORT void JNICALL Java_yzrilyzr_simplesynth_synth_source_AmpBuilderN_destructor(JNIEnv*, jobject, jlong ptr);
	JNIEXPORT jlong JNICALL Java_yzrilyzr_simplesynth_synth_source_AmpBuilderN_getMs(JNIEnv*, jobject, jlong dtPtr);
#ifdef __cplusplus
}
#endif
#endif
