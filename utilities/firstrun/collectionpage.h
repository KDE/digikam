/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-28-04
 * Description : first run assistant dialog
 *
 * Copyright (C) 2009-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef COLLECTION_PAGE_H
#define COLLECTION_PAGE_H

// Qt includes

#include <QString>

// KDE includes

#include <kurl.h>

// Local includes

#include "assistantdlgpage.h"

namespace Digikam
{

class CollectionPage : public AssistantDlgPage
{
    Q_OBJECT

public:

    explicit CollectionPage(KAssistantDialog* const dlg);
    ~CollectionPage();

    bool checkSettings();
    void saveSettings();

    QString databasePath() const;
    QString firstAlbumPath() const;

private Q_SLOTS:

    void slotAlbumRootChanged(const KUrl& url);
    void slotDbPathChanged(const KUrl& url);

private:

    bool checkRootAlbum(QString& rootAlbumFolder);
    bool checkDatabase(QString& dbFolder);

private:

    class Private;
    Private* const d;
};

}   // namespace Digikam

#endif /* COLLECTION_PAGE_H */
