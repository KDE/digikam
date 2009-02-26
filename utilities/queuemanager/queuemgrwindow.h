/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-11-21
 * Description : Batch Queue Manager GUI
 *
 * Copyright (C) 2008-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef QUEUEMGRWINDOW_H
#define QUEUEMGRWINDOW_H

// Qt includes.

#include <QString>
#include <QCloseEvent>

// KDE includes.

#include <kurl.h>
#include <kxmlguiwindow.h>

// Local includes.

#include "imageinfo.h"
#include "setup.h"

class KAction;

namespace Digikam
{

class ActionData;
class BatchToolsManager;
class AssignedBatchTools;
class QueueMgrWindowPriv;

class QueueMgrWindow : public KXmlGuiWindow
{
    Q_OBJECT

public:

    ~QueueMgrWindow();

    static QueueMgrWindow *queueManagerWindow();
    static bool            queueManagerWindowCreated();

    void addNewQueue();
    void loadImageInfos(const ImageInfoList &list, const ImageInfo &current);
    void refreshView();
    void applySettings();

    BatchToolsManager* batchToolsManager() const;

signals:

    void signalWindowHasMoved();

protected:

    void moveEvent(QMoveEvent *e);

public slots:

    void slotRun();
    void slotStop();
    void slotItemsUpdated(const KUrl::List&);

private:

    void closeEvent(QCloseEvent* e);
    void setupActions();
    void setupConnections();
    void setupUserArea();
    void setupStatusBar();
    void showToolBars();
    void hideToolBars();
    void readSettings();
    void writeSettings();
    void refreshStatusBar();
    void populateToolsList();
    void setup(Setup::Page page);

    bool checkTargetAlbum(int queueId);
    void busy(bool busy);
    void processOne();
    void processing(const KUrl& url);
    void processed(const KUrl& url, const KUrl& tmp);
    void processingFailed(const KUrl& url);
    void processingAborted();

    QueueMgrWindow();

private slots:

    void slotToggleFullScreen();
    void slotEscapePressed();
    void slotDonateMoney();
    void slotContribute();
    void slotEditKeys();
    void slotShowMenuBar();
    void slotConfToolbars();
    void slotNewToolbarConfig();
    void slotSetup();
    void slotRawCameraList();
    void slotComponentsInfo();
    void slotAction(const ActionData&);
    void slotProgressTimerDone();
    void slotAssignedToolsChanged(const AssignedBatchTools&);
    void slotQueueContentsChanged();
    void slotItemSelectionChanged();

    void slotThemeChanged();
    void slotChangeTheme(const QString&);

private:

    QueueMgrWindowPriv* const d;

    static QueueMgrWindow *m_instance;
};

}  // namespace Digikam

#endif /* QUEUEMGRWINDOW_H */
