#include <wchar.h>
#include <locale.h>
#include "video_osd_private.h"
#include "hi3518/HISI_private.h"

#define FONT_FILE        "/ipnc/share/font/gkai00mp.ttf"
//OSD区域个数, 0 - 时间； 1 - 通道名
#define REGION_NUM	(2)
#define STROKE

#define REGION_TEXT0 (0)
#define REGION_TEXT1 (1)
#define REGION_TIME0 (2)
#define REGION_TIME1 (3)
#define TIME_REGION_WIDTH	(320)
#define TIME_REGION_HEIGHT	(32)

#define SET_A(p, value)   (p = ((p & 0x0FFF) | (value << 12)))
#define SET_R(p, value)   (p = ((p & 0xF0FF) | (value << 8)))
#define SET_G(p, value)   (p = ((p & 0xFF0F) | (value << 4)))
#define SET_B(p, value)   (p = ((p & 0xFFF0) | (value)))

extern HISI_SYS_CFG_OBJ hi_sys;

typedef struct _tagBITMAP_CFG
{
	HB_U32 width;
	HB_U32 height;
	HB_U16 *pbuff;
}BITMAP_CFG_OBJ, *BITMAP_CFG_HANDLE;

HB_S32 hb_mbstowcs(wchar_t *dst, HB_CHAR *src, HB_S32 len)
{
	HB_CHAR outbuf[128];		//最多64个中文
	HB_S32 ret = -1;
	HB_S32 i = 0;
	HB_CHAR *pin = src;
	HB_CHAR *pout = outbuf;
	HB_S32 inlen = -1;
	HB_S32 outlen = -1;
	iconv_t iconv_fd = hi_sys.osd_cfg.iconv_fd;

	memset(outbuf, 0, sizeof(outbuf));
	inlen = strlen(pin);
	outlen = len;
	ret = iconv(iconv_fd, &pin, (size_t *)&inlen, &pout, (size_t *)&outlen);
	if ((size_t)-1 == ret)
	{
		perror("iconv error.");
		return -1;
	}

	for (i = 0; outbuf[2 * i] != '\0'; i++)
	{
		dst[i] = GET16(&outbuf[2 * i]);
	}
	dst[i] = L'\0';
	
	ret = wcslen(dst);

	return ret;
}

HI_S32 RGN_ShowOrHide(RGN_HANDLE RgnHandle, VENC_GRP VencGrp, HI_BOOL bShow)
{
    MPP_CHN_S stChn;
    RGN_CHN_ATTR_S stChnAttr;
    HI_S32 s32Ret;

    stChn.enModId = HI_ID_GROUP;
    stChn.s32DevId = VencGrp;
    stChn.s32ChnId = 0;
        
    s32Ret = HI_MPI_RGN_GetDisplayAttr(RgnHandle, &stChn, &stChnAttr);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_RGN_GetDisplayAttr (%d)) failed with %#x!\n",\
               RgnHandle, s32Ret);
        return HI_FAILURE;
    }

    stChnAttr.bShow = bShow;
    
    s32Ret = HI_MPI_RGN_SetDisplayAttr(RgnHandle,&stChn,&stChnAttr);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_RGN_SetDisplayAttr (%d)) failed with %#x!\n",\
               RgnHandle, s32Ret);
        return HI_FAILURE;
    }
    
    return HI_SUCCESS;
}

HB_S32 hi_region_create(HB_S32 rgn_handle, HB_S32 width, HB_S32 height)
{
	RGN_ATTR_S rgn_attr;

	HI_S32 ret = HI_FAILURE;

	HI_MPI_RGN_Destroy(rgn_handle);

	rgn_attr.enType = OVERLAY_RGN;
	rgn_attr.unAttr.stOverlay.enPixelFmt = PIXEL_FORMAT_RGB_4444;
	rgn_attr.unAttr.stOverlay.stSize.u32Width = ALIGN_N(width, 16);
	rgn_attr.unAttr.stOverlay.stSize.u32Height = ALIGN_N(height, 16);
	rgn_attr.unAttr.stOverlay.u32BgColor = 0x0;

	ret = HI_MPI_RGN_Create(rgn_handle, &rgn_attr);
	if (HI_SUCCESS != ret)
	{
		VIDEO_OSD_ERROR("HI_MPI_RGC_Create(handle %d) failed with 0x%x .\n", rgn_handle, ret);
		return VIDEO_ERROR;
	}

	return VIDEO_OK;
}

