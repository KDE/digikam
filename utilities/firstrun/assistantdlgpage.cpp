/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-28-04
 * Description : first run assistant dialog
 *
 * Copyright (C) 2009-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "assistantdlgpage.h"

// Qt includes

#include <QHBoxLayout>
#include <QLabel>
#include <QApplication>
#include <QStyle>
#include <QStandardPaths>
#include <QScrollArea>

// Libkdcraw includes

#include <KDCRAW/RWidgetUtils>

// Local includes

#include "assistantdlg.h"

using namespace KDcrawIface;

namespace Digikam
{

class AssistantDlgPage::Private
{
public:

    Private() :
        iconSize(qApp->style()->pixelMetric(QStyle::PM_MessageBoxIconSize,  0, qApp->activeWindow())),
        logo(0),
        leftBottomPix(0),
        hlay(0)
    {
    }

    int          iconSize;

    QLabel*      logo;
    QLabel*      leftBottomPix;

    QHBoxLayout* hlay;
};

AssistantDlgPage::AssistantDlgPage(AssistantDlg* const dlg, const QString& title)
    : QWizardPage(dlg),
      d(new Private)
{
    setTitle(title);

    QScrollArea* const sv = new QScrollArea(this);
    QWidget* const panel  = new QWidget(sv->viewport());
    sv->setWidget(panel);
    sv->setWidgetResizable(true);
    sv->setWidget(panel);

    d->hlay           = new QHBoxLayout(panel);
    RVBox* const vbox = new RVBox(panel);
    d->logo           = new QLabel(vbox);
    d->logo->setAlignment(Qt::AlignCenter);
    d->logo->setPixmap(QPixmap(QStandardPaths::locate(QStandardPaths::GenericDataLocation, "digikam/data/logo-digikam.png"))
                       .scaled(d->iconSize, d->iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    QLabel* const space = new QLabel(vbox);
    d->leftBottomPix    = new QLabel(vbox);
    d->leftBottomPix->setAlignment(Qt::AlignCenter);
    vbox->setStretchFactor(space, 10);
    vbox->setMargin(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    vbox->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    
    RLineWidget* const line = new RLineWidget(Qt::Vertical, panel);

    d->hlay->addWidget(vbox);
    d->hlay->addWidget(line);
    d->hlay->setMargin(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    d->hlay->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    
    QVBoxLayout* const layout = new QVBoxLayout;
    layout->addWidget(sv);
    setLayout(layout);
     
    dlg->addPage(this);
}

AssistantDlgPage::~AssistantDlgPage()
{
    delete d;
}

void AssistantDlgPage::setPageWidget(QWidget* const w)
{
    d->hlay->addWidget(w);
    d->hlay->setStretchFactor(w, 10);
}

void AssistantDlgPage::setLeftBottomPix(const QIcon& icon)
{
    d->leftBottomPix->setPixmap(icon.pixmap(d->iconSize));
}

}   // namespace Digikam
