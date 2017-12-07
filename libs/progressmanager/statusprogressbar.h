/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-01-24
 * Description : a progress bar used to display action
 *               progress or a text in status bar.
 *               Progress events are dispatched to ProgressManager.
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

#ifndef STATUS_PROGRESS_BAR_H
#define STATUS_PROGRESS_BAR_H

// Qt includes

#include <QStackedWidget>
#include <QString>

// Local includes

#include "digikam_export.h"

namespace Digikam
{
class ProgressItem;

class DIGIKAM_EXPORT StatusProgressBar : public QStackedWidget
{
    Q_OBJECT

public:

    enum StatusProgressBarMode
    {
        TextMode=0,
        ProgressBarMode,
        CancelProgressBarMode
    };

public:

    explicit StatusProgressBar(QWidget* const parent=0);
    ~StatusProgressBar();

    void setAlignment(Qt::Alignment a);

    void setProgressBarMode(int mode, const QString& text=QString());

    int  progressValue() const;

    int  progressTotalSteps() const;
    void setProgressTotalSteps(int v);

    void setNotify(bool b);
    void setNotificationTitle(const QString& title, const QIcon& icon);

public Q_SLOTS:

    void setText(const QString& text);
    void setProgressValue(int v);
    void setProgressText(const QString& text);

Q_SIGNALS:

    void signalCancelButtonPressed();

private:

    ProgressItem* currentProgressItem() const;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // STATUS_PROGRESS_BAR_H