HB_S32 hi_region_destroy(RGN_HANDLE rgn_handle)
{
	HI_S32 ret = HI_FAILURE;

	ret = HI_MPI_RGN_Destroy(rgn_handle);
	if (HI_SUCCESS != ret)
	{
		VIDEO_OSD_ERROR("HI_MPI_RGN_Destroy(handle %d) failed with 0x%x .\n", rgn_handle, ret);
		return VIDEO_ERROR;
	}

	return VIDEO_OK;
}

HB_S32 hi_region_recreate(HB_S32 rgn_handle, HB_S32 width, HB_S32 height)
{
	if (VIDEO_OK != hi_region_destroy(rgn_handle))
	{
		VIDEO_OSD_ERROR("hi_region_destroy failed .");
		return VIDEO_ERROR;
	}

	if (VIDEO_OK != hi_region_create(rgn_handle, width, height))
	{
		VIDEO_OSD_ERROR("hi_region_create failed .");
		return VIDEO_ERROR;
	}

	return VIDEO_OK;
}

HB_VOID set_color(HB_U16 *addr, OSD_COLOR_E color)
{
	switch (color)
	{
		case COLOR_BLACK :
			{
				SET_R(*addr, 0x0);
				SET_G(*addr, 0x0);
				SET_B(*addr, 0x0);
				break;
			}
		case COLOR_WHITE :
			{
				SET_R(*addr, 0xf);
				SET_G(*addr, 0xf);
				SET_B(*addr, 0xf);
				break;
			}
		case COLOR_RED :
			{
				SET_R(*addr, 0xf);
				SET_G(*addr, 0x0);
				SET_B(*addr, 0x0);
				break;
			}
		case COLOR_GREEN :
			{
				SET_R(*addr, 0x0);
				SET_G(*addr, 0xf);
				SET_B(*addr, 0x0);
				break;
			}
		case COLOR_BLUE :
			{
				SET_R(*addr, 0x0);
				SET_G(*addr, 0x0);
				SET_B(*addr, 0xf);
				break;
			}
		default :
			{
				SET_R(*addr, 0x0);
				SET_G(*addr, 0x0);
				SET_B(*addr, 0x0);
			}
	}
}

