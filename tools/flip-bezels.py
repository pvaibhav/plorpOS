"""The Miyoo Mini Flip's bezel images for Look's Retro with Bezels, written to sd/miyoo/bezels/.

The committed PNGs are the artefact; mk/miyoo-deploy.sh copies them to TortOS/shaders/bezels/ and nothing runs
this at build time. Run it again only to change the art:

    ~/Code/Me/authentic-shaders/nextui/.venv/bin/python -I tools/flip-bezels.py

It needs PIL and numpy, and the lettering is Gill Sans from macOS (/System/Library/Fonts/Supplemental/GillSans.ttc),
so it runs on a Mac. The Game Boy Color's lens is drawn from tools/flip-bezels-lens-black.png.

Each image is 752x560 RGBA, straight alpha, upright: alpha 255 is lens, 0 the window onto the panel, and between
them the lens's shadow on the panel and its anti-aliased edge. The name is <lens>-<w>x<h>+<x>+<y>.png: diatom uses
it when the game's display rect is w x h, and draws the picture at x,y. The window is the picture rect grown by a
margin, which diatom fills with the look's panel colour.
"""
import os

import numpy as np
from PIL import Image, ImageDraw, ImageFont

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(os.path.dirname(HERE), "sd", "miyoo", "bezels")
BLACK = os.path.join(HERE, "flip-bezels-lens-black.png")
GILL = "/System/Library/Fonts/Supplemental/GillSans.ttc"   # faces: 0 regular, 1 bold, 2 italic, 3 bold italic
PW, PH = 752, 560
SS = 4   # the lenses are drawn at SS times the panel and brought down


# ---------------------------------------------------------------- layers: float RGBA, straight alpha
def blank():
    return np.zeros((PH, PW, 4), np.float64)


def over(base, top):
    """top over base."""
    at, ab = top[..., 3:4], base[..., 3:4]
    a = at + ab * (1 - at)
    c = np.where(a > 1e-9, (top[..., :3] * at + base[..., :3] * ab * (1 - at)) / np.maximum(a, 1e-9), 0.0)
    return np.concatenate([c, a], -1)


def layer(rect, colour, alpha):
    """`colour` at `alpha` (a number or an (h, w) array) over rect, clear elsewhere."""
    x, y, w, h = rect
    out = blank()
    out[y:y + h, x:x + w, :3] = np.asarray(colour, np.float64) / 255.0
    out[y:y + h, x:x + w, 3] = alpha
    return out


def quantise(ov):
    return np.clip(np.round(ov * 255), 0, 255).astype(np.uint8)


def grow(rect, l, t, r, b):
    x, y, w, h = rect
    x0, y0, x1, y1 = max(0, x - l), max(0, y - t), min(PW, x + w + r), min(PH, y + h + b)
    return (x0, y0, x1 - x0, y1 - y0)


def inner_shadow(window, depth, strength):
    """The lip's shadow on the panel: strongest along the top and left of the window."""
    x, y, w, h = window
    yy, xx = np.mgrid[0:h, 0:w] + 0.5
    top = np.clip(1 - yy / depth, 0, 1) ** 2
    left = np.clip(1 - xx / depth, 0, 1) ** 2
    bot = np.clip(1 - (h - yy) / (depth * 0.5), 0, 1) ** 2 * 0.4
    right = np.clip(1 - (w - xx) / (depth * 0.5), 0, 1) ** 2 * 0.4
    return layer(window, (0, 0, 0), strength * np.clip(top + left + bot + right, 0, 1))


# ---------------------------------------------------------------- the Game Boy Color's lens, from art
def art_window(art):
    """The black window's bounding box in the lens art: (x0, y0, x1, y1)."""
    dark = art[..., :3].max(-1) < 14
    cols = np.nonzero(dark.mean(0) > 0.5)[0]
    rows = np.nonzero(dark.mean(1) > 0.5)[0]
    return cols.min(), rows.min(), cols.max() + 1, rows.max() + 1


