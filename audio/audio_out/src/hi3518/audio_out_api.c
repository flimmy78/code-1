////////////////////////////////////////////////////////////////////////////////
// 版权所有，2009-2012，北京汉邦高科数字技术有限公司
// 本文件是未公开的，包含了汉邦高科的机密和专利内容
////////////////////////////////////////////////////////////////////////////////
// 文件名： hisi_audio_out.c
// 作者：   涂强
// 版本：   1.0
// 日期：   2013-07-23
// 描述：   音频输入模块实现
// 历史记录：
///////////////////////////////////////////////////////////////////////////////
#include "audio_out_private.h"
#include "sample_comm.h"
#include "acodec.h"

#define ACODEC_DEV  "/dev/acodec"

#define ENABLE_RESAMPLE             0

#ifndef SAMPLE_AUDIO_PTNUMPERFRM
#define SAMPLE_AUDIO_PTNUMPERFRM   320
#endif

#define AUDIO_OUT_DEBUG 0

static AUDIO_RESAMPLE_ATTR_S *gs_pstAoReSmpAttr = NULL;
static PAYLOAD_TYPE_E gs_AdecType = PT_G711A;
HB_S32 start_flag;

////////////////////////////////////////////////////////////////////////////////
// 函数名：audio_AdecAo
// 描述：音频输出和解码配置以及使能
// 参数：
//  ［IN］pstAioAttr - 配置参数结构
// 返回值：
//  错误代码。
// 说明：
///////////////////////////////////////////////////////////////////////////////
HI_S32 audio_AdecAo(AIO_ATTR_S *pstAioAttr)
{
    HI_S32      s32Ret;

    if (NULL == pstAioAttr)
    {
        printf("%s: Parameter is NULL!\n", __FUNCTION__);
        return AUDIO_FAIL;
    }

    s32Ret = SAMPLE_COMM_AUDIO_CfgAcodec(pstAioAttr, HI_FALSE);
    if (HI_SUCCESS != s32Ret)
    {
        printf("%s: SAMPLE_COMM_AUDIO_CfgAcode!\n", __FUNCTION__);
        return AUDIO_FAIL;
    }

    s32Ret = SAMPLE_COMM_AUDIO_StartAdec(0 , gs_AdecType);
    if (s32Ret != HI_SUCCESS)
    {
        printf("%s: SAMPLE_COMM_AUDIO_StartAdec!\n", __FUNCTION__);
        return AUDIO_FAIL;
    }

    s32Ret = SAMPLE_COMM_AUDIO_StartAo(0, 0, pstAioAttr, gs_pstAoReSmpAttr);
    if (s32Ret != HI_SUCCESS)
    {
        printf("%s: SAMPLE_COMM_AUDIO_StartAo!\n", __FUNCTION__);
        return AUDIO_FAIL;
    }

    s32Ret = SAMPLE_COMM_AUDIO_AoBindAdec(0, 0, 0);
    if (s32Ret != HI_SUCCESS)
    {
        printf("%s: SAMPLE_COMM_AUDIO_AoBindAdec!\n", __FUNCTION__);
        return AUDIO_FAIL;
    }

    return AUDIO_OK;
}
HB_S32 audio_AdecType(AUDIO_DECODE_TYPE_E format)
{
	switch(format)
	{
	case AUDIO_OUT_G711_A_LAW:
		gs_AdecType = PT_G711A;
		break;
	case AUDIO_OUT_G711_U_LAW:
		gs_AdecType = PT_G711U;
		break;
	case AUDIO_OUT_G722:
		gs_AdecType = PT_G722;
		break;
	case AUDIO_OUT_AAC:
		gs_AdecType = PT_AAC;
		break;
	default:
		return AUDIO_OK;
	}
	return AUDIO_OK;
	
}
////////////////////////////////////////////////////////////////////////////////
// 函数名：device_out_open
// 描述：音频输入设备打开。
// 参数：
//  ［IN］paudio - 模块句柄。
//   [IN] pdev_name - 设备名称的地址指针
// 返回值：
//  错误代码。
// 说明：
//
///////////////////////////////////////////////////////////////////////////////
HB_S32 device_out_open(AUDIO_OUT_SYS_CFG_HANDLE p_audio, HB_CHAR *pdev_name)
{
#ifdef DEBUG
    assert(NULL != p_audio);
    assert(NULL != pdev_name);
#endif

    if (NULL == p_audio || NULL == pdev_name){
        printf("Device Out Open Failed\n");
        return AUDIO_FAIL;
    }
    if (AUDIO_OUT_DEBUG){
		VB_CONF_S stVbConf;
		HB_S32 s32Ret = HI_SUCCESS;
		memset(&stVbConf, 0, sizeof(VB_CONF_S));
		s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
		if (HI_SUCCESS != s32Ret){
			printf("%s: Sys Init Failed : %d!\n", __FUNCTION__, s32Ret);
			return AUDIO_FAIL;
		}

	}
	start_flag = 0;

    return AUDIO_OK;
}

