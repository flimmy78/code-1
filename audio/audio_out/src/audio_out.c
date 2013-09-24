////////////////////////////////////////////////////////////////////////////////
// ��Ȩ���У�2009-2012����������߿����ּ������޹�˾
// ���ļ���δ�����ģ������˺���߿ƵĻ��ܺ�ר������
////////////////////////////////////////////////////////////////////////////////
// �ļ����� audio_out.c
// ���ߣ�   ������
// �汾��   1.0
// ���ڣ�   2012-11-23
// ������   ��Ƶ����ģ��˽��ͷ�ļ�
// ��ʷ��¼��
///////////////////////////////////////////////////////////////////////////////

#include "audio_out_private.h"

static AUDIO_OUT_SYS_CFG_OBJ    audio_out_cfg;

////////////////////////////////////////////////////////////////////////////////
// ��������audio_out_open
// ����������Ƶ�����豸��
// ������
//  ��OUT��handle - ģ������
//  ��IN��config - ��Ƶ���������
//  ��IN��pcontext - ����Ҫ���ġ�
// ����ֵ��
//  ������롣
// ˵����
//
////////////////////////////////////////////////////////////////////////////////
HB_S32 audio_out_open(HB_HANDLE * handle, AUDIO_OUT_DECODE_CFG_OBJ config, HB_VOID *pcontext)
{
#ifdef DEBUG
    assert(NULL != handle);
    //assert(NULL != pcontext);//pcontext��ʱδ�ã�pcontext = NULL
#endif

    if (NULL == handle)//|| NULL == pcontext
    {
        AUDIO_ERROR("invalid param\n");
        return AUDIO_OUT_ERRNO(COM_ERR_INVALID_PARAMETER);
    }

    AUDIO_OUT_SYS_CFG_HANDLE pfd = &audio_out_cfg;
    HB_CHAR audio_device[] = "default";

    if (NULL == pfd)
    {
        return AUDIO_OUT_ERRNO(COM_ERR_GENERIC);
    }

	AUDIO_DEBUG("line_out_num: [%d]", config.reserved.line_out);

	/*
	if (0 == config.reserved.line_out)
	{
        pfd->is_support = DISABLE;
        *handle = pfd;
        AUDIO_DEBUG("audio out isn't support");
        return AUDIO_OUT_ERRNO(COM_ERR_NOT_SUPPORTED);
    }
	*/

    if(0 == pfd->counter)
    {
        memset(pfd, 0, sizeof(AUDIO_OUT_SYS_CFG_OBJ));

        if (AUDIO_OK != device_out_open(pfd, audio_device))
        {
            AUDIO_ERROR("Audio device open failed !");
            return AUDIO_OUT_ERRNO(COM_ERR_GENERIC);
        }

        memcpy(&pfd->config, &config, sizeof(AUDIO_OUT_DECODE_CFG_OBJ));    //��Ƶ��ʼ��������ֵ
        pfd->alsa.period_size = 320;
#if 0
        if (AUDIO_OK != device_out_set_params(pfd))
        {
            AUDIO_ERROR("set audio init params failed !");
            return AUDIO_OUT_ERRNO(COM_ERR_GENERIC);
        }
#endif
        pfd->running_flag = MODULE_OPEN;
        pfd->pcontext = pcontext;
        pfd->flag = MODULE_AUDIO_OUT;
        pfd->healthy = MODULE_HEALTHY;
    }
    pfd->is_support = ENABLE;
    pfd->counter += 1;
    *handle = pfd;

    AUDIO_DEBUG("audio_out_open success !");
    return COMMON_OK;
}

