/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-01
 * Description : Content aware resizer class.
 *
 * Copyright (C) 2009 by Julien Pontabry <julien dot pontabry at ulp dot u-strasbg dot fr>
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "contentawareresizer.h"

// Qt includes

#include <QColor>

// Liquid rescale library include

#include "lqr.h"

// Local includes

#include "debug.h"

namespace DigikamContentAwareResizingImagesPlugin
{

// Static methods.
LqrRetVal s_carverProgressInit(const gchar *init_message);
LqrRetVal s_carverProgressUpdate(gdouble percentage);
LqrRetVal s_carverProgressEnd(const gchar *end_message);

// Static members.

/** Resizement is decomposed in 2 stages: horizontal and vertical.
 */
bool s_stage                   = false;
bool s_wResize                 = false;
bool s_hResize                 = false;

ContentAwareResizer *s_resiser = 0;

class ContentAwareResizerPriv
{
public:

    ContentAwareResizerPriv()
    {
        width    = 0;
        height   = 0;
        carver   = 0;
        progress = 0;
    }

    uint         width;
    uint         height;

    LqrCarver   *carver;
    LqrProgress *progress;

};

ContentAwareResizer::ContentAwareResizer(DImg *orgImage, uint width, uint height,
                                         int step, double rigidity, int side_switch_freq,
                                         LqrEnergyFuncBuiltinType func,
                                         LqrResizeOrder resize_order, const QImage& mask,
                                         bool preserve_skin_tones, QObject *parent)
                   : Digikam::DImgThreadedFilter(orgImage, parent, "ContentAwareResizer"),
                     d(new ContentAwareResizerPriv)
{
    initFilter();

    s_stage   = false;
    s_resiser = this;
    d->width  = width;
    d->height = height;

    d->carver = lqr_carver_new_ext(m_orgImage.bits(), m_orgImage.width(), m_orgImage.height(), 4,
                                   m_orgImage.sixteenBit() ? LQR_COLDEPTH_16I : LQR_COLDEPTH_8I);

    if (d->carver)
    {
        // Non null carver object operations

        // Ask Lqr library to preserve our picture
        lqr_carver_set_preserve_input_image(d->carver);

        // Initialize the carver object
        lqr_carver_init(d->carver, step, rigidity);

        // Create a progress object
        d->progress = lqr_progress_new();
        lqr_progress_set_init(d->progress, s_carverProgressInit);
        lqr_progress_set_update(d->progress, s_carverProgressUpdate);
        lqr_progress_set_end(d->progress, s_carverProgressEnd);
        lqr_carver_set_progress(d->carver, d->progress);

        lqr_carver_set_side_switch_frequency(d->carver,side_switch_freq);

        // Set enlargement steps as suggested by Carlo Baldassi
        lqr_carver_set_enl_step(d->carver, 1.5);

        // Choose a gradient function
        lqr_carver_set_energy_function_builtin(d->carver, func);

        // Choose the resize order
        if (resize_order == 0)
            lqr_carver_set_resize_order(d->carver, LQR_RES_ORDER_HOR);
        else
            lqr_carver_set_resize_order(d->carver, LQR_RES_ORDER_VERT);

        // Set a bias if any mask
        if (!mask.isNull())
            buildBias(mask);

        // Set skin tone mask if option is activated
        if (preserve_skin_tones)
            buildSkinToneBias();
    }
}

ContentAwareResizer::~ContentAwareResizer()
{
    if (d->carver)
        lqr_carver_destroy(d->carver);

    delete d;
}

void ContentAwareResizer::filterImage()
{
    if (!d->carver) return;

    uint  x   = 0;
    uint  y   = 0;
    uint  w   = 0;
    uint  h   = 0;

    s_wResize = (m_orgImage.width()  == d->width)  ? false : true;
    s_hResize = (m_orgImage.height() == d->height) ? false : true;

    // Liquid rescale
    lqr_carver_resize(d->carver, d->width, d->height);
    if (m_cancel) return;

    // Create a new image
    w           = lqr_carver_get_width(d->carver);
    h           = lqr_carver_get_height(d->carver);
    m_destImage = DImg(w, h, m_orgImage.sixteenBit());

    // Write pixels in the DImg structure image
    lqr_carver_scan_reset(d->carver);

    void           *rgb=0;
    uchar          *rgbOut8=0;
    unsigned short *rgbOut16=0;

    if (m_orgImage.sixteenBit())
    {
        while(!m_cancel && lqr_carver_scan_ext(d->carver, (gint*)&x, (gint*)&y, &rgb))
        {
            rgbOut16 = (unsigned short*)rgb;
            m_destImage.setPixelColor(x, y, DColor(rgbOut16[2], rgbOut16[1], rgbOut16[0], 65535, true));
        }
    }
    else
    {
        while(!m_cancel && lqr_carver_scan_ext(d->carver, (gint*)&x, (gint*)&y, &rgb))
        {
            rgbOut8 = (uchar*)rgb;
            m_destImage.setPixelColor(x, y, DColor(rgbOut8[2], rgbOut8[1], rgbOut8[0], 255, false));
        }
    }
}

void ContentAwareResizer::progressCallback(int progress)
{
    if (progress%5 == 0)
        postProgress( progress );

    //kDebug(digiKamAreaCode) << "Content Aware Resizing: " << progress << " %";
}

void ContentAwareResizer::cancelFilter()
{
    // Handle cancel operations with lqr library.
    kDebug(digiKamAreaCode) << "Stop LibLqr computation...";
    lqr_carver_cancel(d->carver);
    DImgThreadedFilter::cancelFilter();
}

bool ContentAwareResizer::isSkinTone(const DColor& color)
{
    // NOTE: color is previously converted to eight bits.
    double R = color.red()   / 255.0;
    double G = color.green() / 255.0;
    double B = color.blue()  / 255.0;
    double S = R + G + B;

    return( (B/G         < 1.249) &&
            (S/3.0*R     > 0.696) &&
            (1.0/3.0-B/S > 0.014) &&
            (G/(3.0*S)   < 0.108)
          );
}

void ContentAwareResizer::buildSkinToneBias()
{
    DColor c;
    for(uint x=0; x < m_orgImage.width(); ++x)
    {
        for(uint y=0; y < m_orgImage.height(); ++y)
        {
            c = m_orgImage.getPixelColor(x, y);
            c.convertToEightBit();
            gdouble bias = 10000*isSkinTone(c);
            lqr_carver_bias_add_xy(d->carver,bias,x,y);
        }
    }
}

void ContentAwareResizer::buildBias(const QImage& mask)
{
    QColor pixColor;
    int    r,g,b,a;
    for(int x=0; x < mask.width(); ++x)
    {
        for(int y=0; y < mask.height(); ++y)
        {
            pixColor = QColor::fromRgba(mask.pixel(x,y));
            pixColor.getRgb(&r, &g, &b, &a);
            gdouble bias=0.0;

            if (g == 255)
                bias=1000000.0;
            if (r == 255)
                bias=-1000000.0;

            lqr_carver_bias_add_xy(d->carver,bias,x,y);
        }
    }
}

// ------------------------------------------------------------------------------------
// Static methods.

LqrRetVal s_carverProgressInit(const gchar* /*init_message*/)
{
    if (!s_stage)
        s_resiser->progressCallback(0);
    else
        s_resiser->progressCallback(50);

    return LQR_OK;
}

LqrRetVal s_carverProgressUpdate(gdouble percentage)
{
    int progress;

    if (!s_stage)
    {
        if (!s_wResize || !s_hResize)
            progress = (int)(percentage*100.0);
        else
            progress = (int)(percentage*50.0);
    }
    else
    {
        progress = (int)(50.0 + percentage*50.0);
    }

    s_resiser->progressCallback(progress);
    return LQR_OK;
}

LqrRetVal s_carverProgressEnd(const gchar* /*end_message*/)
{
    if (!s_stage)
    {
        if (!s_wResize || !s_hResize)
            s_resiser->progressCallback(100);
        else
            s_resiser->progressCallback(50);

        s_stage = true;
    }
    else
    {
        s_resiser->progressCallback(100);
    }

    return LQR_OK;
}

} // namespace DigikamContentAwareResizingImagesPlugin
