/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-09-30
 * Description : a tool to add rain drop over an image
 *
 * Copyright (C) 2004-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef RAINDROPTOOL_H
#define RAINDROPTOOL_H

// Local includes

#include "editortool.h"

namespace Digikam
{

class RainDropTool : public EditorToolThreaded
{
    Q_OBJECT

public:

    explicit RainDropTool(QObject* const parent);
    ~RainDropTool();

private Q_SLOTS:

    void slotResetSettings();

private:

    void readSettings();
    void writeSettings();
    void preparePreview();
    void prepareFinal();
    void setPreviewImage();
    void setFinalImage();

    void blockWidgetSignals(bool b);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* RAINDROPTOOL_H */
