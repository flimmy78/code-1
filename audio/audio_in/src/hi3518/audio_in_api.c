////////////////////////////////////////////////////////////////////////////////
// 版权所有，2009-2012，北京汉邦高科数字技术有限公司
// 本文件是未公开的，包含了汉邦高科的机密和专利内容
////////////////////////////////////////////////////////////////////////////////
// 文件名： hi3518_audio_in.c
// 作者：   liyang
// 版本：   1.0
// 日期：   2012-11-23
// 描述：   音频输入模块实现
// 历史记录：
///////////////////////////////////////////////////////////////////////////////
#include "audio_in_private.h"
#include "G711.h"

#include "sample_comm.h"
#include "acodec.h"

#define ACODEC_DEV  "/dev/acodec"

static PAYLOAD_TYPE_E gs_AencType = PT_G711A;
static HI_BOOL gs_bMicIn = HI_FALSE;
static HI_BOOL gs_bAiAnr = HI_FALSE;
static HI_BOOL gs_bAioReSample = HI_FALSE;
static AUDIO_RESAMPLE_ATTR_S *gs_pstAiReSmpAttr = NULL;


#ifndef SAMPLE_AUDIO_PTNUMPERFRM
#define SAMPLE_AUDIO_PTNUMPERFRM   320
#endif
#define JORNEY_DEBUG 1
////////////////////////////////////////////////////////////////////////////////
// 函数名：audio_InAenc
// 描述：音频输入到编码被配置以及使能
// 参数：
//  ［IN］pstAioAttr - 配置结构体
// 返回值：
//  错误代码。
// 说明：
//
///////////////////////////////////////////////////////////////////////////////
HB_S32 audio_InAenc(AIO_ATTR_S *pstAioAttr)
{
	HB_S32 s32Ret;

	if (pstAioAttr == NULL){
		printf("%s : Parameter NULL\n", __FUNCTION__);
		return AUDIO_FAIL;
	}
	s32Ret = SAMPLE_COMM_AUDIO_CfgAcodec(pstAioAttr, gs_bMicIn);
	if (HI_SUCCESS != s32Ret){
		printf("%s : SAMPLE_COMM_AUDIO_CfgAcodec", __FUNCTION__);
		return AUDIO_FAIL;
	}

	s32Ret = SAMPLE_COMM_AUDIO_StartAi(0, 1, pstAioAttr, gs_bAiAnr, gs_pstAiReSmpAttr);
	if (s32Ret != HI_SUCCESS){
		printf("%s : SAMPLE_COMM_AUDIO_StartAi", __FUNCTION__);
		return AUDIO_FAIL;
	}

	s32Ret = SAMPLE_COMM_AUDIO_StartAenc(1, gs_AencType);
	if (s32Ret != HI_SUCCESS){
		printf("%s : SAMPLE_COMM_AUDIO_StartAenc", __FUNCTION__);
		return AUDIO_FAIL;
	}

	s32Ret = SAMPLE_COMM_AUDIO_AencBindAi(0, 0, 0);
	if (s32Ret != HI_SUCCESS){
		printf("%s : SAMPLE_COMM_AUDIO_AencBindAi", __FUNCTION__);
		return AUDIO_FAIL;
	}
	return AUDIO_OK;
}
HB_S32 audio_Aenc_type(AUDIO_ENCODE_TYPE_E format)
{
	switch(format)
	{
	case AUDIO_IN_G711_A_LAW:
		gs_AencType = PT_G711A;
		break;
	case AUDIO_IN_G711_U_LAW:
		gs_AencType = PT_G711U;
		break;
	case AUDIO_IN_G722:
		gs_AencType = PT_G722;
		break;
	case AUDIO_IN_AAC:
		gs_AencType = PT_AAC;
		break;
	default:
		return AUDIO_OK;
	}
	return AUDIO_OK;
}
////////////////////////////////////////////////////////////////////////////////
// 函数名：device_in_open
// 描述：音频输入设备打开。
// 参数：
//  ［IN］paudio - 模块句柄。
//   [IN] pdev_name - 设备名称的地址指针
// 返回值：
//  错误代码。
// 说明：
//
///////////////////////////////////////////////////////////////////////////////
HB_S32 device_in_open(AUDIO_IN_SYS_CFG_HANDLE p_audio, HB_CHAR *pdev_name)
{
#ifdef DEBUG
	assert(NULL != p_audio);
	assert(NULL != pdev_name);
#endif
	if (NULL == p_audio || NULL == pdev_name)
	{
		printf("%s : Invalid param\n", __FUNCTION__);
		return AUDIO_FAIL;
	}

	if(JORNEY_DEBUG)
	{
		VB_CONF_S stVbConf;
		HB_S32 s32Ret = HI_SUCCESS;

		memset(&stVbConf, 0, sizeof(VB_CONF_S));
		s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
		if (HI_SUCCESS != s32Ret){
			printf("%s: system init failed with %d!\n", __FUNCTION__, s32Ret);
			return AUDIO_FAIL;
		}
	}

	return AUDIO_OK;
}

