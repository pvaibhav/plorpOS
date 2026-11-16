/* SPDX-License-Identifier: MIT */
/* The in-game Shader list (plorpos-gkd.72.4): res/shaders/shaders.cfg read
 * into entries, and an entry turned into the SETDISPLAY fields diatom takes
 * (diatom's ADR-0041 - the launcher owns the list, diatom only compiles what
 * it is sent). Split out of main.c under ADR-0001 so tools/shaderlist-check.c
 * drives the real parser with no SDL. */
#ifndef TORTOS_SHADERLIST_H
#define TORTOS_SHADERLIST_H

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

/* The menu row: the Flip has no GPU, and its one entry is the scanline filter, an On / Off row
 * (plorpos-f1p.75). SHADER_VALUE turns the list's "None" into the Off it shows there. */
#if defined(PLATFORM_MIYOO)
#define SHADER_LABEL "Scanline Filter"
#define SHADER_VALUE(n) (strcmp((n), "None") ? (n) : "Off")
#else
#define SHADER_LABEL "Shader"
#define SHADER_VALUE(n) (n)
#endif

#define SL_MAX  32
#define SL_NAME 32

typedef struct {
	char name[SL_NAME];   /* the menu's label and what shader.<TAG> stores */
	char passes[256];     /* "file:filter:scale[,...]", files without .glsl */
	char final[8];        /* nearest | linear */
} sl_entry;

/* e[0] is always None, whatever the file says or whether it exists. */
typedef struct {
	int      count;
	sl_entry e[SL_MAX];
} sl_list;

/* Entries from `path`, after None. A malformed line is reported on stderr
 * and left out rather than taken half-understood. Returns l->count. */
int sl_load(sl_list *l, const char *path);

/* The entry named `name`; 0 (None) for NULL, "" or a name no longer listed. */
int sl_find(const sl_list *l, const char *name);

/* "shader=none", or "shader=<dir>/<file>.glsl:filter:scale,...\tfinal=<f>"
 * for entry i, ready to follow "SETDISPLAY\t". False if it does not fit. */
bool sl_fields(const sl_list *l, int i, const char *dir, char *out, size_t cap);

/* What diatom's line `line` says about the SETDISPLAY `fields` sent (sl_fields, look_fields). DRAWN: a DISPLAY
 * whose shader= is the one asked for, or any DISPLAY for NULL `fields`. REFUSED: ERROR code=bad_shader, after
 * which diatom draws the chain it had. FAILED: any other ERROR. NONE: anything else, a DISPLAY still in flight
 * from an earlier request among them. */
typedef enum { SL_ANSWER_NONE, SL_ANSWER_DRAWN, SL_ANSWER_REFUSED, SL_ANSWER_FAILED } sl_answer;
sl_answer sl_answer_of(const char *fields, const char *line);

#endif
