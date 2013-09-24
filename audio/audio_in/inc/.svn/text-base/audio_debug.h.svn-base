////////////////////////////////////////////////////////////////////////////////
// 版权所有，2009-2012，北京汉邦高科数字技术有限公司
// 本文件是未公开的，包含了汉邦高科的机密和专利内容
////////////////////////////////////////////////////////////////////////////////
// 文件名： audio_debug.h
// 作者：   封欣明
// 版本：   1.0
// 日期：   2012-11-23
// 描述：   音频调试头文件
// 历史记录：
///////////////////////////////////////////////////////////////////////////////

#ifndef _AUDIO_DEBUG_H
#define _AUDIO_DEBUG_H

#define __AUDIO_DEBUG__

#ifdef __AUDIO_DEBUG__
#define AUDIO_DEBUG(format,...) printf("AUDIO DEBUG : "format"\n", ##__VA_ARGS__)
#else
#define AUDIO_DEBUG(format,...)
#endif

#define AUDIO_ERROR(format,...) printf("AUDIO ERROR : ("__FILE__",%d) : "format"\n", __LINE__, ##__VA_ARGS__)

#define AUDIO_FAIL (-1)
#define AUDIO_OK    (0)

#endif
