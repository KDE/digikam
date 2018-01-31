/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : a digiKam image tool to reduce
 *               vignetting on an image.
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef ANTIVIGNETTINGTOOL_H
#define ANTIVIGNETTINGTOOL_H

// Local includes

#include "editortool.h"

namespace Digikam
{

class AntiVignettingTool : public EditorToolThreaded
{
    Q_OBJECT

public:

    explicit AntiVignettingTool(QObject* const parent);
    ~AntiVignettingTool();

private Q_SLOTS:

    void slotResetSettings();

private:

    void writeSettings();
    void readSettings();
    void preparePreview();
    void prepareFinal();
    void setPreviewImage();
    void setFinalImage();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* ANTIVIGNETTINGTOOL_H */
