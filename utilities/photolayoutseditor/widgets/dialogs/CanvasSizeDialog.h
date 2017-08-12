/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-09-01
 * Description : a plugin to create photo layouts by fusion of several images.
 * 
 *
 * Copyright (C) 2011      by Lukasz Spas <lukasz dot spas at gmail dot com>
 * Copyright (C) 2009-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef CANVAS_SIZE_DIALOG_H
#define CANVAS_SIZE_DIALOG_H

#include "CanvasSize.h"

#include <QDialog>
#include <QSize>

namespace PhotoLayoutsEditor
{
class CanvasSizeDialogPrivate;

class CanvasSizeDialog : public QDialog
{
    Q_OBJECT

public:

    explicit CanvasSizeDialog(QWidget * parent = 0);
    explicit CanvasSizeDialog(const CanvasSize & canvasSize, QWidget * parent = 0);
    ~CanvasSizeDialog();

    Q_PROPERTY(CanvasSize canvas_size READ canvasSize)
    CanvasSize canvasSize() const;

protected Q_SLOTS:

    void recalculatePaperSize(const QString & paperSize);
    void sizeUnitsChanged(const QString & unit);
    void resolutionUnitsChanged(const QString & unit);
    void setHorizontal(bool);
    void setVertical(bool);
    void widthChanged(double width);
    void heightChanged(double height);
    void xResolutionChanged(double xResolution);
    void yResolutionChanged(double yResolution);

private:

    void setupDialog(const QSizeF & size, const QString & sizeUnits, const QSizeF & resolution, const QString & resolutionUnits);
    void prepareSignalsConnections();

    CanvasSizeDialogPrivate * d;
};

} // namespace PhotoLayoutsEditor

#endif // CANVAS_SIZE_DIALOG_H
