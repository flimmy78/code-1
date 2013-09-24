/***************************************************************************
  Brief Description:
        Decoder of ITU-T Rec. G.722

  Author of Code:
        Jia Zhike (¼ÖÖ¾¿Æ), Department of Electronic Engineering, Tsinghua University
        Tel: (010) 62781417
        E-mail: zkjia@263.net

   Release Date:
        23-04-99
***************************************************************************/
#include <stdlib.h>
#include <string.h>
#include "g722codec.h"
#define CUSTOM_OPS


#define FUNSHIFT2(a,b) (((a)<<16)|(((b)>>16)&0xffff))
#define PACK16LSB(a,b) (((a)<<16)|((b)&0xffff))
#define IFIR16(a,b)    ((short)((a)>>16)*(short)((b)>>16)+(short)((a)&0xffff)*(short)((b)&0xffff))
#define CLIP(a,b,c)    (((a)<(b))?(b):(((a)>(c))?(c):(a)))
#define ICLIPI(a,b)    CLIP(a,~b,b)
#define UCLIPI(a,b)    CLIP(a,0,b)
#define MUX(a,b,c)     ((a)?(b):(c))
#define IZERO(a,b)     MUX(a,b,0)


// common const
static int HE[12] = { 3, -11, 12, 32, -210, 951, 3876, -805, 362, -156, 53, -11 };
static int HO[12] = { -11, 53, -156, 362, -805, 3876, 951, -210, 32, 12, -11, 3 };
static short QQ4[8] = { 0, 150, 323, 530, 786, 1121, 1612, 2557 };
static short QQ2[3] = { 0, 202, 926 };
static short WL[8] = { -60, -30, 58, 172, 334, 538, 1198, 3042 };
static short WH[3] = { 0, -214, 798 };

static short ILA[353] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4,
    4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6,
    7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 9, 9, 9, 9, 10,
    10, 10, 10, 11, 11, 11, 11, 12, 12, 12, 13, 13, 13, 13, 14, 14,
    15, 15, 15, 16, 16, 16, 17, 17, 18, 18, 18, 19, 19, 20, 20, 21,
    21, 22, 22, 23, 23, 24, 24, 25, 25, 26, 27, 27, 28, 28, 29, 30,
    31, 31, 32, 33, 33, 34, 35, 36, 37, 37, 38, 39, 40, 41, 42, 43,
    44, 45, 46, 47, 48, 49, 50, 51, 52, 54, 55, 56, 57, 58, 60, 61,
    63, 64, 65, 67, 68, 70, 71, 73, 75, 76, 78, 80, 82, 83, 85, 87,
    89, 91, 93, 95, 97, 99, 102, 104, 106, 109, 111, 113, 116, 118, 121, 124,
    127, 129, 132, 135, 138, 141, 144, 147, 151, 154, 157, 161, 165, 168, 172, 176,
    180, 184, 188, 192, 196, 200, 205, 209, 214, 219, 223, 228, 233, 238, 244, 249,
    255, 260, 266, 272, 278, 284, 290, 296, 303, 310, 316, 323, 331, 338, 345, 353,
    361, 369, 377, 385, 393, 402, 411, 420, 429, 439, 448, 458, 468, 478, 489, 500,
    511, 522, 533, 545, 557, 569, 582, 594, 607, 621, 634, 648, 663, 677, 692, 707,
    723, 739, 755, 771, 788, 806, 823, 841, 860, 879, 898, 918, 938, 958, 979, 1001,
    1023, 1045, 1068, 1092, 1115, 1140, 1165, 1190, 1216, 1243, 1270, 1298, 1327, 1356, 1386, 1416,
    1447, 1479, 1511, 1544, 1578, 1613, 1648, 1684, 1721, 1759, 1797, 1837, 1877, 1918, 1960, 2003,
    2047, 2092, 2138, 2185, 2232, 2281, 2331, 2382, 2434, 2488, 2542, 2598, 2655, 2713, 2773, 2833,
    2895, 2959, 3024, 3090, 3157, 3227, 3297, 3370, 3443, 3519, 3596, 3675, 3755, 3837, 3921, 4007, 4095
};
static unsigned short RIL4[16] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08 };
static short SIL4[16] = { 0, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0 };
static short IL4[16] = { 0, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6, 7 };
static unsigned short RIH2[4] = { 0x00, 0x01, 0x03, 0x02 };
static short SIH2[4] = { -1, -1, 0, 0 };
static short IH2[4] = { 2, 1, 1, 2 };

