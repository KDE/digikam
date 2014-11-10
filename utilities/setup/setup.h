/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-02-03
 * Description : digiKam setup dialog.
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2003-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "config-digikam.h"
#include "setuptemplate.h"
#include "template.h"
#include "searchtextbar.h"

namespace Digikam
{

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
        RawPage,
        IOFilesPage,
        ICCPage,
        LightTablePage,
        SlideshowPage,
        ImageQualityPage,
        CameraPage,

#ifdef HAVE_KIPI
        KipiPluginsPage,
#endif /* HAVE_KIPI */

        MiscellaneousPage,
        SetupPageEnumLast
    };

    /** Show a setup dialog. The specified page will be selected.
        True is returned if the dialog was closed with Ok.
     */
    static bool exec(Page page = LastPageUsed);
    static bool exec(QWidget* const parent, Page page = LastPageUsed);

    /** Show a setup dialog. Only the specified page will be available.
     */
    static bool execSinglePage(Page page);
    static bool execSinglePage(QWidget* const parent, Page page);

    static bool execTemplateEditor(QWidget* const parent, const Template& t);
    void setTemplate(const Template& t);

    static bool execMetadataFilters(QWidget* const parent, int tab);

    QSize sizeHint() const;

private Q_SLOTS:

    void slotButtonClicked(int button);
    void slotSearchTextChanged(const SearchTextSettings& settings);

private:

    explicit Setup(QWidget* const parent = 0);
    ~Setup();

    Setup::Page activePageIndex() const;
    void        showPage(Setup::Page page);
    void        okClicked();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif  // SETUP_H
