/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-19
 * Description : a tool for color space conversion
 *
 * Copyright (C) 2009 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
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

#ifndef PROFILECONVERSIONTOOL_H
#define PROFILECONVERSIONTOOL_H

// Local includes

#include "editortool.h"

namespace Digikam { class IccProfile; }

namespace DigikamImagesPluginCore
{

class ProfileConversionToolPriv;

class ProfileConversionTool : public Digikam::EditorToolThreaded
{
    Q_OBJECT

public:

    ProfileConversionTool(QObject *parent);
    ~ProfileConversionTool();

    static QStringList favoriteProfiles();
    static void fastConversion(const Digikam::IccProfile& profile);

private Q_SLOTS:

    void slotResetSettings();
    void slotCurrentProfInfo();
    void slotNewProfInfo();
    void slotProfileChanged();

private:

    void readSettings();
    void writeSettings();
    void prepareEffect();
    void prepareFinal();
    void abortPreview();
    void putPreviewData();
    void putFinalData();
    void renderingFinished();
    void updateTransform();

private:

private:

    ProfileConversionToolPriv* const d;
};

}  // namespace DigikamImagesPluginCore

#endif /* PROFILECONVERSIONTOOL_H */