/* Mail: jorneytu@gamil.com */
HB_S32 char2bitmap(HB_S32 stream_id, wchar_t str, BITMAP_CFG_OBJ *bitmap)
{
	FT_Face face;
	FT_GlyphSlot slot;
	FT_Error error;
	HB_CHAR alpha;
	HB_S32 row, col;

    /* Mail: jorneytu@gamil.com */
	HB_S32 n = 0;
	HB_S32 top_padding = 0;
	HB_S32 bottom_padding = 0;
	HB_S32 right_padding = 0;

	OSD_COLOR_E color = hi_sys.osd_cfg.color;
	face = hi_sys.osd_cfg.freetype.face[stream_id];

	FT_UInt glyph_index = FT_Get_Char_Index(face, str);
	error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
	if (error)
	{
		VIDEO_OSD_ERROR("FT_Load_Glyph error !\n");
		return VIDEO_ERROR;
	}
#ifdef STROKE
    extern char *stroke_outline(FT_Face face, FT_Library library,
            int *width, int *height);
    FT_Library library;
	library = hi_sys.osd_cfg.freetype.library;
    char *buffer = NULL;
    int _width = 0;
    int _height = 0;

	error = FT_Load_Glyph(face, glyph_index, FT_LOAD_NO_BITMAP);
	if (error)
	{
		VIDEO_OSD_ERROR("FT_Load_Glyph error !\n");
		return VIDEO_ERROR;
	}
    buffer = stroke_outline(face, library, &_width, &_height);

	slot = face->glyph;

    if (_width == 0)
        bitmap->width = 16;
    else
		bitmap->width = _width;
	bitmap->pbuff = (HB_U16 *)malloc(2 * bitmap->width * bitmap->height);

    top_padding = bitmap->height - _height;
    for (row = 0; row < top_padding; row++)
    {
        for (col = 0; col < bitmap->width; col++)
        {
            SET_A(bitmap->pbuff[n], 0x0);
            set_color(&bitmap->pbuff[n], color);
            n++;
        }
    }

    for (row = 0; row < _height; row++)
    {
        for (col = 0; col < bitmap->width; col++)
        {

            if (_width == 0){
                SET_A(bitmap->pbuff[n], 0x0);
                set_color(&bitmap->pbuff[n], color);
                n++;
                break;
            }
            if ((buffer[row * bitmap->width + col]) == 0)
            {
                SET_A(bitmap->pbuff[n], 0x0);
                set_color(&bitmap->pbuff[n], color);
            }
            else
            {
                alpha = buffer[row * bitmap->width + col] >> 4;
                if (HB_TRUE == hi_sys.osd_cfg.transparent)
                {
                    alpha = alpha >> 2;
                }
                if (buffer[row * bitmap->width + col] < 128 &&buffer[row * bitmap->width + col] > 0)
                {
                    SET_A(bitmap->pbuff[n], 0xf);
                    set_color(&bitmap->pbuff[n], 1);
                }
                else
                {
                    SET_A(bitmap->pbuff[n], 0xf);
                    set_color(&bitmap->pbuff[n], color);
                }

            }
            n++;
        }
    }
    free(buffer);
#else
	error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
	if (error)
	{
		VIDEO_OSD_ERROR("FT_Render_Glyph error !\n");
		return VIDEO_ERROR;
	}

	slot = face->glyph;
	if(slot->bitmap.width > (slot->advance.x>> 6))
	{
		bitmap->width = slot->bitmap.width;
	}
	else
	{
		bitmap->width = slot->advance.x >> 6;
	}
	bitmap->pbuff = (HB_U16 *)malloc(2 * bitmap->width * bitmap->height);
	if (NULL == bitmap->pbuff)
	{
		VIDEO_OSD_ERROR("malloc error !\n");
		return VIDEO_ERROR;
	}

	if (slot->bitmap_top < 0)
	{
		top_padding = (face->size->metrics.ascender >> 6) - slot->bitmap.rows;
	}
	else
	{
		top_padding = (face->size->metrics.ascender >> 6) - slot->bitmap_top;
	}
	bottom_padding = bitmap->height - slot->bitmap.rows - top_padding;
	right_padding = bitmap->width - slot->bitmap.width;

	slot->bitmap.rows = IMIN(slot->bitmap.rows, bitmap->height);
	slot->bitmap.width = IMIN(slot->bitmap.width, bitmap->width);

	n = 0;
	for (row = 0; row < top_padding; row++)
	{
		for (col = 0; col < bitmap->width; col++)
		{
			SET_A(bitmap->pbuff[n], 0x0);
			set_color(&bitmap->pbuff[n], color);
			n++;
		}
	}

	for (row = 0; row < slot->bitmap.rows; row++)
	{
		for (col = 0; col < slot->bitmap.width; col++)
		{
			if ((slot->bitmap.buffer[row * slot->bitmap.pitch + col]) == 0)
			{
				SET_A(bitmap->pbuff[n], 0x0);
				set_color(&bitmap->pbuff[n], color);
			}
			else
			{
				alpha = slot->bitmap.buffer[row * slot->bitmap.pitch + col] >> 4;
				if (HB_TRUE == hi_sys.osd_cfg.transparent)
				{
					alpha = alpha >> 2;
				}
				SET_A(bitmap->pbuff[n], alpha);
				set_color(&bitmap->pbuff[n], color);
			}
			n++;
		}
		for (col = 0; col < right_padding; col++)
		{
			SET_A(bitmap->pbuff[n], 0x0);
			set_color(&bitmap->pbuff[n], 1);
			n++;
		}
	}

	for (row = 0; row < bottom_padding; row++)
	{
		for (col = 0; col < bitmap->width; col++)
		{
			SET_A(bitmap->pbuff[n], 0x0);
			set_color(&bitmap->pbuff[n], 1);
			n++;
		}
	}
#endif


	return VIDEO_OK;
}

HB_S32 bitmap_merge(BITMAP_S *dst, BITMAP_CFG_OBJ *src, HB_S32 count)
{
	HB_S32 i, j, k, n;
	HB_S32 width = 0;
	HB_S32 height = dst->u32Height;
	HB_U16 *pdst = dst->pData;

	for (i = 0; i < count; i++)
	{
		width += src[i].width;
	}
	if (width > dst->u32Width)
	{
		VIDEO_OSD_ERROR("bitmap width calculate error .");
		return VIDEO_ERROR;
	}

	for (i = 0, n = 0; i < height; i++)
	{
		for(j = 0; j < count; j++){
#ifdef STROKE
            if (i > src[j].height){
				SET_A(pdst[n], 0x0);
				set_color(&pdst[n], 1);
                n++;
            }
            else
#endif
            {
                for (k = 0; k < src[j].width; k++)
                {
                    pdst[n] = src[j].pbuff[i * src[j].width + k];
                    n++;
                }
            }
		}
	}

	return VIDEO_OK;
}

