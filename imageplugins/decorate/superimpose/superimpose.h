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

#ifndef SUPERIMPOSE_H
#define SUPERIMPOSE_H

// Qt includes

#include <QRect>

// Local includes

#include "dimg.h"
#include "dcolor.h"

using namespace Digikam;

namespace DigikamDecorateImagePlugin
{

class SuperImpose
{

public:

    SuperImpose(DImg* orgImage, DImg* templ,
                QRect orgImageSelection,
                DColorComposer::CompositingOperation
                compositeRule = DColorComposer::PorterDuffNone);

    DImg getTargetImage() const
    {
        return m_destImage;
    }

private:

    void filterImage();

private:

    QRect                                m_selection;

    DImg                                 m_orgImage;
    DImg                                 m_template;
    DImg                                 m_destImage;
    DColorComposer::CompositingOperation m_compositeRule;
};

} // namespace DigikamDecorateImagePlugin

#endif /* SUPERIMPOSE_H */
