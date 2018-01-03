/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-09
 * Description : a tool to sharp an image
 *
 * Copyright (C) 2004-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SHARPENTOOL_H
#define SHARPENTOOL_H

// Local includes

#include "editortool.h"

namespace Digikam
{

class SharpenTool : public EditorToolThreaded
{
    Q_OBJECT

public:

    explicit SharpenTool(QObject* const parent);
    virtual ~SharpenTool();

private Q_SLOTS:

    void slotSaveAsSettings();
    void slotLoadSettings();
    void slotResetSettings();
    void slotSettingsChanged();

private:

    void readSettings();
    void writeSettings();
    void preparePreview();
    void prepareFinal();
    void setPreviewImage();
    void setFinalImage();
    void renderingFinished();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // SHARPENTOOL_H
