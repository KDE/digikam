/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-28
 * Description : a digiKam image editor plugin to process image
 *               free rotation.
 *
 * Copyright (C) 2004-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009      by Andi Clemens <andi dot clemens at gmx dot net>
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

#ifndef FREEROTATIONTOOL_H
#define FREEROTATIONTOOL_H

// Local includes

#include "editortool.h"

class QPixmap;
class QString;

namespace DigikamFreeRotationImagesPlugin
{

class FreeRotationToolPriv;

class FreeRotationTool : public Digikam::EditorToolThreaded
{
    Q_OBJECT

public:

    FreeRotationTool(QObject *parent);
    ~FreeRotationTool();

private Q_SLOTS:

    void slotResetSettings();
    void slotColorGuideChanged();

    void slotAutoAdjustP1Clicked();
    void slotAutoAdjustP2Clicked();
    void slotAutoAdjustClicked();

private:

    void readSettings();
    void writeSettings();
    void prepareEffect();
    void prepareFinal();
    void putPreviewData();
    void putFinalData();
    void renderingFinished();

    double  calculateAngle(const QPoint& p1, const QPoint& p2);
    double  calculateAutoAngle();
    QPixmap generateBtnPixmap(const QString& label, const QColor& color);

    bool    pointIsValid(const QPoint& p);
    void    setPointInvalid(QPoint& p);
    void    updatePoints();
    void    resetPoints();
    QString generatePointLabel(const QPoint& p);

private:

    FreeRotationToolPriv* const d;
};

}  // namespace DigikamFreeRotationImagesPlugin

#endif /* FREEROTATIONTOOL_H */
