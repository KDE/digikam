/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-10-10
 * Description : Lut3D color adjustment tool.
 *
 * Copyright (C) 2015 by Andrej Krutak <dev at andree dot sk>
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

#ifndef LUT3DTOOL_H
#define LUT3DTOOL_H

// Qt includes

#include <QPixmap>

// Local includes

#include "lut3dcontainer.h"
#include "editortool.h"
#include "dimg.h"

using namespace Digikam;

namespace DigikamColorImagePlugin
{

class Lut3DTool : public EditorToolThreaded
{
    Q_OBJECT

public:

    explicit Lut3DTool(QObject* const parent);
    ~Lut3DTool();

private Q_SLOTS:

    void slotInit();
    void slotResetSettings();

private:

    void writeSettings();
    void readSettings();
    void preparePreview();
    void prepareFinal();
    void setPreviewImage();
    void setFinalImage();

    void applyCorrection(DImg* const img, const Lut3DContainer& settings);

private:

    class Private;
    Private* const d;
};

}  // namespace DigikamColorImagePlugin

#endif /* LUT3DTOOL_H */
