/* SPDX-License-Identifier: MIT */
/* Does the in-game Palette list send what mgba takes?
 *
 * Links src/gbpal.c and NOT SDL. A value mgba does not declare is not an
 * error anywhere - the core keeps its own default and the row looks broken -
 * so every value is checked against the list the shipped core printed:
 * `diatom --list-options` with mgba_libretro.so (mGBA 0.11.0) on the GKD,
 * 2026-10-03.
 */
#include "../src/gbpal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int fails;

static void ck(int cond, const char *what)
{
	printf("  %s %s\n", cond ? "ok  " : "FAIL", what);
	if (!cond) fails++;
}

static const char *const MGBA[] = {
	"Grayscale", "DMG Green", "GB Pocket", "GB Light",
	"GBC Brown ↑", "GBC Red ↑A", "GBC Dark Brown ↑B", "GBC Pale Yellow ↓",
	"GBC Orange ↓A", "GBC Yellow ↓B", "GBC Blue ←", "GBC Dark Blue ←A",
	"GBC Gray ←B", "GBC Green →", "GBC Dark Green →A", "GBC Reverse →B",
};

/* The values libretro-fceumm e0f3086 (the Flip's build) declares for
 * fceumm_palette. The prebuilt core the other ports ship has every one of
 * them as a string. */
static const char *const FCEUMM[] = {
	"default", "asqrealc", "restored-wii-vc", "wii-vc", "rgb", "yuv-v3", "unsaturated-final",
	"sony-cxa2025as-us", "pal", "bmf-final2", "bmf-final3", "smooth-fbx", "composite-direct-fbx",
	"pvm-style-d93-fbx", "ntsc-hardware-fbx", "nes-classic-fbx-fs", "nescap", "wavebeam",
	"digital-prime-fbx", "magnum-fbx", "smooth-v2-fbx", "nes-classic-fbx", "royaltea", "mugicha",
};

/* Each core's colour correction option, on and off. Gambatte's and gpSP's are the
 * Flip's builds (`diatom --list-options`, and gpSP's libretro_core_options.h at
 * 5819380), armGBA's is v1.6 as its core declares it, and mGBA's is 0.11.0. */
static const struct { const char *core, *on, *off; } CC[] = {
	{ "gambatte",      "gambatte_gbc_color_correction=GBC only", "gambatte_gbc_color_correction=disabled" },
	{ "gpsp",          "gpsp_color_correction=enabled",          "gpsp_color_correction=disabled" },
	{ "armgba",        "armgba_color_correction=enabled",        "armgba_color_correction=disabled" },
	{ "pokedaisy_gba", "armgba_color_correction=enabled",        "armgba_color_correction=disabled" },
	{ "mgba",          "mgba_color_correction=Auto",             "mgba_color_correction=OFF" },
	{ "pokedaisy_gbc", "mgba_color_correction=Auto",             "mgba_color_correction=OFF" },
};

/* Writes a file named `name` in the temporary directory and asks whether it is
 * a Game Boy Color ROM. With `entry` it is a zip's first header naming that
 * entry, else a ROM header whose byte 0x143 is `flag`. */
static int rom_is(const char *name, unsigned char flag, const char *entry)
{
	unsigned char h[0x150] = { 0 };
	char path[256];
	const char *tmp = getenv("TMPDIR");
	FILE *f;
	int is;

	snprintf(path, sizeof path, "%s/gbpal-check-%s", tmp ? tmp : "/tmp", name);
	if (!(f = fopen(path, "wb"))) return -1;
	if (entry) {
		memcpy(h, "PK\3\4", 4);
		h[26] = (unsigned char)strlen(entry);
		fwrite(h, 1, 30, f);
		fwrite(entry, 1, strlen(entry), f);
	} else {
		h[0x143] = flag;
		fwrite(h, 1, sizeof h, f);
	}
	fclose(f);
	is = gbpal_rom_is_cgb(path);
	remove(path);
	return is;
}

static int declared(const char *v)
{
	size_t i;

	for (i = 0; i < sizeof MGBA / sizeof MGBA[0]; i++)
		if (!strcmp(MGBA[i], v)) return 1;
	return 0;
}

