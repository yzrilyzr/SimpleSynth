#pragma once
#include "SimpleSynth.h"
#include "array/Array.hpp"
#include "lang/StringFormat.hpp"
#include "events/ChannelConfig.h"

namespace yzrilyzr_simplesynth{
	/**
	 * @class Note
	 * @brief MIDI音符表示类，包含音符状态和合成参数
	 * @note 采用十二平均律表示法，中央C4对应ID=60
	 */
	EBCLASS(Note){
	public:
		// 预定义音符常量（十二平均律MIDI编号）
	static constexpr s_note_id_i const C0=12;  ///< C0音符ID
	static constexpr s_note_id_i const C1=24;  ///< C1音符ID
	static constexpr s_note_id_i const C2=36;  ///< C2音符ID
	static constexpr s_note_id_i const C3=48;  ///< C3音符ID
	static constexpr s_note_id_i const C4=60;  ///< 中央C4音符ID
	static constexpr s_note_id_i const A4=69;  ///< A4标准音高(440Hz)
	static constexpr s_note_id_i const C5=72;  ///< C5音符ID
	static constexpr s_note_id_i const C6=84;  ///< C6音符ID
	static constexpr s_note_id_i const C7=96;  ///< C7音符ID
	static constexpr s_note_id_i const C8=108; ///< C8音符ID
	static constexpr s_note_id_i const C9=120; ///< C9音符ID

	ChannelConfig * cfg=nullptr;
	// 音符基本属性
	uint8_t uniqueID=-1;                ///< 音符唯一标识符
	s_note_id_i id=0;                   ///< MIDI音符编号(0-127)
	s_note_vel velocitySynth=0;         ///< 实际合成力度[0,1](映射后的线性值)
	s_note_vel velocity=0;              ///< 原始力度[0,1](含触后变化)(未映射的值)

	// 合成相关参数
	s_phase phaseSynth=0;               ///< 当前合成相位(归一化0-1)
	u_freq freqSynth=0;                 ///< 实际合成频率(Hz) 仅用于计算周期，禁止用于直接计算相位

	// 时间状态
	u_time startAtTime=0;               ///< NoteOn触发时间(秒)
	u_time passedTime=0;                ///< NoteOn后经过时间(秒)
	u_time closedAtTime=0;              ///< NoteOff触发时间(秒)
	u_time closedPassedTime=0;          ///< NoteOff后经过时间(秒)
	u_time forceCloseAtTime=0;         ///< 强制NoteOff触发时间(秒)

	// 滑音/弯音参数
	s_note_id_i lastPortamentoID=0;    ///< 上次滑音目标音符ID
	s_note_id portamentoDeltaID=0;      ///< 当前滑音差值(半音数)
	s_note_id idSynth=0;                ///< 实际合成音符ID(含弯音)
	s_note_id pitchBend=0;                ///< 音符弯音

	// 状态标志
	bool dataInvalidated=false;         ///< 数据是否失效
	bool noMoreData=false;              ///< 是否终止数据生成

	/**
	 * @brief 构造函数
	 * @param uniqueID 音符唯一标识符
	 */
	Note(uint8_t uniqueID) : uniqueID(uniqueID){}

	/**
	 * @brief 获取音符描述字符串
	 * @return 格式化为"[Note:{ID} Vel:{力度}]"的字符串
	 */
	yzrilyzr_lang::String toString() const override;

	inline bool closed(const ChannelConfig & cfg)const{
		return closedAtTime != 0 && closedAtTime < cfg.currentTime||fclosed(cfg);
	}

	inline bool fclosed(const ChannelConfig & cfg)const{
		return forceCloseAtTime != 0 && forceCloseAtTime < cfg.currentTime;
	}
	/**
	 * @brief 请求关闭音符(触发NoteOff流程，进入Release)
	 */
	inline void requestClose(const ChannelConfig & cfg){
		if(closedAtTime == 0)closedAtTime=cfg.currentTime;
	}
	/**
	 * @brief 强制立即关闭音符(经过Release阶段，但是更短)
	 */
	inline void forceClose(const ChannelConfig & cfg){
		if(closedAtTime == 0)closedAtTime=cfg.currentTime;
		if(forceCloseAtTime == 0)forceCloseAtTime=cfg.currentTime;
	}

	/**
	 * @brief 从另一个音符复制状态
	 * @param note 源音符对象
	 */
	void set(const Note & note);

	/**
	 * @brief 检查音符ID是否有效
	 * @param id 待检查的MIDI音符ID
	 * @return true表示无效(超出0-127范围)
	 */
	static bool idInvalid(int id){
		return id < 0 || id > 127;
	}
	};
}