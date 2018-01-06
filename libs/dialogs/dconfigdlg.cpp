/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-03
 * Description : A dialog base class which can handle multiple pages.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 1999-2001 by Mirko Boehm <mirko at kde dot org>
 * Copyright (C) 1999-2001 by Espen Sand <espen at kde dot org>
 * Copyright (C) 1999-2001 by Holger Freyther <freyther at kde dot org>
 * Copyright (C) 2005-2006 by Olivier Goffart <ogoffart at kde dot org>
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

#include "dconfigdlg.h"

// Qt includes

#include <QTimer>
#include <QLayout>
#include <QDialogButtonBox>

namespace Digikam
{

class DConfigDlgPrivate
{
    Q_DECLARE_PUBLIC(DConfigDlg)

protected:

    DConfigDlgPrivate(DConfigDlg* const parent)
        : q_ptr(parent),
          mPageWidget(0),
          mButtonBox(0)
    {
    }

    virtual ~DConfigDlgPrivate()
    {
    }

    void init()
    {
        Q_Q(DConfigDlg);
        delete q->layout();

        QVBoxLayout* const layout = new QVBoxLayout;
        q->setLayout(layout);

        if (mPageWidget)
        {
            q->connect(mPageWidget, SIGNAL(currentPageChanged(DConfigDlgWdgItem*, DConfigDlgWdgItem*)),
                       q, SIGNAL(currentPageChanged(DConfigDlgWdgItem*, DConfigDlgWdgItem*)));

            q->connect(mPageWidget, &DConfigDlgWdg::pageRemoved,
                       q, &DConfigDlg::pageRemoved);

            layout->addWidget(mPageWidget);
        }
        else
        {
            layout->addStretch();
        }

        if (mButtonBox)
        {
            q->connect(mButtonBox, &QDialogButtonBox::accepted,
                       q, &QDialog::accept);

            q->connect(mButtonBox, &QDialogButtonBox::rejected,
                       q, &QDialog::reject);

            layout->addWidget(mButtonBox);
        }
    }

protected:

    DConfigDlg* const q_ptr;
    DConfigDlgWdg*    mPageWidget;
    QDialogButtonBox* mButtonBox;
};

// ------------------------------------------------------------------------

DConfigDlg::DConfigDlg(QWidget* parent, Qt::WindowFlags flags)
    : QDialog(parent, flags),
      d_ptr(new DConfigDlgPrivate(this))
{
    Q_D(DConfigDlg);
    d->mPageWidget = new DConfigDlgWdg(this);
    d->mPageWidget->layout()->setMargin(0);

    d->mButtonBox = new QDialogButtonBox(this);
    d->mButtonBox->setObjectName(QLatin1String("buttonbox"));
    d->mButtonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    d->init();
}

DConfigDlg::DConfigDlg(DConfigDlgWdg* widget, QWidget* parent, Qt::WindowFlags flags)
    : QDialog(parent, flags),
      d_ptr(new DConfigDlgPrivate(this))
{
    Q_D(DConfigDlg);
    Q_ASSERT(widget);
    widget->setParent(this);
    d->mPageWidget = widget;
    d->mButtonBox = new QDialogButtonBox(this);
    d->mButtonBox->setObjectName(QLatin1String("buttonbox"));
    d->mButtonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    d->init();
}

DConfigDlg::DConfigDlg(DConfigDlgPrivate& dd, DConfigDlgWdg* widget, QWidget* parent, Qt::WindowFlags flags)
    : QDialog(parent, flags),
      d_ptr(&dd)
{
    Q_D(DConfigDlg);

    if (widget)
    {
        widget->setParent(this);
        d->mPageWidget = widget;
    }
    else
    {
        d->mPageWidget = new DConfigDlgWdg(this);
    }

    d->mButtonBox = new QDialogButtonBox(this);
    d->mButtonBox->setObjectName(QLatin1String("buttonbox"));
    d->mButtonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    d->init();
}

DConfigDlg::~DConfigDlg()
{
    delete d_ptr;
}

void DConfigDlg::setFaceType(FaceType faceType)
{
    d_func()->mPageWidget->setFaceType(static_cast<DConfigDlgWdg::FaceType>(faceType));
}

DConfigDlgWdgItem* DConfigDlg::addPage(QWidget* widget, const QString& name)
{
    return d_func()->mPageWidget->addPage(widget, name);
}

void DConfigDlg::addPage(DConfigDlgWdgItem* item)
{
    d_func()->mPageWidget->addPage(item);
}

DConfigDlgWdgItem* DConfigDlg::insertPage(DConfigDlgWdgItem* before, QWidget* widget, const QString& name)
{
    return d_func()->mPageWidget->insertPage(before, widget, name);
}

void DConfigDlg::insertPage(DConfigDlgWdgItem* before, DConfigDlgWdgItem* item)
{
    d_func()->mPageWidget->insertPage(before, item);
}

DConfigDlgWdgItem* DConfigDlg::addSubPage(DConfigDlgWdgItem* parent, QWidget* widget, const QString& name)
{
    return d_func()->mPageWidget->addSubPage(parent, widget, name);
}

void DConfigDlg::addSubPage(DConfigDlgWdgItem* parent, DConfigDlgWdgItem* item)
{
    d_func()->mPageWidget->addSubPage(parent, item);
}

void DConfigDlg::removePage(DConfigDlgWdgItem* item)
{
    d_func()->mPageWidget->removePage(item);
}

void DConfigDlg::setCurrentPage(DConfigDlgWdgItem* item)
{
    d_func()->mPageWidget->setCurrentPage(item);
}

DConfigDlgWdgItem* DConfigDlg::currentPage() const
{
    return d_func()->mPageWidget->currentPage();
}

void DConfigDlg::setStandardButtons(QDialogButtonBox::StandardButtons buttons)
{
    d_func()->mButtonBox->setStandardButtons(buttons);
}

QPushButton* DConfigDlg::button(QDialogButtonBox::StandardButton which) const
{
    return d_func()->mButtonBox->button(which);
}

void DConfigDlg::addActionButton(QAbstractButton* button)
{
    d_func()->mButtonBox->addButton(button, QDialogButtonBox::ActionRole);
}

DConfigDlgWdg* DConfigDlg::pageWidget()
{
    return d_func()->mPageWidget;
}

void DConfigDlg::setPageWidget(DConfigDlgWdg* widget)
{
    delete d_func()->mPageWidget;
    d_func()->mPageWidget = widget;
    d_func()->init();
}

const DConfigDlgWdg* DConfigDlg::pageWidget() const
{
    return d_func()->mPageWidget;
}

QDialogButtonBox* DConfigDlg::buttonBox()
{
    return d_func()->mButtonBox;
}

const QDialogButtonBox* DConfigDlg::buttonBox() const
{
    return d_func()->mButtonBox;
}

void DConfigDlg::setButtonBox(QDialogButtonBox* box)
{
    delete d_func()->mButtonBox;
    d_func()->mButtonBox = box;
    d_func()->init();
}

}  // namespace Digikam
