/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-11-14
 * Description : a presentation tool.
 *
 * Copyright (C) 2007-2008 by Valerio Fuoglio <valerio dot fuoglio at gmail dot com>
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * Parts of this code are based on smoothslidesaver by Carsten Weinhold
 * <carsten dot weinhold at gmx dot de>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "kbeffect.h"

// C ANSI includes

#include <assert.h>

// Local includes

#include "presentationkb.h"
#include "presentationkb_p.h"

namespace Digikam
{

// -------------------------------------------------------------------------

int KBEffect::m_numKBEffectRepeated = 0;

// -------------------------------------------------------------------------

KBEffect::KBEffect(PresentationKB* const parent, bool needFadeIn)
{
    m_img[0]      = 0;
    m_img[1]      = 0;
    m_slideWidget = parent;
    m_needFadeIn  = needFadeIn;
}

KBEffect::~KBEffect()
{
}

void KBEffect::setupNewImage(int img)
{
    m_slideWidget->setupNewImage(img);
}

void KBEffect::swapImages()
{
    m_slideWidget->swapImages();
}

KBImage* KBEffect::image(int img) const
{
    Q_ASSERT (img >= 0 && img < 2);

    return m_slideWidget->d->image[img];
}

KBEffect::Type KBEffect::chooseKBEffect(KBEffect::Type oldType)
{

    KBEffect::Type type;

    do
    {
        type = (qrand() < RAND_MAX / 2) ? KBEffect::Fade : KBEffect::Blend;
    }
    while ((type == oldType) && (m_numKBEffectRepeated >= 1));

    if (type == oldType)
        m_numKBEffectRepeated++;
    else
        m_numKBEffectRepeated = 0;

    return type;
}

// -------------------------------------------------------------------------

FadeKBEffect::FadeKBEffect(PresentationKB* const parent, bool needFadeIn)
    : KBEffect(parent, needFadeIn)
{
    m_img[0] = image(0);
}

FadeKBEffect::~FadeKBEffect()
{
}

bool FadeKBEffect::done()
{
    if (m_img[0]->m_pos >= 1.0)
    {
        setupNewImage(0);
        return true;
    }

    return false;
}

void FadeKBEffect::advanceTime(float step)
{
    m_img[0]->m_pos += step;

    if (m_img[0]->m_pos >= 1.0)
        m_img[0]->m_pos = 1.0;

    if (m_needFadeIn && m_img[0]->m_pos < 0.1)
        m_img[0]->m_opacity = m_img[0]->m_pos * 10;
    else if (m_img[0]->m_pos > 0.9)
        m_img[0]->m_opacity = (1.0 - m_img[0]->m_pos) * 10;
    else
        m_img[0]->m_opacity = 1.0;
}

// -------------------------------------------------------------------------

BlendKBEffect::BlendKBEffect(PresentationKB* const parent, bool needFadeIn)
    : KBEffect(parent, needFadeIn)
{
    m_img[0] = image(0);
    m_img[1] = 0;
}

BlendKBEffect::~BlendKBEffect()
{
}

bool BlendKBEffect::done()
{
    if (m_img[0]->m_pos >= 1.0)
    {
        m_img[0]->m_paint = false;
        swapImages();
        return true;
    }

    return false;
}

void BlendKBEffect::advanceTime(float step)
{
    m_img[0]->m_pos += step;

    if (m_img[0]->m_pos >= 1.0)
        m_img[0]->m_pos = 1.0;

    if (m_img[1])
        m_img[1]->m_pos += step;

    if (m_needFadeIn && m_img[0]->m_pos < 0.1)
        m_img[0]->m_opacity = m_img[0]->m_pos * 10;

    else if (m_img[0]->m_pos > 0.9)
    {
        m_img[0]->m_opacity = (1.0 - m_img[0]->m_pos) * 10;

        if (m_img[1] == 0)
        {
            setupNewImage(1);
            m_img[1] = image(1);
            m_img[1]->m_opacity = 1.0;
        }
    }
    else
    {
        m_img[0]->m_opacity = 1.0;
    }
}

}  // namespace Digikam
