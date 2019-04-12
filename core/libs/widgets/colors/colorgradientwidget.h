/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-07-28
 * Description : a color gradient widget
 *
 * Copyright (C) 2004-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_COLOR_GRADIENT_WIDGET_H
#define DIGIKAM_COLOR_GRADIENT_WIDGET_H

// KDE includes

#include <QWidget>
#include <QColor>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT ColorGradientWidget : public QWidget
{
    Q_OBJECT

public:

    explicit ColorGradientWidget(Qt::Orientation orientation, int size, QWidget* const parent=nullptr);
    virtual ~ColorGradientWidget();

    void setColors(const QColor& col1, const QColor& col2);

protected:

    void paintEvent(QPaintEvent*) override;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_COLOR_GRADIENT_WIDGET_H
