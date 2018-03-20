/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-13
 * Description : a template to create wizard page.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dwizardpage.h"

// Qt includes

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QStandardPaths>
#include <QApplication>
#include <QStyle>
#include <QScrollArea>
#include <QWizard>

namespace Digikam
{

class DWizardPage::Private
{
public:

    explicit Private()
    {
        hlay          = 0;
        logo          = 0;
        leftBottomPix = 0;
        leftView      = 0;
        isComplete    = true;
        id            = -1;
        dlg           = 0;
    }

    bool         isComplete;
    int          id;

    QWidget*     leftView;
    QLabel*      logo;
    QLabel*      leftBottomPix;

    QHBoxLayout* hlay;

    QWizard*     dlg;
};

DWizardPage::DWizardPage(QWizard* const dlg, const QString& title)
    : QWizardPage(dlg),
      d(new Private)
{
    setTitle(title);

    const int spacing     = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    QScrollArea* const sv = new QScrollArea(this);
    QWidget* const panel  = new QWidget(sv->viewport());
    sv->setWidget(panel);
    sv->setWidgetResizable(true);

    d->hlay                    = new QHBoxLayout(panel);
    d->leftView                = new QWidget(panel);
    QVBoxLayout* const vboxLay = new QVBoxLayout(d->leftView);
    d->logo                    = new QLabel(d->leftView);
    d->logo->setAlignment(Qt::AlignTop);
    d->logo->setPixmap(QIcon::fromTheme(QLatin1String("digikam")).pixmap(QSize(128, 128)));

    QWidget* const space = new QLabel(d->leftView);
    d->leftBottomPix     = new QLabel(d->leftView);
    d->leftBottomPix->setAlignment(Qt::AlignBottom);

    vboxLay->addWidget(d->logo);
    vboxLay->addWidget(space);
    vboxLay->addWidget(d->leftBottomPix);
    vboxLay->setStretchFactor(space, 10);
    vboxLay->setContentsMargins(spacing, spacing, spacing, spacing);
    vboxLay->setSpacing(spacing);

    QFrame* const vline = new QFrame(panel);
    vline->setLineWidth(1);
    vline->setMidLineWidth(0);
    vline->setFrameShape(QFrame::VLine);
    vline->setFrameShadow(QFrame::Sunken);
    vline->setMinimumSize(2, 0);
    vline->updateGeometry();

    d->hlay->addWidget(d->leftView);
    d->hlay->addWidget(vline);
    d->hlay->setContentsMargins(QMargins());
    d->hlay->setSpacing(spacing);

    QVBoxLayout* const layout = new QVBoxLayout;
    layout->addWidget(sv);
    setLayout(layout);

    d->dlg = dlg;
    d->id  = d->dlg->addPage(this);
}

DWizardPage::~DWizardPage()
{
    delete d;
}

void DWizardPage::setComplete(bool b)
{
    d->isComplete = b;
    emit completeChanged();
}

bool DWizardPage::isComplete() const
{
    return d->isComplete;
}

int DWizardPage::id() const
{
    return d->id;
}

void DWizardPage::setShowLeftView(bool v)
{
    d->leftView->setVisible(v);
}

void DWizardPage::setPageWidget(QWidget* const w)
{
    d->hlay->addWidget(w);
    d->hlay->setStretchFactor(w, 10);
}

void DWizardPage::removePageWidget(QWidget* const w)
{
    d->hlay->removeWidget(w);
}

void DWizardPage::setLeftBottomPix(const QPixmap& pix)
{
    d->leftBottomPix->setPixmap(pix);
}

void DWizardPage::setLeftBottomPix(const QIcon& icon)
{
    d->leftBottomPix->setPixmap(icon.pixmap(128));
}

QWizard* DWizardPage::assistant() const
{
    return d->dlg;
}

} // namespace Digikam
