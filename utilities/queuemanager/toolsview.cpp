/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-20
 * Description : a view to available tools in tab view.
 *
 * Copyright (C) 2009 Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include "toolsview.moc"

// Qt includes

#include <QWidget>

// KDE includes

#include <klocale.h>

// Local includes

#include "batchtool.h"
#include "toolslistview.h"

namespace Digikam
{

class ToolsViewPriv
{

public:

    ToolsViewPriv()
    {
        baseTools   = 0;
        customTools = 0;
    }

    ToolsListView *baseTools;
    ToolsListView *customTools;
};

ToolsView::ToolsView(QWidget *parent)
         : KTabWidget(parent), d(new ToolsViewPriv)
{
    setTabBarHidden(false);
    setCloseButtonEnabled(false);

    // --------------------------------------------------------

    d->baseTools = new ToolsListView(this);
    d->baseTools->setWhatsThis(i18n("This is the list of digiKam batch tools available."));
    new ToolListViewGroup(d->baseTools, BatchTool::BaseTool);
    addTab(d->baseTools, i18n("Base"));

    d->customTools = new ToolsListView(this);
    d->customTools->setWhatsThis(i18n("This is the list of user customized batch tools."));
    new ToolListViewGroup(d->customTools, BatchTool::CustomTool);
    addTab(d->customTools, i18n("Custom"));

    // --------------------------------------------------------

    connect(d->baseTools, SIGNAL(signalAssignTools(const QMap<int, QString>&)),
            this, SIGNAL(signalAssignTools(const QMap<int, QString>&)));

    connect(d->customTools, SIGNAL(signalAssignTools(const QMap<int, QString>&)),
            this, SIGNAL(signalAssignTools(const QMap<int, QString>&)));
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
        view->viewport()->setEnabled(!b);
    }
}

void ToolsView::addTool(BatchTool* tool)
{
    if (!tool) return;

    switch(tool->toolGroup())
    {
        case BatchTool::BaseTool:
            d->baseTools->addTool(tool);
            break;
        case BatchTool::KipiTool:
            // TODO
            break;
        default:      // User customized tools.
            d->customTools->addTool(tool);
            break;
    }
}

bool ToolsView::removeTool(BatchTool* tool)
{
    bool ret = false;

    if (tool)
    {
        switch(tool->toolGroup())
        {
            case BatchTool::BaseTool:
                ret = d->baseTools->removeTool(tool);
                break;
            case BatchTool::KipiTool:
                // TODO
                break;
            default:      // User customized tools.
                ret = d->customTools->removeTool(tool);
                break;
        }
    }
    return ret;
}

}  // namespace Digikam
