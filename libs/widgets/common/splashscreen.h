/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-02-10
 * Description : a widget to display splash with progress bar
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

// Qt includes

#include <QtGui/QPainter>

// KDE includes

#include <ksplashscreen.h>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT SplashScreen : public KSplashScreen
{
    Q_OBJECT

public:

    SplashScreen();
    virtual ~SplashScreen();

    void setAlignment(int alignment);
    void setColor(const QColor& color);

protected:

    void drawContents(QPainter*);

public Q_SLOTS:

    void animate();
    void message(const QString& message);

private:

    class Private;
    Private* const d;
};

}   // namespace Digikam

#endif /* SPLASHSCREEN_H */
