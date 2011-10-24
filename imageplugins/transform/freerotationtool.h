/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-28
 * Description : a digiKam image editor plugin to process image
 *               free rotation.
 *
 * Copyright (C) 2004-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2010 by Andi Clemens <andi dot clemens at googlemail dot com>
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

using namespace Digikam;

namespace DigikamTransformImagePlugin
{

class FreeRotationTool : public EditorToolThreaded
{
    Q_OBJECT

public:

    FreeRotationTool(QObject* parent);
    ~FreeRotationTool();

public Q_SLOTS:

    void slotAutoAdjustP1Clicked();
    void slotAutoAdjustP2Clicked();
    void slotAutoAdjustClicked();

private Q_SLOTS:

    void slotResetSettings();
    void slotColorGuideChanged();

private:

    void readSettings();
    void writeSettings();
    void prepareEffect();
    void prepareFinal();
    void putPreviewData();
    void putFinalData();

    double  calculateAutoAngle();
    QPixmap generateBtnPixmap(const QString& label, const QColor& color);

    bool    pointIsValid(const QPoint& p);
    void    setPointInvalid(QPoint& p);
    void    updatePoints();
    void    resetPoints();
    QString generateButtonLabel(const QPoint& p);

    QString centerString(const QString& str, int maxLength = -1);
    QString repeatString(const QString& str, int times);

private:

    class FreeRotationToolPriv;
    FreeRotationToolPriv* const d;
};

}  // namespace DigikamTransformImagePlugin

#endif /* FREEROTATIONTOOL_H */
