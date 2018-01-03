/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-02
 * Description : colors scheme manager
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2007      by Matthew Woehlke <mw_triad at users dot sourceforge dot net>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "schememanager.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QWidget>
#include <QPainter>

// KDE includes

#include <kconfig.h>
#include <kconfiggroup.h>

namespace Digikam
{

// HCY color space management

class HCYColorSpace
{
public:

    explicit HCYColorSpace(const QColor&);
    explicit HCYColorSpace(qreal h_, qreal c_, qreal y_, qreal a_ = 1.0);

    QColor qColor() const;

    static qreal luma(const QColor &);

public:

    qreal h;
    qreal c;
    qreal y;
    qreal a;

private:

    static qreal gamma(qreal);
    static qreal igamma(qreal);
    static qreal lumag(qreal, qreal, qreal);
};

// ------------------------------------------------------------------------------

namespace ColorTools
{

static inline qreal wrap(qreal a, qreal d = 1.0)
{
    qreal r = fmod(a, d);

    return (r < 0.0 ? d + r : (r > 0.0 ? r : 0.0));
}

/**
 * normalize: like qBound(a, 0.0, 1.0) but without needing the args and with
 * "safer" behavior on NaN (isnan(a) -> return 0.0)
 */
static inline qreal normalize(qreal a)
{
    return (a < 1.0 ? (a > 0.0 ? a : 0.0) : 1.0);
}

static inline qreal mixQreal(qreal a, qreal b, qreal bias)
{
    return (a + (b - a) * bias);
}

/**
 * Calculate the luma of a color. Luma is weighted sum of gamma-adjusted
 * R'G'B' components of a color. The result is similar to qGray. The range
 * is from 0.0 (black) to 1.0 (white).
 *
 */
qreal luma(const QColor& color)
{
    return HCYColorSpace::luma(color);
}

/**
 * Calculate hue, chroma and luma of a color in one call.
 */
void getHcy(const QColor& color, qreal* h, qreal* c, qreal* y, qreal* a = 0)
{
    if (!c || !h || !y)
    {
        return;
    }

    HCYColorSpace khcy(color);
    *c = khcy.c;
    *h = khcy.h;
    *y = khcy.y;

    if (a)
    {
        *a = khcy.a;
    }
}

/**
 * Calculate the contrast ratio between two colors, according to the
 * W3C/WCAG2.0 algorithm, (Lmax + 0.05)/(Lmin + 0.05), where Lmax and Lmin
 * are the luma values of the lighter color and the darker color,
 * respectively.
 *
 * A contrast ration of 5:1 (result == 5.0) is the minimum for "normal"
 * text to be considered readable (large text can go as low as 3:1). The
 * ratio ranges from 1:1 (result == 1.0) to 21:1 (result == 21.0).
 */
static qreal contrastRatioForLuma(qreal y1, qreal y2)
{
    if (y1 > y2)
    {
        return (y1 + 0.05) / (y2 + 0.05);
    }

    return (y2 + 0.05) / (y1 + 0.05);
}

qreal contrastRatio(const QColor& c1, const QColor& c2)
{
    return contrastRatioForLuma(luma(c1), luma(c2));
}

/**
 * Adjust the luma of a color by changing its distance from white.
 */
QColor lighten(const QColor& color, qreal ky = 0.5, qreal kc = 1.0)
{
    HCYColorSpace c(color);
    c.y = 1.0 - ColorTools::normalize((1.0 - c.y) * (1.0 - ky));
    c.c = 1.0 - ColorTools::normalize((1.0 - c.c) * kc);

    return c.qColor();
}

/**
 * Adjust the luma of a color by changing its distance from black.
 */
QColor darken(const QColor& color, qreal ky = 0.5, qreal kc = 1.0)
{
    HCYColorSpace c(color);
    c.y = ColorTools::normalize(c.y * (1.0 - ky));
    c.c = ColorTools::normalize(c.c * kc);

    return c.qColor();
}

/**
 * Adjust the luma and chroma components of a color. The amount is added
 * to the corresponding component.
 */
QColor shade(const QColor& color, qreal ky, qreal kc = 0.0)
{
    HCYColorSpace c(color);
    c.y = ColorTools::normalize(c.y + ky);
    c.c = ColorTools::normalize(c.c + kc);

    return c.qColor();
}

/**
 * Blend two colors into a new color by linear combination.
 */
QColor mix(const QColor& c1, const QColor& c2, qreal bias)
{
    if (bias <= 0.0)
    {
        return c1;
    }

    if (bias >= 1.0)
    {
        return c2;
    }

    if (qIsNaN(bias))
    {
        return c1;
    }

    qreal r = mixQreal(c1.redF(),   c2.redF(),   bias);
    qreal g = mixQreal(c1.greenF(), c2.greenF(), bias);
    qreal b = mixQreal(c1.blueF(),  c2.blueF(),  bias);
    qreal a = mixQreal(c1.alphaF(), c2.alphaF(), bias);

    return QColor::fromRgbF(r, g, b, a);
}

static QColor tintHelper(const QColor& base, qreal baseLuma, const QColor& color, qreal amount)
{
    HCYColorSpace result(mix(base, color, pow(amount, 0.3)));
    result.y = mixQreal(baseLuma, result.y, amount);

    return result.qColor();
}

/**
 * Create a new color by tinting one color with another. This function is
 * meant for creating additional colors withings the same class (background,
 * foreground) from colors in a different class. Therefore when @p amount
 * is low, the luma of @p base is mostly preserved, while the hue and
 * chroma of @p color is mostly inherited.
 *
 * @param base color to be tinted
 * @param color color with which to tint
 * @param amount how strongly to tint the base; 0.0 gives @p base,
 * 1.0 gives @p color
 */
QColor tint(const QColor& base, const QColor& color, qreal amount = 0.3)
{
    if (amount <= 0.0)
    {
        return base;
    }

    if (amount >= 1.0)
    {
        return color;
    }

    if (qIsNaN(amount))
    {
        return base;
    }

    qreal baseLuma = luma(base); //cache value because luma call is expensive
    double ri      = contrastRatioForLuma(baseLuma, luma(color));
    double rg      = 1.0 + ((ri + 1.0) * amount * amount * amount);
    double u       = 1.0, l = 0.0;
    QColor result;

    for (int i = 12; i; --i)
    {
        double a  = 0.5 * (l + u);
        result    = tintHelper(base, baseLuma, color, a);
        double ra = contrastRatioForLuma(baseLuma, luma(result));

        if (ra > rg)
        {
            u = a;
        }
        else
        {
            l = a;
        }
    }

    return result;
}

/**
 * Blend two colors into a new color by painting the second color over the
 * first using the specified composition mode.
 *
 * @param base the base color (alpha channel is ignored).
 * @param paint the color to be overlayed onto the base color.
 * @param comp the CompositionMode used to do the blending.
 */
QColor overlayColors(const QColor& base, const QColor& paint,
                     QPainter::CompositionMode comp)
{
    // This isn't the fastest way, but should be "fast enough".
    // It's also the only safe way to use QPainter::CompositionMode
    QImage img(1, 1, QImage::Format_ARGB32_Premultiplied);
    QPainter p(&img);
    QColor start = base;
    start.setAlpha(255); // opaque
    p.fillRect(0, 0, 1, 1, start);
    p.setCompositionMode(comp);
    p.fillRect(0, 0, 1, 1, paint);
    p.end();

    return img.pixel(0, 0);
}

} // namespace ColorTools

// ------------------------------------------------------------------------------

#define HCY_REC 709 // use 709 for now
#if   HCY_REC == 601
static const qreal yc[3] = {0.299,   0.587,  0.114  };
#elif HCY_REC == 709
static const qreal yc[3] = {0.2126,  0.7152, 0.0722 };
#else // use Qt values
static const qreal yc[3] = {0.34375, 0.5,    0.15625};
#endif

qreal HCYColorSpace::gamma(qreal n)
{
    return pow(ColorTools::normalize(n), 2.2);
}

qreal HCYColorSpace::igamma(qreal n)
{
    return pow(ColorTools::normalize(n), 1.0 / 2.2);
}

qreal HCYColorSpace::lumag(qreal r, qreal g, qreal b)
{
    return r * yc[0] + g * yc[1] + b * yc[2];
}

HCYColorSpace::HCYColorSpace(qreal h_, qreal c_, qreal y_, qreal a_)
{
    h = h_;
    c = c_;
    y = y_;
    a = a_;
}

HCYColorSpace::HCYColorSpace(const QColor& color)
{
    qreal r = gamma(color.redF());
    qreal g = gamma(color.greenF());
    qreal b = gamma(color.blueF());
    a       = color.alphaF();

    // luma component
    y       = lumag(r, g, b);

    // hue component
    qreal p = qMax(qMax(r, g), b);
    qreal n = qMin(qMin(r, g), b);
    qreal d = 6.0 * (p - n);

    if (n == p)
    {
        h = 0.0;
    }
    else if (r == p)
    {
        h = ((g - b) / d);
    }
    else if (g == p)
    {
        h = ((b - r) / d) + (1.0 / 3.0);
    }
    else
    {
        h = ((r - g) / d) + (2.0 / 3.0);
    }

    // chroma component
    if (r == g && g == b)
    {
        c = 0.0;
    }
    else
    {
        c = qMax((y - n) / y, (p - y) / (1 - y));
    }
}

QColor HCYColorSpace::qColor() const
{
    // start with sane component values
    qreal _h  = ColorTools::wrap(h);
    qreal _c  = ColorTools::normalize(c);
    qreal _y  = ColorTools::normalize(y);

    // calculate some needed variables
    qreal _hs = _h * 6.0, th, tm;

    if (_hs < 1.0)
    {
        th = _hs;
        tm = yc[0] + yc[1] * th;
    }
    else if (_hs < 2.0)
    {
        th = 2.0 - _hs;
        tm = yc[1] + yc[0] * th;
    }
    else if (_hs < 3.0)
    {
        th = _hs - 2.0;
        tm = yc[1] + yc[2] * th;
    }
    else if (_hs < 4.0)
    {
        th = 4.0 - _hs;
        tm = yc[2] + yc[1] * th;
    }
    else if (_hs < 5.0)
    {
        th = _hs - 4.0;
        tm = yc[2] + yc[0] * th;
    }
    else
    {
        th = 6.0 - _hs;
        tm = yc[0] + yc[2] * th;
    }

    // calculate RGB channels in sorted order
    qreal tn, to, tp;

    if (tm >= _y)
    {
        tp = _y + _y * _c * (1.0 - tm) / tm;
        to = _y + _y * _c * (th  - tm) / tm;
        tn = _y - (_y * _c);
    }
    else
    {
        tp = _y + (1.0 - _y) * _c;
        to = _y + (1.0 - _y) * _c * (th - tm) / (1.0 - tm);
        tn = _y - (1.0 - _y) * _c * tm / (1.0 - tm);
    }

    // return RGB channels in appropriate order
    if (_hs < 1.0)
    {
        return QColor::fromRgbF(igamma(tp), igamma(to), igamma(tn), a);
    }
    else if (_hs < 2.0)
    {
        return QColor::fromRgbF(igamma(to), igamma(tp), igamma(tn), a);
    }
    else if (_hs < 3.0)
    {
        return QColor::fromRgbF(igamma(tn), igamma(tp), igamma(to), a);
    }
    else if (_hs < 4.0)
    {
        return QColor::fromRgbF(igamma(tn), igamma(to), igamma(tp), a);
    }
    else if (_hs < 5.0)
    {
        return QColor::fromRgbF(igamma(to), igamma(tn), igamma(tp), a);
    }
    else
    {
        return QColor::fromRgbF(igamma(tp), igamma(tn), igamma(to), a);
    }
}

qreal HCYColorSpace::luma(const QColor& color)
{
    return lumag(gamma(color.redF()),
                 gamma(color.greenF()),
                 gamma(color.blueF()));
}

// -------------------------------------------------------------------------------------

class StateEffects
{
public:

