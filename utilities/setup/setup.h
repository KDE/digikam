/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-02-03
 * Description : digiKam setup dialog.
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi at pooh.tam.uiuc.edu>
 * Copyright (C) 2003-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// KDE includes

#include <kpagedialog.h>

// Local includes

#include "setuptemplate.h"
#include "template.h"

namespace Digikam
{

class SetupPlugins;

class Setup : public KPageDialog
{
    Q_OBJECT

public:

    enum Page
    {
        LastPageUsed    = -1,

        DatabasePage    = 0,
        CollectionsPage,
        AlbumViewPage,
        CategoryPage,
        ToolTipPage,
        FaceTagsPage,
        MetadataPage,
        TemplatePage,
        MimePage,
        EditorPage,
        VersioningPage,
        DcrawPage,
        IOFilesPage,
        ICCPage,
        LightTablePage,
        SlideshowPage,
        CameraPage,
        KipiPluginsPage,
        ScriptManagerPage,
        MiscellaneousPage,

        SetupPageEnumLast
    };

    /** Show a setup dialog. The specified page will be selected.
        True is returned if the dialog was closed with Ok.
     */
    static bool exec(Page page=LastPageUsed);
    static bool exec(QWidget* parent, Page page=LastPageUsed);
    /** Show a setup dialog. Only the specified page will be available.
     */
    static bool execSinglePage(Page page);
    static bool execSinglePage(QWidget* parent, Page page);

    static bool execTemplateEditor(QWidget* parent, const Template& t);
    void setTemplate(const Template& t);

    QSize sizeHint() const;

private Q_SLOTS:

    void slotOkClicked();

private:

    Setup(QWidget* parent=0);
    ~Setup();

    Setup::Page activePageIndex();
    void showPage(Setup::Page page);

private:

    class SetupPrivate;
    SetupPrivate* const d;
};

}  // namespace Digikam

#endif  // SETUP_H