int main(void)
{
	char p[64], c[64], what[128];
	int i, j, dup = 0;

	printf("lookup:\n");
	ck(gbpal_find(NULL) == 0 && gbpal_find("") == 0, "nothing is Auto");
	ck(gbpal_find("SGB 2-F") == 0, "a label not listed is Auto");
	ck(!strcmp(gbpal_label(0), "Auto"), "Auto leads");
	ck(!strcmp(gbpal_label(GBPAL_COUNT), "Auto"), "out of range reads Auto");

	printf("options:\n");
	ck(gbpal_opts(0, 0, p, sizeof p, c, sizeof c) &&
	   !strcmp(p, "mgba_gb_colors_preset=1") &&
	   !strcmp(c, "mgba_gb_colors=GBC Dark Green →A"),
	   "Auto: the GBC table, Dark Green where a game is not in it");
	ck(gbpal_opts(3, 0, p, sizeof p, c, sizeof c) &&
	   !strcmp(p, "mgba_gb_colors_preset=0") &&
	   !strcmp(c, "mgba_gb_colors=GB Pocket"),
	   "a fixed palette turns the table off");
	ck(!gbpal_opts(1, 0, p, 8, c, sizeof c), "too small does not fit");
	ck(gbpal_opts(0, 1, p, sizeof p, c, sizeof c) &&
	   !strcmp(p, "gambatte_gb_colorization=auto"),
	   "Gambatte's Auto: its own per-game table");
	ck(gbpal_opts(3, 1, p, sizeof p, c, sizeof c) &&
	   !strcmp(p, "gambatte_gb_colorization=internal") &&
	   !strcmp(c, "gambatte_gb_internal_palette=GB - Pocket"),
	   "the same label is the same palette on Gambatte (plorpos-ssa)");
	for (i = 1; i < GBPAL_COUNT; i++) {
		gbpal_opts(i, 1, p, sizeof p, c, sizeof c);
		snprintf(what, sizeof what, "%-16s %s", gbpal_label(i), c);
		ck(!strncmp(c, "gambatte_gb_internal_palette=G", 30), what);
	}

	printf("every entry:\n");
	for (i = 0; i < GBPAL_COUNT; i++) {
		gbpal_opts(i, 0, p, sizeof p, c, sizeof c);
		snprintf(what, sizeof what, "%-16s %s", gbpal_label(i), c + strlen("mgba_gb_colors="));
		ck(declared(c + strlen("mgba_gb_colors=")) && gbpal_find(gbpal_label(i)) == i, what);
		for (j = 0; j < i; j++)
			if (!strcmp(gbpal_label(i), gbpal_label(j))) dup++;
	}
	ck(dup == 0, "no two labels alike");

	printf("NES:\n");
	ck(nespal_find(NULL) == 0 && nespal_find("GB Pocket") == 0 && !strcmp(nespal_label(0), "Default"),
	   "nothing, or a label not listed, is Default");
	ck(nespal_opt(0, p, sizeof p) && !strcmp(p, "fceumm_palette=default"), "Default is the core's own");
	ck(!nespal_opt(1, p, 8), "too small does not fit");
	for (i = 0, dup = 0; i < NESPAL_COUNT; i++) {
		size_t k, hit = 0;

		nespal_opt(i, p, sizeof p);
		for (k = 0; k < sizeof FCEUMM / sizeof FCEUMM[0]; k++)
			if (!strcmp(FCEUMM[k], p + strlen("fceumm_palette="))) hit = 1;
		snprintf(what, sizeof what, "%-20s %s", nespal_label(i), p + strlen("fceumm_palette="));
		ck(hit && nespal_find(nespal_label(i)) == i, what);
		for (j = 0; j < i; j++)
			if (!strcmp(nespal_label(i), nespal_label(j))) dup++;
	}
	ck(dup == 0, "no two labels alike");

	printf("colour correction:\n");
	ck(ccpal_find(NULL) == 0 && ccpal_find("GB Pocket") == 0 && !strcmp(ccpal_label(0), "Colour Corrected"),
	   "nothing, or a label not listed, is Colour Corrected");
	ck(ccpal_find("Uncorrected") == 1 && !strcmp(ccpal_label(1), "Uncorrected"), "Uncorrected is the second");
	for (i = 0; i < (int)(sizeof CC / sizeof CC[0]); i++) {
		ck(ccpal_opt(0, CC[i].core, p, sizeof p) && !strcmp(p, CC[i].on), CC[i].on);
		ck(ccpal_opt(1, CC[i].core, p, sizeof p) && !strcmp(p, CC[i].off), CC[i].off);
	}
	ck(!ccpal_opt(0, "fceumm", p, sizeof p) && !p[0], "a core with no correction gets no option");
	ck(!ccpal_opt(0, "gambatte", p, 8) && !p[0], "too small does not fit");

	printf("Game Boy Color ROMs:\n");
	ck(rom_is("plain.gbc", 0x80, NULL) && rom_is("plain.gb", 0xC0, NULL), "a plain ROM with the Color flag");
	ck(!rom_is("plain.gb", 0x00, NULL), "a plain ROM without it");
	ck(rom_is("a.zip", 0, "Game (USA).gbc") && rom_is("a.zip", 0, "GAME.CGB"), "a zip whose entry is .gbc or .cgb");
	ck(!rom_is("a.zip", 0, "Game (World).gb"), "a zip whose entry is .gb");
	ck(!rom_is("a.zip", 0x80, NULL) && !gbpal_rom_is_cgb("no-such-file.gbc"), "a file that is not a zip, or is not there");

	printf(fails ? "%d FAILED\n" : "ALL PASS\n", fails);
	return fails != 0;
}
