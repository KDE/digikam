/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-26
 * Description : a digiKam image editor plugin to enhance photograph
 *
 * Copyright (C) 2005-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGEPLUGIN_ENHANCE_H
#define IMAGEPLUGIN_ENHANCE_H

// Qt includes

#include <QVariant>

// Local includes

#include "imageplugin.h"
#include "digikam_export.h"

using namespace Digikam;

namespace DigikamEnhanceImagePlugin
{

class ImagePlugin_Enhance : public ImagePlugin
{
    Q_OBJECT

public:

    ImagePlugin_Enhance(QObject* const parent, const QVariantList& args);
    ~ImagePlugin_Enhance();

    void setEnabledActions(bool b);

private Q_SLOTS:

    void slotRestoration();
    void slotBlur();
    void slotSharpen();
    void slotNoiseReduction();
    void slotLocalContrast();
    void slotRedEye();
    void slotInPainting();
    void slotLensAutoFix();
    void slotAntiVignetting();
    void slotLensDistortion();
    void slotHotPixels();

private:

    class Private;
    Private* const d;
};

} // namespace DigikamEnhanceImagePlugin

#endif /* IMAGEPLUGIN_ENHANCE_H */
