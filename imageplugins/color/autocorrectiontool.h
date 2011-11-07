/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-31
 * Description : Auto-Color correction tool.
 *
 * Copyright (C) 2005-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef AUTOCORRECTIONTOOL_H
#define AUTOCORRECTIONTOOL_H

// Qt includes

#include <QPixmap>

// Local includes

#include "editortool.h"
#include "dimg.h"

using namespace Digikam;

namespace DigikamColorImagePlugin
{

class AutoCorrectionTool : public EditorToolThreaded
{
    Q_OBJECT

public:

    AutoCorrectionTool(QObject* parent);
    ~AutoCorrectionTool();

private Q_SLOTS:

    void slotInit();
    void slotResetSettings();

private:

    enum AutoCorrectionType
    {
        AutoLevelsCorrection = 0,
        NormalizeCorrection,
        EqualizeCorrection,
        StretchContrastCorrection,
        AutoExposureCorrection
    };

private:

    void writeSettings();
    void readSettings();
    void prepareEffect();
    void prepareFinal();
    void putPreviewData();
    void putFinalData();

    void autoCorrection(DImg* img, DImg* ref, int type);

private:

    class AutoCorrectionToolPriv;
    AutoCorrectionToolPriv* const d;
};

}  // namespace DigikamColorImagePlugin

#endif /* AUTOCORRECTIONTOOL_H */
