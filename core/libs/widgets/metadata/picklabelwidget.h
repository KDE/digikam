/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-02-14
 * Description : pick label widget
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

#ifndef PICKLABELWIDGET_H
#define PICKLABELWIDGET_H

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

class DIGIKAM_EXPORT PickLabelWidget : public DVBox
{
    Q_OBJECT

public:

    explicit PickLabelWidget(QWidget* const parent=0);
    virtual ~PickLabelWidget();

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
    void setPickLabels(const QList<PickLabel>& list);

    /**
     * Return the list of Color Label buttons turned on or an empty list of none.
     */
    QList<PickLabel> colorLabels() const;

    static QString labelPickName(PickLabel label);

    static QIcon buildIcon(PickLabel label);

Q_SIGNALS:

    void signalPickLabelChanged(int);

protected:

    bool eventFilter(QObject* obj, QEvent* ev);

private:

    void updateDescription(PickLabel label);

private:

    class Private;
    Private* const d;
};

// ------------------------------------------------------------------------------

class DIGIKAM_EXPORT PickLabelSelector : public QPushButton
{
    Q_OBJECT

public:

    explicit PickLabelSelector(QWidget* const parent=0);
    virtual ~PickLabelSelector();

    void setPickLabel(PickLabel label);
    PickLabel colorLabel();

    PickLabelWidget* pickLabelWidget() const;

Q_SIGNALS:

    void signalPickLabelChanged(int);

private Q_SLOTS:

    void slotPickLabelChanged(int);

private:

    class Private;
    Private* const d;
};

// ------------------------------------------------------------------------------

class DIGIKAM_EXPORT PickLabelMenuAction : public QMenu
{
    Q_OBJECT

public:

    explicit PickLabelMenuAction(QMenu* const parent=0);
    virtual ~PickLabelMenuAction();

Q_SIGNALS:

    void signalPickLabelChanged(int);
};

} // namespace Digikam

Q_DECLARE_METATYPE(QList<Digikam::PickLabel>)

#endif // PICKLABELWIDGET_H
