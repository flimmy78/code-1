////////////////////////////////////////////////////////////////////////////////
// 版权所有，2009-2012，北京汉邦高科数字技术有限公司
// 本文件是未公开的，包含了汉邦高科的机密和专利内容
////////////////////////////////////////////////////////////////////////////////
// 文件名： audio_in_private.h
// 作者：   封欣明
// 版本：   1.0
// 日期：   2012-11-23
// 描述：   音频输入模块私有头文件
// 历史记录：
///////////////////////////////////////////////////////////////////////////////
#ifndef _AUDIO_IN_PRIVATE_H
#define _AUDIO_IN_PRIVATE_H

#include "audio/alsa/asoundlib.h"
#include "common/ipc_common.h"
#include "audio/audio_in.h"
#include "audio_debug.h"

#define CHANNEL_NUM        (1)
#define AUDIOIN_FLAG_VERIFY(pfd)  ((pfd)->flag == MODULE_AUDIO_IN)
#define AUDIOIN_MAX_OPENNUM 8
///////////////////////////////////////////////////////////////////////////////
// ALSA配置结构体
///////////////////////////////////////////////////////////////////////////////
typedef struct _tagALSA_CFG
{
    HB_S32         len;
    HB_S32         chunk_size;
    HB_S32         period_size;
    snd_pcm_t      *handle;
    HB_S32         reserved;
}ALSA_CFG_OBJ, *ALSA_CFG_HANDLE;

///////////////////////////////////////////////////////////////////////////////
// 音频输入模块结构体
///////////////////////////////////////////////////////////////////////////////
typedef struct _tagAUDIO_IN_SYS_CFG
{
    HB_S32                     len;                //结构体长度
    HB_S32                     flag;               //模块标志
    HB_S32                     healthy;            //句柄健康状况,0-正常,1-故障
    HB_S32                     counter;            //模块打开次数
    pthread_t                  thr_id;             //线程id
    ALSA_CFG_OBJ               alsa;               //ALSA配置结构
    MODULE_RUN_STATUS_E        running_flag;       //模块运行状态
    THREAD_RUN_STATUS_E        thread_status;      //线程状态
    AUDIO_IN_ENCODE_CFG_OBJ    config;             //音频输入配置
    AUDIO_IN_CALLBACK          callback[AUDIOIN_MAX_OPENNUM];           //回调函数
    HB_VOID                    *pcontext[AUDIOIN_MAX_OPENNUM];          //附加指针
    HB_S32                     is_support;         //是否支持audio_in
}AUDIO_IN_SYS_CFG_OBJ, *AUDIO_IN_SYS_CFG_HANDLE;

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
HB_S32 device_in_open(AUDIO_IN_SYS_CFG_HANDLE paudio, HB_CHAR *pdev_name);

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
HB_S32 device_in_close(AUDIO_IN_SYS_CFG_HANDLE paudio);

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
HB_S32 device_in_set_params(AUDIO_IN_SYS_CFG_HANDLE paudio);

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
HB_S32 device_in_capture(AUDIO_IN_SYS_CFG_HANDLE paudio, HB_S32 *pbuff, HB_S32 *psize);
////////////////////////////////////////////////////////////////////////////////
// 函数名：device_in_get_vol
// 描述：音频输入音量获取
// 参数：
// 返回值：
//   获取的音量
// 说明：
//
///////////////////////////////////////////////////////////////////////////////
HB_S32 device_in_get_vol(HB_VOID);
////////////////////////////////////////////////////////////////////////////////
// 函数名：device_in_set_vol
// 描述：音频输入音量获取
// 参数：
//  volume - 设置音量
// 返回值：
//   错误码
// 说明：
//
///////////////////////////////////////////////////////////////////////////////
HB_S32 device_in_set_vol(HB_S32 volume);
#endif

