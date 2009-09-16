/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-22
 * Description : a control widget for the ManualRenameParser
 *
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

#include "manualrenamedialog.h"
#include "manualrenamedialog.moc"

// Qt includes

#include <QEvent>
#include <QFileInfo>
#include <QGridLayout>
#include <QHeaderView>
#include <QMoveEvent>
#include <QString>
#include <QTreeWidget>
#include <QWidget>

// KDE includes

#include <kdebug.h>
#include <klocale.h>

// Local includes

#include "manualrenamewidget.h"
#include "parseinformation.h"
#include "statusprogressbar.h"

namespace Digikam
{

class ManualRenameListItemPriv
{
public:

    ManualRenameListItemPriv()
    {
    }

    ImageInfo imageInfo;
    QString   baseName;
    QString   extension;
};

// --------------------------------------------------------

ManualRenameListItem::ManualRenameListItem(QTreeWidget* view)
                    : QTreeWidgetItem(view), d(new ManualRenameListItemPriv)
{
}

ManualRenameListItem::ManualRenameListItem(QTreeWidget* view, const ImageInfo& info)
                    : QTreeWidgetItem(view), d(new ManualRenameListItemPriv)
{
    setImageInfo(info);
}

ManualRenameListItem:: ~ManualRenameListItem()
{
}

void ManualRenameListItem::setImageInfo(const ImageInfo& info)
{
    d->imageInfo = info;

    QFileInfo fi(d->imageInfo.filePath());
    d->baseName  = fi.baseName();
    d->extension = fi.completeSuffix();

    setName(d->baseName);
    setNewName(d->baseName);
}

ImageInfo ManualRenameListItem::imageInfo() const
{
    return d->imageInfo;
}

void ManualRenameListItem::setName(const QString& name)
{
    setText(0, name + '.' + d->extension);
}

QString ManualRenameListItem::name() const
{
    return text(0);
}

void ManualRenameListItem::setNewName(const QString& name)
{
    setText(1, name + '.' + d->extension);
}

QString ManualRenameListItem::newName() const
{
    return text(1);
}

// --------------------------------------------------------

class ManualRenameDialogPriv
{
public:

    ManualRenameDialogPriv()
    {
        manualRenameWidget = 0;
        listView           = 0;
        statusProgressBar  = 0;
    }

    QTreeWidget*                     listView;
    ManualRenameWidget*              manualRenameWidget;
    ManualRenameDialog::NewNamesList newNamesList;
    StatusProgressBar*               statusProgressBar;
};

ManualRenameDialog::ManualRenameDialog(QWidget* parent)
                  : KDialog(parent), d(new ManualRenameDialogPriv)
{
    d->manualRenameWidget = new ManualRenameWidget(this);
    d->manualRenameWidget->setInputColumns(3);

    // --------------------------------------------------------

    d->listView = new QTreeWidget(this);
    d->listView->setRootIsDecorated(false);
    d->listView->setSelectionMode(QAbstractItemView::NoSelection);
    d->listView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->listView->setAllColumnsShowFocus(true);
    d->listView->setSortingEnabled(false);
    d->listView->setColumnCount(2);
    d->listView->setHeaderLabels(QStringList() << i18n("Current Name") << i18n("New Name"));
    d->listView->header()->setResizeMode(0, QHeaderView::Stretch);
    d->listView->header()->setResizeMode(1, QHeaderView::Stretch);

    // --------------------------------------------------------

    d->statusProgressBar = new StatusProgressBar(this);
    d->statusProgressBar->progressBarMode(StatusProgressBar::TextMode);
    d->statusProgressBar->hide();

    // --------------------------------------------------------

    QWidget* mainWidget     = new QWidget(this);
    QGridLayout* mainLayout = new QGridLayout;
    mainLayout->addWidget(d->listView,           0, 0, 1, 1);
    mainLayout->addWidget(d->manualRenameWidget, 1, 0, 1, 1);
    mainLayout->addWidget(d->statusProgressBar,  2, 0, 1, 1);
    mainLayout->setRowStretch(0, 10);
    mainWidget->setLayout(mainLayout);

    setMainWidget(mainWidget);
    initDialog();

    // --------------------------------------------------------

    connect(d->manualRenameWidget, SIGNAL(signalTextChanged(const QString&)),
            this, SLOT(slotParseStringChanged()));

    connect(this, SIGNAL(signalWindowHasMoved()),
            d->manualRenameWidget, SLOT(slotUpdateTrackerPos()));

    connect(this, SIGNAL(signalWindowLostFocus()),
            d->manualRenameWidget, SLOT(slotHideToolTipTracker()));
}

ManualRenameDialog::~ManualRenameDialog()
{
    delete d;
}

void ManualRenameDialog::slotParseStringChanged()
{
    d->newNamesList.clear();

    int index = 1;
    QTreeWidgetItemIterator it(d->listView);
    while (*it)
    {
        ManualRenameListItem* item = dynamic_cast<ManualRenameListItem*>((*it));
        if (item)
        {
            ParseInformation parseInfo(item->imageInfo());
            parseInfo.index = index;

            QString newName = d->manualRenameWidget->parse(parseInfo);
            item->setNewName(newName);
            d->newNamesList << NewNameInfo(item->imageInfo(), newName);

            ++index;
        }
        ++it;
    }
}

void ManualRenameDialog::slotAddImages(const KUrl::List& urls)
{
    d->listView->clear();
    ManualRenameListItem* item = 0;

    int itemCount = 0;
    for (KUrl::List::const_iterator it = urls.constBegin(); it != urls.constEnd(); ++it)
    {
        ImageInfo info(*it);
        if (info.isNull())
            continue;
        item = new ManualRenameListItem(d->listView);
        item->setImageInfo(info);
        ++itemCount;
    }
    initDialog(itemCount);
}

void ManualRenameDialog::initDialog(int count)
{
    const int minSize = 450;

    QString title = i18np("Rename", "Rename (%1 images)", count);
    setWindowTitle(title);

    // resize the dialog when only a single image is selected, it doesn't need to be so big
    // in this case
    resize(minSize, (count > 1) ? minSize : 0);
}

ManualRenameDialog::NewNamesList ManualRenameDialog::newNames()
{
    return d->newNamesList;
}

void ManualRenameDialog::moveEvent(QMoveEvent* e)
{
    Q_UNUSED(e)
    emit signalWindowHasMoved();
}

bool ManualRenameDialog::event(QEvent* e)
{
    switch (e->type())
    {
        case QEvent::WindowDeactivate:
            emit signalWindowLostFocus();
            break;
        default:
            break;
    }
    return KDialog::event(e);
}

}  // namespace Digikam
