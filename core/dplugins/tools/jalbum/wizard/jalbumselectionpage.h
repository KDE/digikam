/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate jAlbum image galleries
 *
 * Copyright (C) 2013-2019 by Andrew Goodbody <ajg zero two at elfringham dot co dot uk>
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

#ifndef DIGIKAM_JALBUM_SELECTION_PAGE_H
#define DIGIKAM_JALBUM_SELECTION_PAGE_H

// Qt includes

#include <QList>
#include <QUrl>
#include <QString>

// Local includes

#include "dwizardpage.h"

namespace Digikam
{

class JAlbumSelectionPage : public DWizardPage
{
public:

    explicit JAlbumSelectionPage(QWizard* const dialog, const QString& title);
    ~JAlbumSelectionPage();

    void initializePage();
    bool validatePage();
    bool isComplete() const;

    void setItemsList(const QList<QUrl>& urls);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_JALBUM_SELECTION_PAGE_H