// decode const
static short QQ6[31] = {
    0, 17, 54, 91, 130, 170, 211, 254, 300, 347, 396, 447, 501, 558, 618, 682,
    750, 822, 899, 982, 1072, 1170, 1279, 1399, 1535, 1689, 1873, 2088, 2376, 2738, 3101
};
static short QQ5[16] = { 0, 35, 110, 190, 276, 370, 473, 587, 714, 858, 1023, 1219, 1458, 1765, 2195, 2919 };

static unsigned short RIL6[64] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x3e, 0x3f, 0x3d, 0x3c, 0x3b, 0x3a, 0x39, 0x38, 0x37, 0x36, 0x35, 0x34, 0x33, 0x32, 0x31, 0x30,
    0x2f, 0x2e, 0x2d, 0x2c, 0x2b, 0x2a, 0x29, 0x28, 0x27, 0x26, 0x25, 0x24, 0x23, 0x22, 0x21, 0x20
};

static unsigned short RIL5[32] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x1f, 0x1e, 0x1d, 0x1c, 0x1b, 0x1a, 0x19, 0x18, 0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11, 0x10
};

static short SIL6[64] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static short SIL5[32] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static short IL6[64] = {
    1, 1, 1, 1, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19,
    18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3,
    2, 1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30
};

static short IL5[32] = {
    1, 1, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2,
    1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
};

// Encode const
static short Q6[31] = {
    0, 35, 72, 110, 150, 190, 233, 276, 323, 370, 422, 473, 530, 587, 650, 714,
    786, 858, 940, 1023, 1121, 1219, 1339, 1458, 1612, 1765, 1980, 2195, 2557, 2919, 0
};
static short Q2[3] = { 0, 564, 0 };

static unsigned short ILN[30] = {
    0x3f, 0x3e, 0x1f, 0x1e, 0x1d, 0x1c, 0x1b, 0x1a, 0x19, 0x18, 0x17, 0x16, 0x15, 0x14, 0x13, 0x12,
    0x11, 0x10, 0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04
};

static unsigned short ILP[30] = { 0x3d, 0x3c, 0x3b, 0x3a, 0x39, 0x38, 0x37, 0x36, 0x35, 0x34, 0x33, 0x32,
    0x31, 0x30, 0x2f, 0x2e, 0x2d, 0x2c, 0x2b, 0x2a, 0x29, 0x28, 0x27, 0x26, 0x25, 0x24, 0x23, 0x22, 0x21, 0x20
};
static unsigned short IHN[2] = { 0x01, 0x00 };
static unsigned short IHP[2] = { 0x03, 0x02 };

// Encode func
static void transmit_qmf(pG722Codec pEnc, short xo, short xe, short *xl, short *xh)
{
    int *qmf_e = pEnc->qmf_e, *qmf_o = pEnc->qmf_o;
    int *he = (int *)HE, *ho = (int *)HO;
    int fe, fo;

    // fe and fo are computed with respect to the same 16kHz input sampling instants and are output at an 8 kHz sampling rate.
    qmf_e[5] = FUNSHIFT2(qmf_e[5], qmf_e[4]);
    qmf_e[4] = FUNSHIFT2(qmf_e[4], qmf_e[3]);
    qmf_e[3] = FUNSHIFT2(qmf_e[3], qmf_e[2]);
    qmf_e[2] = FUNSHIFT2(qmf_e[2], qmf_e[1]);
    qmf_e[1] = FUNSHIFT2(qmf_e[1], qmf_e[0]);
    qmf_e[0] = PACK16LSB(qmf_e[0], xe);
    qmf_o[5] = FUNSHIFT2(qmf_o[5], qmf_o[4]);
    qmf_o[4] = FUNSHIFT2(qmf_o[4], qmf_o[3]);
    qmf_o[3] = FUNSHIFT2(qmf_o[3], qmf_o[2]);
    qmf_o[2] = FUNSHIFT2(qmf_o[2], qmf_o[1]);
    qmf_o[1] = FUNSHIFT2(qmf_o[1], qmf_o[0]);
    qmf_o[0] = PACK16LSB(qmf_o[0], xo);

  // Multiply the even order QMF coefficients by the appropriately delayed input signals, and accumulate these products.
  fe = IFIR16(qmf_e[0], he[0])+IFIR16(qmf_e[1], he[1])+IFIR16(qmf_e[2], he[2])+
       IFIR16(qmf_e[3], he[3])+IFIR16(qmf_e[4], he[4])+IFIR16(qmf_e[5], he[5]);

  // Multiply the odd order QMF coefficients by the appropriately delayed input signals, and accumulate these products.
  fo = IFIR16(qmf_o[0], ho[0])+IFIR16(qmf_o[1], ho[1])+IFIR16(qmf_o[2], ho[2])+
       IFIR16(qmf_o[3], ho[3])+IFIR16(qmf_o[4], ho[4])+IFIR16(qmf_o[5], ho[5]);

    // compute the lower sub-band signal component.
    *xl = ICLIPI((fe + fo) >> 13, 16383);

    // Compute the higher sub-band signal component.
    *xh = ICLIPI((fe - fo) >> 13, 16383);
}