HB_S32 fill_bitmap(HB_S32 stream_id, BITMAP_S *bitmap, wchar_t *str)
{
	BITMAP_CFG_HANDLE ptr;
	HB_S32 width, height;
	HB_S32 i, count;

	count = wcslen(str);
	width = bitmap->u32Width;
	height = bitmap->u32Height;

	ptr = (BITMAP_CFG_OBJ *)malloc(count * sizeof(BITMAP_CFG_OBJ));
	if (NULL == ptr)
	{
		VIDEO_OSD_ERROR("malloc error .\n");
		return VIDEO_ERROR;
	}	

	bitmap->pData = malloc(2 * width * height);
	if(NULL == bitmap->pData)
	{
		VIDEO_OSD_ERROR("malloc error .\n");
		return VIDEO_ERROR;
	}

    /* Mail: jorneytu@gamil.com */
    memset(bitmap->pData, 0, 2 * width * height);

	pthread_mutex_lock(&hi_sys.t_lock);
	for (i = 0; i < count; i++)
	{
		ptr[i].height = height;
		char2bitmap(stream_id, str[i], &ptr[i]);
	}
	pthread_mutex_unlock(&hi_sys.t_lock);

	bitmap_merge(bitmap, ptr, count);

	for (i = 0; i < count; i++)
	{
		if (NULL != ptr[i].pbuff)
		{
			free(ptr[i].pbuff);
			ptr[i].pbuff = NULL;
		}

	}
	if (NULL != ptr)
	{
		free(ptr);
		ptr = NULL;
	}


	bitmap->enPixelFormat = PIXEL_FORMAT_RGB_4444;
	return VIDEO_OK;
}

#if 0
HB_S32 time_init()
{
	if (VIDEO_OK != hi_region_create(REGION_TIME, TIME_REGION_WIDTH, TIME_REGION_HEIGHT))
	{
		VIDEO_OSD_ERROR("hi_region_create failed .\n");
		return VIDEO_ERROR;
	}

	return VIDEO_OK;
}

HB_S32 time_deinit()
{	
	if (VIDEO_OK != hi_region_destroy(REGION_TIME))
	{
		VIDEO_OSD_ERROR("hi_region_destroy failed .");
		return VIDEO_ERROR;
	}

	return VIDEO_OK;
}
#endif

HB_S32 video_osd_init()
{
	FT_Library library;
	FT_Face face0, face1;
	FT_Error error;
	iconv_t iconv_fd;

	//hisiv200 glibc use
	//HB_CHAR *locale = setlocale(LC_ALL, "zh_CN.GBK");
	//VIDEO_OSD_DEBUG("Locale : %s .\n", locale);

	iconv_fd = iconv_open("UTF-16LE", "GBK");
	if ((iconv_t)-1 == iconv_fd )
	{
		perror("iconv_open error.");
		return -1;
	}
	hi_sys.osd_cfg.iconv_fd = iconv_fd;

	//step 1 : init freetype2 lib
	error = FT_Init_FreeType(&library);
	if (error)
	{
		VIDEO_OSD_ERROR("FT_Init_FreeType failed with %d .\n", error);
		return VIDEO_ERROR;
	}

	//step 2 : fill face information
	error = FT_New_Face(library, FONT_FILE, 0, &face0);
	if (FT_Err_Unknown_File_Format == error)
	{
		VIDEO_OSD_ERROR("Font file not supported .\n");
		return VIDEO_ERROR;
	}
	else if (error)
	{
		VIDEO_OSD_ERROR("Error! Could not open font file (error code %d) .\n", error);
		return VIDEO_ERROR;
	}

	error = FT_New_Face(library, FONT_FILE, 0, &face1);
	if (FT_Err_Unknown_File_Format == error)
	{
		VIDEO_OSD_ERROR("Font file not supported .\n");
		return VIDEO_ERROR;
	}
	else if (error)
	{
		VIDEO_OSD_ERROR("Error! Could not open font file (error code %d) .\n", error);
		return VIDEO_ERROR;
	}

	hi_sys.osd_cfg.freetype.library = library;
	hi_sys.osd_cfg.freetype.face[0] = face0;
	hi_sys.osd_cfg.freetype.face[1] = face1;
	pthread_mutex_init(&hi_sys.t_lock, NULL);

	return VIDEO_OK;
}

HB_S32 video_osd_deinit()
{	
	iconv_t iconv_fd = hi_sys.osd_cfg.iconv_fd;
	iconv_close(iconv_fd);

	pthread_mutex_destroy(&hi_sys.t_lock);

	FT_Done_Face(hi_sys.osd_cfg.freetype.face[1]);
	hi_sys.osd_cfg.freetype.face[1] = NULL;

	FT_Done_Face(hi_sys.osd_cfg.freetype.face[0]);
	hi_sys.osd_cfg.freetype.face[0] = NULL;

	FT_Done_FreeType(hi_sys.osd_cfg.freetype.library);
	hi_sys.osd_cfg.freetype.library = NULL;

	return VIDEO_OK;
}

