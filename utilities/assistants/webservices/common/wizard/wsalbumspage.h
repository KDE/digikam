/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-06-27
 * Description : a tool to export items to web services.
 *
 * Copyright (C) 2017-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef WS_ALBUMS_PAGE_H
#define WS_ALBUMS_PAGE_H

// Qt includes

#include <QList>
#include <QUrl>
#include <QString>

// Local includes

#include "dwizardpage.h"

namespace Digikam
{

class WSAlbumsPage : public DWizardPage
{
public:

    explicit WSAlbumsPage(QWizard* const dialog, const QString& title);
    ~WSAlbumsPage();

    bool validatePage();
    bool isComplete() const;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // WS_ALBUMS_PAGE_H