////////////////////////////////////////////////////////////////////////////////
// 函数名：device_in_set_params
// 描述：音频输入设备设置参数。
// 参数：
//  ［IN］paudio - 模块句柄。
// 返回值：
//  错误代码。
// 说明：
//
///////////////////////////////////////////////////////////////////////////////
HB_S32 device_in_set_params(AUDIO_IN_SYS_CFG_HANDLE p_audio)
{
	AIO_ATTR_S stAioAttr;

#ifdef DEBUG
	assert(NULL != p_audio);
#endif

	if (NULL == p_audio)
	{
		printf("%s : Invalid param\n", __FUNCTION__);
		return AUDIO_FAIL;
	}

	stAioAttr.enSamplerate   = p_audio->config.sample_rate;
	stAioAttr.enBitwidth     = AUDIO_BIT_WIDTH_16;
	stAioAttr.enWorkmode     = AIO_MODE_I2S_MASTER;
	stAioAttr.enSoundmode    = AUDIO_SOUND_MODE_MONO;
	stAioAttr.u32EXFlag      = 1;
	stAioAttr.u32FrmNum      = 30;
	stAioAttr.u32PtNumPerFrm = SAMPLE_AUDIO_PTNUMPERFRM;
//	stAioAttr.u32PtNumPerFrm = 320;
	stAioAttr.u32ChnCnt      = 2;    /* >@-@< : VI ChnCnt must >= 2 ? */
	stAioAttr.u32ClkSel      = 1;

#if 1
	if (HI_TRUE == gs_bAioReSample)
	{
		AUDIO_RESAMPLE_ATTR_S stAiReSampleAttr;

		stAioAttr.enSamplerate   = AUDIO_SAMPLE_RATE_32000;
		stAioAttr.u32PtNumPerFrm = SAMPLE_AUDIO_PTNUMPERFRM * 4;

		stAiReSampleAttr.u32InPointNum  = SAMPLE_AUDIO_PTNUMPERFRM * 4;
		stAiReSampleAttr.enInSampleRate = AUDIO_SAMPLE_RATE_32000;
		stAiReSampleAttr.enReSampleType = AUDIO_RESAMPLE_4X1;
		gs_pstAiReSmpAttr = &stAiReSampleAttr;

    }else{
		gs_pstAiReSmpAttr = NULL;
	}
#endif

	//audio_Aenc_type(p_audio->config.format);
	audio_InAenc(&stAioAttr);
	return AUDIO_OK;
}


