/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-01-20
 * Description : a digiKam image plugin to add a border
 *               around an image.
 *
 * Copyright 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright 2006-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef BORDERTOOL_H
#define BORDERTOOL_H

// Qt includes.

#include <qstring.h>

// Digikam includes.

#include "editortool.h"

class QLabel;
class QCheckBox;
class QColor;

class KColorButton;

namespace KDcrawIface
{
class RIntNumInput;
class RComboBox;
}

namespace Digikam
{
class EditorToolSettings;
class ImageWidget;
}

namespace DigikamBorderImagesPlugin
{

class BorderTool : public Digikam::EditorToolThreaded
{
    Q_OBJECT

public:

    BorderTool(QObject *parent);
    ~BorderTool();

private:

    QString getBorderPath(int border);

private slots:

    void slotPreserveAspectRatioToggled(bool);
    void slotBorderTypeChanged(int borderType);
    void slotColorForegroundChanged(const QColor &color);
    void slotColorBackgroundChanged(const QColor &color);
    void slotResetSettings();

private:

    void writeSettings();
    void readSettings();
    void prepareEffect();
    void prepareFinal();
    void putPreviewData();
    void putFinalData();
    void renderingFinished();
    void toggleBorderSlider(bool b);

private:

    QLabel                      *m_labelBorderPercent;
    QLabel                      *m_labelBorderWidth;
    QLabel                      *m_labelForeground;
    QLabel                      *m_labelBackground;

    QCheckBox                   *m_preserveAspectRatio;

    QColor                       m_solidColor;
    QColor                       m_niepceBorderColor;
    QColor                       m_niepceLineColor;
    QColor                       m_bevelUpperLeftColor;
    QColor                       m_bevelLowerRightColor;
    QColor                       m_decorativeFirstColor;
    QColor                       m_decorativeSecondColor;

    KDcrawIface::RComboBox      *m_borderType;

    KDcrawIface::RIntNumInput   *m_borderPercent;
    KDcrawIface::RIntNumInput   *m_borderWidth;

    KColorButton                *m_firstColorButton;
    KColorButton                *m_secondColorButton;

    Digikam::ImageWidget        *m_previewWidget;

    Digikam::EditorToolSettings *m_gboxSettings;
};

}  // NameSpace DigikamBorderImagesPlugin

#endif /* BORDERTOOL_H */
