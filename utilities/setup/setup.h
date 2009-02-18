/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-02-03
 * Description : digiKam setup dialog.
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi at pooh.tam.uiuc.edu>
 * Copyright (C) 2003-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SETUP_H
#define SETUP_H

// KDE includes.

#include <kpagedialog.h>

namespace Digikam
{

class SetupPlugins;
class SetupPrivate;

class Setup : public KPageDialog 
{
    Q_OBJECT

public:

    enum Page 
    {
        LastPageUsed = -1,
        CollectionsPage = 0,
        AlbumViewPage,
        CategoryPage,
        ToolTipPage,
        MetadataPage,
        IdentifyPage,
        MimePage,
        LightTablePage,
        QueuePage,
        EditorPage,
        DcrawPage,
        IOFilesPage,
        SlideshowPage,
        ICCPage,
        KipiPluginsPage,
        CameraPage,
        MiscellaneousPage
    };

    Setup(QWidget* parent=0, Page page=LastPageUsed);
    ~Setup();

    SetupPlugins *kipiPluginsPage();

private slots:

    void slotOkClicked();

private:

    Setup::Page activePageIndex();
    void showPage(Setup::Page page);

private:

    SetupPrivate* const d;
};

}  // namespace Digikam

#endif  // SETUP_H
