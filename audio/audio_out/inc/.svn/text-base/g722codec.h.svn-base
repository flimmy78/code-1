#ifndef _G722CODEC_LIB_
#define _G722CODEC_LIB_

typedef struct 
{
    int qmf_e[6], qmf_o[6];

    short sl,detl,nbl,szl,spl;
    short plt[3],rlt[3],al[3];
    short dlt[7],bl[7];

    short sh,deth,nbh,szh,sph;
    short ph[3],rh[3],ah[3];
    short dh[7],bh[7];
} G722Codec, *pG722Codec;


pG722Codec g722Codec_init();
/*
函数说明：创建 G722 编解码器，并对其进行初始化。
函数返回：指向解码器的指针
*/

void g722Codec_free(pG722Codec pCodec);
/*
函数说明：释放 G722 编解码器。
函数返回：无。
*/

unsigned int g722Codec_enc(pG722Codec pEnc, short *signal, unsigned char *serial, unsigned int sample);
/*
函数说明：进行G722 音频解码。压缩比为16:3，例如sample=80，返回长度为30; 将 8K 16BIT 单声道数据压缩
pEnc      指向解码器的指针
signal    音频解码的内存缓冲区指针（音频格式 8K 16BIT 单声道。）
serial    G722 码流的内存缓冲区指针
sample    共要解码多少帧（一帧为16Bit）
函数返回：G722 码流的内存缓冲区serial的长度。
*/

unsigned int g722Codec_dec(pG722Codec pDec, short *signal, unsigned char *serial, unsigned int sample);
/*
函数说明：进行G722 音频解码。将压缩数据解压为 8K 16BIT 单声道数据
pDec      指向解码器的指针
signal    音频解码的内存缓冲区指针（音频格式 8K 16BIT 单声道。）
serial    G722 码流的内存缓冲区指针
sample    共要解码多少帧（一帧为16Bit）
函数返回：音频解码的内存缓冲区长度。
*/

#endif