def bezel_layer(path, window, lip=5):
    """The lens art fitted so its window lands on `window`. Scaled by one factor (the window's height), cut into four
    quadrants at the window's centre, each placed with its window corner on the target's; what is lost or doubled
    falls inside the window or along plain lens. `lip` pixels of the art's own inner edge are kept."""
    art = np.asarray(Image.open(path).convert("RGB"))
    ax0, ay0, ax1, ay1 = art_window(art)
    tx, ty, tw, th = window
    s = (th + 2 * lip) / (ay1 - ay0)
    big = np.asarray(Image.fromarray(art).resize((round(art.shape[1] * s), round(art.shape[0] * s)),
                                                 Image.LANCZOS)).astype(np.float64) / 255
    bx0, by0, bx1, by1 = [round(v * s) for v in (ax0, ay0, ax1, ay1)]
    out = blank()
    X0, Y0, X1, Y1 = tx - lip, ty - lip, tx + tw + lip, ty + th + lip
    cx, cy = (X0 + X1) // 2, (Y0 + Y1) // 2
    for (px0, px1, anchor_p, anchor_a) in ((0, cx, X0, bx0), (cx, PW, X1, bx1)):
        for (py0, py1, anchor_q, anchor_b) in ((0, cy, Y0, by0), (cy, PH, Y1, by1)):
            xs = np.clip(np.arange(px0, px1) - anchor_p + anchor_a, 0, big.shape[1] - 1)
            ys = np.clip(np.arange(py0, py1) - anchor_q + anchor_b, 0, big.shape[0] - 1)
            out[py0:py1, px0:px1, :3] = big[ys][:, xs]
    out[..., 3] = 1.0
    out[ty:ty + th, tx:tx + tw, 3] = 0.0
    return out


# ---------------------------------------------------------------- drawn lenses
def font(face, px):
    return ImageFont.truetype(GILL, int(round(px * SS)), index=face)


def new_lens(top, bottom):
    """A lens filled with a soft vertical gradient, at SS times the panel."""
    g = np.linspace(0, 1, PH * SS)[:, None, None]
    arr = np.asarray(top, np.float64) * (1 - g) + np.asarray(bottom, np.float64) * g
    return Image.fromarray(np.broadcast_to(arr, (PH * SS, PW * SS, 3)).astype(np.uint8).copy())


def rr(d, box, r, **kw):
    x0, y0, x1, y1 = box
    d.rounded_rectangle([x0 * SS, y0 * SS, x1 * SS - 1, y1 * SS - 1], radius=r * SS, **kw)


def recess(img, window, ring, dark, light, r=3):
    """The step down from the lens to the panel: a darker ring, lit along its lower right."""
    d = ImageDraw.Draw(img)
    x, y, w, h = window
    rr(d, (x - ring, y - ring, x + w + ring, y + h + ring), r + ring, fill=light)
    rr(d, (x - ring, y - ring, x + w + ring - 1, y + h + ring - 1), r + ring, fill=dark)


def finish(img, window, r=3):
    """Down to panel size, opaque outside the window and clear in it, the edge anti-aliased."""
    rgb = np.asarray(img.resize((PW, PH), Image.LANCZOS)).astype(np.float64) / 255
    m = Image.new("L", (PW * SS, PH * SS), 255)
    x, y, w, h = window
    rr(ImageDraw.Draw(m), (x, y, x + w, y + h), r, fill=0)
    a = np.asarray(m.resize((PW, PH), Image.BOX)).astype(np.float64) / 255
    return np.concatenate([rgb, a[..., None]], -1)


def text(d, xy, s, f, fill, anchor="mm", track=0.0):
    if not track:
        d.text((xy[0] * SS, xy[1] * SS), s, font=f, fill=fill, anchor=anchor)
        return
    widths = [d.textlength(c, font=f) + track * SS for c in s]
    x = xy[0] * SS - (sum(widths) - track * SS) / 2 if anchor[0] == "m" else xy[0] * SS
    for c, wd in zip(s, widths):
        d.text((x, xy[1] * SS), c, font=f, fill=fill, anchor="l" + anchor[1])
        x += wd


