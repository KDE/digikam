/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-01-28
 * Description : color label widget
 *
 * Copyright (C) 2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef COLORLABELWIDGET_H
#define COLORLABELWIDGET_H

// Qt includes

#include <QColor>
#include <QPushButton>

// KDE includes

#include <khbox.h>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT ColorLabelWidget : public KHBox
{
    Q_OBJECT

public:

    enum ColorLabel
    {
        NoneLabel=0,
        RedLabel,
        OrangeLabel,
        YellowLabel,
        GreenLabel,
        BlueLabel,
        MagentaLabel,
        GrayLabel,
        BlackLabel,
        WhiteLabel
    };

public:

    ColorLabelWidget(QWidget* parent=0);
    ~ColorLabelWidget();

    void setColorLabel(ColorLabel label);
    ColorLabel colorLabel();

    QColor labelColor(ColorLabel label) const;

    static QString labelColorName(ColorLabel label);

Q_SIGNALS:

    void signalColorLabelChanged(int);

private:

    QIcon buildIcon(ColorLabel label) const;

private:

    class ColorLabelWidgetPriv;
    ColorLabelWidgetPriv* const d;
};

// ------------------------------------------------------------------------------

class DIGIKAM_EXPORT ColorLabelSelector : public QPushButton
{
    Q_OBJECT

public:

    ColorLabelSelector(QWidget* parent=0);
    ~ColorLabelSelector();

    void setColorLabel(ColorLabelWidget::ColorLabel label);
    ColorLabelWidget::ColorLabel colorLabel();

private Q_SLOTS:

    void slotColorLabelChanged(int);

private:

    class ColorLabelSelectorPriv;
    ColorLabelSelectorPriv* const d;
};

}  // namespace Digikam

#endif // COLORLABELWIDGET_H
