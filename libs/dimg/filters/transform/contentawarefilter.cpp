/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-01
 * Description : Content aware resizer class.
 *
 * Copyright (C) 2009      by Julien Pontabry <julien dot pontabry at ulp dot u-strasbg dot fr>
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
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

#include "contentawarefilter.h"

// Liquid rescale library include

#include <lqr.h>

// Qt includes

#include <QColor>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

// Static methods.
LqrRetVal s_carverProgressInit(const gchar* init_message);
LqrRetVal s_carverProgressUpdate(gdouble percentage);
LqrRetVal s_carverProgressEnd(const gchar* end_message);

// Static members.

/** Resizement is decomposed in 2 stages: horizontal and vertical.
 */
bool s_stage                  = false;
bool s_wResize                = false;
bool s_hResize                = false;
ContentAwareFilter* s_resiser = 0;

static LqrEnergyFuncBuiltinType toLqrEnergy(ContentAwareContainer::EnergyFunction func)
{
    switch (func)
    {
        case ContentAwareContainer::GradientNorm:
        default:
            return LQR_EF_GRAD_NORM;

        case ContentAwareContainer::SumOfAbsoluteValues:
            return LQR_EF_GRAD_SUMABS;

        case ContentAwareContainer::XAbsoluteValue:
            return LQR_EF_GRAD_XABS;

        case ContentAwareContainer::LumaGradientNorm:
            return LQR_EF_LUMA_GRAD_NORM;

        case ContentAwareContainer::LumaSumOfAbsoluteValues:
            return LQR_EF_LUMA_GRAD_SUMABS;

        case ContentAwareContainer::LumaXAbsoluteValue:
            return LQR_EF_LUMA_GRAD_XABS;
    }
}

static LqrResizeOrder toLqrOrder(Qt::Orientation direction)
{
    switch (direction)
    {
        case Qt::Horizontal:
        default:
            return LQR_RES_ORDER_HOR;

        case Qt::Vertical:
            return LQR_RES_ORDER_VERT;
    }
}

// --------------------------------------------------------------------------------------

class ContentAwareFilter::Private
{
public:

    Private()
    {
        carver   = 0;
        progress = 0;
    }

    ContentAwareContainer settings;

    LqrCarver*            carver;
    LqrProgress*          progress;

};

ContentAwareFilter::ContentAwareFilter(QObject* const parent)
    : DImgThreadedFilter(parent),
      d(new Private)
{
    initFilter();
}

ContentAwareFilter::ContentAwareFilter(DImg* const orgImage, QObject* const parent, const ContentAwareContainer& settings)
    : DImgThreadedFilter(orgImage, parent, QLatin1String("ContentAwareFilter")),
      d(new Private)
{
    initFilter();

    s_stage     = false;
    s_resiser   = this;
    d->settings = settings;
    d->carver   = lqr_carver_new_ext(m_orgImage.bits(), m_orgImage.width(), m_orgImage.height(), 4,
                                     m_orgImage.sixteenBit() ? LQR_COLDEPTH_16I : LQR_COLDEPTH_8I);

    if (d->carver)
    {
        // Non null carver object operations

        // Ask Lqr library to preserve our picture
        lqr_carver_set_preserve_input_image(d->carver);

        // Initialize the carver object
        lqr_carver_init(d->carver, d->settings.step, d->settings.rigidity);

        // Create a progress object
        d->progress = lqr_progress_new();
        lqr_progress_set_init(d->progress, s_carverProgressInit);
        lqr_progress_set_update(d->progress, s_carverProgressUpdate);
        lqr_progress_set_end(d->progress, s_carverProgressEnd);
        lqr_carver_set_progress(d->carver, d->progress);

        lqr_carver_set_side_switch_frequency(d->carver, d->settings.side_switch_freq);

        // Set enlargement steps as suggested by Carlo Baldassi
        lqr_carver_set_enl_step(d->carver, 1.5);

        // Choose a gradient function
        lqr_carver_set_energy_function_builtin(d->carver, toLqrEnergy(d->settings.func));

        // Choose the resize order
        lqr_carver_set_resize_order(d->carver, toLqrOrder(d->settings.resize_order));

        // Set a bias if any mask
        if (!d->settings.mask.isNull())
        {
            buildBias(d->settings.mask);
        }

        // Set skin tone mask if option is activated
        if (d->settings.preserve_skin_tones)
        {
            buildSkinToneBias();
        }
    }
}