static void lower_sb_encode(pG722Codec pEnc, short xl, unsigned short *il)
{
    short *plt = pEnc->plt, *rlt = pEnc->rlt, *dlt = pEnc->dlt;
    short *al = pEnc->al, *bl = pEnc->bl;

    short bpl[7], apl[3], sg[7];
    short el, depl, nbpl, sil, il4 = 0;
    short wd, wd1, wd2, wd3, wd4, wd5;
    unsigned short ril;
    int i;

    // Compute the difference signal by subtracting from the input signal its predicted value.
    el = ICLIPI(xl - pEnc->sl, 32767);

    // Quantize the difference signal in the lower sub-band.
    sil = el >> 15;
    wd = MUX(sil, ~el, el);
    wd1 = (Q6[29] * pEnc->detl) >> 12;
    if (wd >= wd1)
        *il = MUX(sil, ILN[29], ILP[29]);
    else
    {
        for (i = 0; i < 29; i++)
        {
            wd1 = (Q6[i] * pEnc->detl) >> 12;
            wd2 = (Q6[i + 1] * pEnc->detl) >> 12;
            if (wd == CLIP(wd, wd1, wd2 - 1))
            {
                *il = MUX(sil, ILN[i], ILP[i]);
                break;
            }
        }
    }
    // Compute the quantized difference signal for the adaptive predictor in the lower sub-band.
    ril = (*il) >> 2;    // Delete 2 LSBs
    for (i = 0; i < 16; i++)
    {            // Derive sign of dlt[0]
        if (ril == RIL4[i])
        {
            sil = SIL4[i];
            il4 = IL4[i];
            break;
        }
    }
    wd1 = QQ4[il4] << 3;    // Scale table constant
    wd2 = MUX(sil, -wd1, wd1);
    dlt[0] = (pEnc->detl * wd2) >> 15;

    // Update the logarithmic quantizer scale factor in the lower sub-band.
    wd = (pEnc->nbl * 32512) >> 15;    // Leakage factor of 127/128
    pEnc->nbl = nbpl = UCLIPI(wd + WL[il4], 18432);    // Add scale factor multiplier

    // Compute the quantizer scale factor in the lower sub-band.
    wd1 = (nbpl >> 6) & 0x1ff;
    wd2 = wd1 + 64;        // Compute table address for ILA
    pEnc->detl = depl = (ILA[wd2] + 1) << 2;    //Scaling by 2-bit shift

    // Compute partially reconstructed signal.
    plt[0] = ICLIPI(dlt[0] + pEnc->szl, 32767);

    // Compute reconstructed signal for the adaptive predictor.
    rlt[0] = ICLIPI(pEnc->sl + dlt[0], 32767);

    // Update sixth-order predictor (zero section) coefficients.
    wd1 = IZERO(dlt[0], 128);
    sg[0] = dlt[0] >> 15;    // Sign of dlt[0]
    for (i = 1; i <= 6; i++)
    {
        sg[i] = dlt[i] >> 15;    // Sign of dlt[i]
        wd2 = MUX(sg[0] == sg[i], wd1, -wd1);
        wd3 = (bl[i] * 32640) >> 15;    // Leak factor of 255/256
        bpl[i] = ICLIPI(wd2 + wd3, 32767);    // Update zero-section coefficients
    }

    // Update second predictor coefficient (pole section).
    sg[0] = plt[0] >> 15;    // Sign of plt[0]
    sg[1] = plt[1] >> 15;    // Sign of plt[1]
    sg[2] = plt[2] >> 15;    // Sign of plt[2]
    wd1 = ICLIPI(al[1] << 2, 32767);    // Compute f(al[1])
    wd2 = MUX(sg[0] == sg[1], -wd1, wd1) >> 7;
    wd3 = MUX(sg[0] == sg[2], 128, -128);
    wd4 = wd2 + wd3;    // Compute gain factor
    wd5 = (al[2] * 32512) >> 15;    // Leak factor of 127/128
    apl[2] = CLIP(wd4 + wd5, -12288, 12288);    // Update second pole section coefficient

    // Update first predictor coefficient (pole section).
    wd1 = MUX(sg[0] == sg[1], 192, -192);    // Gain of 3/256
    wd2 = (al[1] * 32640) >> 15;    // Leak factor of 255/256
    wd3 = 15360 - apl[2];    // Compute (1-2^(-4)-apl[2])
    apl[1] = CLIP(wd1 + wd2, -wd3, wd3);    // Update first pole section coefficient

    dlt[6] = dlt[5];
    dlt[5] = dlt[4];
    dlt[4] = dlt[3];
    dlt[3] = dlt[2];
    dlt[2] = dlt[1];
    dlt[1] = dlt[0];
    plt[2] = plt[1];
    plt[1] = plt[0];
    rlt[2] = rlt[1];
    rlt[1] = rlt[0];
    bl[6] = bpl[6];
    bl[5] = bpl[5];
    bl[4] = bpl[4];
    bl[3] = bpl[3];
    bl[2] = bpl[2];
    bl[1] = bpl[1];
    al[2] = apl[2];
    al[1] = apl[1];

    // Compute predictor output signal (zero section).
    pEnc->szl = 0;
    for (i = 6; i > 0; i--)
        pEnc->szl = ICLIPI(pEnc->szl + ((bl[i] * dlt[i]) >> 14), 32767);    // Compute partial

    // Compute predictor output signal (pole section).
    wd1 = (al[1] * ICLIPI(rlt[1], 16383)) >> 14;    // Compute partial pole section output
    pEnc->spl = ICLIPI(((al[2] * ICLIPI(rlt[2], 16383)) >> 14) + wd1, 32767);

    // Compute the predictor output value.
    pEnc->sl = ICLIPI(pEnc->spl + pEnc->szl, 32767);
}

