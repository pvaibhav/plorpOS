/* SPDX-License-Identifier: MIT */
/* See look.h. */
#include "look.h"

#include <stdio.h>
#include <string.h>

static const char *const LABEL[LOOK_COUNT]  = { "Clean", "Retro", "Retro with Bezels" };
static const char *const STORED[LOOK_COUNT] = { "None",  "Retro", "Retro with Bezels" };

const char *look_label(look_value v)
{
	return LABEL[(unsigned)v < LOOK_COUNT ? v : LOOK_CLEAN];
}

const char *look_stored(look_value v)
{
	return STORED[(unsigned)v < LOOK_COUNT ? v : LOOK_CLEAN];
}

look_value look_find(const sl_list *l, const char *name)
{
	if (!name) return LOOK_CLEAN;
	if (!strcmp(name, STORED[LOOK_RETRO]))        return LOOK_RETRO;
	if (!strcmp(name, STORED[LOOK_RETRO_BEZELS])) return LOOK_RETRO_BEZELS;
	return sl_find(l, name) ? LOOK_RETRO : LOOK_CLEAN;
}

look_screen look_screen_of(const char *tag, int cgb)
{
	if (!strcmp(tag, "GB") || !strcmp(tag, "GBC")) return cgb ? SCREEN_GBC : SCREEN_GB;
	if (!strcmp(tag, "GBA")) return SCREEN_GBA;
	return SCREEN_NONE;
}

const char *look_colour_shelf(const char *tag)
{
	switch (look_screen_of(tag, 1)) {
	case SCREEN_GBC: return "GBC";
	case SCREEN_GBA: return "GBA";
	default:         return NULL;
	}
}

look_plan look_plan_of(look_value was, look_value now, look_screen s, bool corrected, bool confirmed)
{
	look_plan p = { false, false, LOOK_CORE_KEEP };
	bool leaving = was == LOOK_CLEAN && now != LOOK_CLEAN, back = was != LOOK_CLEAN && now == LOOK_CLEAN;

	if (!confirmed || was == now || s == SCREEN_NONE) return p;
	p.keep = true;
	if (s == SCREEN_GB) p.core = leaving ? LOOK_CORE_GREY : back ? LOOK_CORE_OWN : LOOK_CORE_KEEP;
	/* Only on leaving Clean: the player may turn correction off under Retro, and that choice stays. */
	else p.correct = leaving && !corrected;
	return p;
}

static const char *const GBSCREEN[GBSCREEN_COUNT] = { "DMG", "Pocket", "Light" };

const char *gbscreen_label(int i)
{
	return GBSCREEN[i >= 0 && i < GBSCREEN_COUNT ? i : 0];
}

int gbscreen_find(const char *label)
{
	for (int i = 0; label && i < GBSCREEN_COUNT; i++)
		if (!strcmp(GBSCREEN[i], label)) return i;
	return 0;
}

bool look_fields(look_value v, look_screen s, int gbscreen, const char *dir, char *out, size_t cap)
{
	/* The Light's lens is the Pocket's: the two are the same body. */
	static const char *const GB_KIND[GBSCREEN_COUNT] = { "dots-dmg", "dots-pocket", "dots-light" };
	static const char *const GB_LENS[GBSCREEN_COUNT] = { "dmg", "pocket", "pocket" };
	const char *kind, *lens;
	int n;

	if (gbscreen < 0 || gbscreen >= GBSCREEN_COUNT) gbscreen = 0;
	switch (s) {
	case SCREEN_GB:  kind = GB_KIND[gbscreen]; lens = GB_LENS[gbscreen]; break;
	case SCREEN_GBC: kind = "grid-gbc"; lens = "gbc"; break;
	case SCREEN_GBA: kind = "grid-gba"; lens = "gba"; break;
	default:         kind = NULL; lens = NULL; break;
	}
	if (v == LOOK_CLEAN || !kind) n = snprintf(out, cap, "shader=none");
	else if (v == LOOK_RETRO)
		n = snprintf(out, cap, "shader=%s/flipfx-%s.glsl:nearest:1\tfinal=nearest", dir, kind);
	else
		n = snprintf(out, cap, "shader=%s/flipfx-%s.glsl:nearest:1,%s/flipfx-bezel-%s.glsl:nearest:1\tfinal=nearest",
		             dir, kind, dir, lens);
	return n > 0 && (size_t)n < cap;
}
