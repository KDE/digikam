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

#include "toolsview.h"

// Qt includes

#include <QList>
#include <QWidget>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "workflowmanager.h"
#include "batchtool.h"
#include "toolslistview.h"
#include "workflowlist.h"

namespace Digikam
{

class ToolsView::Private
{

public:

    Private() :
        baseTools(0),
        historyView(0),
        workflow(0)
    {
    }

    ToolsListView* baseTools;
    DHistoryView*  historyView;
    WorkflowList*  workflow;
};

ToolsView::ToolsView(QWidget* const parent)
    : QTabWidget(parent),
      d(new Private)
{
    setTabsClosable(false);

    // --------------------------------------------------------

    d->baseTools = new ToolsListView(this);
    d->baseTools->setWhatsThis(i18n("This is the list of digiKam batch tools available."));
    new ToolListViewGroup(d->baseTools, BatchTool::ColorTool);
    new ToolListViewGroup(d->baseTools, BatchTool::EnhanceTool);
    new ToolListViewGroup(d->baseTools, BatchTool::TransformTool);
    new ToolListViewGroup(d->baseTools, BatchTool::DecorateTool);
    new ToolListViewGroup(d->baseTools, BatchTool::FiltersTool);
    new ToolListViewGroup(d->baseTools, BatchTool::ConvertTool);
    new ToolListViewGroup(d->baseTools, BatchTool::MetadataTool);
    new ToolListViewGroup(d->baseTools, BatchTool::CustomTool);
    insertTab(TOOLS, d->baseTools, QIcon::fromTheme(QLatin1String("digikam")), i18n("Base Tools"));

    // --------------------------------------------------------

    d->workflow    = new WorkflowList(this);
    d->workflow->setWhatsThis(i18n("This is the list of your customized workflow settings."));
    insertTab(WORKFLOW, d->workflow, QIcon::fromTheme(QLatin1String("step")), i18n("Workflow"));

    // --------------------------------------------------------

    d->historyView = new DHistoryView(this);
    d->historyView->setWhatsThis(i18n("You can see below the history of last batch operations processed."));
    insertTab(HISTORY, d->historyView, QIcon::fromTheme(QLatin1String("edit-find")), i18n("History"));

    // --------------------------------------------------------

    connect(d->baseTools, SIGNAL(signalAssignTools(QMap<int,QString>)),
            this, SIGNAL(signalAssignTools(QMap<int,QString>)));

    connect(d->workflow, SIGNAL(signalAssignQueueSettings(QString)),
            this, SIGNAL(signalAssignQueueSettings(QString)));

    connect(WorkflowManager::instance(), SIGNAL(signalQueueSettingsAdded(QString)),
            d->workflow, SLOT(slotsAddQueueSettings(QString)));

    connect(d->historyView, SIGNAL(signalEntryClicked(QVariant)),
            this, SLOT(slotHistoryEntryClicked(QVariant)));
}

ToolsView::~ToolsView()
{
    delete d;
}

void ToolsView::setBusy(bool b)
{
    for (int i = 0; i < count(); ++i)
    {
        ToolsListView* const view = dynamic_cast<ToolsListView*>(widget(i));

        if (view)
        {
            view->viewport()->setEnabled(!b);
        }
    }
}

void ToolsView::addTool(BatchTool* const tool)
{
    if (!tool)
    {
        return;
    }

    switch (tool->toolGroup())
    {
        case BatchTool::ColorTool:
        case BatchTool::EnhanceTool:
        case BatchTool::TransformTool:
        case BatchTool::DecorateTool:
        case BatchTool::FiltersTool:
        case BatchTool::ConvertTool:
        case BatchTool::MetadataTool:
        case BatchTool::CustomTool:
            d->baseTools->addTool(tool);
            break;

        default:
            break;
    }
}

bool ToolsView::removeTool(BatchTool* const tool)
{
    bool ret = false;

    if (tool)
    {
        switch (tool->toolGroup())
        {
            case BatchTool::ColorTool:
            case BatchTool::EnhanceTool:
            case BatchTool::TransformTool:
            case BatchTool::DecorateTool:
            case BatchTool::FiltersTool:
            case BatchTool::ConvertTool:
            case BatchTool::MetadataTool:
            case BatchTool::CustomTool:
                ret = d->baseTools->removeTool(tool);
                break;

            default:
                break;
        }
    }

    return ret;
}

void ToolsView::addHistoryEntry(const QString& msg, DHistoryView::EntryType type, int queueId, qlonglong itemId)
{
    if (queueId != -1 && itemId != -1)
    {
        QList<QVariant> list;
        list << queueId << itemId;
        d->historyView->addEntry(msg, type, QVariant(list));
    }
    else
    {
        d->historyView->addEntry(msg, type);
    }
}

void ToolsView::showTab(ViewTabs t)
{
    setCurrentIndex(t);
}

void ToolsView::slotHistoryEntryClicked(const QVariant& metadata)
{
    QList<QVariant> list = metadata.toList();

    if (!list.isEmpty())
    {
        int queueId      = list.at(0).toInt();
        qlonglong itemId = list.at(1).toLongLong();

        emit signalHistoryEntryClicked(queueId, itemId);
    }
}

}  // namespace Digikam
