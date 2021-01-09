#ifndef RESOURCE_H
#define RESOURCE_H
#include "gfx.h"

enum resource_id
{
  RES_FONT_PRESSSTART2P,
  RES_ICON_CHECK,
  RES_ICON_BUTTONS,
  RES_ICON_PAUSE,
  RES_ICON_MACRO,
  RES_ICON_MOVIE,
  RES_ICON_ARROW,
  RES_ICON_FILE,
  RES_ICON_SAVE,
  RES_TEXTURE_CROSSHAIR,
  RES_MAX,
};

void               *resource_get(enum resource_id res);
void                resource_free(enum resource_id res);
struct gfx_texture *resource_load_grc_texture(const char *grc_resource_name);

#endif