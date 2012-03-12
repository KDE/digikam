/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-10-03
 * Description : kipi loader implementation
 *
 * Copyright (C) 2012      by Supreet Pal Singh <supreetpal@gmail.com>
 * Copyright (C) 2004-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

class KActionCollection;

namespace Digikam
{

class SplashScreen;

class KipiPluginLoader : public QObject
{
    Q_OBJECT

public:

    KipiPluginLoader(QObject* const parent, SplashScreen* const splash);
    ~KipiPluginLoader();

    /// KIPI menu collections.
    const QList<QAction*>& menuExportActions();
    const QList<QAction*>& menuImportActions();
    const QList<QAction*>& menuMetadataActions();
    const QList<QAction*>& menuPrintActions();
    const QList<QAction*>& menuImageActions();
    const QList<QAction*>& menuToolsActions();
    const QList<QAction*>& menuBatchActions();
    const QList<QAction*>& menuAlbumActions();

    /// KIPI action collections.
    KActionCollection* pluginsActionCollection() const;

    static KipiPluginLoader* instance();

private Q_SLOTS:

    void slotKipiPluginPlug();

private:

    void loadPlugins();

private:

    static KipiPluginLoader* m_instance;

    class KipiPluginLoaderPriv;
    KipiPluginLoaderPriv* const d;
};

}  // namespace Digikam

#endif // KIPIPLUGINLOADER_H