static void higher_sb_encode(pG722Codec pEnc, short xh, unsigned short *ih)
{
    short *ph = pEnc->ph, *rh = pEnc->rh, *ah = pEnc->ah;
    short *dh = pEnc->dh, *bh = pEnc->bh;

    short bph[7], aph[3], sg[7];
    short eh, deph, nbph, sih, ih2 = 0;
    short wd, wd1, wd2, wd3, wd4, wd5;
    int i;

    // Compute the difference signal by subtracting from the input signal its predicted value.
    eh = ICLIPI(xh - pEnc->sh, 32767);

    // Quantize the difference signal in the higher sub-band.
    sih = eh >> 15;
    wd = MUX(sih, ~eh, eh);
    wd1 = 0;
    wd2 = (Q2[1] * pEnc->deth) >> 12;
    i = (wd != CLIP(wd, wd1, wd2 - 1));
    *ih = MUX(sih, IHN[i], IHP[i]);

    // compute the quantized difference signal in the higher sub-band.
    for (i = 0; i < 4; i++)
    {            // Derive sign of dh[0]
        if (*ih == RIH2[i])
        {
            sih = SIH2[i];
            ih2 = IH2[i];
            break;
        }
    }
    wd1 = QQ2[ih2] << 3;    // Scale table constant
    wd2 = MUX(sih, -wd1, wd1);
    dh[0] = (pEnc->deth * wd2) >> 15;

    // Update the logarithmic quantized scale factor in the higher sub-band.
    wd = (pEnc->nbh * 32512) >> 15;    // Leakage factor of 127/128
    pEnc->nbh = nbph = CLIP(wd + WH[ih2], 0, 22528);    // Add scale factor multiplier

    // Compute the quantizer scale factor in the higher sub-band.
    wd = (nbph >> 6) & 0x1ff;    // Compute table constant for ILA
    pEnc->deth = deph = (ILA[wd] + 1) << 2;    // Scaling by 2-bit shift

    // Compute partially reconstructed signal.
    ph[0] = ICLIPI(dh[0] + pEnc->szh, 32767);

    // Compute reconstructed signal for the adaptive predictor.
    rh[0] = ICLIPI(pEnc->sh + dh[0], 32767);

    // Update sixth-order predictor (zero section) coefficients.
    wd1 = IZERO(dh[0], 128);
    sg[0] = dh[0] >> 15;    // Sign of dh[0]
    for (i = 1; i <= 6; i++)
    {
        sg[i] = dh[i] >> 15;    // Sign of dh[i]
        wd2 = MUX(sg[0] == sg[i], wd1, -wd1);
        wd3 = (bh[i] * 32640) >> 15;    // Leak factor of 255/256
        bph[i] = ICLIPI(wd2 + wd3, 32767);
    }

    // Update second predictor coefficient (pole section).
    sg[0] = ph[0] >> 15;    // Sign of ph[0]
    sg[1] = ph[1] >> 15;    // Sign of ph[1]
    sg[2] = ph[2] >> 15;    // Sign of ph[2]
    wd1 = ICLIPI(ah[1] << 2, 32767);
    wd2 = MUX(sg[0] == sg[1], -wd1, wd1) >> 7;
    wd3 = MUX(sg[0] == sg[1], 128, -128);
    wd4 = wd2 + wd3;    // Compute gain factor
    wd5 = (ah[2] * 32512) >> 15;    // Leak factor of 127/128
    aph[2] = CLIP(wd4 + wd5, -12288, 12288);    // Update second pole section coefficient

    // Update first predictor coefficient (pole section).
    wd1 = MUX(sg[0] == sg[1], 192, -192);    // Gain of 3/256
    wd2 = (ah[1] * 32640) >> 15;    // Leak factor of 255/256
    wd3 = 15360 - aph[2];    // Compute (1-2^(-4)-aph[2])
    aph[1] = CLIP(wd1 + wd2, -wd3, wd3);    // Update first pole section coefficient

    dh[6] = dh[5];
    dh[5] = dh[4];
    dh[4] = dh[3];
    dh[3] = dh[2];
    dh[2] = dh[1];
    dh[1] = dh[0];
    ph[2] = ph[1];
    ph[1] = ph[0];
    rh[2] = rh[1];
    rh[1] = rh[0];

    bh[6] = bph[6];
    bh[5] = bph[5];
    bh[4] = bph[4];
    bh[3] = bph[3];
    bh[2] = bph[2];
    bh[1] = bph[1];;
    ah[2] = aph[2];
    ah[1] = aph[1];;

    // Compute predictor output signal (zero section).
    pEnc->szh = 0;
    for (i = 6; i >= 1; i--)
        pEnc->szh = ICLIPI(pEnc->szh + ((bh[i] * dh[i]) >> 14), 32767);

    // Compute predictor output signal (pole section).
    wd1 = (ah[1] * ICLIPI(rh[1], 16383)) >> 14;    // Compute partial pole section output
    pEnc->sph = ICLIPI(wd1 + ((ah[2] * ICLIPI(rh[2], 16383)) >> 14), 32767);

    // Compute the predictor output value
    pEnc->sh = ICLIPI(pEnc->sph + pEnc->szh, 32767);
}

