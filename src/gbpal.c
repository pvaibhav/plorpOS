/* SPDX-License-Identifier: MIT */
#include "gbpal.h"

#include <stdio.h>
#include <string.h>
#include <strings.h>

/* Values exactly as mgba 0.11 declares mgba_gb_colors - arrows and all, they
 * are the buttons held at a Game Boy Color's boot - and labels without the
 * arrows, which the menu font may not have.
 *
 * Auto is what a Game Boy Color does with nothing held: mgba_gb_colors_preset
 * 1 looks the game up in the GBC boot ROM's table (144 games - Pokemon Red
 * comes out red) and anything not in it gets Dark Green, the GBC's own
 * default. The preset is read at load, so switching TO Auto mid-game shows the
 * fallback until the next launch; every fixed palette applies at once. Checked
 * on the GKD 2026-10-03 with one-shots: Pokemon Red under Auto red and green,
 * a homebrew game under Auto identical to Dark Green, and a resume state made
 * under Auto loaded grey under Grayscale - a state does not carry the palette,
 * so the choice holds on a game already played. */
static const struct { const char *label, *value, *gambatte; } PAL[GBPAL_COUNT] = {
	{ "Auto",            "GBC Dark Green →A", NULL },
	{ "Grayscale",       "Grayscale", "GBC - Grayscale" },
	{ "DMG Green",       "DMG Green", "GB - DMG" },
	{ "GB Pocket",       "GB Pocket", "GB - Pocket" },
	{ "GB Light",        "GB Light", "GB - Light" },
	{ "GBC Brown",       "GBC Brown ↑", "GBC - Brown" },
	{ "GBC Red",         "GBC Red ↑A", "GBC - Red" },
	{ "GBC Dark Brown",  "GBC Dark Brown ↑B", "GBC - Dark Brown" },
	{ "GBC Pale Yellow", "GBC Pale Yellow ↓", "GBC - Pastel Mix" },
	{ "GBC Orange",      "GBC Orange ↓A", "GBC - Orange" },
	{ "GBC Yellow",      "GBC Yellow ↓B", "GBC - Yellow" },
	{ "GBC Blue",        "GBC Blue ←", "GBC - Blue" },
	{ "GBC Dark Blue",   "GBC Dark Blue ←A", "GBC - Dark Blue" },
	{ "GBC Gray",        "GBC Gray ←B", "GBC - Grayscale" },
	{ "GBC Green",       "GBC Green →", "GBC - Green" },
	{ "GBC Dark Green",  "GBC Dark Green →A", "GBC - Dark Green" },
	{ "GBC Reverse",     "GBC Reverse →B", "GBC - Inverted" },
};

const char *gbpal_label(int i)
{
	return i >= 0 && i < GBPAL_COUNT ? PAL[i].label : PAL[0].label;
}

int gbpal_find(const char *label)
{
	int i;

	if (!label) return 0;
	for (i = 0; i < GBPAL_COUNT; i++)
		if (!strcmp(PAL[i].label, label)) return i;
	return 0;
}

int gbpal_opts(int i, int gambatte, char *preset, size_t pcap, char *colors, size_t ccap)
{
	int a, b;

	if (i < 0 || i >= GBPAL_COUNT) i = 0;
	if (gambatte) {
	/* Gambatte (plorpos-f1p.77, every port's since plorpos-ssa): a colorization mode, and under Internal
	 * the palette's name. Both apply as the game runs, Auto included once the core re-reads them, but
	 * Auto's per-game table is read at load, as mgba's was. Auto sends a fixed one beside it, which
	 * the core ignores, so a switch from a fixed palette to Auto is the same pair of keys. Grayscale
	 * and GBC Gray are the one palette there (the owner's call). */
	a = snprintf(preset, pcap, "gambatte_gb_colorization=%s", i == 0 ? "auto" : "internal");
	b = snprintf(colors, ccap, "gambatte_gb_internal_palette=%s", i == 0 ? "GBC - Dark Green" : PAL[i].gambatte);
	} else {
	a = snprintf(preset, pcap, "mgba_gb_colors_preset=%d", i == 0 ? 1 : 0);
	b = snprintf(colors, ccap, "mgba_gb_colors=%s", PAL[i].value);
	}
	return a > 0 && (size_t)a < pcap && b > 0 && (size_t)b < ccap;
}

/* The values are the ones FCEUmm declares for fceumm_palette. The Flip's build
 * (libretro-fceumm e0f3086) and the prebuilt core the other ports ship declare
 * the same ones. The labels are the core's own, shortened to fit the menu. The
 * core reads the option as the game runs, so a choice shows on Continue. */
