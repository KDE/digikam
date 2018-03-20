/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-02-10
 * Description : a widget to display splash with progress bar
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DSPLASH_SCREEN_H
#define DSPLASH_SCREEN_H

// Qt includes

#include <QPainter>
#include <QSplashScreen>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DSplashScreen : public QSplashScreen
{
    Q_OBJECT

public:

    explicit DSplashScreen();
    ~DSplashScreen();

    void setAlignment(int alignment);
    void setColor(const QColor& color);
    void setMessage(const QString& message);

protected:

    void drawContents(QPainter*);

private Q_SLOTS:

    void slotAnimate();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DSPLASH_SCREEN_H