////////////////////////////////////////////////////////////////////////////////
// ��������audio_in_start
// ��������ʼ��Ƶ�����豸��
// ������
//  ��IN��handle - ģ������
//   [IN] priority - ���ȼ�
// ����ֵ��
//  ������롣
// ˵����
//
////////////////////////////////////////////////////////////////////////////////
HB_S32 audio_out_start(HB_HANDLE handle, HB_S32 priority)
{
#ifdef DEBUG
    assert(NULL != handle);
#endif
    if (NULL == handle)
    {
        AUDIO_ERROR("Invalid handle for audio !");
        return AUDIO_OUT_ERRNO(COM_ERR_HANDLEINVAL);
    }
    AUDIO_OUT_SYS_CFG_HANDLE pfd = (AUDIO_OUT_SYS_CFG_HANDLE)handle;
#if 0  
    if (DISABLE == pfd->is_support)
    {
        AUDIO_DEBUG("audio out isn't support");
        return AUDIO_OUT_ERRNO(COM_ERR_NOT_SUPPORTED);
    }
#endif

    if (0 == AUDIOOUT_FLAG_VERIFY(pfd))
    {
        AUDIO_ERROR("Invalid handle for audio !");
        return AUDIO_OUT_ERRNO(COM_ERR_INVALID_PARAMETER);
    }
    
    if(MODULE_START == pfd->running_flag)
    {
        AUDIO_DEBUG("audio_out_start success again !");
        return COMMON_OK;
    }

    if (MODULE_OPEN != pfd->running_flag && MODULE_STOP != pfd->running_flag)
    {
        return AUDIO_OUT_ERRNO(COM_ERR_GENERIC);
    }
    pfd->running_flag = MODULE_START;

    AUDIO_DEBUG("audio_out_start success !");

    return COMMON_OK;
}

////////////////////////////////////////////////////////////////////////////////
// ��������audio_out_stop
// ������ֹͣ��Ƶ����豸��
// ������
//  ��IN��handle - ģ������
// ����ֵ��
//  ������롣
// ˵����
//
////////////////////////////////////////////////////////////////////////////////
HB_S32 audio_out_stop(HB_HANDLE handle)
{
#ifdef DEBUG
    assert(NULL != handle);
#endif
    if (NULL == handle)
    {
        AUDIO_ERROR("Invalid handle for audio !");
        return AUDIO_OUT_ERRNO(COM_ERR_HANDLEINVAL);
    }
	
    AUDIO_OUT_SYS_CFG_HANDLE pfd = (AUDIO_OUT_SYS_CFG_HANDLE)handle;

    if (DISABLE == pfd->is_support)
    {
        AUDIO_DEBUG("audio out isn't support");
        return AUDIO_OUT_ERRNO(COM_ERR_NOT_SUPPORTED);
    }
	
    if (0 == AUDIOOUT_FLAG_VERIFY(pfd))
    {
        AUDIO_ERROR("Invalid handle for audio !");
        return AUDIO_OUT_ERRNO(COM_ERR_INVALID_PARAMETER);
    }

    if (MODULE_START != pfd->running_flag)
    {
        return AUDIO_OUT_ERRNO(COM_ERR_GENERIC);
    }
    pfd->running_flag = MODULE_STOP;

    AUDIO_DEBUG("audio_out_stop success !");
    return COMMON_OK;
}

////////////////////////////////////////////////////////////////////////////////
// ��������audio_in_close
// �������ر���Ƶ�����豸��
// ������
//  ��IN��handle - ģ������
// ����ֵ��
//  ������롣
// ˵����
//
////////////////////////////////////////////////////////////////////////////////
HB_S32 audio_out_close(HB_HANDLE handle)
{
#ifdef DEBUG
    assert(NULL != handle);
#endif
    if (NULL == handle)
    {
        AUDIO_ERROR("Invalid handle for audio !");
        return AUDIO_OUT_ERRNO(COM_ERR_HANDLEINVAL);
    }
    AUDIO_OUT_SYS_CFG_HANDLE pfd = (AUDIO_OUT_SYS_CFG_HANDLE)handle;

    if (DISABLE == pfd->is_support)
    {
        AUDIO_DEBUG("audio out isn't support");
        return AUDIO_OUT_ERRNO(COM_ERR_NOT_SUPPORTED);
    }

    if (0 == AUDIOOUT_FLAG_VERIFY(pfd))
    {
        AUDIO_ERROR("Invalid handle for audio !");
        return AUDIO_OUT_ERRNO(COM_ERR_INVALID_PARAMETER);
    }

    if (MODULE_STOP != pfd->running_flag && MODULE_OPEN != pfd->running_flag)
    {
        return AUDIO_OUT_ERRNO(COM_ERR_GENERIC);
    }

    if (AUDIO_OK != device_out_close(pfd))
    {
        return AUDIO_OUT_ERRNO(COM_ERR_GENERIC);
    }

    pfd->running_flag = MODULE_CLOSE;
    pfd->counter -= 1;

    AUDIO_DEBUG("audio_out_close success !");

    return COMMON_OK;
}

