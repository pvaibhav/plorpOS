/* SPDX-License-Identifier: MIT */
/* Does the Flip's Look row ask diatom for what its Flip port reads?
 *
 * Links src/look.c and src/shaderlist.c and NOT SDL. diatom reads only each
 * pass's file name, so every line is checked whole: a name diatom does not
 * know is refused with an error line and the game shows plain.
 */
#include "../src/look.h"

#include <stdio.h>
#include <string.h>

static int fails;

static void ck(int cond, const char *what)
{
	printf("  %s %s\n", cond ? "ok  " : "FAIL", what);
	if (!cond) fails++;
}

#define DIR "/mnt/SDCARD/TortOS/shaders"

static int fields_are(look_value v, look_screen s, int gbscreen, const char *want)
{
	char out[512];

	return look_fields(v, s, gbscreen, DIR, out, sizeof out) && !strcmp(out, want);
}

int main(void)
{
	sl_list l;
	char small[40];

	printf("values:\n");
	sl_load(&l, "sd/miyoo/shaders.cfg");
	ck(l.count == 2 && !strcmp(l.e[1].name, "On"), "the Flip's list is None and On");
	ck(!strcmp(look_label(LOOK_CLEAN), "Clean") && !strcmp(look_label(LOOK_RETRO), "Retro") &&
	   !strcmp(look_label(LOOK_RETRO_BEZELS), "Retro with Bezels"), "the row's three labels");
	ck(!strcmp(look_stored(LOOK_CLEAN), "None") && !strcmp(look_stored(LOOK_RETRO), "Retro") &&
	   !strcmp(look_stored(LOOK_RETRO_BEZELS), "Retro with Bezels"), "Clean stored as None, as Off is");
	for (int v = 0; v < LOOK_COUNT; v++)
		ck(look_find(&l, look_stored(v)) == (look_value)v, look_label(v));
	ck(look_find(&l, "On") == LOOK_RETRO, "a stored scanline filter reads as Retro");
	ck(look_find(&l, NULL) == LOOK_CLEAN && look_find(&l, "") == LOOK_CLEAN &&
	   look_find(&l, "Real LCD") == LOOK_CLEAN && look_find(&l, "Clean") == LOOK_CLEAN,
	   "nothing, or a name nobody stores, reads as Clean");

	printf("screens:\n");
	ck(look_screen_of("GB", 0) == SCREEN_GB && look_screen_of("GBC", 0) == SCREEN_GB, "a monochrome game on either shelf");
	ck(look_screen_of("GB", 1) == SCREEN_GBC && look_screen_of("GBC", 1) == SCREEN_GBC, "a Color game on either shelf");
	ck(look_screen_of("GBA", 0) == SCREEN_GBA, "an Advance game");
	ck(look_screen_of("NES", 0) == SCREEN_NONE && look_screen_of("SNES", 1) == SCREEN_NONE &&
	   look_screen_of("GG", 0) == SCREEN_NONE, "no other shelf");
	ck(!strcmp(gbscreen_label(0), "DMG") && !strcmp(gbscreen_label(1), "Pocket") && !strcmp(gbscreen_label(2), "Light"),
	   "the Game Boy's three screens");
	ck(gbscreen_find("Light") == 2 && gbscreen_find(NULL) == 0 && gbscreen_find("Grayscale") == 0, "DMG by default");

	printf("fields:\n");
	ck(fields_are(LOOK_CLEAN, SCREEN_GB, 0, "shader=none") && fields_are(LOOK_CLEAN, SCREEN_GBA, 0, "shader=none"),
	   "Clean is no chain");
	ck(fields_are(LOOK_RETRO, SCREEN_NONE, 0, "shader=none"), "nor is a game with no screen");
	ck(fields_are(LOOK_RETRO, SCREEN_GB, 0, "shader=" DIR "/flipfx-dots-dmg.glsl:nearest:1\tfinal=nearest"), "dots-dmg");
	ck(fields_are(LOOK_RETRO, SCREEN_GB, 1, "shader=" DIR "/flipfx-dots-pocket.glsl:nearest:1\tfinal=nearest"), "dots-pocket");
	ck(fields_are(LOOK_RETRO, SCREEN_GB, 2, "shader=" DIR "/flipfx-dots-light.glsl:nearest:1\tfinal=nearest"), "dots-light");
	ck(fields_are(LOOK_RETRO, SCREEN_GBC, 2, "shader=" DIR "/flipfx-grid-gbc.glsl:nearest:1\tfinal=nearest"),
	   "grid-gbc, whatever the screen choice says");
	ck(fields_are(LOOK_RETRO, SCREEN_GBA, 0, "shader=" DIR "/flipfx-grid-gba.glsl:nearest:1\tfinal=nearest"), "grid-gba");
	ck(fields_are(LOOK_RETRO_BEZELS, SCREEN_GB, 0, "shader=" DIR "/flipfx-dots-dmg.glsl:nearest:1,"
	              DIR "/flipfx-bezel-dmg.glsl:nearest:1\tfinal=nearest"), "dots-dmg in the dmg lens");
	ck(fields_are(LOOK_RETRO_BEZELS, SCREEN_GB, 1, "shader=" DIR "/flipfx-dots-pocket.glsl:nearest:1,"
	              DIR "/flipfx-bezel-pocket.glsl:nearest:1\tfinal=nearest"), "dots-pocket in the pocket lens");
	ck(fields_are(LOOK_RETRO_BEZELS, SCREEN_GB, 2, "shader=" DIR "/flipfx-dots-light.glsl:nearest:1,"
	              DIR "/flipfx-bezel-pocket.glsl:nearest:1\tfinal=nearest"), "dots-light in the pocket lens");
	ck(fields_are(LOOK_RETRO_BEZELS, SCREEN_GBC, 0, "shader=" DIR "/flipfx-grid-gbc.glsl:nearest:1,"
	              DIR "/flipfx-bezel-gbc.glsl:nearest:1\tfinal=nearest"), "grid-gbc in the gbc lens");
	ck(fields_are(LOOK_RETRO_BEZELS, SCREEN_GBA, 0, "shader=" DIR "/flipfx-grid-gba.glsl:nearest:1,"
	              DIR "/flipfx-bezel-gba.glsl:nearest:1\tfinal=nearest"), "grid-gba in the gba lens");
	ck(fields_are(LOOK_RETRO, SCREEN_GB, 7, "shader=" DIR "/flipfx-dots-dmg.glsl:nearest:1\tfinal=nearest"),
	   "a screen out of range is DMG");
	ck(!look_fields(LOOK_RETRO_BEZELS, SCREEN_GBA, 0, DIR, small, sizeof small), "false when it does not fit");

	printf(fails ? "%d FAILED\n" : "ALL PASS\n", fails);
	return fails != 0;
}
