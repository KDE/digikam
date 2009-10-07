/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-09
 * Description : a plugin to enhance image with local contrasts (as human eye does).
 *
 * Copyright (C) 2009 by Julien Pontabry <julien dot pontabry at gmail dot com>
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

#ifndef LOCALCONTRASTTOOL_H
#define LOCALCONTRASTTOOL_H

// Local includes

#include "editortool.h"
#include "ToneMappingParameters.h"

namespace DigikamLocalContrastImagesPlugin
{

class LocalContrastToolPriv;

class LocalContrastTool : public Digikam::EditorToolThreaded
{
    Q_OBJECT

public:

    LocalContrastTool(QObject* parent);
    ~LocalContrastTool();

private:

    void readSettings();
    void writeSettings();
    ToneMappingParameters *createParams();
    void prepareEffect();
    void prepareFinal();
    void putPreviewData();
    void putFinalData();
    void renderingFinished();

private Q_SLOTS:

//     void slotSaveAsSettings();
//     void slotLoadSettings();
    void slotResetSettings();
    void slotStage1Enabled(bool);
    void slotStage2Enabled(bool);
    void slotStage3Enabled(bool);
    void slotStage4Enabled(bool);

private:

    LocalContrastToolPriv* const d;
};

}  // namespace DigikamLocalContrastImagesPlugin

#endif /* LOCALCONTRASTTOOL_H */
