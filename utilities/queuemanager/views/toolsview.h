/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-20
 * Description : a view to available tools in tab view.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef TOOLS_VIEW_H
#define TOOLS_VIEW_H

// Local includes

#include "dhistoryview.h"

namespace Digikam
{

class BatchTool;

class ToolsView : public QTabWidget
{
    Q_OBJECT

public:

    enum ViewTabs
    {
        TOOLS = 0,
        WORKFLOW,
        HISTORY
    };

public:

    explicit ToolsView(QWidget* const parent = 0);
    ~ToolsView();

    void addTool(BatchTool* const tool);
    bool removeTool(BatchTool* const tool);

    void setBusy(bool b);

    void showTab(ViewTabs t);
    void addHistoryEntry(const QString& msg, DHistoryView::EntryType type, int queueId = -1, qlonglong itemId = -1);

Q_SIGNALS:

    void signalAssignQueueSettings(QString);
    void signalAssignTools(const QMap<int, QString>&);
    void signalHistoryEntryClicked(int, qlonglong);

private Q_SLOTS:

    void slotHistoryEntryClicked(const QVariant&);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* QUEUE_SETTINGS_VIEW_H */
