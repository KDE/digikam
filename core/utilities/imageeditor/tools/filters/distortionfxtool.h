/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-11
 * Description : a tool to apply Distortion FX to an image.
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * Original Distortion algorithms copyrighted 2004-2005 by
 * Pieter Z. Voloshyn <pieter dot voloshyn at gmail dot com>.
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

#ifndef DISTORTIONFXTOOL_H
#define DISTORTIONFXTOOL_H

// Local includes

#include "editortool.h"

namespace Digikam
{

class DistortionFXTool : public EditorToolThreaded
{
    Q_OBJECT

public:

    explicit DistortionFXTool(QObject* const parent);
    ~DistortionFXTool();

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

#endif /* DISTORTIONFXTOOL_H */
