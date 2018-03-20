/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-13
 * Description : progress manager
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2004      by Till Adam <adam at kde dot org>
 * Copyright (C) 2004      by David Faure <faure at kde dot org>
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

#ifndef STATUS_BAR_PROGRESS_WIDGET_H
#define STATUS_BAR_PROGRESS_WIDGET_H

// Qt includes

#include <QFrame>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class ProgressItem;
class ProgressView;

class DIGIKAM_EXPORT StatusbarProgressWidget : public QFrame
{
    Q_OBJECT

public:

    StatusbarProgressWidget(ProgressView* const progressView, QWidget* const parent, bool button = true);
    ~StatusbarProgressWidget();

public Q_SLOTS:

    void slotClean();

    void slotProgressItemAdded(ProgressItem* i);
    void slotProgressItemCompleted(ProgressItem* i);
    void slotProgressItemProgress(ProgressItem* i, unsigned int value);

protected Q_SLOTS:

    void slotProgressViewVisible(bool);
    void slotShowItemDelayed();
    void slotBusyIndicator();
    void updateBusyMode();

protected:

    void setMode();
    void connectSingleItem();
    void activateSingleItemMode();

    virtual bool eventFilter(QObject*, QEvent*);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // STATUS_BAR_PROGRESS_WIDGET_H
