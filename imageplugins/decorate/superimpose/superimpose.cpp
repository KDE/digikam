/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-18-03
 * Description : Superimpose filter.
 *
 * Copyright (C) 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "superimpose.h"

namespace DigikamDecorateImagePlugin
{

SuperImpose::SuperImpose(DImg* orgImage, DImg* templ,
                         QRect orgImageSelection,
                         DColorComposer::CompositingOperation compositeRule)
{
    m_orgImage      = *orgImage;
    m_template      = *templ;
    m_selection     = orgImageSelection;
    m_compositeRule = compositeRule;

    filterImage();
}

void SuperImpose::filterImage()
{
    if (m_template.isNull())
    {
        return;
    }

    int templateWidth  = m_template.width();
    int templateHeight = m_template.height();

    // take selection of src image and scale it to size of template
    m_destImage = m_orgImage.smoothScaleSection(m_selection.x(), m_selection.y(),
                  m_selection.width(), m_selection.height(), templateWidth, templateHeight);

    // convert depth if necessary
    m_template.convertToDepthOfImage(&m_destImage);

    // get composer for compositing rule
    DColorComposer* composer = DColorComposer::getComposer(m_compositeRule);
    DColorComposer::MultiplicationFlags flags = DColorComposer::NoMultiplication;

    if (m_compositeRule != DColorComposer::PorterDuffNone)
    {
        flags = DColorComposer::MultiplicationFlagsDImg;
    }

    // do alpha blending of template on dest image
    m_destImage.bitBlendImage(composer, &m_template, 0, 0, templateWidth, templateHeight, 0, 0, flags);

    delete composer;
}

} // namespace DigikamDecorateImagePlugin
