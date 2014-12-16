/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-28-04
 * Description : first run assistant dialog
 *
 * Copyright (C) 2009-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// KDE includes

#include <kvbox.h>
#include <kiconloader.h>
#include <kseparator.h>
#include <kstandarddirs.h>
#include <kassistantdialog.h>
#include <kpagewidgetmodel.h>

namespace Digikam
{

class AssistantDlgPage::Private
{
public:

    Private() :
        logo(0),
        leftBottomPix(0),
        hlay(0),
        page(0)
    {
    }

    QLabel*          logo;
    QLabel*          leftBottomPix;

    QHBoxLayout*     hlay;

    KPageWidgetItem* page;
};

AssistantDlgPage::AssistantDlgPage(KAssistantDialog* const dlg, const QString& title)
    : QScrollArea(dlg), d(new Private)
{
    QWidget* const panel = new QWidget(viewport());
    setWidget(panel);
    setWidgetResizable(true);

    d->hlay           = new QHBoxLayout(panel);
    KVBox* const vbox = new KVBox(panel);
    d->logo           = new QLabel(vbox);
    d->logo->setAlignment(Qt::AlignCenter);
    d->logo->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-digikam.png"))
                       .scaled(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    QLabel* const space = new QLabel(vbox);
    d->leftBottomPix    = new QLabel(vbox);
    d->leftBottomPix->setAlignment(Qt::AlignCenter);
    vbox->setStretchFactor(space, 10);
    vbox->setMargin(KDialog::spacingHint());
    vbox->setSpacing(KDialog::spacingHint());
    
    KSeparator* const line = new KSeparator(Qt::Vertical, panel);

    d->hlay->addWidget(vbox);
    d->hlay->addWidget(line);
    d->hlay->setMargin(0);
    d->hlay->setSpacing(KDialog::spacingHint());

    d->page = dlg->addPage(this, title);
}

AssistantDlgPage::~AssistantDlgPage()
{
    delete d;
}

KPageWidgetItem* AssistantDlgPage::page() const
{
    return d->page;
}

void AssistantDlgPage::setPageWidget(QWidget* const w)
{
    d->hlay->addWidget(w);
    d->hlay->setStretchFactor(w, 10);
}

void AssistantDlgPage::setLeftBottomPix(const QPixmap& pix)
{
    d->leftBottomPix->setPixmap(pix);
}

}   // namespace Digikam
