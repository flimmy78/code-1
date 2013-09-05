#include "video_osd_private.h"
#include <freetype/ftstroke.h>
#include "hi3518/HISI_private.h"

#ifndef MIN
#define MIN(x,y)		({	\
		typeof(x) _x = (x); \
		typeof(y) _y = (y); \
		(void) (&_x == &_y);\
		_x < _y ? _x : _y; })
#endif

#ifndef MAX
#define MAX(x,y)		({	\
		typeof(x) _x = (x); \
		typeof(y) _y = (y); \
		(void) (&_x == &_y);\
		_x > _y ? _x : _y; })
#endif
#define WIDTH   128
#define HEIGHT  96
unsigned char image[HEIGHT][WIDTH];
struct _FT_Bitmap{
    int width;
    int rows;
    char *buffer;
};
typedef struct stroke_pixel{
    int pixel_background;
    int pixel_outline;
    int pixel_font;
} pixel_type_t;

static pixel_type_t default_pixel_type = {
		.pixel_background = 0,
		.pixel_outline = 1,
		.pixel_font = 255
};
pixel_type_t *p_pixel_type;

typedef struct span_s {
	int x;
	int y;
	int width;
	int coverage;
} span_t;

typedef struct spans_s {
	span_t span;
	struct spans_s* next;
} spans_t;

typedef struct rect_s {
	int xmin;
	int xmax;
	int ymin;
	int ymax;
} rect_t;

typedef struct _tagBITMAP_CFG
{
	HB_U32 width;
	HB_U32 height;
	HB_U16 *pbuff;
}BITMAP_CFG_OBJ, *BITMAP_CFG_HANDLE;
static inline void rect_include(rect_t *rect, int a, int b)
{
	rect->xmin = MIN(rect->xmin, a);
	rect->ymin = MIN(rect->ymin, b);
	rect->xmax = MAX(rect->xmax, a);
	rect->ymax = MAX(rect->ymax, b);
}

void raster_callback(const int y, const int count, const FT_Span *spans,
        void* const user)
{
	int i;
	spans_t *sptr = (spans_t *)user, *sp = NULL;
	while (sptr->next != NULL) {
		sptr = sptr->next;
	}
	for (i = 0; i < count; i++) {
		sp = (spans_t *)malloc(sizeof(spans_t));
		sp->span.x = spans[i].x;
		sp->span.y = y;
		sp->span.width = spans[i].len;
		sp->span.coverage = spans[i].coverage;
		sp->next = NULL;

		sptr->next = sp;
		sptr = sptr->next;
	}
}

static void render_spans(FT_Library *library, FT_Outline *outline,
        spans_t *spans)
{
	FT_Raster_Params params;
	memset(&params, 0, sizeof(params));
	params.flags = FT_RASTER_FLAG_AA | FT_RASTER_FLAG_DIRECT;
	params.gray_spans = raster_callback;
	params.user = spans;

	FT_Outline_Render(*library, outline, &params);
}

static void free_spans(spans_t *spans)
{
	spans_t *iter = spans, *tmp = NULL;
	while (iter != NULL) {
		tmp = iter;
		iter = iter->next;
		free(tmp);
	}
}