// Decode func
static void lower_sb_decode(pG722Codec pDec, int mode, unsigned short ilr, short *rcl)
{
    short *plt = pDec->plt, *rlt = pDec->rlt, *dlt = pDec->dlt;
    short *al = pDec->al, *bl = pDec->bl;

    short bpl[7], apl[3], sg[7];
    short dl, nbpl, depl, sil = 0, il6 = 0, il5 = 0, il4 = 0;
    short wd, wd1 = 0, wd2, wd3, wd4, wd5;
    unsigned short ril;
    int i;

// Compute quantized difference signal for the decoder output in the lower sub-band.
    if (mode == 1)
    {            // 6-bit codeword
        ril = ilr;
        for (i = 0; i < 64; i++)
        {
            if (ril == RIL6[i])
            {
                sil = SIL6[i];
                il6 = IL6[i];
                break;
            }
        }
        wd1 = QQ6[il6] << 3;    // Scale table constant
    }
    else if (mode == 2)
    {            // 5-bit codeword
        ril = ilr >> 1;
        for (i = 0; i < 32; i++)
        {
            if (ril == RIL5[i])
            {
                sil = SIL5[i];
                il5 = IL5[i];
                break;
            }
        }
        wd1 = QQ5[il5] << 3;    // Scale table constant
    }
    else if (mode == 3)
    {            // 4-bit codeword
        ril = ilr;
        for (i = 0; i < 16; i++)
        {
            if (ril == RIL4[i])
            {
                sil = SIL4[i];
                il4 = IL4[i];
                break;
            }
        }
        wd1 = QQ4[il4] << 3;    // Scale table constant
    }
    wd2 = MUX(sil, -wd1, wd1);
    dl = (pDec->detl * wd2) >> 15;

// Compute reconstructed signal for the adaptive predictor.
    *rcl = ICLIPI(pDec->sl + dl, 16383);

// Compute the quantized difference signal for the adaptive predictor in the lower sub-band.                        */
    ril = ilr;
    for (i = 0; i < 16; i++)
    {            // Derive sign of dlt[0]
        if (ril == RIL4[i])
        {
            sil = SIL4[i];
            il4 = IL4[i];
            break;
        }
    }
    wd1 = QQ4[il4] << 3;    // Scale table constant
    wd2 = MUX(sil, -wd1, wd1);
    dlt[0] = (pDec->detl * wd2) >> 15;

// Update the logarithmic quantizer scale factor in the lower sub-band.
    wd = (pDec->nbl * 32512) >> 15;    // Leakage factor of 127/128
    pDec->nbl = nbpl = CLIP(wd + WL[il4], 0, 18432);    // Add scale factor multiplier

// Compute the quantizer scale factor in the sub-band.
    wd1 = (nbpl >> 6) & 511;
    wd2 = wd1 + 64;        // Compute table address for ILA
    pDec->detl = depl = (ILA[wd2] + 1) << 2;    // Scaling by 2-bit shift

// Compute partially reconstructed signal.
    plt[0] = ICLIPI(dlt[0] + pDec->szl, 32767);

// Compute reconstructed signal for the adaptive predictor.
    rlt[0] = ICLIPI(pDec->sl + dlt[0], 32767);

// Update sixth-order predictor (zero section) coefficients.
    wd1 = IZERO(dlt[0], 128);
    sg[0] = dlt[0] >> 15;    // Sign of dlt[0]
    for (i = 1; i <= 6; i++)
    {
        sg[i] = dlt[i] >> 15;    // Sign of dlt[i]
        wd2 = MUX(sg[0] == sg[i], wd1, -wd1);
        wd3 = (bl[i] * 32640) >> 15;    // Leak factor of 255/256
        bpl[i] = ICLIPI(wd2 + wd3, 32767);
    }

// Update second predictor coefficient (pole seciton).
    sg[0] = plt[0] >> 15;    // Sign of plt[0]
    sg[1] = plt[1] >> 15;    // Sign of plt[1]
    sg[2] = plt[2] >> 15;    // Sign of plt[2]
    wd1 = ICLIPI(ICLIPI(al[1], 16383) << 2, 32767);
    wd2 = MUX(sg[0] == sg[1], -wd1, wd1) >> 7;    // Gain of 1/128
    wd3 = MUX(sg[0] == sg[2], 128, -128);
    wd4 = wd2 + wd3;    // Compute gain factor
    wd5 = (al[2] * 32512) >> 15;    // Leak factor of 127/128
    apl[2] = CLIP(wd4 + wd5, -12288, 12288);    // Update second pole section coefficient

// Update first predictor coefficient (pole section).
    wd1 = MUX(sg[0] == sg[1], 192, -192);
    wd2 = (al[1] * 32640) >> 15;    // Leak factor of 255/256
    wd3 = 15360 - apl[2];    // Compute (1-2^(-4)-apl[2])
    apl[1] = CLIP(wd1 + wd2, -wd3, wd3);    // Update first pole section coefficient

    dlt[6] = dlt[5];
    dlt[5] = dlt[4];
    dlt[4] = dlt[3];
    dlt[3] = dlt[2];
    dlt[2] = dlt[1];
    dlt[1] = dlt[0];
    plt[2] = plt[1];
    plt[1] = plt[0];
    rlt[2] = rlt[1];
    rlt[1] = rlt[0];

    bl[6] = bpl[6];
    bl[5] = bpl[5];
    bl[4] = bpl[4];
    bl[3] = bpl[3];
    bl[2] = bpl[2];
    bl[1] = bpl[1];
    al[2] = apl[2];
    al[1] = apl[1];

// Compute predictor output signal (zero seciton).
    pDec->szl = 0;
    for (i = 6; i >= 1; i--)
        pDec->szl = ICLIPI(pDec->szl + ((bl[i] * dlt[i]) >> 14), 32767);

// Compute predictor output signal (pole section).
    wd1 = (al[1] * ICLIPI(rlt[1], 16383)) >> 14;
    pDec->spl = ICLIPI(wd1 + ((al[2] * ICLIPI(rlt[2], 16383)) >> 14), 32767);    // Sum the partial pole section outputs

// Compute the predictor outptu value.
    pDec->sl = ICLIPI(pDec->spl + pDec->szl, 32767);
}