////////////////////////////////////////////////////////////////////////////////
// 函数名：device_out_set_params
// 描述：音频输出设备设置参数。
// 参数：
//  ［IN］paudio - 模块句柄。
// 返回值：
//  错误代码。
// 说明：
//
///////////////////////////////////////////////////////////////////////////////
HB_S32 device_out_set_params(AUDIO_OUT_SYS_CFG_HANDLE p_audio)
{
#ifdef DEBUG
    assert(NULL != p_audio);
#endif
    AIO_ATTR_S stAioAttr;
    if (NULL == p_audio){
        printf("%s : Parameter is NULL\n", __FUNCTION__);
        return AUDIO_FAIL;
    }

    stAioAttr.enSamplerate   = p_audio->config.sample_rate;
    stAioAttr.enBitwidth     = AUDIO_BIT_WIDTH_16;
    stAioAttr.enWorkmode     = AIO_MODE_I2S_MASTER;
    stAioAttr.enSoundmode    = AUDIO_SOUND_MODE_MONO;
    stAioAttr.u32EXFlag      = 1;
    stAioAttr.u32FrmNum      = 30;
    stAioAttr.u32PtNumPerFrm = SAMPLE_AUDIO_PTNUMPERFRM;
    stAioAttr.u32ChnCnt      = 2;
    stAioAttr.u32ClkSel      = 1;

	if(ENABLE_RESAMPLE){

        AUDIO_RESAMPLE_ATTR_S stAoReSampleAttr;

        stAioAttr.enSamplerate = AUDIO_SAMPLE_RATE_32000;
        stAioAttr.u32PtNumPerFrm = SAMPLE_AUDIO_PTNUMPERFRM * 4;

        /* ao 8k -> 32k */
        stAoReSampleAttr.u32InPointNum = SAMPLE_AUDIO_PTNUMPERFRM;
        stAoReSampleAttr.enInSampleRate = AUDIO_SAMPLE_RATE_8000;
        stAoReSampleAttr.enReSampleType = AUDIO_RESAMPLE_1X4;
        gs_pstAoReSmpAttr = &stAoReSampleAttr;
	}else{
        gs_pstAoReSmpAttr = NULL;
    }

	//audio_AdecType(p_audio->config.format);
    audio_AdecAo(&stAioAttr);
	start_flag = 1;
    return AUDIO_OK;
}

////////////////////////////////////////////////////////////////////////////////
// 函数名：device_out_close
// 描述：音频输入设备打开。
// 参数：
//  ［IN］paudio - 模块句柄。
// 返回值：
//  错误代码。
// 说明：
//
///////////////////////////////////////////////////////////////////////////////
HB_S32 device_out_close(AUDIO_OUT_SYS_CFG_HANDLE p_audio)
{
#ifdef DEBUG
    assert(NULL != p_audio);
#endif
    if (NULL == p_audio)
    {
        printf("device_out_close failed");
        return AUDIO_FAIL;
    }

    SAMPLE_COMM_AUDIO_StopAo(0, 0, HI_FALSE);
    SAMPLE_COMM_AUDIO_StopAdec(0);
    SAMPLE_COMM_AUDIO_AoUnbindAdec(0, 0, 0);
	start_flag = 0;
    if (AUDIO_OUT_DEBUG){
    SAMPLE_COMM_SYS_Exit();
	}
    return AUDIO_OK;
}


