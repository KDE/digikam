/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-23-03
 * Description : A tool tip widget which follows cursor movements.
 *               Tool tip content is displayed without delay.
 *
 * Copyright (C) 2007-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C)      2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

// Qt includes.

#include <QtCore/QEvent>
#include <QtCore/QString>
#include <QtGui/QLabel>

// Local includes.

#include "digikam_export.h"

class QTimer;

namespace Digikam
{

class DCursorTrackerPriv;

/**
 * This class implements a decoration-less window which will follow the cursor
 * when it's over a specified widget.
 */
class DIGIKAM_EXPORT DCursorTracker : public QLabel
{
    Q_OBJECT

public:

    DCursorTracker(const QString& txt, QWidget *parent);
    ~DCursorTracker();

    void setText(const QString& txt);
    void setEnable(bool b);
    void setKeepOpen(bool b);

    void triggerAutoShow(int timeout = 2000);
    void refresh();

protected:

    bool eventFilter(QObject*, QEvent*);

private Q_SLOTS:

    void slotAutoHide();

private:

    void moveToParent(QWidget* parent);

private:

    DCursorTrackerPriv* const d;
};


/**
 * A specialized CursorTracker class, which looks like a tool tip.
 */
class DIGIKAM_EXPORT DTipTracker : public DCursorTracker
{

public:

    DTipTracker(const QString& txt, QWidget *parent);
};

} // namespace Digikam

#endif /* DCURSOR_TRACKER_H */
