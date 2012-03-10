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

#ifndef KIPIPLUGINLOADER_H
#define KIPIPLUGINLOADER_H

// Qt includes

#include <QObject>
#include <QList>
#include <QString>

class QAction;

namespace Digikam
{

class SplashScreen;

class KipiPluginLoader : public QObject
{
    Q_OBJECT

public:

    KipiPluginLoader(QObject* const parent);
    virtual ~KipiPluginLoader();

    // KIPI Actions collections access.
    const QList<QAction*>& menuImageActions();
    const QList<QAction*>& menuBatchActions();
    const QList<QAction*>& menuAlbumActions();
    const QList<QAction*>& menuImportActions();
    const QList<QAction*>& menuExportActions();

    void setSplashScreen(SplashScreen* const splash);

private:

    void loadPlugins();

private slots:

    void slotKipiPluginPlug();

private:

    class KipiPluginLoaderPriv;
    KipiPluginLoaderPriv* const d;
};

}  // namespace Digikam

#endif // KIPIPLUGINLOADER_H
