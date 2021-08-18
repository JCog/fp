#include <stdlib.h>
#include <grc.h>
#include "gfx.h"
#include "gu.h"
#include "resource.h"
#include "pm64.h"

/* resource data table */
static void *res_data[RES_MAX] = {NULL};

/* resource constructors */
static void *rc_font_generic(struct gfx_texdesc *texdesc,
                             int char_width, int char_height,
                             int code_start,
                             int letter_spacing, int line_spacing,
                             int baseline, int median, int x)
{
  struct gfx_font *font = malloc(sizeof(*font));
  if (!font)
    return font;
  struct gfx_texture *texture = gfx_texture_load(texdesc, NULL);
  if (!texture) {
    free(font);
    return NULL;
  }
  font->texture = texture;
  font->char_width = char_width;
  font->char_height = char_height;
  font->chars_xtile = texture->tile_width / font->char_width;
  font->chars_ytile = texture->tile_height / font->char_height;
  font->code_start = code_start;
  font->letter_spacing = letter_spacing;
  font->line_spacing = line_spacing;
  font->baseline = baseline;
  font->median = median;
  font->x = x;
  return font;
}

static void *rc_grc_font_generic(const char *grc_resource_name,
                                 int char_width, int char_height,
                                 int code_start,
                                 int letter_spacing, int line_spacing,
                                 int baseline, int median, int x)
{
  void *p_t;
  grc_resource_get(grc_resource_name, &p_t, NULL);
  if (!p_t)
    return p_t;
  struct grc_texture *t = p_t;
  struct gfx_texdesc td =
  {
    t->im_fmt, t->im_siz, (uint32_t)&t->texture_data,
    t->tile_width, t->tile_height, t->tiles_x, t->tiles_y,
    GFX_FILE_DRAM, 0,
  };
  return rc_font_generic(&td, char_width, char_height, code_start,
                         letter_spacing, line_spacing, baseline, median, x);
}

static void *rc_font_fipps(void)
{
    return rc_grc_font_generic("fipps", 10, 14, 33, -2, -5, 10, 3, 2);
}

static void *rc_font_notalot35(void)
{
    return rc_grc_font_generic("notalot35", 8, 9, 33, -1, -1, 7, 2, 2);
}

static void *rc_font_origamimommy(void)
{
    return rc_grc_font_generic("origamimommy", 8, 10, 33, -2, -2, 8, 1, 0);
}

static void *rc_font_pcsenior(void)
{
    return rc_grc_font_generic("pcsenior", 8, 8, 33, 0, 0, 7, 2, 0);
}

static void *rc_font_pixelintv(void)
{
    return rc_grc_font_generic("pixelintv", 8, 12, 33, 0, -4, 10, 5, 1);
}

static void *rc_font_pressstart2p(void)
{
    return rc_grc_font_generic("pressstart2p", 8, 8, 33, 0, 0, 7, 2, 0);
}

static void *rc_font_smwtextnc(void)
{
    return rc_grc_font_generic("smwtextnc", 12, 8, 33, -4, 0, 7, 2, 3);
}

static void *rc_font_werdnasreturn(void)
{
    return rc_grc_font_generic("werdnasreturn", 8, 12, 33, 0, -4, 11, 6, 1);
}

static void *rc_font_pixelzim(void)
{
    return rc_grc_font_generic("pixelzim", 3, 6, 33, 1, 0, 5, 3, 0);
}

static void *rc_icon_check(void)
{
  return resource_load_grc_texture("check_icons");
}

static void *rc_icon_buttons(void)
{
  return resource_load_grc_texture("button_icons");
}

static void *rc_icon_pause(void)
{
  return resource_load_grc_texture("pause_icons");
}

static void *rc_icon_macro(void)
{
  return resource_load_grc_texture("macro_icons");
}

static void *rc_icon_movie(void)
{
  return resource_load_grc_texture("movie_icons");
}

static void *rc_icon_arrow(void)
{
  return resource_load_grc_texture("arrow_icons");
}

static void *rc_icon_file(void)
{
  return resource_load_grc_texture("file_icons");
}

static void *rc_icon_save(void)
{
  return resource_load_grc_texture("save_icons");
}

static void *rc_texture_crosshair(void)
{
  return resource_load_grc_texture("crosshair");
}

/* resource destructors */
static void rd_font_generic(void *data)
{
  struct gfx_font *font = data;
  gfx_texture_free(font->texture);
  free(font);
}

/* resource management tables */
static void *(*res_ctor[RES_MAX])(void) =
{
  rc_font_fipps,
  rc_font_notalot35,
  rc_font_origamimommy,
  rc_font_pcsenior,
  rc_font_pixelintv,
  rc_font_pressstart2p,
  rc_font_smwtextnc,
  rc_font_werdnasreturn,
  rc_font_pixelzim,
  rc_icon_check,
  rc_icon_buttons,
  rc_icon_pause,
  rc_icon_macro,
  rc_icon_movie,
  rc_icon_arrow,
  rc_icon_file,
  rc_icon_save,
  rc_texture_crosshair,
};

static void (*res_dtor[RES_MAX])() =
{
  rd_font_generic,
  gfx_texture_free,
  gfx_texture_free,
  gfx_texture_free,
  gfx_texture_free,
  gfx_texture_free,
  gfx_texture_free,
  gfx_texture_free,
  gfx_texture_free,
  gfx_texture_free,
};

/* resource interface */
void *resource_get(enum resource_id res)
{
  if (!res_data[res])
    res_data[res] = res_ctor[res]();
  return res_data[res];
}

void resource_free(enum resource_id res)
{
  if (res_data[res]) {
    res_dtor[res](res_data[res]);
    res_data[res] = NULL;
  }
}

struct gfx_texture *resource_load_grc_texture(const char *grc_resource_name)
{
  void *p_t;
  grc_resource_get(grc_resource_name, &p_t, NULL);
  if (!p_t)
    return p_t;
  struct grc_texture *t = p_t;
  struct gfx_texdesc td =
  {
    t->im_fmt, t->im_siz, (uint32_t)&t->texture_data,
    t->tile_width, t->tile_height, t->tiles_x, t->tiles_y,
    GFX_FILE_DRAM, 0,
  };
  return gfx_texture_load(&td, NULL);
}