    explicit StateEffects(QPalette::ColorGroup state, const KSharedConfigPtr&);
    ~StateEffects() {}

    QBrush brush(const QBrush& background) const;
    QBrush brush(const QBrush& foreground, const QBrush& background) const;

private:

    enum Effects
    {
        // Effects
        Intensity         = 0,
        Color             = 1,
        Contrast          = 2,
        // Intensity
        IntensityNoEffect = 0,
        IntensityShade    = 1,
        IntensityDarken   = 2,
        IntensityLighten  = 3,
        // Color
        ColorNoEffect     = 0,
        ColorDesaturate   = 1,
        ColorFade         = 2,
        ColorTint         = 3,
        // Contrast
        ContrastNoEffect  = 0,
        ContrastFade      = 1,
        ContrastTint      = 2
    };

private:

    int    _effects[3];
    double _amount[3];
    QColor _color;
};

StateEffects::StateEffects(QPalette::ColorGroup state, const KSharedConfigPtr& config)
    : _color(0, 0, 0, 0)
{
    QString group;

    if (state == QPalette::Disabled)
    {
        group = QLatin1String("ColorEffects:Disabled");
    }
    else if (state == QPalette::Inactive)
    {
        group = QLatin1String("ColorEffects:Inactive");
    }

    _effects[0] = 0;
    _effects[1] = 0;
    _effects[2] = 0;

    if (!group.isEmpty())
    {
        KConfigGroup cfg(config, group);
        const bool enabledByDefault = (state == QPalette::Disabled);

        if (cfg.readEntry("Enable", enabledByDefault))
        {
            _effects[Intensity] = cfg.readEntry("IntensityEffect", (int)((state == QPalette::Disabled) ?  IntensityDarken : IntensityNoEffect));
            _effects[Color]     = cfg.readEntry("ColorEffect",     (int)((state == QPalette::Disabled) ?  ColorNoEffect   : ColorDesaturate));
            _effects[Contrast]  = cfg.readEntry("ContrastEffect",  (int)((state == QPalette::Disabled) ?  ContrastFade    : ContrastTint));
            _amount[Intensity]  = cfg.readEntry("IntensityAmount", (state == QPalette::Disabled) ? 0.10 :  0.0);
            _amount[Color]      = cfg.readEntry("ColorAmount",     (state == QPalette::Disabled) ?  0.0 : -0.9);
            _amount[Contrast]   = cfg.readEntry("ContrastAmount",  (state == QPalette::Disabled) ? 0.65 :  0.25);

            if (_effects[Color] > ColorNoEffect)
            {
                _color = cfg.readEntry("Color", (state == QPalette::Disabled) ? QColor(56, 56, 56)
                                                                              : QColor(112, 111, 110));
            }
        }
    }
}

QBrush StateEffects::brush(const QBrush& background) const
{
    QColor color = background.color(); // TODO - actually work on brushes

    switch (_effects[Intensity])
    {
        case IntensityShade:
            color = ColorTools::shade(color, _amount[Intensity]);
            break;
        case IntensityDarken:
            color = ColorTools::darken(color, _amount[Intensity]);
            break;
        case IntensityLighten:
            color = ColorTools::lighten(color, _amount[Intensity]);
            break;
    }

    switch (_effects[Color])
    {
        case ColorDesaturate:
            color = ColorTools::darken(color, 0.0, 1.0 - _amount[Color]);
            break;
        case ColorFade:
            color = ColorTools::mix(color, _color, _amount[Color]);
            break;
        case ColorTint:
            color = ColorTools::tint(color, _color, _amount[Color]);
            break;
    }

    return QBrush(color);
}

QBrush StateEffects::brush(const QBrush& foreground, const QBrush& background) const
{
    QColor color = foreground.color();
    QColor bg    = background.color();

    // Apply the foreground effects

    switch (_effects[Contrast])
    {
        case ContrastFade:
            color = ColorTools::mix(color, bg, _amount[Contrast]);
            break;
        case ContrastTint:
            color = ColorTools::tint(color, bg, _amount[Contrast]);
            break;
    }

    // Now apply global effects

    return brush(color);
}

// ------------------------------------------------------------------------------------

struct SetDefaultColors
{
    int NormalBackground[3];
    int AlternateBackground[3];
    int NormalText[3];
    int InactiveText[3];
    int ActiveText[3];
    int LinkText[3];
    int VisitedText[3];
    int NegativeText[3];
    int NeutralText[3];
    int PositiveText[3];
};

struct DecoDefaultColors
{
    int Hover[3];
    int Focus[3];
};

// these numbers come from the Breeze color scheme
static const SetDefaultColors defaultViewColors =
{
    { 252, 252, 252 }, // Background
    { 239, 240, 241 }, // Alternate
    {  49,  54,  59 }, // Normal
    { 127, 140, 141 }, // Inactive
    {  61, 174, 233 }, // Active
    {  41, 128, 185 }, // Link
    { 127, 140, 141 }, // Visited
    { 218,  68,  83 }, // Negative
    { 246, 116,   0 }, // Neutral
    {  39, 174,  96 }  // Positive
};

static const SetDefaultColors defaultWindowColors =
{
    { 239, 240, 241 }, // Background
    { 189, 195, 199 }, // Alternate
    {  49,  54,  59 }, // Normal
    { 127, 140, 141 }, // Inactive
    {  61, 174, 233 }, // Active
    {  41, 128, 185 }, // Link
    { 127, 140, 141 }, // Visited
    { 218,  68,  83 }, // Negative
    { 246, 116,   0 }, // Neutral
    {  39, 174,  96 }  // Positive
};

static const SetDefaultColors defaultButtonColors =
{
    { 239, 240, 241 }, // Background
    { 189, 195, 199 }, // Alternate
    {  49,  54,  59 }, // Normal
    { 127, 140, 141 }, // Inactive
    {  61, 174, 233 }, // Active
    {  41, 128, 185 }, // Link
    { 127, 140, 141 }, // Visited
    { 218,  68,  83 }, // Negative
    { 246, 116,   0 }, // Neutral
    {  39, 174,  96 }  // Positive
};

static const SetDefaultColors defaultSelectionColors =
{
    {  61, 174, 233 }, // Background
    {  29, 153, 243 }, // Alternate
    { 239, 240, 241 }, // Normal
    { 239, 240, 241 }, // Inactive
    { 252, 252, 252 }, // Active
    { 253, 188,  75 }, // Link
    { 189, 195, 199 }, // Visited
    { 218,  68,  83 }, // Negative
    { 246, 116,   0 }, // Neutral
    {  39, 174,  96 }  // Positive
};

static const SetDefaultColors defaultTooltipColors =
{
    {  49,  54,  59 }, // Background
    {  77,  77,  77 }, // Alternate
    { 239, 240, 241 }, // Normal
    { 189, 195, 199 }, // Inactive
    {  61, 174, 233 }, // Active
    {  41, 128, 185 }, // Link
    { 127, 140, 141 }, // Visited
    { 218,  68,  83 }, // Negative
    { 246, 116,   0 }, // Neutral
    {  39, 174,  96 }  // Positive
};

static const SetDefaultColors defaultComplementaryColors =
{
    {  49,  54,  59 }, // Background
    {  77,  77,  77 }, // Alternate
    { 239, 240, 241 }, // Normal
    { 189, 195, 199 }, // Inactive
    {  61, 174, 233 }, // Active
    {  41, 128, 185 }, // Link
    { 127, 140, 141 }, // Visited
    { 218,  68,  83 }, // Negative
    { 246, 116,   0 }, // Neutral
    {  39, 174,  96 }  // Positive
};

static const DecoDefaultColors defaultDecorationColors =
{
    { 147, 206, 233 }, // Hover
    {  61, 174, 233 }, // Focus
};

// ------------------------------------------------------------------------------------

class SchemeManagerPrivate : public QSharedData
{
public:

