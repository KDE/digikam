/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-23-03
 * Description : A tool tip widget which follows cursor movements.
 *               Tool tip content is displayed without delay.
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2010 by Andi Clemens <andi dot clemens at gmail dot com>
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

#ifndef DCURSOR_TRACKER_H
#define DCURSOR_TRACKER_H

// Qt includes

#include <QEvent>
#include <QString>
#include <QLabel>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

/**
 * This class implements a window which looks like a tool tip. It will follow the cursor
 * when it's over a specified widget.
 */
class DIGIKAM_EXPORT DCursorTracker : public QLabel
{
    Q_OBJECT

public:

    explicit DCursorTracker(const QString& txt, QWidget* const parent, Qt::Alignment align = Qt::AlignCenter);
    ~DCursorTracker();

    void setText(const QString& txt);
    void setEnable(bool b);
    void setKeepOpen(bool b);
    void setTrackerAlignment(Qt::Alignment alignment);

    void triggerAutoShow(int timeout = 2000);
    void refresh();

protected:

    bool eventFilter(QObject*, QEvent*);
    void paintEvent(QPaintEvent*);

private Q_SLOTS:

    void slotAutoHide();

private:

    void moveToParent(QWidget* const parent);
    bool canBeDisplayed();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DCURSOR_TRACKER_H
