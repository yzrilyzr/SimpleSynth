//#pragma once
//#include "interface/NoteProcessor.h"
//#include "interpolator/Interpolator.h"
//
//namespace yzrilyzr_simplesynth{
//	class ValueProvider{
//		public:
//		virtual u_sample get(Note * note)=0;
//	};
//	class ConstValueProvider:public ValueProvider{
//		public:
//		u_sample v;
//		ConstValueProvider(u_sample v):v(v){}
//		u_sample get(Note * note)override{
//			return v;
//		}
//	};
//	class NoteProcProvider:public ValueProvider{
//		public:
//		NoteProcPtr v;
//		NoteProcProvider(NoteProcPtr v):v(v){}
//		u_sample get(Note * note)override{
//			return v->getAmp(*note);
//		}
//	};
//	class InterpolatorProvider:public ValueProvider{
//		public:
//		Interpolator v;
//		InterpolatorProvider(NoteProcPtr v):v(v){}
//		u_sample get(Note * note)override{
//			return v->getAmp(*note);
//		}
//	};
//	class SampleValue{
//		ValueProvider *provider;
//		u_sample temp;
//		public:
//		SampleValue(const u_sample v){
//			provider=new ConstValueProvider(v);
//		}
//		SampleValue(const NoteProcPtr v){
//			provider=new NoteProcProvider(v);
//		}
//		SampleValue(const Interpolator v){}
//		void setData(Note * n){
//			temp=provider->get(n);
//		}
//		u_sample get(){
//			return temp;
//		}
//		u_sample operator+(u_sample v1){
//			return v1 + temp;
//		}
//	};
//}