char *stroke_outline(FT_Face face, FT_Library library, int* width, int* height)
{
	FT_GlyphSlot slot = face->glyph;
	char *pixel = NULL;
	spans_t *spans = NULL, *outlinespans = NULL;

	spans = (spans_t *)malloc(sizeof(spans_t));
	if (spans == NULL) {
		printf("spans malloc error\n");
		return NULL;
	}
	spans->span.x = 0;
	spans->span.y = 0;
	spans->span.width = 0;
	spans->span.coverage = 0;
	spans->next = NULL;
	render_spans(&library, &(slot->outline), spans);

	// spans for the outline
	outlinespans = (spans_t *)malloc(sizeof(spans_t));
	if (outlinespans == NULL) {
		printf("outlinespans malloc error\n");
		return NULL;
	}
	outlinespans->span.x = 0;
	outlinespans->span.y = 0;
	outlinespans->span.width = 0;
	outlinespans->span.coverage = 0;
	outlinespans->next = NULL;

	// Set up a stroker
	FT_Stroker stroker;
	FT_Stroker_New(library, &stroker);
	FT_Stroker_Set(stroker,
			32,
			FT_STROKER_LINECAP_ROUND,
			FT_STROKER_LINEJOIN_ROUND,
			0);

	FT_Glyph glyph;
	if (FT_Get_Glyph(slot, &glyph) == 0) {
		FT_Glyph_StrokeBorder(&glyph, stroker, 0, 1);   // outside border. Destroyed on success.
		if (glyph->format == FT_GLYPH_FORMAT_OUTLINE) {
			// Render the outline spans to the span list
			FT_Outline *ol = &(((FT_OutlineGlyph)glyph)->outline);
			render_spans(&library, ol, outlinespans);
		}

		// Clean up afterwards.
		FT_Stroker_Done(stroker);
		FT_Done_Glyph(glyph);

		// Put them together
		if (outlinespans->next != NULL) {
			rect_t rect;
			rect.xmin = spans->span.x;
			rect.xmax = spans->span.x;
			rect.ymin = spans->span.y;
			rect.ymax = spans->span.y;

			spans_t *iter = NULL;
			for (iter = spans; iter != NULL; iter = iter->next) {
				rect_include(&rect, iter->span.x, iter->span.y);
				rect_include(&rect, iter->span.x+iter->span.width-1,
                        iter->span.y);
			}

			for (iter = outlinespans; iter != NULL; iter = iter->next) {
				rect_include(&rect, iter->span.x, iter->span.y);
				rect_include(&rect, iter->span.x+iter->span.width-1,
                        iter->span.y);
			}
			int pwidth = rect.xmax - rect.xmin + 1;
			int pheight = rect.ymax - rect.ymin + 1;
			int w = 0;
			pixel = (char *)malloc(pwidth * pheight * sizeof(char));
			if (pixel == NULL) {
				printf("Mem alloc error. Strings may be too long.\n");
				return NULL;
			}
            p_pixel_type = &default_pixel_type;
			memset(pixel, p_pixel_type->pixel_background, pwidth * pheight *sizeof(char));

			for (iter = outlinespans; iter != NULL; iter = iter->next) {
				for (w = 0; w < (iter->span.width); w++)
					pixel[(int)((pheight-1-(iter->span.y-rect.ymin)) * pwidth + iter->span.x-rect.xmin+w)] = p_pixel_type->pixel_outline;
			}
			for (iter = spans; iter != NULL; iter = iter->next) {
				for (w = 0; w < (iter->span.width); w++)
					pixel[(int)((pheight-1-(iter->span.y-rect.ymin)) * pwidth + iter->span.x-rect.xmin+w)] = MAX(iter->span.coverage, p_pixel_type->pixel_outline+1) ;
			}

			*width = pwidth;
			*height = pheight;
		}
	}
	free_spans(outlinespans);
	free_spans(spans);

	return pixel;
}
void show_image(void)
{
  int  i, j;
  for ( i = 0; i < HEIGHT; i++ )
  {
      for ( j = 0; j < WIDTH; j++ )
          putchar( image[i][j] == 0 ? ' '
                  : image[i][j] < 128 ? '+'
                  : ' ' );
    putchar( '\n' );
  }
}

void draw_bitmap(struct _FT_Bitmap  *bitmap, FT_Int x, FT_Int y)
{
  FT_Int  i, j, p, q;
  FT_Int  x_max = x + bitmap->width;
  FT_Int  y_max = y + bitmap->rows;


  for ( i = x, q = 0; i < y_max; i++, q++ )
  {
    for ( j = y, p = 0; j < x_max; j++, p++ )
    {
      if ( i < 0      || j < 0       ||
           j >= WIDTH || i >= HEIGHT )
        continue;

      image[i][j] |= bitmap->buffer[q * bitmap->width + p];
    }
  }
}