HB_VOID str2wstr(wchar_t *dst, HB_CHAR *src)
{
	HB_S32 count, i;
	HB_CHAR tmp[5] = "";

	count = hb_mbstowcs(dst, src, 2 * strlen(src));

	//debug only
	/*
	   for (i = 0; i < wcslen(dst); i++)
	   {
	   printf("%04x ", GET16(&dst[i]));
	   }
	   printf("\n");
	   */

	while(-1 == count)
	{
		for (i = 0; i < strlen(src); i++)
		{
			if (src[i] <= 0x80)
			{
				memset(tmp, 0, sizeof(tmp));
				tmp[0] = src[i];
				count = hb_mbstowcs(dst, tmp, 2 * strlen(tmp));
				if (-1 == count)
				{
					src[i] = '?';
				}
			}
			else
			{
				tmp[0] = src[i];
				tmp[1] = src[i + 1];
				count = hb_mbstowcs(dst, tmp, 2 * strlen(tmp));
				if (count < 0)
				{
					src[i] = '?';
					src[i + 1] = '?';
				}
				i++;
			}
		}
		count = hb_mbstowcs(dst, src, 2 * strlen(src));
	}
}

HB_S32 get_font_size(HB_S32 stream_id, HB_S32 *size)
{
	HB_S32 width, height;

	width = hi_sys.stream_cfg[stream_id].resolution.width;
	height = hi_sys.stream_cfg[stream_id].resolution.height;
	//printf("streamID %d (%dx%d)\n", stream_id, hi_sys.stream_cfg[stream_id].resolution.width, hi_sys.stream_cfg[stream_id].resolution.height);

	if ((1280 == width) && (960 == height))
	{
		*size = 32;
	}
	else if ((1280 == width) && (720 == height))
	{
		*size = 32;
	}
	else if ((768 == width) && (432 == height))
	{
		*size = 26;
	}
	else if ((720 == width) && (576 == height))
	{
		*size = 26;
	}
	else if ((704 == width) && (576 == height))
	{
		*size = 26;
	}
	else if ((640 == width) && (480 == height))
	{
		*size = 22;
	}
	else if ((640 == width) && (368 == height))
	{
		*size = 20;
	}
	else if ((352 == width) && (288 == height))
	{
		*size = 15;
	}
	else if ((320 == width) && (192 == height))
	{
		*size = 12;
	}
	else
	{
		VIDEO_OSD_ERROR("Unknown resolution %dx%d !\n", width, height);
	}

	return VIDEO_OK;
}

HB_S32 get_string_size(HB_S32 *width, HB_S32 *height, wchar_t *str, HB_S32 stream_id)
{
	FT_Face face; 
	FT_GlyphSlot slot;
	FT_UInt index;
	FT_Error error;
	HB_S32 n = 0;
	HB_S32 tmp = 0;
	HB_S32 font_size;

	if (NULL == str)
	{
		VIDEO_OSD_ERROR("NULL pointer .\n");
		return VIDEO_ERROR;
	}

	if (VIDEO_OK != get_font_size(stream_id, &font_size))
	{
		return VIDEO_ERROR;
	}

	face = hi_sys.osd_cfg.freetype.face[stream_id];

	pthread_mutex_lock(&hi_sys.t_lock);
	error = FT_Set_Pixel_Sizes(face, 0, font_size);
	if (error)
	{
		VIDEO_OSD_ERROR("set pixel size error !\n");
		return -1;
	}

	for(n = 0; n < wcslen(str); n++) 
	{
		index = FT_Get_Char_Index(face, str[n]);
#ifdef STROKE
        extern char *stroke_outline(FT_Face face, FT_Library library,
                int *width, int *height);
        FT_Library library;
        library = hi_sys.osd_cfg.freetype.library;
        int _width = 0;
        int _height = 0;
        char *buffer = NULL;

		FT_Load_Glyph(face,index, FT_LOAD_NO_BITMAP);
        buffer = stroke_outline(face, library, &_width, &_height);

		slot = face->glyph;
        if (_width == 0)
            _width = 16;

        tmp += _width;
        free(buffer);
#else
		FT_Load_Glyph(face,index, FT_LOAD_DEFAULT);
		FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);

		slot = face->glyph;
		if(slot->bitmap.width > (slot->advance.x>> 6))
		{
			tmp += slot->bitmap.width;
		}
		else
		{
			tmp += slot->advance.x>> 6;
		}
#endif
	}
	pthread_mutex_unlock(&hi_sys.t_lock);
	*width = tmp;
	*height = font_size;

	return VIDEO_OK;
}

