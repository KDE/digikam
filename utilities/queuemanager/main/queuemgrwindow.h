/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-11-21
 * Description : Batch Queue Manager GUI
 *
 * Copyright (C) 2008-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes

#include <QMap>
#include <QString>
#include <QCloseEvent>

// KDE includes

#include <kurl.h>
#include <kxmlguiwindow.h>

// Local includes

#include "imageinfo.h"
#include "setup.h"
#include "dhistoryview.h"

class KAction;

namespace Digikam
{

class ActionData;
class BatchToolsManager;
class AssignedBatchTools;

class QueueMgrWindow : public KXmlGuiWindow
{
    Q_OBJECT

public:

    ~QueueMgrWindow();

    static QueueMgrWindow* queueManagerWindow();
    static bool            queueManagerWindowCreated();

    bool isBusy() const;
    void addNewQueue();
    void loadImageInfos(const ImageInfoList& list, int queueId);
    void loadImageInfosToCurrentQueue(const ImageInfoList& list);
    void loadImageInfosToNewQueue(const ImageInfoList& list);
    void refreshView();
    void applySettings();

    BatchToolsManager* batchToolsManager() const;

    /** Return a map of all queues available from pool (index and title).
     */
    QMap<int, QString> queuesMap() const;

    int currentQueueId();

    bool queryClose();

Q_SIGNALS:

    void signalWindowHasMoved();
    void signalBqmIsBusy(bool);

protected:

    void moveEvent(QMoveEvent* e);

public Q_SLOTS:

    void slotRun();
    void slotStop();

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
    void addHistoryMessage(const QString& msg, DHistoryView::EntryType type);

    bool checkTargetAlbum(int queueId);
    void busy(bool busy);
    void processOne();
    void processing(const KUrl& url);
    void processed(const KUrl& url, const KUrl& tmp);
    void processingFailed(const KUrl& url, const QString& errMsg);
    void processingCanceled(const KUrl& url);
    void processingAborted();

    QueueMgrWindow();

private Q_SLOTS:

    void slotToggleFullScreen();
    void slotEscapePressed();
    void slotEditKeys();
    void slotShowMenuBar();
    void slotConfToolbars();
    void slotConfNotifications();
    void slotNewToolbarConfig();
    void slotSetup();
    void slotComponentsInfo();
    void slotDBStat();
    void slotAction(const Digikam::ActionData&);
    void slotProgressTimerDone();
    void slotHistoryEntryClicked(int, qlonglong);
    void slotAssignedToolsChanged(const AssignedBatchTools&);
    void slotQueueContentsChanged();
    void slotItemSelectionChanged();

private:

    class Private;
    Private* const d;

    static QueueMgrWindow* m_instance;
};

}  // namespace Digikam

#endif /* QUEUEMGRWINDOW_H */
