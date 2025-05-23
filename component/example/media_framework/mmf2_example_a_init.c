/******************************************************************************
 *
* Copyright(c) 2007 - 2018 Realtek Corporation. All rights reserved.
*
******************************************************************************/
#include "mmf2_link.h"
#include "mmf2_siso.h"

#include "module_audio.h"
#include "module_i2s.h"
#include "module_aac.h"
#include "module_rtsp2.h"

#define I2S_INTERFACE   0
#define AUDIO_INTERFACE 1

#define AUDIO_SRC AUDIO_INTERFACE

#if AUDIO_SRC==AUDIO_INTERFACE
static mm_context_t *audio_ctx      = NULL;
static audio_params_t audio_params;
#elif AUDIO_SRC==I2S_INTERFACE
static mm_context_t *i2s_ctx        = NULL;
static i2s_params_t i2s_params;
#else
#error "please set correct AUDIO_SRC"
#endif
static mm_context_t *aac_ctx        = NULL;
static mm_context_t *rtsp2_ctx      = NULL;

static mm_siso_t *siso_audio_aac    = NULL;
static mm_siso_t *siso_aac_rtsp     = NULL;

static aac_params_t aac_params = {
	.sample_rate = 16000,
	.channel = 1,
	.trans_type = AAC_TYPE_ADTS,
	.object_type = AAC_AOT_LC,
	.bitrate = 32000,

	.mem_total_size = 10 * 1024,
	.mem_block_size = 128,
	.mem_frame_size = 1024
};

static rtsp2_params_t rtsp2_a_params = {
	.type = AVMEDIA_TYPE_AUDIO,
	.u = {
		.a = {
			.codec_id   = AV_CODEC_ID_MP4A_LATM,
			.channel    = 1,
			.samplerate = 16000
		}
	}
};

void mmf2_example_a_init(void)
{
#if AUDIO_SRC==AUDIO_INTERFACE
	audio_ctx = mm_module_open(&audio_module);
	if (audio_ctx) {
		mm_module_ctrl(audio_ctx, CMD_AUDIO_GET_PARAMS, (int)&audio_params);
		audio_params.sample_rate = ASR_16KHZ;
		mm_module_ctrl(audio_ctx, CMD_AUDIO_SET_PARAMS, (int)&audio_params);
		mm_module_ctrl(audio_ctx, MM_CMD_SET_QUEUE_LEN, 6);
		mm_module_ctrl(audio_ctx, MM_CMD_INIT_QUEUE_ITEMS, MMQI_FLAG_STATIC);
		mm_module_ctrl(audio_ctx, CMD_AUDIO_APPLY, 0);
	} else {
		printf("audio open fail\n\r");
		goto mmf2_exmaple_a_fail;
	}
#elif AUDIO_SRC==I2S_INTERFACE
	i2s_ctx = mm_module_open(&i2s_module);
	if (i2s_ctx) {
		mm_module_ctrl(i2s_ctx, CMD_I2S_GET_PARAMS, (int)&i2s_params);
		i2s_params.sample_rate = SR_16KHZ;
		i2s_params.i2s_direction = I2S_RX_ONLY;
		mm_module_ctrl(i2s_ctx, CMD_I2S_SET_PARAMS, (int)&i2s_params);
		mm_module_ctrl(i2s_ctx, MM_CMD_SET_QUEUE_LEN, 6);
		mm_module_ctrl(i2s_ctx, MM_CMD_INIT_QUEUE_ITEMS, MMQI_FLAG_STATIC);
		mm_module_ctrl(i2s_ctx, CMD_I2S_APPLY, 0);
	} else {
		printf("i2s open fail\n\r");
		goto mmf2_exmaple_a_fail;
	}
#endif

	aac_ctx = mm_module_open(&aac_module);
	if (aac_ctx) {
		mm_module_ctrl(aac_ctx, CMD_AAC_SET_PARAMS, (int)&aac_params);
		mm_module_ctrl(aac_ctx, MM_CMD_SET_QUEUE_LEN, 6);
		mm_module_ctrl(aac_ctx, MM_CMD_INIT_QUEUE_ITEMS, MMQI_FLAG_DYNAMIC);
		mm_module_ctrl(aac_ctx, CMD_AAC_INIT_MEM_POOL, 0);
		mm_module_ctrl(aac_ctx, CMD_AAC_APPLY, 0);
	} else {
		printf("AAC open fail\n\r");
		goto mmf2_exmaple_a_fail;
	}

	rtsp2_ctx = mm_module_open(&rtsp2_module);
	if (rtsp2_ctx) {
		mm_module_ctrl(rtsp2_ctx, CMD_RTSP2_SELECT_STREAM, 0);
		mm_module_ctrl(rtsp2_ctx, CMD_RTSP2_SET_PARAMS, (int)&rtsp2_a_params);
		mm_module_ctrl(rtsp2_ctx, CMD_RTSP2_SET_APPLY, 0);
		mm_module_ctrl(rtsp2_ctx, CMD_RTSP2_SET_STREAMMING, ON);
	} else {
		printf("RTSP2 open fail\n\r");
		goto mmf2_exmaple_a_fail;
	}

	printf("RTSP2 opened\n\r");

	siso_audio_aac = siso_create();
	if (siso_audio_aac) {
#if AUDIO_SRC==AUDIO_INTERFACE
		siso_ctrl(siso_audio_aac, MMIC_CMD_ADD_INPUT, (uint32_t)audio_ctx, 0);
#elif AUDIO_SRC==I2S_INTERFACE
		siso_ctrl(siso_audio_aac, MMIC_CMD_ADD_INPUT, (uint32_t)i2s_ctx, 0);
#endif
		siso_ctrl(siso_audio_aac, MMIC_CMD_ADD_OUTPUT, (uint32_t)aac_ctx, 0);
		siso_ctrl(siso_audio_aac, MMIC_CMD_SET_STACKSIZE, 44 * 1024, 0);
		siso_start(siso_audio_aac);
	} else {
		printf("siso1 open fail\n\r");
		goto mmf2_exmaple_a_fail;
	}

	printf("siso1 started\n\r");

	siso_aac_rtsp = siso_create();
	if (siso_aac_rtsp) {
		siso_ctrl(siso_aac_rtsp, MMIC_CMD_ADD_INPUT, (uint32_t)aac_ctx, 0);
		siso_ctrl(siso_aac_rtsp, MMIC_CMD_ADD_OUTPUT, (uint32_t)rtsp2_ctx, 0);
		siso_start(siso_aac_rtsp);
	} else {
		printf("siso2 open fail\n\r");
		goto mmf2_exmaple_a_fail;
	}

	printf("siso2 started\n\r");

	return;
mmf2_exmaple_a_fail:

	return;
}