HB_S32 hi_region_display(HB_S32 rgn_handle, HB_S32 stream_id, HISI_POS_OBJ pos, HB_BOOL inv_color)
{
	MPP_CHN_S chn;
	RGN_CHN_ATTR_S chn_attr;
	HB_S32 ret;

	memset(&chn_attr, 0, sizeof(RGN_CHN_ATTR_S));
	memset(&chn, 0, sizeof(MPP_CHN_S));

	chn.enModId = HI_ID_GROUP;
	chn.s32DevId = hi_sys.stream_cfg[stream_id].venc_grp;
	chn.s32ChnId = 0;

	chn_attr.bShow = HI_TRUE;
	chn_attr.enType = OVERLAY_RGN;
	chn_attr.unChnAttr.stOverlayChn.stPoint.s32X = ALIGN_N(pos.startx, 16);
	chn_attr.unChnAttr.stOverlayChn.stPoint.s32Y = ALIGN_N(pos.starty, 16);
	chn_attr.unChnAttr.stOverlayChn.u32BgAlpha = 0;
	chn_attr.unChnAttr.stOverlayChn.u32FgAlpha = 128;
	chn_attr.unChnAttr.stOverlayChn.u32Layer = rgn_handle;
	chn_attr.unChnAttr.stOverlayChn.stQpInfo.bAbsQp = HI_FALSE;
	chn_attr.unChnAttr.stOverlayChn.stQpInfo.s32Qp = 0;
	chn_attr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.u32Width = 16;
	chn_attr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.u32Height = 16;
	chn_attr.unChnAttr.stOverlayChn.stInvertColor.u32LumThresh = 128;
	chn_attr.unChnAttr.stOverlayChn.stInvertColor.enChgMod = MORETHAN_LUM_THRESH;
	chn_attr.unChnAttr.stOverlayChn.stInvertColor.bInvColEn = (HB_TRUE == inv_color) ? HI_TRUE : HI_FALSE;

	ret = HI_MPI_RGN_AttachToChn(rgn_handle, &chn, &chn_attr);
	if (HI_SUCCESS != ret)
	{
		VIDEO_OSD_ERROR("HI_MPI_RGN_AttachToChn failed with 0x%x .\n", ret);
		return VIDEO_ERROR;
	}

	return VIDEO_OK;
}

HB_S32 hi_set_text(HB_S32 stream_id, HISI_POS_OBJ config, HB_CHAR *text, HB_S32 enable, HB_BOOL inv_color)
{
	wchar_t w_str[64];
	BITMAP_S bitmap;
	HB_S32 width, height, ret;
	RGN_HANDLE rgn_handle;
	VENC_GRP venc_grp;

	venc_grp= hi_sys.stream_cfg[stream_id].venc_grp;
	rgn_handle = ((0 == stream_id) ? REGION_TEXT0 : REGION_TEXT1);

	str2wstr(w_str, text);
	if (VIDEO_OK != get_string_size(&width, &height, w_str, stream_id))
	{
		VIDEO_OSD_ERROR("get_string_size error .\n");
		return VIDEO_ERROR;
	}

	if (VIDEO_OK != hi_region_create(rgn_handle, width, height))
	{
		VIDEO_OSD_ERROR("hi_region_create failed .\n");
		return VIDEO_ERROR;
	}

	if (VIDEO_OK != hi_region_display(rgn_handle, stream_id, config, inv_color))
	{
		VIDEO_OSD_ERROR("hi_region_display failed .\n");
		return VIDEO_ERROR;
	}

	bitmap.u32Width = width;
	bitmap.u32Height = height;
	if (VIDEO_OK != fill_bitmap(stream_id, &bitmap, w_str))
	{
		VIDEO_OSD_ERROR("fill_bitmap error .\n");
		return VIDEO_ERROR;
	}

	ret = HI_MPI_RGN_SetBitMap(rgn_handle, &bitmap);
	if(ret != HI_SUCCESS)
	{
		VIDEO_OSD_ERROR("HI_MPI_RGN_SetBitMap failed with %#x !\n", ret);
		return VIDEO_ERROR;
	}

	if (NULL != bitmap.pData)
	{
		free(bitmap.pData);
		bitmap.pData = NULL;
	}

	ret = RGN_ShowOrHide(rgn_handle, venc_grp, enable);
	if (HI_SUCCESS != ret)
	{
		VIDEO_OSD_ERROR("RGN_ShowOrHide failed with 0x%x .\n", ret);
		return VIDEO_ERROR;
	}

	return VIDEO_OK;
}