////////////////////////////////////////////////////////////////////////////////
// ��������audio_out_ioctrl
// ��������Ƶ������ơ�
// ������
//  ��IN��handle - ģ������
//  ��IN��cmd - �����롣
//  ��IN��pinput �C �������ݵ�ַ��
//  ��IN�� ninlen �C �������ݳ��ȡ�
//  ��OUT��poutput �C ������ݵ�ַ��
//  ��OUT��poutlen �C ������ݳ��ȡ�
// ����ֵ��
//  ������롣
// ˵����
//
////////////////////////////////////////////////////////////////////////////////
HB_S32 audio_out_ioctrl(HB_HANDLE handle, HB_S32 cmd, HB_VOID * pinput, HB_S32 ninlen, HB_VOID * poutput, HB_S32 *poutlen)
{
#ifdef DEBUG
    assert(NULL != handle);
#endif

    if (NULL == handle)
    {
        AUDIO_ERROR("Invalid handle for audio ! -");
        return AUDIO_OUT_ERRNO(COM_ERR_HANDLEINVAL);
    }
    AUDIO_OUT_SYS_CFG_HANDLE pfd = (AUDIO_OUT_SYS_CFG_HANDLE)handle;

	/*
    if (DISABLE == pfd->is_support)
    {
        AUDIO_DEBUG("audio out isn't support");
        return AUDIO_OUT_ERRNO(COM_ERR_NOT_SUPPORTED);
    }
	*/

    if (0 == AUDIOOUT_FLAG_VERIFY(pfd))
    {
        AUDIO_ERROR("Invalid handle for audio !");
        return AUDIO_OUT_ERRNO(COM_ERR_INVALID_PARAMETER);
    }

    if (MODULE_START != pfd->running_flag)
    {
        AUDIO_ERROR("Start flags !");
        return AUDIO_OUT_ERRNO(COM_ERR_GENERIC);
    }

    switch (cmd)
    {
        case SET_AUDIO_OUT_CFG :
            {
                AUDIO_DEBUG("NOT SUPPORTED NOW !");
                break;
            }
        case GET_AUDIO_OUT_CFG :
            {
                AUDIO_DEBUG("NOT SUPPORTED NOW !");
                break;
            }
        case SET_AUDIO_OUT_VOLUME :
            {
				device_out_set_vol(*(HB_S32 *)pinput);
                break;
            }
        case GET_AUDIO_OUT_VOLUME :
            {
				*(HB_S32 *)poutput = device_out_get_vol();
                break;
            }
        case SND_AUDIO_DATA :
            {
            #ifdef DEBUG
                assert(NULL != pinput);
                assert(ninlen >= sizeof(AUDIO_OUT_DATA_OBJ));
            #endif

                if (NULL == pinput || ninlen < sizeof(AUDIO_OUT_DATA_OBJ))
                {
                    AUDIO_ERROR("error size for SND_AUDIO_DATA !");
                    return AUDIO_OUT_ERRNO(COM_ERR_GENERIC);
                }
                AUDIO_OUT_DATA_HANDLE pdata_input = (AUDIO_OUT_DATA_OBJ *)pinput;

                if (device_out_playback(pfd, pdata_input->pdata_addr, pdata_input->data_size) != AUDIO_OK)
                {
                    AUDIO_ERROR("audio play error !");
                    return AUDIO_OUT_ERRNO(COM_ERR_GENERIC);
                }
                break;
            }
		case 100:
        if (AUDIO_OK != device_out_set_params(pfd))
        {
            AUDIO_ERROR("set audio init params failed !");
            return AUDIO_OUT_ERRNO(COM_ERR_GENERIC);
        }
		break;

        default :
            {
                return AUDIO_ERROR("Unknown CMD for audio_out_ioctrl !");
            }
    }

    return COMMON_OK;
}