ContentAwareFilter::~ContentAwareFilter()
{
    cancelFilter();

    if (d->carver)
    {
        lqr_carver_destroy(d->carver);
    }

    delete d;
}

void ContentAwareFilter::getEnergyImage()
{
    if (!d->carver)
    {
        return;
    }

    int w        = lqr_carver_get_width(d->carver);
    int h        = lqr_carver_get_height(d->carver);
    guchar* buff = (guchar*) malloc(w * h * 3 * sizeof(guchar));

    lqr_carver_get_energy_image(d->carver, buff, 1, LQR_COLDEPTH_8I, LQR_RGBA_IMAGE);
}

void ContentAwareFilter::filterImage()
{
    if (!d->carver)
    {
        return;
    }

    uint  x   = 0;
    uint  y   = 0;
    uint  w   = 0;
    uint  h   = 0;

    s_wResize = (m_orgImage.width()  == d->settings.width)  ? false : true;
    s_hResize = (m_orgImage.height() == d->settings.height) ? false : true;

    // Liquid rescale
    lqr_carver_resize(d->carver, d->settings.width, d->settings.height);

    if (!runningFlag())
    {
        return;
    }

    // Create a new image
    w           = lqr_carver_get_width(d->carver);
    h           = lqr_carver_get_height(d->carver);
    m_destImage = DImg(w, h, m_orgImage.sixteenBit());

    // Write pixels in the DImg structure image
    lqr_carver_scan_reset(d->carver);

    void*           rgb      = 0;
    uchar*          rgbOut8  = 0;
    unsigned short* rgbOut16 = 0;

    if (m_orgImage.sixteenBit())
    {
        while (runningFlag() && lqr_carver_scan_ext(d->carver, (gint*)&x, (gint*)&y, &rgb))
        {
            rgbOut16 = (unsigned short*)rgb;
            m_destImage.setPixelColor(x, y, DColor(rgbOut16[2], rgbOut16[1], rgbOut16[0], 65535, true));
        }
    }
    else
    {
        while (runningFlag() && lqr_carver_scan_ext(d->carver, (gint*)&x, (gint*)&y, &rgb))
        {
            rgbOut8 = (uchar*)rgb;
            m_destImage.setPixelColor(x, y, DColor(rgbOut8[2], rgbOut8[1], rgbOut8[0], 255, false));
        }
    }
}

void ContentAwareFilter::progressCallback(int progress)
{
    if (progress % 5 == 0)
    {
        postProgress(progress);
    }

    //qCDebug(DIGIKAM_DIMG_LOG) << "Content Aware Resizing: " << progress << " %";
}

void ContentAwareFilter::cancelFilter()
{
    // Handle cancel operations with lqr library.
    qCDebug(DIGIKAM_DIMG_LOG) << "Stop LibLqr computation...";
    lqr_carver_cancel(d->carver);
    DImgThreadedFilter::cancelFilter();
}

bool ContentAwareFilter::isSkinTone(const DColor& color)
{
    // NOTE: color is previously converted to eight bits.
    double R = color.red()   / 255.0;
    double G = color.green() / 255.0;
    double B = color.blue()  / 255.0;
    double S = R + G + B;

    return(((B / G)             < 1.249) &&
           ((S / 3.0 * R)       > 0.696) &&
           ((1.0 / 3.0 - B / S) > 0.014) &&
           ((G / (3.0 * S))     < 0.108)
          );
}

