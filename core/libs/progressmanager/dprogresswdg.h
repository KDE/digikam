/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-26
 * Description : a progress bar with information dispatched to progress manager
 *
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DPROGRESS_WDG_H
#define DPROGRESS_WDG_H

// Qt includes

#include <QProgressBar>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DProgressWdg : public QProgressBar
{
    Q_OBJECT

public:

    DProgressWdg(QWidget* const parent);
    ~DProgressWdg();

    /** Call this method to start a new instance of progress notification into progress manager
     *  You can pass title string to name progress item, and set it as cancelable. In this case,
     *  signalProgressCanceled() is fired when user press cancel button from progress manager.
     *  This item can also accept a thumbnail that you can change through progresssThumbnailChanged().
     */
    void progressScheduled(const QString& title, bool canBeCanceled, bool hasThumb);

    /** Change thumbnail in progress manager
     */
    void progressThumbnailChanged(const QPixmap& thumb);

    /** Change status string in progress manager
     */
    void progressStatusChanged(const QString& status);

    /** Call this method to query progress manager that process is done.
     */
    void progressCompleted();

Q_SIGNALS:

    /** Fired when user press cancel button from progress manager.
     */
    void signalProgressCanceled();

private Q_SLOTS:

    void slotValueChanged(int);
    void slotProgressCanceled(const QString& id);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DPROGRESS_WDG_H
