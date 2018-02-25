/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-04-12
 * Description : A tool to export items to Rajce web service
 *
 * Copyright (C) 2011      by Lukas Krejci <krejci.l at centrum dot cz>
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "rajcewindow.h"

// Qt includes

#include <QAction>
#include <QMenu>
#include <QCloseEvent>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "rajcewidget.h"

namespace Digikam
{

RajceWindow::RajceWindow(DInfoInterface* const iface, QWidget* const /*parent*/)
    : WSToolDialog(0)
{
    m_widget = new RajceWidget(iface, this);
    m_widget->readSettings();

    setMainWidget(m_widget);
    setModal(false);
    setWindowTitle(i18n("Export to Rajce.net"));

    startButton()->setText(i18n("Start Upload"));
    startButton()->setToolTip(i18n("Start upload to Rajce.net"));

    m_widget->setMinimumSize(700, 500);

    connect(startButton(), SIGNAL(clicked()),
            m_widget, SLOT(slotStartUpload()));

    connect(this, SIGNAL(finished(int)),
            this, SLOT(slotFinished()));

    connect(m_widget, SIGNAL(signalLoginStatusChanged(bool)),
            this, SLOT(slotSetUploadButtonEnabled(bool)));

    startButton()->setEnabled(false);
}

RajceWindow::~RajceWindow()
{
}

void RajceWindow::reactivate()
{
    m_widget->reactivate();
    show();
}

void RajceWindow::slotSetUploadButtonEnabled(bool enabled)
{
    startButton()->setEnabled(enabled);
}

void RajceWindow::slotFinished()
{
    m_widget->cancelUpload();
    m_widget->writeSettings();
}

void RajceWindow::closeEvent(QCloseEvent* e)
{
    if (!e)
    {
        return;
    }

    slotFinished();
    e->accept();
}

} // namespace Digikam
