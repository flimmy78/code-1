////////////////////////////////////////////////////////////////////////////////
// ��Ȩ���У�2009-2012����������߿����ּ������޹�˾
// ���ļ���δ�����ģ������˺���߿ƵĻ��ܺ�ר������
////////////////////////////////////////////////////////////////////////////////
// �ļ����� audio_debug.h
// ���ߣ�   ������
// �汾��   1.0
// ���ڣ�   2012-11-23
// ������   ��Ƶ����ͷ�ļ�
// ��ʷ��¼��
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
