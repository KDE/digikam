/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-17
 * Description : a plugin to change image perspective .
 *
 * Copyright (C) 2005-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <QRect>

// Local includes.

#include "editortool.h"

class QLabel;
class QCheckBox;
class QSpinBox;

class KColorButton;

namespace Digikam
{
class EditorToolSettings;
}

namespace DigikamPerspectiveImagesPlugin
{

class PerspectiveWidget;

class PerspectiveTool : public Digikam::EditorTool
{
    Q_OBJECT

public:

    PerspectiveTool(QObject* parent);
    ~PerspectiveTool();

private slots:

    void slotResetSettings();
    void slotUpdateInfo(QRect newSize, float topLeftAngle, float topRightAngle,
                        float bottomLeftAngle, float bottomRightAngle);

private:

    void readSettings();
    void writeSettings();
    void finalRendering();

private:

    QLabel                      *m_newWidthLabel;
    QLabel                      *m_newHeightLabel;
    QLabel                      *m_topLeftAngleLabel;
    QLabel                      *m_topRightAngleLabel;
    QLabel                      *m_bottomLeftAngleLabel;
    QLabel                      *m_bottomRightAngleLabel;

    QCheckBox                   *m_drawWhileMovingCheckBox;
    QCheckBox                   *m_drawGridCheckBox;
    QCheckBox                   *m_inverseTransformation;

    QSpinBox                    *m_guideSize;

    KColorButton                *m_guideColorBt;

    PerspectiveWidget           *m_previewWidget;

    Digikam::EditorToolSettings *m_gboxSettings;
};

}  // namespace DigikamPerspectiveImagesPlugin

#endif /* PERSPECTIVETOOL_H */
