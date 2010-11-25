/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-08-03
 * Description : widget displaying all image versions in a list
 *
 * Copyright (C) 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
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

#include "versionswidget.moc"

// Qt includes

#include <QListView>
#include <QGridLayout>
#include <QLabel>
#include <QToolButton>

// KDE includes

#include <KLocale>
#include <KIconLoader>
#include <KUrl>
#include <KDebug>

// Local includes

#include "imageversionsmodel.h"
#include "imagepropertiesversionsdelegate.h"
#include "imageinfo.h"
#include "imageinfolist.h"
#include "dimagehistory.h"

namespace Digikam
{

class VersionsWidget::VersionsWidgetPriv
{
public:

    VersionsWidgetPriv()
    {
        view     = 0;
        model    = 0;
        layout   = 0;
        delegate = 0;
    }

    QListView*                       view;
    ImageVersionsModel*              model;
    QGridLayout*                     layout;
    QHBoxLayout*                     iconTextLayout;
    QLabel*                          headerText;
    QLabel*                          headerTextIcon;
    QToolButton*                     treeSwitchButton;
    ImagePropertiesVersionsDelegate* delegate;
};

VersionsWidget::VersionsWidget(QWidget* parent)
    : QWidget(parent), d(new VersionsWidgetPriv)
{
    d->view                 = new QListView(this);
    d->layout               = new QGridLayout(this);
    d->model                = new ImageVersionsModel();
    d->delegate             = new ImagePropertiesVersionsDelegate();
    d->headerText           = new QLabel(this);
    d->headerTextIcon       = new QLabel(this);
    d->iconTextLayout       = new QHBoxLayout();
    d->treeSwitchButton     = new QToolButton(this);

    KIconLoader* iconLoader = KIconLoader::global();

    setLayout(d->layout);

    d->headerText->setText(i18n("Available versions"));
    d->headerTextIcon->setPixmap(SmallIcon("image-x-generic"));
    d->headerTextIcon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    d->treeSwitchButton->setIcon(iconLoader->loadIcon("view-list-tree", (KIconLoader::Group)KIconLoader::Toolbar));
    d->treeSwitchButton->setCheckable(true);
    d->treeSwitchButton->setToolTip(i18n("Switch between list view and tree view"));

    d->view->setItemDelegate(d->delegate);
    d->view->setModel(d->model);
    d->view->setSelectionRectVisible(true);
    d->view->setSelectionMode(QAbstractItemView::SingleSelection);
    d->view->setSelectionBehavior(QAbstractItemView::SelectItems);

    d->iconTextLayout->addWidget(d->headerTextIcon);
    d->iconTextLayout->addWidget(d->headerText);
    d->iconTextLayout->addWidget(d->treeSwitchButton);

    d->layout->addLayout(d->iconTextLayout, 0, 0, 1, 1);
    d->layout->addWidget(d->view,           1, 0, 1, 1);

    connect(d->delegate, SIGNAL(throbberUpdated()),
            d->model, SLOT(slotAnimationStep()));

    connect(d->view, SIGNAL(clicked(QModelIndex)),
            this, SLOT(slotViewItemSelected(QModelIndex)));

    connect(d->treeSwitchButton, SIGNAL(toggled(bool)),
            d->model, SLOT(setPaintTree(bool)));

    connect(d->treeSwitchButton, SIGNAL(pressed()),
            d->view->viewport(), SLOT(repaint()));

    //make sure the view gets repainted in all cases
    connect(d->treeSwitchButton, SIGNAL(pressed()),
            d->model, SLOT(slotAnimationStep()));
}

VersionsWidget::~VersionsWidget()
{
    delete d->delegate;
    delete d->model;
    delete d;
}

void VersionsWidget::setupModelData(QList<QPair<QString, int> >& list) const
{
    d->model->setupModelData(list);
    d->delegate->resetThumbsCounter();
}

void VersionsWidget::slotDigikamViewNoCurrentItem()
{
    d->model->clearModelData();
    d->delegate->resetThumbsCounter();
}

void VersionsWidget::slotViewItemSelected(QModelIndex index)
{
    KUrl url(index.data(Qt::DisplayRole).toString());
    emit newVersionSelected(url);
}

void VersionsWidget::setCurrentSelectedImage(const QString& path) const
{
    d->model->setCurrentSelectedImage(path);
    d->view->selectionModel()->select(d->model->currentSelectedImageIndex(), QItemSelectionModel::SelectCurrent);
}

} // namespace Digikam