static void higher_sb_decode(pG722Codec pDec, unsigned short ih, short *rch)
{
    short *ph = pDec->ph, *rh = pDec->rh, *ah = pDec->ah;
    short *dh = pDec->dh, *bh = pDec->bh;

    short bph[7], aph[3], sg[7];
    short nbph, deph, sih = 0, ih2 = 0;
    short wd, wd1, wd2, wd3, wd4, wd5;
    int i;

// Compute the quantized difference signal in the higher sub-band.
    for (i = 0; i < 4; i++)
    {            // Derive sign of dh[0]
        if (ih == RIH2[i])
        {
            sih = SIH2[i];
            ih2 = IH2[i];
            break;
        }
    }
    wd1 = QQ2[ih2] << 3;    // Scale table constant
    wd2 = MUX(sih, -wd1, wd1);
    dh[0] = (pDec->deth * wd2) >> 15;

// Compute partially reconstructed signal.
    *rch = ICLIPI(pDec->sh + dh[0], 16383);

// Update the logarithmic quantizer scale factor in the higher sub-band.
    wd = (pDec->nbh * 32512) >> 15;    // Leakage factor of 127/128
    pDec->nbh = nbph = UCLIPI(wd + WH[ih2], 22528);    // Add scale factor multiplier

// Compute the quantizer scale factor in the higher sub-band.

    wd = (nbph >> 6) & 511;    // Compute table address for ILA
    pDec->deth = deph = (ILA[wd] + 1) << 2;    // Scaling by 2-bit shift

// Compute partially reconstructed signal.
    ph[0] = ICLIPI(dh[0] + pDec->szh, 32767);

// Compute reconstructed signal for the adaptive predictor.
    rh[0] = ICLIPI(pDec->sh + dh[0], 32767);

// Update sixth-order predictor (zero section) coefficients.
    wd1 = IZERO(dh[0], 128);
    sg[0] = dh[0] >> 15;    // Sign of dh[0]
    for (i = 1; i <= 6; i++)
    {
        sg[i] = dh[i] >> 15;    // Sign of dh[i]
        wd2 = MUX(sg[0] == sg[i], wd1, -wd1);
        wd3 = (bh[i] * 32640) >> 15;    // Leak factor 255/256
        bph[i] = ICLIPI(wd2 + wd3, 32767);
    }

// Update second order predictor coefficient (pole section).
    sg[0] = ph[0] >> 15;    // Sign of ph[0]
    sg[1] = ph[1] >> 15;    // Sign of ph[1]
    sg[2] = ph[2] >> 15;    // Sign of ph[2]
    wd1 = ICLIPI(ICLIPI(ah[1], 16383) << 2, 32767);
    wd2 = MUX(sg[0] == sg[1], -wd1, wd1) >> 7;
    wd3 = MUX(sg[0] == sg[1], 128, -128);
    wd4 = wd2 + wd3;    // Compute gain factor
    wd5 = (ah[2] * 32512) >> 15;    // Leak factor of 127/128
    aph[2] = CLIP(wd4 + wd5, -12288, 12288);    // Update second pole section coefficient

// Update first predictor coefficient (pole section).
    wd1 = MUX(sg[0] == sg[1], 192, -192);    // Gain of 3/256
    wd2 = (ah[1] * 32640) >> 15;    // Leak factor of 255/256
    wd3 = 15360 - aph[2];    // Compute (1-2^(-4)-aph[2])
    aph[1] = CLIP(wd1 + wd2, -wd3, wd3);    // Update first pole section coefficient

    dh[6] = dh[5];
    dh[5] = dh[4];
    dh[4] = dh[3];
    dh[3] = dh[2];
    dh[2] = dh[1];
    dh[1] = dh[0];
    ph[2] = ph[1];
    ph[1] = ph[0];
    rh[2] = rh[1];
    rh[1] = rh[0];

    bh[6] = bph[6];
    bh[5] = bph[5];
    bh[4] = bph[4];
    bh[3] = bph[3];
    bh[2] = bph[2];
    bh[1] = bph[1];
    ah[2] = aph[2];
    ah[1] = aph[1];

// Compute predictor output signal (zero seciton).
    pDec->szh = 0;
    for (i = 6; i >= 1; i--)
        pDec->szh = ICLIPI(pDec->szh + ((bh[i] * dh[i]) >> 14), 32767);

// Compute predictor output signal (pole section).
    wd1 = (ah[1] * ICLIPI(rh[1], 16383)) >> 14;    // Compute partial pole section output
    pDec->sph = ICLIPI(wd1 + ((ah[2] * ICLIPI(rh[2], 16383)) >> 14), 32767);

// Compute the predictor output value.
    pDec->sh = ICLIPI(pDec->sph + pDec->szh, 32767);
}