void ContentAwareFilter::buildSkinToneBias()
{
    DColor c;

    for (uint x = 0; x < m_orgImage.width(); ++x)
    {
        for (uint y = 0; y < m_orgImage.height(); ++y)
        {
            c            = m_orgImage.getPixelColor(x, y);
            c.convertToEightBit();
            gdouble bias = 10000 * isSkinTone(c);
            lqr_carver_bias_add_xy(d->carver, bias, x, y);
        }
    }
}

void ContentAwareFilter::buildBias(const QImage& mask)
{
    QColor pixColor;
    int    r, g, b, a;

    for (int x = 0; x < mask.width(); ++x)
    {
        for (int y = 0; y < mask.height(); ++y)
        {
            pixColor     = QColor::fromRgba(mask.pixel(x, y));
            pixColor.getRgb(&r, &g, &b, &a);
            gdouble bias = 0.0;

            if (g == 255)
            {
                bias = 1000000.0;
            }

            if (r == 255)
            {
                bias = -1000000.0;
            }

            lqr_carver_bias_add_xy(d->carver, bias, x, y);
        }
    }
}

FilterAction ContentAwareFilter::filterAction()
{
    bool isReproducible = d->settings.mask.isNull();
    DefaultFilterAction<ContentAwareFilter> action(isReproducible);

    action.addParameter(QLatin1String("height"),              d->settings.height);
    action.addParameter(QLatin1String("preserve_skin_tones"), d->settings.preserve_skin_tones);
    action.addParameter(QLatin1String("rigidity"),            d->settings.rigidity);
    action.addParameter(QLatin1String("side_switch_freq"),    d->settings.side_switch_freq);
    action.addParameter(QLatin1String("step"),                d->settings.step);
    action.addParameter(QLatin1String("width"),               d->settings.width);
    action.addParameter(QLatin1String("func"),                d->settings.func);
    action.addParameter(QLatin1String("resize_order"),        d->settings.resize_order);

    return action;
}

void ContentAwareFilter::readParameters(const FilterAction& action)
{
    d->settings.height              = action.parameter(QLatin1String("height")).toUInt();
    d->settings.preserve_skin_tones = action.parameter(QLatin1String("preserve_skin_tones")).toBool();
    d->settings.rigidity            = action.parameter(QLatin1String("rigidity")).toDouble();
    d->settings.side_switch_freq    = action.parameter(QLatin1String("side_switch_freq")).toInt();
    d->settings.step                = action.parameter(QLatin1String("step")).toInt();
    d->settings.width               = action.parameter(QLatin1String("width")).toUInt();
    d->settings.func                = (ContentAwareContainer::EnergyFunction)action.parameter(QLatin1String("func")).toInt();
    d->settings.resize_order        = (Qt::Orientation)action.parameter(QLatin1String("resize_order")).toInt();
}

// ------------------------------------------------------------------------------------
// Static methods.

LqrRetVal s_carverProgressInit(const gchar* /*init_message*/)
{
    if (!s_stage)
    {
        s_resiser->progressCallback(0);
    }
    else
    {
        s_resiser->progressCallback(50);
    }

    return LQR_OK;
}

LqrRetVal s_carverProgressUpdate(gdouble percentage)
{
    int progress;

    if (!s_stage)
    {
        if (!s_wResize || !s_hResize)
        {
            progress = (int)(percentage * 100.0);
        }
        else
        {
            progress = (int)(percentage * 50.0);
        }
    }
    else
    {
        progress = (int)(50.0 + percentage * 50.0);
    }

    s_resiser->progressCallback(progress);
    return LQR_OK;
}

LqrRetVal s_carverProgressEnd(const gchar* /*end_message*/)
{
    if (!s_stage)
    {
        if (!s_wResize || !s_hResize)
        {
            s_resiser->progressCallback(100);
        }
        else
        {
            s_resiser->progressCallback(50);
        }

        s_stage = true;
    }
    else
    {
        s_resiser->progressCallback(100);
    }

    return LQR_OK;
}

} // namespace Digikam