HI_S32 change_position(RGN_HANDLE RgnHandle, VENC_GRP VencGrp, POINT_S *pstPoint)
{
	MPP_CHN_S stChn;
	RGN_CHN_ATTR_S stChnAttr;
	HI_S32 s32Ret;

	stChn.enModId = HI_ID_GROUP;
	stChn.s32DevId = VencGrp;
	stChn.s32ChnId = 0;

	if (NULL == pstPoint)
	{
		SAMPLE_PRT("input parameter is null. it is invaild!\n");
		return HI_FAILURE;
	}

	s32Ret = HI_MPI_RGN_GetDisplayAttr(RgnHandle, &stChn, &stChnAttr);
	if(HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("HI_MPI_RGN_GetDisplayAttr (%d)) failed with %#x!\n",\
				RgnHandle, s32Ret);
		return HI_FAILURE;
	}

	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = ALIGN_N(pstPoint->s32X, 16);
	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = ALIGN_N(pstPoint->s32Y, 16);
	s32Ret = HI_MPI_RGN_SetDisplayAttr(RgnHandle,&stChn,&stChnAttr);
	if(HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("HI_MPI_RGN_SetDisplayAttr (%d)) failed with %#x!\n",\
				RgnHandle, s32Ret);
		return HI_FAILURE;
	}

	return HI_SUCCESS;
}


HB_S32 hi_set_time(HB_S32 stream_id, HISI_POS_OBJ config, HB_S32 enable, HB_BOOL inv_color)
{
	HB_S32 ret;
	HB_S32 width, height;
	HB_S32 week_display = 0;
	VENC_GRP venc_grp;
	RGN_HANDLE rgn_handle;

	venc_grp= hi_sys.stream_cfg[stream_id].venc_grp;
	rgn_handle = ((0 == stream_id) ? REGION_TIME0 : REGION_TIME1);

	week_display = (2 == enable) ? 1 : 0;
	enable = (0 == enable) ? 0 : 1;

	if (VIDEO_OK != get_osd_time_size(0, stream_id, &width, &height, week_display))
	{
		VIDEO_OSD_ERROR("get_osd_time_size(%d) error .\n", stream_id);
		return VIDEO_ERROR;
	}

	venc_grp = hi_sys.stream_cfg[stream_id].venc_grp;

	if (VIDEO_OK != hi_region_create(rgn_handle, width, height))
	{
		VIDEO_OSD_ERROR("hi_region_create failed .\n");
		return VIDEO_ERROR;
	}

	if (VIDEO_OK != hi_region_display(rgn_handle, stream_id, config, inv_color))
	{
		VIDEO_OSD_ERROR("hi_region_display failed .\n");
		return VIDEO_ERROR;
	}

	ret = RGN_ShowOrHide(rgn_handle, venc_grp, enable);
	if (HI_SUCCESS != ret)
	{
		VIDEO_OSD_ERROR("RGN_ShowOrHide failed with 0x%x .\n", ret);
		return VIDEO_ERROR;
	}

	return VIDEO_OK;
}

HB_S32 set_osd(HB_S32 vin_id, HB_S32 stream_id, OSD_PRI_OBJ	config)
{
	VALUE_LIST_OBJ list;
	HISI_POS_OBJ pos;
	HB_S32 ret;

	if (0 != vin_id)
	{
		VIDEO_OSD_ERROR("Error vin id for set_osd .\n");
		return VIDEO_ERROR;
	}
	if ((0 != stream_id) && (1 != stream_id))
	{
		VIDEO_OSD_ERROR("Error stream id for set_osd .\n");
		return VIDEO_ERROR;
	}

	hi_sys.osd_cfg.transparent = (1 == config.transparent) ? HB_TRUE : HB_FALSE;

	get_osd_color_list(vin_id, &list);
	if (config.color_index > list.list_len)
	{
		VIDEO_OSD_ERROR("color not support !\n");
		return VIDEO_ERROR;
	}
	hi_sys.osd_cfg.color = list.value[config.color_index];

	pos.startx = config.area[1].startx;
	pos.starty = config.area[1].starty;
	if (0 == strlen(config.area[1].text))
	{
		strcpy(config.area[1].text, " ");
	}
	ret = hi_set_text(stream_id, pos, config.area[1].text, config.area[1].enable, config.inv_color);
	if (VIDEO_OK != ret)
	{
		VIDEO_OSD_ERROR("hi_set_text error !\n");
		return VIDEO_ERROR;
	}

	pos.startx = config.area[0].startx;
	pos.starty = config.area[0].starty;
	ret = hi_set_time(stream_id, pos, config.area[0].enable, config.inv_color);
	if (VIDEO_OK != ret)
	{
		VIDEO_OSD_ERROR("hi_set_time error !\n");
		return VIDEO_ERROR;
	}

	return VIDEO_OK;
}

