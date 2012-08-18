/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-22
 * Description : stand alone digiKam image editor GUI
 *
 * Copyright (C) 2004-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2005-2006 by Tom Albers <tomalbers at kde dot nl>
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

#ifndef SHOWFOTO_H
#define SHOWFOTO_H

// KDE includes

#include <kurl.h>

// Local includes

#include "editorwindow.h"

class KJob;

namespace Digikam
{
class ThumbBarView;
}

namespace ShowFoto
{

class ShowFoto : public Digikam::EditorWindow
{
    Q_OBJECT

public:

    ShowFoto(const KUrl::List& urlList);
    ~ShowFoto();

    virtual void show();
    bool setup();
    bool setupICC();

private:

    bool setup(bool iccSetupPage=false);

    bool queryClose();
    bool queryExit();

    void setupActions();
    void setupConnections();
    void setupUserArea();

    void readSettings();
    void saveSettings();
    void applySettings();

    void toggleActions(bool val);

    void toggleNavigation(int index);

    bool save();
    bool saveAs();
    void finishSaving(bool success);
    KUrl saveDestinationUrl();
    bool saveNewVersion();
    bool saveCurrentVersion();
    bool saveNewVersionAs();
    bool saveNewVersionInFormat(const QString&);

    void saveIsComplete();
    void saveAsIsComplete();
    void saveVersionIsComplete();

    void slideShow(Digikam::SlideShowSettings& settings);

    void openFolder(const KUrl& url);

    Digikam::ThumbBarDock* thumbBar() const;
    Digikam::Sidebar*      rightSideBar() const;

private Q_SLOTS:

    void slotForward();
    void slotBackward();
    void slotLast();
    void slotFirst();
    void slotFilePrint();

    void slotOpenFile();
    void slotOpenUrl(const KUrl&);
    void slotOpenFolder(const KUrl&);
    void slotOpenFilesInFolder();
    void slotDeleteCurrentItem();

    void slotChanged();
    void slotUpdateItemInfo();

    void slotDeleteCurrentItemResult(KJob*);

    void slotPrepareToLoad();
    void slotLoadingStarted(const QString& filename);
    void slotLoadingFinished(const QString& filename, bool success);
    void slotSavingStarted(const QString& filename);

    void slotContextMenu();
    void slotRevert();

private:

    class ShowFotoPriv;
    ShowFotoPriv* const d;
};

}   // namespace ShowFoto

#endif /* SHOWFOTO_H */
