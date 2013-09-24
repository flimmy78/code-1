////////////////////////////////////////////////////////////////////////////////
// 版权所有，2009-2012，北京汉邦高科数字技术有限公司
// 本文件是未公开的，包含了汉邦高科的机密和专利内容
////////////////////////////////////////////////////////////////////////////////
// 文件名： audio_demo.c
// 作者：   封欣明
// 版本：   1.0
// 日期：   2012-11-23
// 描述：   音频输入输出测试程序
// 历史记录：
///////////////////////////////////////////////////////////////////////////////
#include "common/ipc_common.h"
#include "audio/audio_in.h"
#include "audio/audio_out.h"
#include "sample_comm.h"

HB_S32 fd_talkback = -1;
HB_S32 fd_listen = -1;
HB_S32 fd_audioout = -1;

HB_HANDLE handle_in[2] = {0};
static HB_HANDLE handle_out = NULL;
AUDIO_OUT_DATA_OBJ data_out;

//直接到音频输出
HB_S32 audio_talkback_callback(HB_HANDLE handle, AUDIO_IN_DATA_HANDLE pdata, HB_VOID *pcontext)
{
    HB_S32 ret = -1;

    //ret = write(fd_talkback, pdata->pdata_addr, pdata->data_size);
    //if (ret != pdata->data_size || ret < 0)
    //{
    //    printf("write file error !\n");
	//	return 0;
    //}

	if (pdata->data_size < 0||handle_out == NULL)
	{
		return 0;
	}
    data_out.pdata_addr =(void *) pdata->pdata_addr;
    data_out.data_size = pdata->data_size;

    ret = audio_out_ioctrl(handle_out, SND_AUDIO_DATA, (void *)&data_out,sizeof(AUDIO_OUT_DATA_OBJ), NULL, NULL);
    if (ret)
    {
        printf("IOCTRL ERROR --- ! %d\n",pdata->data_size);
        return -1;
    }

    return HB_SUCCESS;
}


// 录制到文件
HB_S32 audio_listen_callback(HB_HANDLE handle, AUDIO_IN_DATA_HANDLE pdata, HB_VOID *pcontext)
{
    HB_S32 ret = -1;

    ret = write(fd_listen, pdata->pdata_addr, pdata->data_size);
    if (ret != pdata->data_size)
    {
        printf("write file error !\n");
    }
	return 0;
}

HB_S32 main(HB_S32 argc, HB_CHAR *argv[])
{
    //HB_CHAR rc[102400];
    //AUDIO_OUT_DATA_OBJ data;
	HB_S32 volume;
    AUDIO_IN_ARG_OBJ arg[2];
    AUDIO_OUT_DECODE_CFG_OBJ arg_out;

    arg[0].callback = audio_talkback_callback;
    arg[0].config.sample_rate = 8000;

    arg_out.sample_rate = 8000;
    HB_S32 ret = HB_FAILURE;

	/*
    fd_talkback = open("./audio_data.g721" ,O_RDWR|O_CREAT, 0777);
    if (fd_talkback < 0)
    {
        printf("open file ./audio_data.g721 error !\n");
        return HB_FAILURE;
    }
	*/

    ret = audio_in_open(&handle_in[0], arg[0], NULL);
    if (ret)
    {
        printf("OPEN handle_in[0] ERROR !\n");
        return HB_FAILURE;
    }


    ret = audio_in_start(handle_in[0], 0);
    if (ret)
    {
        printf("START handle_in[0] ERROR !\n");
        return HB_FAILURE;
    }
	audio_in_ioctrl(handle_in[0], 100, NULL, 0, NULL, NULL);

	//printf("set VI volume +>");
	//scanf("%d", &volume);

    // ------------------------------------------------------
	//printf("Press twice ENTER to Start play\n");
	//getchar();
	//getchar();
	/*
    fd_audioout = open("./audio_data.g721" ,  O_RDWR , 0777);
    if (fd_audioout < 0)
    {
        printf("open file ./audio_data.g721 error !\n");
        return HB_FAILURE;
    }
	*/

    ret = audio_out_open(&handle_out, arg_out, NULL);
    if (ret)
    {
        printf("OPEN handle_out ERROR !\n");
        return HB_FAILURE;
    }
    ret = audio_out_start(handle_out, 0);
    if (ret)
    {
        printf("START handle_out ERROR !\n");
        return HB_FAILURE;
    }
	audio_out_ioctrl(handle_out, 100, NULL, 0, NULL, NULL);
	volume = 0x10;
	if(volume != 0)
	{
		audio_in_ioctrl(handle_in[0], SET_AUDIO_IN_VOLUME, (void *)&volume,sizeof(HB_S32), NULL, NULL);
		volume =10;
		audio_in_ioctrl(handle_in[0], GET_AUDIO_IN_VOLUME, NULL, 0, (void *)&volume, (void *)&ret);
		printf("volume - %d\n", volume);
	}
	volume = 0x10;
	if(volume != 0)
	{
		audio_out_ioctrl(handle_out, SET_AUDIO_OUT_VOLUME, (void *)&volume,sizeof(HB_S32), NULL, NULL);
		volume = 10;
		audio_out_ioctrl(handle_out, GET_AUDIO_OUT_VOLUME, NULL, 0, (void *)&volume, (void *)&ret);
		printf("volume - %d\n", volume);
	}
	sleep(10);


#if 0
    while(1)
    {
        if (0 == read(fd_audioout, rc, 120))
        {
            break;
        }
        data.pdata_addr = (void *)rc;
        data.data_size = 120;
        ret = audio_out_ioctrl(handle_out, SND_AUDIO_DATA, (void *)&data,sizeof(AUDIO_OUT_DATA_OBJ), NULL, NULL);
        if (ret)
        {
            printf("IOCTRL handle_out ERROR !\n");
            return HB_FAILURE;
        }
    }
#endif

    // ------------------------------------------------------
    ret = audio_in_stop(handle_in[0]);
    if (ret)
    {
        printf("STOP handle_in[0] ERROR !\n");
        return HB_FAILURE;
    }

    ret = audio_out_stop(handle_out);
    if (ret)
    {
        printf("STOP handle_out ERROR !\n");
        return HB_FAILURE;
    }
    ret = audio_out_close(handle_out);
    if (ret)
    {
        printf("STOP handle_out ERROR !\n");
        return HB_FAILURE;
    }


    ret = audio_in_close(handle_in[0]);
    if (ret)
    {
        printf("STOP handle_in[0] ERROR !\n");
        return HB_FAILURE;
    }
   // close(fd_talkback);

    return HB_SUCCESS;
}