static void receive_qmf(pG722Codec pDec, short rcl, short rch, short *xout1, short *xout2)
{
    int *xe = pDec->qmf_e, *xo = pDec->qmf_o;
    int *he = (int *)HE, *ho = (int *)HO;
    int fo, fe;

// Function : Compute the input signal to the receive QMF
    xe[5] = FUNSHIFT2(xe[5], xe[4]);
    xe[4] = FUNSHIFT2(xe[4], xe[3]);
    xe[3] = FUNSHIFT2(xe[3], xe[2]);
    xe[2] = FUNSHIFT2(xe[2], xe[1]);
    xe[1] = FUNSHIFT2(xe[1], xe[0]);
    xe[0] = PACK16LSB(xe[0], ICLIPI(rcl - rch, 32767));

// Function : Compute the input signal to the receive QMF
    xo[5] = FUNSHIFT2(xo[5], xo[4]);
    xo[4] = FUNSHIFT2(xo[4], xo[3]);
    xo[3] = FUNSHIFT2(xo[3], xo[2]);
    xo[2] = FUNSHIFT2(xo[2], xo[1]);
    xo[1] = FUNSHIFT2(xo[1], xo[0]);
    xo[0] = PACK16LSB(xo[0], ICLIPI(rcl + rch, 32767));
// Multiply the even order QMF coefficients by the appropriately delayed input signals, and accumulate these products.
    fe = IFIR16(xe[5], he[5]) + IFIR16(xe[4], he[4]) + IFIR16(xe[3], he[3]) + IFIR16(xe[2], he[2]) + IFIR16(xe[1], he[1]) + IFIR16(xe[0], he[0]);

//Multiply the odd order QMF coefficients by the appropriately delayed input signals, and accumulate these products.
    fo = IFIR16(xo[5], ho[5]) + IFIR16(xo[4], ho[4]) + IFIR16(xo[3], ho[3]) + IFIR16(xo[2], ho[2]) + IFIR16(xo[1], ho[1]) + IFIR16(xo[0], ho[0]);

    *xout1 = ICLIPI(fe >> 12, 16383);
    *xout2 = ICLIPI(fo >> 12, 16383);
}