    explicit SchemeManagerPrivate(const KSharedConfigPtr&, QPalette::ColorGroup, const char*, SetDefaultColors);
    explicit SchemeManagerPrivate(const KSharedConfigPtr&, QPalette::ColorGroup, const char*, SetDefaultColors, const QBrush&);
    ~SchemeManagerPrivate() {}

    QBrush background(SchemeManager::BackgroundRole) const;
    QBrush foreground(SchemeManager::ForegroundRole) const;
    QBrush decoration(SchemeManager::DecorationRole) const;
    qreal  contrast() const;

private:

    void init(const KSharedConfigPtr&, QPalette::ColorGroup, const char*, SetDefaultColors);

private:

    struct
    {
        QBrush fg[8],
               bg[8],
               deco[2];
    } _brushes;

    qreal _contrast;
};

#define DEFAULT(c)      QColor( c[0], c[1], c[2] )
#define SET_DEFAULT(a)  DEFAULT( defaults.a )
#define DECO_DEFAULT(a) DEFAULT( defaultDecorationColors.a )

SchemeManagerPrivate::SchemeManagerPrivate(const KSharedConfigPtr& config,
                                           QPalette::ColorGroup state,
                                           const char* group,
                                           SetDefaultColors defaults)
{
    KConfigGroup cfg(config, group);
    _contrast      = SchemeManager::contrastF(config);

    // loaded-from-config colors (no adjustment)
    _brushes.bg[0] = cfg.readEntry("BackgroundNormal",    SET_DEFAULT(NormalBackground));
    _brushes.bg[1] = cfg.readEntry("BackgroundAlternate", SET_DEFAULT(AlternateBackground));

    // the rest
    init(config, state, group, defaults);
}

SchemeManagerPrivate::SchemeManagerPrivate(const KSharedConfigPtr& config,
                                           QPalette::ColorGroup state,
                                           const char* group,
                                           SetDefaultColors defaults,
                                           const QBrush& tint)
{
    KConfigGroup cfg(config, group);
    _contrast      = SchemeManager::contrastF(config);

    // loaded-from-config colors
    _brushes.bg[0] = cfg.readEntry("BackgroundNormal", SET_DEFAULT(NormalBackground));
    _brushes.bg[1] = cfg.readEntry("BackgroundAlternate", SET_DEFAULT(AlternateBackground));

    // adjustment
    _brushes.bg[0] = ColorTools::tint(_brushes.bg[0].color(), tint.color(), 0.4);
    _brushes.bg[1] = ColorTools::tint(_brushes.bg[1].color(), tint.color(), 0.4);

    // the rest
    init(config, state, group, defaults);
}

void SchemeManagerPrivate::init(const KSharedConfigPtr& config,
                                QPalette::ColorGroup state,
                                const char* group,
                                SetDefaultColors defaults)
{
    KConfigGroup cfg(config, group);

    // loaded-from-config colors
    _brushes.fg[0]   = cfg.readEntry("ForegroundNormal",   SET_DEFAULT(NormalText));
    _brushes.fg[1]   = cfg.readEntry("ForegroundInactive", SET_DEFAULT(InactiveText));
    _brushes.fg[2]   = cfg.readEntry("ForegroundActive",   SET_DEFAULT(ActiveText));
    _brushes.fg[3]   = cfg.readEntry("ForegroundLink",     SET_DEFAULT(LinkText));
    _brushes.fg[4]   = cfg.readEntry("ForegroundVisited",  SET_DEFAULT(VisitedText));
    _brushes.fg[5]   = cfg.readEntry("ForegroundNegative", SET_DEFAULT(NegativeText));
    _brushes.fg[6]   = cfg.readEntry("ForegroundNeutral",  SET_DEFAULT(NeutralText));
    _brushes.fg[7]   = cfg.readEntry("ForegroundPositive", SET_DEFAULT(PositiveText));
    _brushes.deco[0] = cfg.readEntry("DecorationHover",    DECO_DEFAULT(Hover));
    _brushes.deco[1] = cfg.readEntry("DecorationFocus",    DECO_DEFAULT(Focus));

    // apply state adjustments

    if (state != QPalette::Active)
    {
        StateEffects effects(state, config);

        for (int i = 0; i < 8; i++)
        {
            _brushes.fg[i] = effects.brush(_brushes.fg[i], _brushes.bg[0]);
        }

        _brushes.deco[0] = effects.brush(_brushes.deco[0], _brushes.bg[0]);
        _brushes.deco[1] = effects.brush(_brushes.deco[1], _brushes.bg[0]);
        _brushes.bg[0]   = effects.brush(_brushes.bg[0]);
        _brushes.bg[1]   = effects.brush(_brushes.bg[1]);
    }

    // calculated backgrounds
    _brushes.bg[2] = ColorTools::tint(_brushes.bg[0].color(), _brushes.fg[2].color());
    _brushes.bg[3] = ColorTools::tint(_brushes.bg[0].color(), _brushes.fg[3].color());
    _brushes.bg[4] = ColorTools::tint(_brushes.bg[0].color(), _brushes.fg[4].color());
    _brushes.bg[5] = ColorTools::tint(_brushes.bg[0].color(), _brushes.fg[5].color());
    _brushes.bg[6] = ColorTools::tint(_brushes.bg[0].color(), _brushes.fg[6].color());
    _brushes.bg[7] = ColorTools::tint(_brushes.bg[0].color(), _brushes.fg[7].color());
}

QBrush SchemeManagerPrivate::background(SchemeManager::BackgroundRole role) const
{
    switch (role)
    {
        case SchemeManager::AlternateBackground:
            return _brushes.bg[1];
        case SchemeManager::ActiveBackground:
            return _brushes.bg[2];
        case SchemeManager::LinkBackground:
            return _brushes.bg[3];
        case SchemeManager::VisitedBackground:
            return _brushes.bg[4];
        case SchemeManager::NegativeBackground:
            return _brushes.bg[5];
        case SchemeManager::NeutralBackground:
            return _brushes.bg[6];
        case SchemeManager::PositiveBackground:
            return _brushes.bg[7];
        default:
            return _brushes.bg[0];
    }
}

QBrush SchemeManagerPrivate::foreground(SchemeManager::ForegroundRole role) const
{
    switch (role)
    {
        case SchemeManager::InactiveText:
            return _brushes.fg[1];
        case SchemeManager::ActiveText:
            return _brushes.fg[2];
        case SchemeManager::LinkText:
            return _brushes.fg[3];
        case SchemeManager::VisitedText:
            return _brushes.fg[4];
        case SchemeManager::NegativeText:
            return _brushes.fg[5];
        case SchemeManager::NeutralText:
            return _brushes.fg[6];
        case SchemeManager::PositiveText:
            return _brushes.fg[7];
        default:
            return _brushes.fg[0];
    }
}

QBrush SchemeManagerPrivate::decoration(SchemeManager::DecorationRole role) const
{
    switch (role)
    {
        case SchemeManager::FocusColor:
            return _brushes.deco[1];
        default:
            return _brushes.deco[0];
    }
}

qreal SchemeManagerPrivate::contrast() const
{
    return _contrast;
}

// ------------------------------------------------------------------------------------

SchemeManager::SchemeManager(const SchemeManager& other)
    : d(other.d)
{
}

SchemeManager& SchemeManager::operator=(const SchemeManager& other)
{
    d = other.d;
    return *this;
}

SchemeManager::~SchemeManager()
{
}

SchemeManager::SchemeManager(QPalette::ColorGroup state, ColorSet set, KSharedConfigPtr config)
{
    if (!config)
    {
        config = KSharedConfig::openConfig();
    }

    switch (set)
    {
        case Window:
            d = new SchemeManagerPrivate(config, state, "Colors:Window", defaultWindowColors);
            break;
        case Button:
            d = new SchemeManagerPrivate(config, state, "Colors:Button", defaultButtonColors);
            break;
        case Selection:
            {
                KConfigGroup group(config, "ColorEffects:Inactive");
                // NOTE: keep this in sync with kdebase/workspace/kcontrol/colors/colorscm.cpp
                bool inactiveSelectionEffect = group.readEntry("ChangeSelectionColor", group.readEntry("Enable", true));
                // if enabled, inactiver/disabled uses Window colors instead, ala gtk
                // ...except tinted with the Selection:NormalBackground color so it looks more like selection
                if (state == QPalette::Active || (state == QPalette::Inactive && !inactiveSelectionEffect))
                {
                    d = new SchemeManagerPrivate(config, state, "Colors:Selection", defaultSelectionColors);
                }
                else if (state == QPalette::Inactive)
                {
                    d = new SchemeManagerPrivate(config, state, "Colors:Window", defaultWindowColors,
                                                 SchemeManager(QPalette::Active, Selection, config).background());
                }
                else
                {
                    // disabled (...and still want this branch when inactive+disabled exists)
                    d = new SchemeManagerPrivate(config, state, "Colors:Window", defaultWindowColors);
                }
            }
            break;
        case Tooltip:
            d = new SchemeManagerPrivate(config, state, "Colors:Tooltip", defaultTooltipColors);
            break;
        case Complementary:
            d = new SchemeManagerPrivate(config, state, "Colors:Complementary", defaultComplementaryColors);
            break;
        default:
            d = new SchemeManagerPrivate(config, state, "Colors:View", defaultViewColors);
    }
}

int SchemeManager::contrast()
{
    KConfigGroup g(KSharedConfig::openConfig(), "KDE");

    return g.readEntry("contrast", 7);
}

qreal SchemeManager::contrastF(const KSharedConfigPtr& config)
{
    if (config)
    {
        KConfigGroup g(config, "KDE");

        return 0.1 * g.readEntry("contrast", 7);
    }

    return 0.1 * (qreal)contrast();
}

QBrush SchemeManager::background(BackgroundRole role) const
{
    return d->background(role);
}

QBrush SchemeManager::foreground(ForegroundRole role) const
{
    return d->foreground(role);
}

QBrush SchemeManager::decoration(DecorationRole role) const
{
    return d->decoration(role);
}

QColor SchemeManager::shade(ShadeRole role) const
{
    return shade(background().color(), role, d->contrast());
}

QColor SchemeManager::shade(const QColor& color, ShadeRole role)
{
    return shade(color, role, SchemeManager::contrastF());
}

QColor SchemeManager::shade(const QColor& color, ShadeRole role, qreal contrast, qreal chromaAdjust)
{
    // nan -> 1.0
    contrast = ((1.0 > contrast) ? ((-1.0 < contrast) ? contrast
                                                      : -1.0)
                                 : 1.0);
    qreal y  = ColorTools::luma(color);
    qreal yi = 1.0 - y;

    // handle very dark colors (base, mid, dark, shadow == midlight, light)
    if (y < 0.006)
    {
        switch (role)
        {
            case SchemeManager::LightShade:
                return ColorTools::shade(color, 0.05 + 0.95 * contrast, chromaAdjust);
            case SchemeManager::MidShade:
                return ColorTools::shade(color, 0.01 + 0.20 * contrast, chromaAdjust);
            case SchemeManager::DarkShade:
                return ColorTools::shade(color, 0.02 + 0.40 * contrast, chromaAdjust);
            default:
                return ColorTools::shade(color, 0.03 + 0.60 * contrast, chromaAdjust);
        }
    }

    // handle very light colors (base, midlight, light == mid, dark, shadow)
    if (y > 0.93)
    {
        switch (role)
        {
            case SchemeManager::MidlightShade:
                return ColorTools::shade(color, -0.02 - 0.20 * contrast, chromaAdjust);
            case SchemeManager::DarkShade:
                return ColorTools::shade(color, -0.06 - 0.60 * contrast, chromaAdjust);
            case SchemeManager::ShadowShade:
                return ColorTools::shade(color, -0.10 - 0.90 * contrast, chromaAdjust);
            default:
                return ColorTools::shade(color, -0.04 - 0.40 * contrast, chromaAdjust);
        }
    }

    // handle everything else
    qreal lightAmount = (0.05 + y * 0.55) * (0.25 + contrast * 0.75);
    qreal darkAmount  = (- y)             * (0.55 + contrast * 0.35);

    switch (role)
    {
        case SchemeManager::LightShade:
            return ColorTools::shade(color, lightAmount, chromaAdjust);
        case SchemeManager::MidlightShade:
            return ColorTools::shade(color, (0.15 + 0.35 * yi) * lightAmount, chromaAdjust);
        case SchemeManager::MidShade:
            return ColorTools::shade(color, (0.35 + 0.15 * y) * darkAmount, chromaAdjust);
        case SchemeManager::DarkShade:
            return ColorTools::shade(color, darkAmount, chromaAdjust);
        default:
            return ColorTools::darken(ColorTools::shade(color, darkAmount, chromaAdjust), 0.5 + 0.3 * y);
    }
}

void SchemeManager::adjustBackground(QPalette& palette, BackgroundRole newRole, QPalette::ColorRole color,
                                     ColorSet set, KSharedConfigPtr config)
{
    palette.setBrush(QPalette::Active,   color, SchemeManager(QPalette::Active,   set, config).background(newRole));
    palette.setBrush(QPalette::Inactive, color, SchemeManager(QPalette::Inactive, set, config).background(newRole));
    palette.setBrush(QPalette::Disabled, color, SchemeManager(QPalette::Disabled, set, config).background(newRole));
}

void SchemeManager::adjustForeground(QPalette& palette, ForegroundRole newRole, QPalette::ColorRole color,
                                     ColorSet set, KSharedConfigPtr config)
{
    palette.setBrush(QPalette::Active,   color, SchemeManager(QPalette::Active,   set, config).foreground(newRole));
    palette.setBrush(QPalette::Inactive, color, SchemeManager(QPalette::Inactive, set, config).foreground(newRole));
    palette.setBrush(QPalette::Disabled, color, SchemeManager(QPalette::Disabled, set, config).foreground(newRole));
}

QPalette SchemeManager::createApplicationPalette(const KSharedConfigPtr& config)
{
    QPalette palette;

    static const QPalette::ColorGroup states[3] =
    {
        QPalette::Active,
        QPalette::Inactive,
        QPalette::Disabled
    };

    // TT thinks tooltips shouldn't use active, so we use our active colors for all states
    SchemeManager schemeTooltip(QPalette::Active, SchemeManager::Tooltip, config);

    for (int i = 0; i < 3; i++)
    {
        QPalette::ColorGroup state = states[i];
        SchemeManager schemeView(state,      SchemeManager::View,      config);
        SchemeManager schemeWindow(state,    SchemeManager::Window,    config);
        SchemeManager schemeButton(state,    SchemeManager::Button,    config);
        SchemeManager schemeSelection(state, SchemeManager::Selection, config);

        palette.setBrush(state, QPalette::WindowText,      schemeWindow.foreground());
        palette.setBrush(state, QPalette::Window,          schemeWindow.background());
        palette.setBrush(state, QPalette::Base,            schemeView.background());
        palette.setBrush(state, QPalette::Text,            schemeView.foreground());
        palette.setBrush(state, QPalette::Button,          schemeButton.background());
        palette.setBrush(state, QPalette::ButtonText,      schemeButton.foreground());
        palette.setBrush(state, QPalette::Highlight,       schemeSelection.background());
        palette.setBrush(state, QPalette::HighlightedText, schemeSelection.foreground());
        palette.setBrush(state, QPalette::ToolTipBase,     schemeTooltip.background());
        palette.setBrush(state, QPalette::ToolTipText,     schemeTooltip.foreground());

        palette.setColor(state, QPalette::Light,           schemeWindow.shade(SchemeManager::LightShade));
        palette.setColor(state, QPalette::Midlight,        schemeWindow.shade(SchemeManager::MidlightShade));
        palette.setColor(state, QPalette::Mid,             schemeWindow.shade(SchemeManager::MidShade));
        palette.setColor(state, QPalette::Dark,            schemeWindow.shade(SchemeManager::DarkShade));
        palette.setColor(state, QPalette::Shadow,          schemeWindow.shade(SchemeManager::ShadowShade));

        palette.setBrush(state, QPalette::AlternateBase,   schemeView.background(SchemeManager::AlternateBackground));
        palette.setBrush(state, QPalette::Link,            schemeView.foreground(SchemeManager::LinkText));
        palette.setBrush(state, QPalette::LinkVisited,     schemeView.foreground(SchemeManager::VisitedText));
    }

    return palette;
}

}  // namespace Digikam
