/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-06-15
 * Description : a tool to replace part of the image using another
 *
 * Copyright (C) 2004-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2017      by Shaza Ismail Kaoud <shaza dot ismail dot k at gmail dot com>
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

#ifndef HEALINGCLONETOOL_H
#define HEALINGCLONETOOL_H

// Local includes

#include "editortool.h"

namespace Digikam
{

class HealingCloneTool : public EditorToolThreaded
{
    Q_OBJECT

public:
    explicit HealingCloneTool(QObject* const parent);
    ~HealingCloneTool();

private Q_SLOTS:

    void slotResetSettings();
    //void slotSrcSet();

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

} // namespace Digikam

#endif /* HEALINGCLONETOOL_H */
