/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2006-01-24
 * Description : a progress bar used to display file access
 *               progress or a text in status bar.
 *
 * Copyright 2007 by Gilles Caulier
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

#ifndef STATUSPROGRESSBAR_H
#define STATUSPROGRESSBAR_H

// KDE includes.

#include <qwidgetstack.h>
#include <qstring.h>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class StatusProgressBarPriv;

class DIGIKAM_EXPORT StatusProgressBar : public QWidgetStack
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

    StatusProgressBar( QWidget *parent=0 );
    ~StatusProgressBar();

    void setText(const QString& text);
    void setAlignment(int a);

    void progressBarMode(int mode, const QString& text=QString::null);
    void setProgressValue(int v);
    void setProgressText(const QString& text);

signals:

    void signalCancelButtonPressed();

private:

    StatusProgressBarPriv* d;
};

}  // namespace Digikam

#endif /* IOFILEPROGRESSBAR_H */
