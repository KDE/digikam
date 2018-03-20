/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-06
 * Description : Black and White conversion tool.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef BWSEPIATOOL_H
#define BWSEPIATOOL_H

// Local includes

#include "editortool.h"

namespace Digikam
{

class BWSepiaTool : public EditorToolThreaded
{
    Q_OBJECT

public:

    explicit BWSepiaTool(QObject* const parent);
    ~BWSepiaTool();

private Q_SLOTS:

    void slotInit();
    void slotSaveAsSettings();
    void slotLoadSettings();
    void slotResetSettings();

private:

    void readSettings();
    void writeSettings();
    void preparePreview();
    void prepareFinal();
    void setPreviewImage();
    void setFinalImage();
    void slotScaleChanged();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* BWSEPIATOOL_H */
