/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-20
 * Description : a view to available tools in tab view.
 *
 * Copyright (C) 2009-2010 Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "toolsview.moc"

// Qt includes

#include <QList>
#include <QWidget>

// KDE includes

#include <kdeversion.h>
#include <kiconloader.h>
#include <klocale.h>

// Local includes

#include "batchtool.h"
#include "toolslistview.h"

namespace Digikam
{

class ToolsView::ToolsViewPriv
{

public:

    ToolsViewPriv() :
        baseTools(0),
        //        customTools = 0;
        historyView(0)
    {
    }

    ToolsListView* baseTools;
    //    ToolsListView* customTools;

    DHistoryView*  historyView;
};

ToolsView::ToolsView(QWidget* parent)
    : KTabWidget(parent), d(new ToolsViewPriv)
{
    setTabBarHidden(false);

#if KDE_IS_VERSION(4,3,0)
    setTabsClosable(false);
#else
    setCloseButtonEnabled(false);
#endif
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
    addTab(d->baseTools, SmallIcon("digikam"), i18n("Base Tools"));

    //    d->customTools = new ToolsListView(this);
    //    d->customTools->setWhatsThis(i18n("This is the list of user customized batch tools."));
    //    new ToolListViewGroup(d->customTools, BatchTool::CustomTool);
    //    addTab(d->customTools, SmallIcon("user-properties"), i18n("Custom Tools"));

    d->historyView = new DHistoryView(this);
    d->historyView->setWhatsThis(i18n("You can see below the history of last batch operations processed."));
    addTab(d->historyView, SmallIcon("view-history"), i18n("History"));

    // --------------------------------------------------------

    connect(d->baseTools, SIGNAL(signalAssignTools(QMap<int,QString>)),
            this, SIGNAL(signalAssignTools(QMap<int,QString>)));

    //    connect(d->customTools, SIGNAL(signalAssignTools(QMap<int,QString>)),
    //            this, SIGNAL(signalAssignTools(QMap<int,QString>)));

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
        ToolsListView* view = dynamic_cast<ToolsListView*>(widget(i));

        if (view)
        {
            view->viewport()->setEnabled(!b);
        }
    }
}

void ToolsView::addTool(BatchTool* tool)
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
            d->baseTools->addTool(tool);
            break;

        case BatchTool::KipiTool:
            // TODO
            break;

        default:      // User customized tools.
            //            d->customTools->addTool(tool);
            break;
    }
}

bool ToolsView::removeTool(BatchTool* tool)
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
                ret = d->baseTools->removeTool(tool);
                break;

            case BatchTool::KipiTool:
                // TODO
                break;

            default:      // User customized tools.
                //                ret = d->customTools->removeTool(tool);
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
        d->historyView->addedEntry(msg, type, QVariant(list));
    }
    else
    {
        d->historyView->addedEntry(msg, type, QVariant());
    }
}

void ToolsView::showHistory()
{
    setCurrentWidget(d->historyView);
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
