/* ============================================================
 * File  : superimpose.cpp
 * Author: Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Date  : 2006-18-03
 * Description : Superimpose filter
 * 
 * Copyright 2006 by Marcel Wiesweg
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

#include <qrect.h>

// Digikam includes.

#include <digikamheaders.h>

namespace DigikamSuperImposeImagesPlugin
{

class SuperImpose
{
public:
    SuperImpose(Digikam::DImg *orgImage, Digikam::DImg *templ,
                QRect orgImageSelection,
                Digikam::DColorComposer::CompositingOperation
                compositeRule = Digikam::DColorComposer::PorterDuffNone);

    Digikam::DImg getTargetImage() { return m_destImage; }

private:

    void filterImage(void);

    Digikam::DImg m_orgImage;
    Digikam::DImg m_template;
    Digikam::DImg m_destImage;
    QRect         m_selection;
    Digikam::DColorComposer::CompositingOperation m_compositeRule;
};

}

#endif
