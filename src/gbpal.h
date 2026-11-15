/* SPDX-License-Identifier: MIT */
/* The in-game Palette list for Game Boy games (plorpos-gkd.76): a choice per
 * game, turned into the two mgba core options that make it. Split out of
 * main.c under ADR-0001 so tools/gbpal-check.c drives it with no SDL. */
#ifndef TORTOS_GBPAL_H
#define TORTOS_GBPAL_H

#include <stddef.h>

/* Auto first, then mgba's four plain palettes and the twelve a Game Boy Color
 * offers at boot. Its 32 Super Game Boy palettes are left out: "SGB 2-F"
 * tells a player nothing. */
#define GBPAL_COUNT 17

/* The menu's label, and what palette.GB.<file> stores. */
const char *gbpal_label(int i);

/* The entry labelled `label`; 0 (Auto) for NULL, "" or a label not listed. */
int gbpal_find(const char *label);

/* The two core options that make entry i, in the shape plat_coreopt hands
 * out: "mgba_gb_colors_preset=N" and "mgba_gb_colors=<value>" for an mGBA core
 * (mGBA, PokeDaisy's Game Boy core), Gambatte's colorization pair when
 * `gambatte`. One list for both, kept by label, so a game's palette and a
 * shelf's default survive a change of core (plorpos-ssa). False if either
 * does not fit. */
int gbpal_opts(int i, int gambatte, char *preset, size_t pcap, char *colors, size_t ccap);

/* The NES shelf's list is FCEUmm's. Default is first, then every palette the
 * core declares except Raw, which needs a decoder shader, and Custom, which
 * needs a palette file. */
#define NESPAL_COUNT 24

/* As gbpal_label and gbpal_find; entry 0 is Default. */
const char *nespal_label(int i);
int nespal_find(const char *label);

/* Writes the core option for entry i, "fceumm_palette=<value>". False if it
 * does not fit. */
int nespal_opt(int i, char *opt, size_t cap);

/* A Game Boy Color or Game Boy Advance game draws its own colours, so its list
 * is the core's colour correction: Colour Corrected first, then Uncorrected. */
#define CCPAL_COUNT 2

/* As gbpal_label and gbpal_find; entry 0 is Colour Corrected. */
const char *ccpal_label(int i);
int ccpal_find(const char *label);

/* Writes the option that makes entry i on `core`, the core's name as the shelf
 * has it ("gambatte", "gpsp"). False, with `opt` empty, for a core that is not
 * listed or an option that does not fit. */
int ccpal_opt(int i, const char *core, char *opt, size_t cap);

/* Whether the ROM at `path` is a Game Boy Color game. A plain ROM says so in
 * byte 0x143 of its header (0x80 or 0xC0). A zip is judged by the name of its
 * first entry, .gbc or .cgb, which is read without inflating anything. */
int gbpal_rom_is_cgb(const char *path);

#endif