////////////////////////////////////////////////////////////////////////////////
// 函数名：device_in_close
// 描述：音频输入设备打开。
// 参数：
//  ［IN］paudio - 模块句柄。
// 返回值：
//  错误代码。
// 说明：
//
///////////////////////////////////////////////////////////////////////////////
HB_S32 device_in_close(AUDIO_IN_SYS_CFG_HANDLE p_audio)
{
#ifdef DEBUG
	assert(NULL != p_audio);
#endif

	if (NULL == p_audio)
	{
		printf("invalid param\n");
		return AUDIO_FAIL;
	}

	SAMPLE_COMM_AUDIO_StopAenc(1);
	SAMPLE_COMM_AUDIO_StopAi(0, 1, gs_bAiAnr, gs_bAioReSample);
	if(JORNEY_DEBUG){
		SAMPLE_COMM_SYS_Exit();
	}

	return AUDIO_OK;
}

////////////////////////////////////////////////////////////////////////////////
// 函数名：device_in_capture
// 描述：音频输入设备读取数据。
// 参数：
//  ［IN］paudio - 模块句柄。
//   [OUT] pbuff - 数据缓冲区
//   [OUT] psize - 数据的长度
// 返回值：
//  错误代码。
// 说明：
//
///////////////////////////////////////////////////////////////////////////////
HB_S32 device_in_capture(AUDIO_IN_SYS_CFG_HANDLE p_audio, HB_S32 *pbuff, HB_S32 *psize)
{


#ifdef DEBUG
	assert(NULL != p_audio);
	assert(NULL != pbuff);
	assert(NULL != psize);
#endif

	if (NULL == p_audio || NULL == pbuff || NULL == psize)
	{
		printf("%s : Invalid param\n", __FUNCTION__);
		return AUDIO_FAIL;
	}

	HI_S32 s32Ret;
	AUDIO_STREAM_S stStream;
    HI_S32 AencFd;
    fd_set read_fds;
    struct timeval TimeoutVal;

	FD_ZERO(&read_fds);    
	AencFd = HI_MPI_AENC_GetFd(0);
	FD_SET(AencFd, &read_fds);

	TimeoutVal.tv_sec = 1;
	TimeoutVal.tv_usec = 0;

	FD_ZERO(&read_fds);
	FD_SET(AencFd,&read_fds);

	s32Ret = select(AencFd+1, &read_fds, NULL, NULL, &TimeoutVal);
	if (s32Ret < 0) {
		return AUDIO_FAIL;
	}else if (0 == s32Ret) {
		return AUDIO_FAIL;
	}
	if (FD_ISSET(AencFd, &read_fds)){

		s32Ret = HI_MPI_AENC_GetStream(0, &stStream, HI_FALSE);

		if (HI_SUCCESS != s32Ret ){
			printf("%s: HI_MPI_AENC_GetStream, failed with %#x!\n",__FUNCTION__,s32Ret);
			return AUDIO_FAIL;
		}

		memset(pbuff, 0, stStream.u32Len);
		memcpy(pbuff, &stStream.pStream[4], stStream.u32Len - 4);
		*psize = stStream.u32Len - 4;

		HI_MPI_AENC_ReleaseStream(0, &stStream);
	}    

	return AUDIO_OK;
}


HB_S32 device_in_set_vol(HB_S32 volume)
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

	if (ioctl(acodec_fd, ACODEC_SET_ADCL_VOL, &acodec_vol)
			|| ioctl(acodec_fd, ACODEC_SET_ADCR_VOL, &acodec_vol))
	{
		printf("%s : Set AI volume ERR!\n", __FUNCTION__);
		return AUDIO_FAIL;
	}
#if 1
	close(acodec_fd);
#endif
	return AUDIO_OK;
}

HB_S32 device_in_get_vol(void)
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
	if (ioctl(acodec_fd, ACODEC_GET_ADCL_VOL, &acodec_vol))
	{
		printf("%s : Get AI volume ERR!\n", __FUNCTION__);
		return AUDIO_FAIL;
	}
#if 1
	close(acodec_fd);
#endif
	return acodec_vol.vol_ctrl;
}
