/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-28-04
 * Description : first run assistant dialog
 *
 * Copyright (C) 2009-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "firstrundlgpage.h"

// Qt includes

#include <QHBoxLayout>
#include <QLabel>
#include <QApplication>
#include <QStyle>
#include <QStandardPaths>
#include <QScrollArea>

// Local includes

#include "dlayoutbox.h"
#include "firstrundlg.h"
#include "dexpanderbox.h"

namespace Digikam
{

class FirstRunDlgPage::Private
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

FirstRunDlgPage::FirstRunDlgPage(FirstRunDlg* const dlg, const QString& title)
    : QWizardPage(dlg),
      d(new Private)
{
    setTitle(title);

    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    QScrollArea* const sv = new QScrollArea(this);
    QWidget* const panel  = new QWidget(sv->viewport());
    sv->setWidget(panel);
    sv->setWidgetResizable(true);

    d->hlay           = new QHBoxLayout(panel);
    DVBox* const vbox = new DVBox(panel);
    d->logo           = new QLabel(vbox);
    d->logo->setAlignment(Qt::AlignCenter);
    d->logo->setPixmap(QIcon::fromTheme(QLatin1String("digikam")).pixmap(QSize(48,48)));

    QLabel* const space = new QLabel(vbox);
    d->leftBottomPix    = new QLabel(vbox);
    d->leftBottomPix->setAlignment(Qt::AlignCenter);
    vbox->setStretchFactor(space, 10);
    vbox->setContentsMargins(spacing, spacing, spacing, spacing);
    vbox->setSpacing(spacing);

    DLineWidget* const line = new DLineWidget(Qt::Vertical, panel);

    d->hlay->addWidget(vbox);
    d->hlay->addWidget(line);
    d->hlay->setContentsMargins(spacing, spacing, spacing, spacing);
    d->hlay->setSpacing(spacing);

    QVBoxLayout* const layout = new QVBoxLayout;
    layout->addWidget(sv);
    setLayout(layout);

    dlg->addPage(this);
}

FirstRunDlgPage::~FirstRunDlgPage()
{
    delete d;
}

void FirstRunDlgPage::setPageWidget(QWidget* const w)
{
    d->hlay->addWidget(w);
    d->hlay->setStretchFactor(w, 10);
}

void FirstRunDlgPage::setLeftBottomPix(const QIcon& icon)
{
    d->leftBottomPix->setPixmap(icon.pixmap(d->iconSize));
}

}   // namespace Digikam