// For user interface
pG722Codec g722Codec_init()
{
    pG722Codec pCodec = (pG722Codec) malloc(sizeof(G722Codec));
    memset(pCodec, 0, sizeof(G722Codec));
    pCodec->deth = 8;
    pCodec->detl = 32;
    //_cache_copyback(pCodec, sizeof(G722Codec));
    return pCodec;
}

void g722Codec_free(pG722Codec pCodec)
{
    free(pCodec);
}

unsigned int g722Codec_enc(pG722Codec pEnc, short *signal, unsigned char *serial, unsigned int sample)
{
    unsigned short il, ih;
    short xl, xh, i;

    // ±àÂë
    for (i = 0; i < sample; i += 8, signal += 8, serial += 3)
    {
        // 0)
        transmit_qmf(pEnc, signal[0], signal[1], &xl, &xh);
        lower_sb_encode(pEnc, xl, &il);
        higher_sb_encode(pEnc, xh, &ih);
        serial[0] = (il & 0x3c) | (ih & 0x3);

        // 1)
        transmit_qmf(pEnc, signal[2], signal[3], &xl, &xh);
        lower_sb_encode(pEnc, xl, &il);
        higher_sb_encode(pEnc, xh, &ih);
        serial[0] |= ih << 6;
        serial[1] = (il & 0x3c) >> 2;

        // 2)
        transmit_qmf(pEnc, signal[4], signal[5], &xl, &xh);
        lower_sb_encode(pEnc, xl, &il);
        higher_sb_encode(pEnc, xh, &ih);
        serial[1] |= ((il & 0x3c) | (ih & 0x3)) << 4;
        serial[2] = (il >> 4) & 0x3;

        // 3)
        transmit_qmf(pEnc, signal[6], signal[7], &xl, &xh);
        lower_sb_encode(pEnc, xl, &il);
        higher_sb_encode(pEnc, xh, &ih);
        serial[2] |= ((il & 0x3c) | (ih & 0x3)) << 2;
    }

    return (unsigned int)((sample >> 3) * 3.0);
}

unsigned int g722Codec_dec(pG722Codec pDec, short *signal, unsigned char *serial, unsigned int sample)
{
    short rcl, rch, xout1, xout2, i;

    for (i = 0; i < sample; i += 8, serial += 3)
    {
        lower_sb_decode(pDec, 3, (serial[0] >> 2) & 0xf, &rcl);
        higher_sb_decode(pDec, serial[0] & 0x3, &rch);
        receive_qmf(pDec, rcl, rch, &xout1, &xout2);
        *signal++ = xout1;
        *signal++ = xout2;

        lower_sb_decode(pDec, 3, serial[1] & 0xf, &rcl);
        higher_sb_decode(pDec, serial[0] >> 6, &rch);
        receive_qmf(pDec, rcl, rch, &xout1, &xout2);
        *signal++ = xout1;
        *signal++ = xout2;

        lower_sb_decode(pDec, 3, ((serial[1] >> 6) | (serial[2] << 2)) & 0xf, &rcl);
        higher_sb_decode(pDec, (serial[1] >> 4) & 0x3, &rch);
        receive_qmf(pDec, rcl, rch, &xout1, &xout2);
        *signal++ = xout1;
        *signal++ = xout2;

        lower_sb_decode(pDec, 3, serial[2] >> 4, &rcl);
        higher_sb_decode(pDec, (serial[2] >> 2) & 0x3, &rch);
        receive_qmf(pDec, rcl, rch, &xout1, &xout2);
        *signal++ = xout1;
        *signal++ = xout2;
    }

    return sample << 1;
}
