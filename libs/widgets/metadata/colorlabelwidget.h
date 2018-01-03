/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-01-28
 * Description : color label widget
 *
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <QEvent>
#include <QList>
#include <QMetaType>
#include <QMenu>

// Local includes

#include "dlayoutbox.h"
#include "digikam_globals.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT ColorLabelWidget : public DVBox
{
    Q_OBJECT

public:

    explicit ColorLabelWidget(QWidget* const parent=0);
    virtual ~ColorLabelWidget();

    /**
     * Show or not on the bottom view the description of label with shorcuts.
     */
    void setDescriptionBoxVisible(bool b);

    /**
     * Set all Color Label buttons exclusive or not. Default is true as only one can be selected.
     * Non-exclusive mode is dedicated for Advanced Search tool.
     */
    void setButtonsExclusive(bool b);

    /**
     * Turn on Color Label buttons using list. Pass an empty list to clear all selection.
     */
    void setColorLabels(const QList<ColorLabel>& list);

    /**
     * Return the list of Color Label buttons turned on or an empty list of none.
     */
    QList<ColorLabel> colorLabels() const;

    static QColor  labelColor(ColorLabel label);
    static QString labelColorName(ColorLabel label);

    static QIcon buildIcon(ColorLabel label, int size=12);

Q_SIGNALS:

    void signalColorLabelChanged(int);

protected:

    bool eventFilter(QObject* obj, QEvent* ev);

private:

    void updateDescription(ColorLabel label);

private:

    class Private;
    Private* const d;
};

// ------------------------------------------------------------------------------

class DIGIKAM_EXPORT ColorLabelSelector : public QPushButton
{
    Q_OBJECT

public:

    explicit ColorLabelSelector(QWidget* const parent=0);
    virtual ~ColorLabelSelector();

    void setColorLabel(ColorLabel label);
    ColorLabel colorLabel();

    ColorLabelWidget* colorLabelWidget() const;

Q_SIGNALS:

    void signalColorLabelChanged(int);

private Q_SLOTS:

    void slotColorLabelChanged(int);

private:

    class Private;
    Private* const d;
};

// ------------------------------------------------------------------------------

class DIGIKAM_EXPORT ColorLabelMenuAction : public QMenu
{
    Q_OBJECT

public:

    explicit ColorLabelMenuAction(QMenu* const parent=0);
    virtual ~ColorLabelMenuAction();

Q_SIGNALS:

    void signalColorLabelChanged(int);
};

} // namespace Digikam

Q_DECLARE_METATYPE(QList<Digikam::ColorLabel>)

#endif // COLORLABELWIDGET_H
