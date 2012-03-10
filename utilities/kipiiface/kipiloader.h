/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-10-03
 * Description : kipi loader implementation
 *
 * Copyright (C) 2012 by Supreet Pal Singh <supreetpal@gmail.com>
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

#ifndef DIGIKAMAPP_H
#define DIGIKAMAPP_H

// Qt includes

#include <QList>
#include <QAction>
#include <QString>

class KAction;

class kipiLoader
{

public:

    kipiLoader();
    ~kipiLoader();	    
 	
    // KIPI Actions collections access.
    const QList<QAction*>& menuImageActions();
    const QList<QAction*>& menuBatchActions();
    const QList<QAction*>& menuAlbumActions();
    const QList<QAction*>& menuImportActions();
    const QList<QAction*>& menuExportActions();

private:

void loadPlugins();

private slots:

void slotKipiPluginPlug();

private:

    class kipiLoaderPriv;
    kipiLoader* const d;

};

}  // namespace Digikam