static const struct { const char *label, *value; } NES[NESPAL_COUNT] = {
	{ "Default",              "default" },
	{ "ASQ Real",             "asqrealc" },
	{ "Restored Wii VC",      "restored-wii-vc" },
	{ "Wii VC",               "wii-vc" },
	{ "Nintendo RGB",         "rgb" },
	{ "FBX YUV-V3",           "yuv-v3" },
	{ "FBX Unsaturated",      "unsaturated-final" },
	{ "Sony CXA2025AS",       "sony-cxa2025as-us" },
	{ "PAL",                  "pal" },
	{ "BMF Final 2",          "bmf-final2" },
	{ "BMF Final 3",          "bmf-final3" },
	{ "FBX Smooth",           "smooth-fbx" },
	{ "FBX Composite Direct", "composite-direct-fbx" },
	{ "FBX PVM D93",          "pvm-style-d93-fbx" },
	{ "FBX NTSC Hardware",    "ntsc-hardware-fbx" },
	{ "FBX NES Classic FS",   "nes-classic-fbx-fs" },
	{ "NESCAP",               "nescap" },
	{ "Wavebeam",             "wavebeam" },
	{ "FBX Digital Prime",    "digital-prime-fbx" },
	{ "FBX Magnum",           "magnum-fbx" },
	{ "FBX Smooth V2",        "smooth-v2-fbx" },
	{ "FBX NES Classic",      "nes-classic-fbx" },
	{ "Royaltea",             "royaltea" },
	{ "Mugicha",              "mugicha" },
};

const char *nespal_label(int i)
{
	return i >= 0 && i < NESPAL_COUNT ? NES[i].label : NES[0].label;
}

int nespal_find(const char *label)
{
	int i;

	if (!label) return 0;
	for (i = 0; i < NESPAL_COUNT; i++)
		if (!strcmp(NES[i].label, label)) return i;
	return 0;
}

int nespal_opt(int i, char *opt, size_t cap)
{
	int a;

	if (i < 0 || i >= NESPAL_COUNT) i = 0;
	a = snprintf(opt, cap, "fceumm_palette=%s", NES[i].value);
	return a > 0 && (size_t)a < cap;
}

/* Each core's colour correction option, with the value that turns it on and
 * the one that turns it off. PokeDaisy's cores carry the options of the
 * emulator under them: mGBA's on the Game Boy shelves, armGBA's on the Game Boy
 * Advance shelf. mGBA's Auto corrects for the console the game runs on. Read
 * from the cores: Gambatte and gpSP are the Flip's builds, armGBA is v1.6. */
static const struct { const char *core, *key, *on, *off; } CC[] = {
	{ "gambatte",      "gambatte_gbc_color_correction", "GBC only", "disabled" },
	{ "gpsp",          "gpsp_color_correction",         "enabled",  "disabled" },
	{ "armgba",        "armgba_color_correction",       "enabled",  "disabled" },
	{ "pokedaisy_gba", "armgba_color_correction",       "enabled",  "disabled" },
	{ "mgba",          "mgba_color_correction",         "Auto",     "OFF" },
	{ "pokedaisy_gbc", "mgba_color_correction",         "Auto",     "OFF" },
};

static const char *const CC_LABEL[CCPAL_COUNT] = { "Colour Corrected", "Uncorrected" };

const char *ccpal_label(int i)
{
	return CC_LABEL[i == 1];
}

int ccpal_find(const char *label)
{
	return label && !strcmp(label, CC_LABEL[1]);
}

int ccpal_opt(int i, const char *core, char *opt, size_t cap)
{
	size_t k;
	int a;

	for (k = 0; k < sizeof CC / sizeof CC[0]; k++) {
		if (strcmp(CC[k].core, core)) continue;
		a = snprintf(opt, cap, "%s=%s", CC[k].key, i == 1 ? CC[k].off : CC[k].on);
		if (a > 0 && (size_t)a < cap) return 1;
		break;
	}
	if (cap) opt[0] = '\0';
	return 0;
}

static int ext_is(const char *name, size_t n, const char *ext)
{
	size_t e = strlen(ext);

	return n > e && !strncasecmp(name + n - e, ext, e);
}

int gbpal_rom_is_cgb(const char *path)
{
	size_t n = strlen(path);
	unsigned char h[30], b = 0;
	FILE *f;

	if (ext_is(path, n, ".zip")) {
		char end[4];
		size_t len;
		int cgb = 0;

		/* A zip opens with its first entry's header: the name's length at 26, the name at 30. Its last four
		 * bytes are the extension. */
		if (!(f = fopen(path, "rb"))) return 0;
		if (fread(h, 1, sizeof h, f) == sizeof h && !memcmp(h, "PK\3\4", 4)) {
			len = h[26] | (size_t)h[27] << 8;
			if (len > sizeof end && !fseek(f, (long)(len - sizeof end), SEEK_CUR) &&
			    fread(end, 1, sizeof end, f) == sizeof end)
				cgb = !strncasecmp(end, ".gbc", sizeof end) || !strncasecmp(end, ".cgb", sizeof end);
		}
		fclose(f);
		return cgb;
	}
	if (!ext_is(path, n, ".gb") && !ext_is(path, n, ".gbc") && !ext_is(path, n, ".cgb") && !ext_is(path, n, ".dmg"))
		return 0;
	if (!(f = fopen(path, "rb"))) return 0;
	if (fseek(f, 0x143, SEEK_SET) != 0 || fread(&b, 1, 1, f) != 1) b = 0;
	fclose(f);
	return b == 0x80 || b == 0xC0;
}
