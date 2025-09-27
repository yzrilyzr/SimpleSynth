#pragma once
#include "yzrutil.h"

typedef int32_t			s_midichannel_id;//通道 ID
typedef int32_t			s_program_id;//程序id
typedef int32_t			s_bank_id;//id
typedef uint64_t		s_sample_index;//采样点计数器位置
typedef uint8_t			s_note_id_i;//整数的音符id

typedef double			s_phase;//相位
typedef u_normal_01_f	s_note_vel;//音符velocity
typedef double			s_note_id;//音符id

#define CHANNEL_MAX_VOICE 128
#define CHANNEL_MAX_NOTE_ID 128