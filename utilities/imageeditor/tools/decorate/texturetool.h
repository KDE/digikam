/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-10
 * Description : a tool to apply texture over an image
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef TEXTURETOOL_H
#define TEXTURETOOL_H

// Qt includes

#include <QString>

// Local includes

#include "editortool.h"

namespace Digikam
{

class TextureTool : public EditorToolThreaded
{
    Q_OBJECT

public:

    explicit TextureTool(QObject* const parent);
    ~TextureTool();

private:

    QString getTexturePath(int texture);

private Q_SLOTS:

    void slotResetSettings();

private:

    void readSettings();
    void writeSettings();
    void preparePreview();
    void prepareFinal();
    void setPreviewImage();
    void setFinalImage();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* TEXTURETOOL_H */
