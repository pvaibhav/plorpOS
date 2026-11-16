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

/* The menu's wait for diatom's answer to `fields` (plat_resident_sync_rect): the first line that answers it. */
static sl_answer answer_to(const char *fields, const char *const *lines)
{
	for (; *lines; lines++) {
		sl_answer a = sl_answer_of(fields, *lines);
		if (a != SL_ANSWER_NONE) return a;
	}
	return SL_ANSWER_NONE;
}

static int plan_is(look_plan p, int keep, int correct, look_core core)
{
	return p.keep == keep && p.correct == correct && p.core == core;
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

	printf("diatom's answer in a game's menu:\n");
	{
		char want[512], now[512], stale[1024], drawn[1024];
		const char *refused = "ERROR\tcode=bad_shader\tmsg=flipfx: unknown pass";

		look_fields(LOOK_RETRO_BEZELS, SCREEN_GB, 0, DIR, want, sizeof want);
		look_fields(LOOK_RETRO, SCREEN_GB, 0, DIR, now, sizeof now);
		snprintf(stale, sizeof stale, "DISPLAY\tmode=integer\tfilter=sharp\trect=480x432+80+24\t%s", now);
		snprintf(drawn, sizeof drawn, "DISPLAY\tmode=integer\tfilter=sharp\trect=480x432+136+72\t%s", want);
		ck(answer_to(want, (const char *[]){ drawn, NULL }) == SL_ANSWER_DRAWN, "a DISPLAY carrying the chain asked for draws it");
		ck(answer_to(want, (const char *[]){ refused, NULL }) == SL_ANSWER_REFUSED, "bad_shader refuses it");
		ck(answer_to(want, (const char *[]){ stale, refused, NULL }) == SL_ANSWER_REFUSED,
		   "an earlier step's DISPLAY, the old look in it, then bad_shader: refused, not kept");
		ck(answer_to(want, (const char *[]){ stale, "LEVEL\tkind=volume\tindex=3\tcount=20", drawn, NULL }) == SL_ANSWER_DRAWN,
		   "an earlier step's DISPLAY, then the one carrying the chain: drawn");
		ck(answer_to(want, (const char *[]){ stale, NULL }) == SL_ANSWER_NONE, "the earlier DISPLAY alone answers nothing");
		ck(answer_to("shader=none", (const char *[]){ stale, "DISPLAY\tmode=integer\tshader=none\tfinal=nearest", NULL }) ==
		   SL_ANSWER_DRAWN, "back to Clean: the DISPLAY saying none");
		ck(answer_to(want, (const char *[]){ "DISPLAY\tmode=integer\tshader=" DIR "/flipfx-dots-dmg.glsl:nearest:1\tfinal=nearest",
		                                     refused, NULL }) == SL_ANSWER_REFUSED,
		   "a chain that only begins as the one asked for is not it");
	}

	printf("transitions in a game's menu:\n");
	ck(plan_is(look_plan_of(LOOK_CLEAN, LOOK_RETRO, SCREEN_GB, false, true), 1, 0, LOOK_CORE_GREY),
	   "a monochrome game leaving Clean: kept, the core on plain grey");
	ck(plan_is(look_plan_of(LOOK_CLEAN, LOOK_RETRO_BEZELS, SCREEN_GB, false, true), 1, 0, LOOK_CORE_GREY),
	   "the same into Retro with Bezels");
	ck(plan_is(look_plan_of(LOOK_RETRO, LOOK_RETRO_BEZELS, SCREEN_GB, false, true), 1, 0, LOOK_CORE_KEEP),
	   "between the two Retro values the core stays grey");
	ck(plan_is(look_plan_of(LOOK_RETRO_BEZELS, LOOK_CLEAN, SCREEN_GB, false, true), 1, 0, LOOK_CORE_OWN),
	   "back to Clean: the game's own palette again");
	ck(plan_is(look_plan_of(LOOK_CLEAN, LOOK_RETRO, SCREEN_GBC, false, true), 1, 1, LOOK_CORE_KEEP),
	   "an Uncorrected Color game leaving Clean is corrected");
	ck(plan_is(look_plan_of(LOOK_CLEAN, LOOK_RETRO, SCREEN_GBA, false, true), 1, 1, LOOK_CORE_KEEP),
	   "an Uncorrected Advance game too");
	ck(plan_is(look_plan_of(LOOK_CLEAN, LOOK_RETRO, SCREEN_GBC, true, true), 1, 0, LOOK_CORE_KEEP),
	   "a Corrected one needs nothing written");
	ck(plan_is(look_plan_of(LOOK_RETRO, LOOK_RETRO_BEZELS, SCREEN_GBC, false, true), 1, 0, LOOK_CORE_KEEP),
	   "Uncorrected chosen under Retro stays");
	ck(plan_is(look_plan_of(LOOK_RETRO, LOOK_CLEAN, SCREEN_GBA, true, true), 1, 0, LOOK_CORE_KEEP),
	   "back to Clean leaves correction as it is");
	{
		int all = 1;

		for (int w = 0; w < LOOK_COUNT; w++)
			for (int v = 0; v < LOOK_COUNT; v++)
				for (int s = SCREEN_GB; s <= SCREEN_GBA; s++)
					all &= plan_is(look_plan_of((look_value)w, (look_value)v, (look_screen)s, false, false),
					               0, 0, LOOK_CORE_KEEP);
		ck(all, "refused by diatom: nothing kept and the palette untouched, from any value to any, on any screen");
	}
	ck(plan_is(look_plan_of(LOOK_CLEAN, LOOK_RETRO, SCREEN_NONE, false, true), 0, 0, LOOK_CORE_KEEP),
	   "no screen, no look");

	printf("a shelf's Default Look:\n");
	ck(!strcmp(look_colour_shelf("GB"), "GBC") && !strcmp(look_colour_shelf("GBC"), "GBC") &&
	   !strcmp(look_colour_shelf("GBA"), "GBA") && !look_colour_shelf("NES"),
	   "colour games follow palettedef.GBC on both Game Boy shelves, palettedef.GBA on the Advance's");
	ck(look_screen_of("GB", 1) == SCREEN_GBC && look_screen_of("GBA", 1) == SCREEN_GBA, "and have the grid's screens");
	ck(plan_is(look_plan_of(LOOK_CLEAN, LOOK_RETRO, look_screen_of("GB", 1), false, true), 1, 1, LOOK_CORE_KEEP),
	   "Clean to Retro with an Uncorrected default: the games inheriting it are corrected");
	ck(plan_is(look_plan_of(LOOK_CLEAN, LOOK_RETRO_BEZELS, look_screen_of("GBA", 1), false, true), 1, 1, LOOK_CORE_KEEP),
	   "the Advance shelf the same");
	ck(plan_is(look_plan_of(LOOK_RETRO, LOOK_RETRO_BEZELS, look_screen_of("GBC", 1), false, true), 1, 0, LOOK_CORE_KEEP),
	   "an Uncorrected default chosen under Retro stays");
	ck(plan_is(look_plan_of(LOOK_RETRO_BEZELS, LOOK_CLEAN, look_screen_of("GBC", 1), true, true), 1, 0, LOOK_CORE_KEEP),
	   "back to Clean writes no palette");

	printf(fails ? "%d FAILED\n" : "ALL PASS\n", fails);
	return fails != 0;
}
