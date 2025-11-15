#include "jni.h"
#include "Mixer2.h"
#include "SynthUtil.h"
#include "MixerSequence.h"
#include <io/ByteArrayInputStream.cpp>
#include "instrument/SimpleMIDIInstrument.h"
#include "lang/Runtime.h"
using namespace yzrilyzr_simplesynth;
using namespace yzrilyzr_io;
using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;

extern "C" JNIEXPORT jlong JNICALL Java_yzrilyzr_simplesynth_ntv_Mixer2_constructor(JNIEnv *, jobject, jint bufferSize){
	Mixer2 * mixer=new Mixer2(bufferSize);
	mixer->setSynthMode(Mixer2::MODE_THREAD_POOL, -1);
	mixer->getGlobalConfig().setInstrumentProvider(std::make_shared<SimpleMIDIInstrument>());
	return reinterpret_cast<jlong>(mixer);
}
extern "C" JNIEXPORT void JNICALL Java_yzrilyzr_simplesynth_ntv_Mixer2_destructor(JNIEnv *, jobject, jlong paramRegPtr){
	Mixer2 * mixer=reinterpret_cast<Mixer2 *>(paramRegPtr);
	delete mixer;
}
extern "C" JNIEXPORT void JNICALL Java_yzrilyzr_simplesynth_ntv_Mixer2_nSetSampleRate(JNIEnv *, jobject, jlong paramRegPtr, jint sr){
	Mixer2 * mixer=reinterpret_cast<Mixer2 *>(paramRegPtr);
	mixer->setSampleRate(sr);
}

extern "C" JNIEXPORT jboolean JNICALL Java_yzrilyzr_simplesynth_ntv_Mixer2_nHasData(JNIEnv *, jobject, jlong paramRegPtr){
	Mixer2 * mixer=reinterpret_cast<Mixer2 *>(paramRegPtr);
	return mixer->hasData()?JNI_TRUE:JNI_FALSE;
}
extern "C" JNIEXPORT jlong JNICALL Java_yzrilyzr_simplesynth_ntv_Mixer2_nGetCurrentSampleIndex(JNIEnv *, jobject, jlong paramRegPtr){
	Mixer2 * mixer=reinterpret_cast<Mixer2 *>(paramRegPtr);
	return mixer->getCurrentSampleIndex();
}

extern "C" JNIEXPORT void JNICALL Java_yzrilyzr_simplesynth_ntv_Mixer2_nSendMIDIBytes(JNIEnv *, jobject, jlong paramRegPtr, jint b1, jint b2, jint b3){
	Mixer2 * mixer=reinterpret_cast<Mixer2 *>(paramRegPtr);
	SynthUtil::sendMIDIBytes(mixer, b1 & 0xff, b2 & 0xff, b3 & 0xff);
}

extern "C" JNIEXPORT void JNICALL Java_yzrilyzr_simplesynth_ntv_Mixer2_nMix(JNIEnv *, jobject, jlong paramRegPtr){
	Mixer2 * mixer=reinterpret_cast<Mixer2 *>(paramRegPtr);
	mixer->mix();
}

extern "C" JNIEXPORT void JNICALL Java_yzrilyzr_simplesynth_ntv_Mixer2_nGetBuffer(
	JNIEnv * env, jobject, jlong paramRegPtr, jdoubleArray ch1, jdoubleArray ch2){
	Mixer2 * mixer=reinterpret_cast<Mixer2 *>(paramRegPtr);
	uint32_t limit=mixer->getBufferSize();

	// 检查数组长度
	//if(env->GetArrayLength(ch1) != limit || env->GetArrayLength(ch2) != limit){
		// 更好的异常抛出方式
		//jclass exClass=env->FindClass("java/lang/IllegalArgumentException");
		//env->ThrowNew(exClass, "Buffer length mismatch");
		//return;
	//}

	// 获取直接指针（避免拷贝）
	jdouble * nativeArray1=static_cast<jdouble *>(env->GetPrimitiveArrayCritical(ch1, NULL));
	jdouble * nativeArray2=static_cast<jdouble *>(env->GetPrimitiveArrayCritical(ch2, NULL));

	if(nativeArray1 == NULL || nativeArray2 == NULL){
		// 处理内存不足情况
		if(nativeArray1) env->ReleasePrimitiveArrayCritical(ch1, nativeArray1, JNI_ABORT);
		if(nativeArray2) env->ReleasePrimitiveArrayCritical(ch2, nativeArray2, JNI_ABORT);
		return;
	}

	// 直接内存拷贝
	memcpy(nativeArray1, mixer->getOutput(0), limit * sizeof(jdouble));
	memcpy(nativeArray2, mixer->getOutput(1), limit * sizeof(jdouble));

	// 释放临界区数组
	env->ReleasePrimitiveArrayCritical(ch1, nativeArray1, 0);
	env->ReleasePrimitiveArrayCritical(ch2, nativeArray2, 0);
}

static std::unordered_map<jlong, std::shared_ptr<MixerSequence>> sequenceMap;

extern "C" JNIEXPORT jlong JNICALL Java_yzrilyzr_simplesynth_ntv_Mixer2Sequence_constructor(JNIEnv * env, jobject, jint type, jbyteArray data){
	jbyte * dataPtr=env->GetByteArrayElements(data, NULL);
	jsize length=env->GetArrayLength(data);
	ByteArray ba(length);
	memcpy(ba._array, dataPtr, length);
	env->ReleaseByteArrayElements(data, dataPtr, 0);
	std::shared_ptr<MixerSequence> seq=nullptr;
	ByteArrayInputStream bba(ba);
	if(type == 0)seq=SynthUtil::parseMIDI(bba);
	else if(type == 1)seq=SynthUtil::parseXM(bba);
	if(seq){
		jlong handle=reinterpret_cast<jlong>(seq.get());
		sequenceMap[handle]=seq;
		return handle;
	}
	return 0;
}
extern "C" JNIEXPORT void JNICALL Java_yzrilyzr_simplesynth_ntv_Mixer2Sequence_destructor(JNIEnv *, jobject, jlong paramRegPtr){
	sequenceMap.erase(paramRegPtr);
}

extern "C" JNIEXPORT void JNICALL Java_yzrilyzr_simplesynth_ntv_Mixer2Sequence_nPost(JNIEnv *, jobject, jlong paramRegPtr, jlong mixerPtr){
	std::shared_ptr<MixerSequence> seq=sequenceMap[paramRegPtr];
	Mixer2 * mixer=reinterpret_cast<Mixer2 *>(mixerPtr);
	seq->postToMixer(mixer, 0);
}