////////////////////////////////////////////////////////////////////////////////
// 函数名：device_out_playback
// 描述：音频输出设备播放数据。
// 参数：
//  ［IN］paudio - 模块句柄。
//   [OUT] pbuff - 数据缓冲区
//   [OUT] psize - 数据的长度
// 返回值：
//  错误代码。
// 说明：
//
///////////////////////////////////////////////////////////////////////////////
HB_S32 device_out_playback(AUDIO_OUT_SYS_CFG_HANDLE p_audio, HB_U32 *pbuff, HB_S32 size)
{
#ifdef DEBUG
    assert(NULL != p_audio);
    assert(NULL != pbuff);
#endif
    HB_U8 buffer[1024];
    if (NULL == p_audio || NULL == pbuff)
    {
        printf("%s : Playback buffer can't be NULL !\n", __FUNCTION__);
        return AUDIO_FAIL;
    }
	if (!start_flag)
		return AUDIO_OK;
    HI_S32 s32Ret;
    AUDIO_STREAM_S stAudioStream;
    memset(buffer, 0, sizeof(buffer));
    buffer[0] = 0;
    buffer[1] = 1;
    buffer[2] = 160;
    buffer[3] = 0;
    memcpy(&buffer[4], pbuff, size);
	stAudioStream.pStream = buffer;
    printf("###############%d\n", size);

	//stAudioStream.pStream = (HB_U8 *)pbuff;
	stAudioStream.u32Len = size + 4;
	s32Ret = HI_MPI_ADEC_SendStream(0, &stAudioStream, HI_TRUE);
	if (s32Ret)
	{
		printf("%s: HI_MPI_ADEC_SendStream failed with %#x!\n",
				__FUNCTION__, s32Ret);
		return AUDIO_FAIL;
	}
    return AUDIO_OK;
}

HB_S32 device_out_set_vol(HB_S32 volume)
{
    HB_S32 acodec_fd;
	ACODEC_VOL_CTRL acodec_vol;
	acodec_vol.vol_ctrl_mute = 0;
	acodec_vol.vol_ctrl      = volume;

#if 1
	acodec_fd = open(ACODEC_DEV, O_RDWR);
	if (acodec_fd < 0)
	{
		printf("%s : Open acodec dev fail!\n", __FUNCTION__);
		return AUDIO_FAIL;
	}
#endif
	if (ioctl(acodec_fd, ACODEC_SET_DACL_VOL, &acodec_vol)
			|| ioctl(acodec_fd, ACODEC_SET_DACR_VOL, &acodec_vol))
	{
		printf("%s : Set AI volume ERR!\n", __FUNCTION__);
		return AUDIO_FAIL;
	}
#if 1
	close(acodec_fd);
#endif
	return AUDIO_OK;
}

HB_S32 device_out_get_vol(void)
{
    HB_S32 acodec_fd;
	ACODEC_VOL_CTRL acodec_vol;

#if 1
	acodec_fd = open(ACODEC_DEV, O_RDWR);
	if (acodec_fd < 0)
	{
		printf("%s : Open acodec dev fail!\n", __FUNCTION__);
		return AUDIO_FAIL;
	}
#endif
	if (ioctl(acodec_fd, ACODEC_GET_DACL_VOL, &acodec_vol))
	{
		printf("%s : Get AI volume ERR!\n", __FUNCTION__);
		return AUDIO_FAIL;
	}
#if 1
	close(acodec_fd);
#endif
	return acodec_vol.vol_ctrl;
}