def lens_dmg(window, shade=1.0):
    """The Game Boy's grey lens: two stripes and the legend above, the battery lamp at the left. The Pocket's is the
    same lens darker."""
    base = np.array([124, 124, 131]) * shade
    img = new_lens(base * 1.05, base * 0.93)
    d = ImageDraw.Draw(img)
    x, y, w, h = window
    if y >= 40:                                   # room above the window for the stripes
        sy = y // 2 - 8
        d.rectangle([0, sy * SS, PW * SS, (sy + 5) * SS - 1], fill=(118, 30, 66))
        d.rectangle([0, (sy + 9) * SS, PW * SS, (sy + 14) * SS - 1], fill=(36, 40, 110))
        f = font(0, 11.5)
        s = "DOT MATRIX WITH STEREO SOUND"
        tw = sum(d.textlength(c, font=f) + 0.8 * SS for c in s) / SS
        tx = x + w - tw - 4
        d.rectangle([(tx - 8) * SS, (sy - 2) * SS, (tx + tw + 8) * SS, (sy + 16) * SS],
                    fill=tuple(int(v) for v in base * 1.03))
        text(d, (tx, sy + 7), s, f, (214, 214, 220), "lm", track=0.8)
    if x >= 40:                                   # room at the left for the lamp
        cx, cy = x // 2, y + int(h * 0.34)
        for i in range(40, 0, -1):                # a soft glow, then the lamp itself
            t = i / 40
            c = tuple(int(base[j] * 0.97 * t + (235, 40, 46)[j] * (1 - t)) for j in range(3))
            rad = 3.5 + 6 * t
            d.ellipse([(cx - rad) * SS, (cy - rad) * SS, (cx + rad) * SS, (cy + rad) * SS], fill=c)
        d.ellipse([(cx - 1.6) * SS, (cy - 2.0) * SS, (cx + 0.2) * SS, (cy - 0.4) * SS], fill=(255, 150, 150))
        text(d, (cx, cy + 18), "BATTERY", font(0, 8.5), (214, 214, 220), "mm", track=0.6)
    recess(img, window, 3, tuple(int(v) for v in base * 0.55), tuple(int(v) for v in base * 1.25))
    return finish(img, window)


def lens_sp(window, logo_y):
    """The Game Boy Advance SP's black lens, its logo centred in the room under the window."""
    img = new_lens((24, 24, 27), (12, 12, 14))
    d = ImageDraw.Draw(img)
    recess(img, window, 2, (4, 4, 5), (52, 52, 57))
    silver = (150, 152, 160)
    parts = [("GAME BOY ", font(3, 17)), ("ADVANCE ", font(2, 17)), ("SP", font(3, 23))]
    total = sum(d.textlength(s, font=f) for s, f in parts)
    x = PW * SS / 2 - total / 2
    for s, f in parts:
        d.text((x, (logo_y + 6) * SS), s, font=f, fill=silver, anchor="ls")
        x += d.textlength(s, font=f)
    return finish(img, window)


# ---------------------------------------------------------------- the set diatom looks up
def bezel(window, lens, depth=12, strength=0.5):
    """The lens over its shadow on the panel."""
    return quantise(over(inner_shadow(window, depth, strength), lens))


def images():
    """(name, RGBA) for each lens in the Flip's Integer and Aspect rects."""
    gb_i, gb_a = (136, 72, 480, 432), (65, 0, 622, 560)    # Integer moved down 8: the stripes sit above
    gbc_i, gbc_a = (136, 64, 480, 432), (65, 0, 622, 560)
    gba_i, gba_a = (16, 12, 720, 480), (0, 4, 752, 501)    # moved up: the logo takes the room under the window
    out = []
    for lens, shade in (("dmg", 1.0), ("pocket", 0.5)):
        for rect, win in ((gb_i, grow(gb_i, 9, 9, 9, 9)), (gb_a, grow(gb_a, 6, 0, 6, 0))):
            out.append((lens, rect, bezel(win, lens_dmg(win, shade))))
    for rect, win in ((gbc_i, grow(gbc_i, 9, 9, 9, 9)), (gbc_a, grow(gbc_a, 6, 0, 6, 0))):
        out.append(("gbc", rect, bezel(win, bezel_layer(BLACK, win))))
    for rect, win in ((gba_i, grow(gba_i, 5, 5, 5, 5)), (gba_a, grow(gba_a, 0, 4, 0, 4))):
        logo_y = (win[1] + win[3] + PH) // 2
        out.append(("gba", rect, bezel(win, lens_sp(win, logo_y), 14, 0.55)))
    return [(f"{lens}-{w}x{h}+{x}+{y}", img) for lens, (x, y, w, h), img in out]


if __name__ == "__main__":
    os.makedirs(OUT, exist_ok=True)
    for name, img in images():
        path = os.path.join(OUT, name + ".png")
        Image.fromarray(img, "RGBA").save(path, optimize=True)
        print(path)
