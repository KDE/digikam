/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-02
 * Description : showFoto setup dialog.
 *
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

#ifndef SHOWFOTOSETUP_H
#define SHOWFOTOSETUP_H

// Local includes

#include "dconfigdlg.h"

using namespace Digikam;

namespace ShowFoto
{

class Setup : public DConfigDlg
{
    Q_OBJECT

public:

    enum Page
    {
        LastPageUsed = -1,

        EditorPage = 0,
        MetadataPage,
        ToolTipPage,
        RawPage,
        IOFilesPage,
        SlideshowPage,
        ICCPage,
        MiscellaneousPage,

        SetupPageEnumLast
    };

public:

    explicit Setup(QWidget* const parent=0, Page page=LastPageUsed);
    ~Setup();

    static bool execMetadataFilters(QWidget* const parent, int tab);

private Q_SLOTS:

    void slotOkClicked();
    void slotHelp();

private:

    Setup::Page activePageIndex();
    void showPage(Setup::Page page);

private:

    class Private;
    Private* const d;
};

}   // namespace ShowFoto

#endif // SHOWFOTOSETUP_H
