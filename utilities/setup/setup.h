/* ============================================================
 * Authors: Renchi Raju <renchi at pooh.tam.uiuc.edu>
 *          Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2003-02-03
 * Description : digiKam setup dialog.
 * 
 * Copyright 2003-2005 by Renchi Raju and Gilles Caulier
 * Copyright 2006-2007 by Gilles Caulier
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

#include <kdialogbase.h>

namespace Digikam
{

class SetupPlugins;
class SetupPrivate;

class Setup : public KDialogBase 
{
    Q_OBJECT

public:

    enum Page 
    {
        LastPageUsed = -1,
        General = 0,
        ToolTip,
        Metadata,
        Identify,
        Collections,
        Mime,
        Editor,
	Dcraw,
        IOFiles,
        Slideshow,
        IccProfiles,
        KipiPlugins,
        Camera,
        Miscellaneous
    };

    Setup(QWidget* parent=0, const char* name=0, Page page=LastPageUsed);
    ~Setup();

    SetupPlugins    *kipiPluginsPage();

private slots:

    void slotOkClicked();

private:

    SetupPrivate* d;
};

}  // namespace Digikam

#endif  // SETUP_H
