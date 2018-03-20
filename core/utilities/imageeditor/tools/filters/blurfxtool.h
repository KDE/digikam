/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-09
 * Description : a tool to apply Blur FX to images
 *
 * Copyright 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef BLURFXTOOL_H
#define BLURFXTOOL_H

// Local includes

#include "editortool.h"

namespace Digikam
{

class BlurFXTool : public EditorToolThreaded
{
    Q_OBJECT

public:

    explicit BlurFXTool(QObject* const parent);
    ~BlurFXTool();

private Q_SLOTS:

    void slotEffectTypeChanged(int type);
    void slotResetSettings();

private:

    void readSettings();
    void writeSettings();
    void preparePreview();
    void prepareFinal();
    void setPreviewImage();
    void setFinalImage();
    void renderingFinished();
    void blockWidgetSignals(bool b);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* BLURFXTOOL_H */
