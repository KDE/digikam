/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-09-06
 * Description : a widget to display camera capture preview.
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef CAPTUREWIDGET_H
#define CAPTUREWIDGET_H

// Qt includes

#include <QWidget>
#include <QByteArray>

// Local includes

#include "digikam_export.h"

class QPixmap;

namespace Digikam
{

class CaptureWidget : public QWidget
{
    Q_OBJECT

public:

    explicit CaptureWidget(QWidget* const parent = 0);
    ~CaptureWidget();

    void setPreview(const QImage& preview);

protected:

    void paintEvent(QPaintEvent*);
    void resizeEvent(QResizeEvent*);

private:

    void updatePixmap();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* CAPTUREWIDGET_H */
