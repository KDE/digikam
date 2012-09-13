/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-06-04
 * Description : digiKam image editor plugin to correct color
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
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

#ifndef IMAGEPLUGIN_COLOR_H
#define IMAGEPLUGIN_COLOR_H

// Qt includes

#include <QVariant>

// Local includes

#include "imageplugin.h"
#include "digikam_export.h"

namespace Digikam
{
    class IccProfile;
}

using namespace Digikam;

namespace DigikamColorImagePlugin
{

class ImagePlugin_Color : public ImagePlugin
{
    Q_OBJECT

public:

    ImagePlugin_Color(QObject* const parent, const QVariantList& args);
    ~ImagePlugin_Color();

    void setEnabledSelectionActions(bool b);
    void setEnabledActions(bool b);

    typedef Digikam::IccProfile IccProfile; // to make signal/slot work

private Q_SLOTS:

    void slotBCG();
    void slotCB();
    void slotHSL();
    void slotAutoCorrection();
    void slotInvert();
    void slotBW();
    void slotWhiteBalance();
    void slotConvertTo8Bits();
    void slotConvertTo16Bits();
    void slotConvertToColorSpace(const IccProfile&);
    void slotProfileConversionTool();
    void slotChannelMixer();
    void slotCurvesAdjust();
    void slotLevelsAdjust();
    void slotFilm();

    void slotUpdateColorSpaceMenu();
    void slotSetupICC();

private:

    class Private;
    Private* const d;
};

} // namespace DigikamColorImagePlugin

#endif /* IMAGEPLUGIN_COLOR_H */
