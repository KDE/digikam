/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-02-01
 * Description : album view configuration setup tab
 *
 * Copyright (C) 2003-2004 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SETUPALBUMVIEW_H
#define SETUPALBUMVIEW_H

// Qt includes

#include <QScrollArea>

namespace Digikam
{

class SetupAlbumView : public QScrollArea
{
    Q_OBJECT

public:

    enum AlbumTab
    {
        IconView = 0,
        FolderView,
        Preview,
        FullScreen,
        MimeType,
        Category
    };

public:

    explicit SetupAlbumView(QWidget* const parent = 0);
    virtual ~SetupAlbumView();

    void applySettings();

    bool useLargeThumbsHasChanged() const;

private:

    void readSettings();

private Q_SLOTS:

    void slotUseLargeThumbsToggled(bool);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // SETUPALBUMVIEW_H
