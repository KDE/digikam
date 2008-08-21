/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-12-20
 * Description : a view to display a welcome page
 *               on root album.
 *
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef WELCOMEPAGEVIEW_H
#define WELCOMEPAGEVIEW_H

// Qt includes.

#include <qstring.h>

// KDE includes.

#include <khtml_part.h>

// Local includes.

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT WelcomePageView : public KHTMLPart
{
    Q_OBJECT

public:

    WelcomePageView(QWidget* parent);
    ~WelcomePageView();

private:

    QCString fileToString(const QString &aFileName);
    QString  infoPage();

private slots:

    void slotUrlOpen(const KURL &);
};

}  // namespace Digikam

#endif /* WELCOMEPAGEVIEW_H */
