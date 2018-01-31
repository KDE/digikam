/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-17
 * Description : a tool to change image perspective .
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

#ifndef PERSPECTIVETOOL_H
#define PERSPECTIVETOOL_H

// Local includes

#include "editortool.h"

class QRect;

namespace Digikam
{

class PerspectiveTool : public EditorTool
{
    Q_OBJECT

public:

    explicit PerspectiveTool(QObject* const parent);
    ~PerspectiveTool();

private Q_SLOTS:

    void slotInverseTransformationChanged(bool b);
    void slotResetSettings();
    void slotUpdateInfo(const QRect& newSize, float topLeftAngle, float topRightAngle,
                        float bottomLeftAngle, float bottomRightAngle, bool valid);
    void slotColorGuideChanged();

private:

    void readSettings();
    void writeSettings();
    void finalRendering();
    void setBackgroundColor(const QColor& bg);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* PERSPECTIVETOOL_H */
