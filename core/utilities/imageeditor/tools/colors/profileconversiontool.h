/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-19
 * Description : a tool for color space conversion
 *
 * Copyright (C) 2009-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include "iccprofile.h"

namespace Digikam
{

class ProfileConversionTool : public EditorToolThreaded
{
    Q_OBJECT

public:

    explicit ProfileConversionTool(QObject* const parent);
    ~ProfileConversionTool();

    static QStringList favoriteProfiles();
    static void fastConversion(const IccProfile& profile);

private Q_SLOTS:

    void slotResetSettings();
    void slotCurrentProfInfo();
    void slotProfileChanged();

private:

    void readSettings();
    void writeSettings();
    void preparePreview();
    void prepareFinal();
    void setPreviewImage();
    void setFinalImage();

    void updateTransform();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* PROFILECONVERSIONTOOL_H */
