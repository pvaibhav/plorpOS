/* SPDX-License-Identifier: MIT */
/* The Flip's Look row (Game Boy, Game Boy Color and Game Boy Advance games): its values, the screen a game has,
 * and the SETDISPLAY shader fields that ask diatom for them. diatom's Flip port reads only each pass's file name,
 * as it does for flipfx-scanlines, so no .glsl file exists. Split out of main.c under ADR-0001 so
 * tools/look-check.c drives it with no SDL. */
#ifndef TORTOS_LOOK_H
#define TORTOS_LOOK_H

#include <stdbool.h>
#include <stddef.h>

#include "shaderlist.h"

typedef enum { LOOK_CLEAN, LOOK_RETRO, LOOK_RETRO_BEZELS } look_value;
#define LOOK_COUNT 3

/* Decided by the game, as the Palette row's list is: a Color game on the Game Boy shelf is SCREEN_GBC. */
typedef enum { SCREEN_NONE, SCREEN_GB, SCREEN_GBC, SCREEN_GBA } look_screen;

/* The row's label: Clean, Retro, Retro with Bezels. */
const char *look_label(look_value v);

/* What shader.<TAG> and shader.<TAG>.<file> store. Clean is the list's None, as Off is. */
const char *look_stored(look_value v);

/* The value `name` stores. A name the scanline list `l` has (its On) reads as Retro; NULL, "", None or anything
 * else as Clean. */
look_value look_find(const sl_list *l, const char *name);

/* The screen of a game on the shelf `tag`, `cgb` saying whether its ROM is a Game Boy Color game
 * (gbpal_rom_is_cgb). SCREEN_NONE for every shelf but GB, GBC and GBA. */
look_screen look_screen_of(const char *tag, int cgb);

/* A monochrome game's screen under Retro, stored as screen.<TAG>.<file>: DMG (the default), Pocket, Light. */
#define GBSCREEN_COUNT 3
const char *gbscreen_label(int i);
int gbscreen_find(const char *label);   /* 0 (DMG) for NULL or a label not listed */

/* The shelf whose Default Palette the colour games on shelf `tag` follow: a Color game on the Game Boy shelf follows
 * the Color shelf's, as the Palette row has it. NULL for a shelf with no Look. */
const char *look_colour_shelf(const char *tag);

/* What follows a change of Look from `was` to `now`. In a game's menu, once diatom has answered the SETDISPLAY for
 * `now` (`confirmed`); a refused look changes nothing. For a shelf's Default Look, which nothing confirms, `confirmed`
 * is true and `s` is the screen of the colour games that follow it (look_screen_of(tag, 1)). `corrected` says whether
 * the Palette in question, the game's or the shelf's Default Palette, is Colour Corrected already. */
typedef enum { LOOK_CORE_KEEP, LOOK_CORE_GREY, LOOK_CORE_OWN } look_core;
typedef struct {
	bool      keep;      /* store `now` */
	bool      correct;   /* store that Palette as Colour Corrected, and send it to a running core */
	look_core core;      /* a monochrome game's core palette: as it is, plain grey, or its own again */
} look_plan;
look_plan look_plan_of(look_value was, look_value now, look_screen s, bool corrected, bool confirmed);

/* "shader=none" for Clean or SCREEN_NONE. Else "shader=<dir>/flipfx-<kind>.glsl:nearest:1\tfinal=nearest", and
 * under Retro with Bezels a second pass, <dir>/flipfx-bezel-<lens>.glsl:nearest:1, before the final. `gbscreen`
 * picks the kind of a SCREEN_GB game. Ready to follow "SETDISPLAY\t"; false if it does not fit. */
bool look_fields(look_value v, look_screen s, int gbscreen, const char *dir, char *out, size_t cap);

#endif