HB_S32 get_osd(HB_S32 vin_id, HB_S32 stream_id, OSD_PRI_HANDLE pconfig)
{
	return VIDEO_ERR_GET_UNSUPPORTED;
}

HB_S32 get_osd_color_list(HB_S32 vin_id, VALUE_LIST_HANDLE plist)
{
#ifdef DEBUG
	assert(NULL != plist);
#endif
	static HB_S32 color_table[] = 
	{
		COLOR_WHITE, 
		COLOR_BLACK,
		COLOR_RED,
		COLOR_GREEN,
		COLOR_BLUE
	};

	if (NULL == plist)
	{
		VID_ERROR("get_osd_color_list error .\n");
		return VIDEO_ERROR;
	}

	HB_S32 i = 0;

	plist->list_len = sizeof(color_table) / sizeof(color_table[0]);
	for (i = 0; i < plist->list_len; i++)
	{
		plist->value[i] = color_table[i];
	}

	return VIDEO_OK;
}

HB_S32 time_update(HB_CHAR *time)
{
	RGN_HANDLE rgn_handle;
	wchar_t w_str[32];
	BITMAP_S bitmap;
	HB_S32 width, height;
	HB_S32 ret = VIDEO_ERROR;
	HB_S32 i;

	str2wstr(w_str, time);
	for (i = 0; i < hi_sys.stream_count; i++)
	{
		if (VIDEO_OK != get_string_size(&width, &height, w_str, i))
		{
			VIDEO_OSD_ERROR("get_string_size error .\n");
			return VIDEO_ERROR;
		}

		bitmap.u32Width = width;
		bitmap.u32Height = height;
		if (VIDEO_OK != fill_bitmap(i, &bitmap, w_str))
		{
			VIDEO_OSD_ERROR("fill_bitmap error .\n");
			return VIDEO_ERROR;
		}

		rgn_handle = ((0 == i) ? REGION_TIME0 : REGION_TIME1);
		ret = HI_MPI_RGN_SetBitMap(rgn_handle, &bitmap);
		if(ret != HI_SUCCESS)
		{
			//VIDEO_OSD_ERROR("HI_MPI_RGN_SetBitMap failed with %#x !\n", ret);
			return VIDEO_ERROR;
		}

		if (NULL != bitmap.pData)
		{
			free(bitmap.pData);
			bitmap.pData = NULL;
		}
	}

	return VIDEO_OK;
}

HB_S32 get_osd_string_size(HB_S32 vin_id, HB_S32 stream_id, HB_CHAR *string, HB_S32 *width, HB_S32 *height)
{
	if ((0 != stream_id) && (1 != stream_id))
	{
		return VIDEO_ERROR;
	}

	wchar_t w_str[32];
	str2wstr(w_str, string);
	if (VIDEO_OK != get_string_size(width, height, w_str, stream_id))
	{
		VIDEO_OSD_ERROR("get_string_size error .\n");
		return VIDEO_ERROR;
	}
	*width += 16;
	*height += 16;

	return VIDEO_OK;
}

HB_S32 get_osd_time_size(HB_S32 vin_id, HB_S32 stream_id, HB_S32 *width, HB_S32 *height, HB_S32 week_display)
{
	HB_S32 font_size;

	if ((0 != stream_id) && (1 != stream_id))
	{
		return VIDEO_ERROR;
	}

	if (VIDEO_OK != get_font_size(stream_id, &font_size))
	{
		return VIDEO_ERROR;
	}

	switch (font_size)
	{
		case 32:
			*width = (0 == week_display) ? 316 : 420;
			break;
		case 26:
			*width = (0 == week_display) ? 256 : 350;
			break;
		case 22:
			*width = (0 == week_display) ? 212 : 320;
			break;
		case 20:
			*width = (0 == week_display) ? 205 : 275;
			break;
		case 15:
			*width = (0 == week_display) ? 162 : 220;
			break;
		case 12:
			*width = (0 == week_display) ? 128 : 170;
			break;
		default :
			return VIDEO_ERROR;
	}
	*height = font_size + 16;

	return VIDEO_OK;
}
