/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-14
 * Description : a rename dialog for the AdvancedRename utility
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

#include "advancedrenamedialog.h"
#include "advancedrenamedialog.moc"

// Qt includes

#include <QEvent>
#include <QFileInfo>
#include <QGridLayout>
#include <QHeaderView>
#include <QMoveEvent>
#include <QString>
#include <QTreeWidget>

// KDE includes

#include <klocale.h>

// Local includes

#include "advancedrenamewidget.h"
#include "parseinformation.h"

namespace Digikam
{

class AdvancedRenameListItemPriv
{
public:

    AdvancedRenameListItemPriv()
    {
    }

    ImageInfo     imageInfo;
    QString       baseName;
    QString       extension;
};

// --------------------------------------------------------

AdvancedRenameListItem::AdvancedRenameListItem(QTreeWidget* view)
                    : QTreeWidgetItem(view), d(new AdvancedRenameListItemPriv)
{
}

AdvancedRenameListItem::AdvancedRenameListItem(QTreeWidget* view, const ImageInfo& info)
                    : QTreeWidgetItem(view), d(new AdvancedRenameListItemPriv)
{
    setImageInfo(info);
}

AdvancedRenameListItem:: ~AdvancedRenameListItem()
{
    delete d;
}

void AdvancedRenameListItem::setImageInfo(const ImageInfo& info)
{
    d->imageInfo = info;

    QFileInfo fi(d->imageInfo.filePath());
    d->baseName  = fi.baseName();
    d->extension = fi.completeSuffix();

    setName(d->baseName);
    setNewName(d->baseName);
}

ImageInfo AdvancedRenameListItem::imageInfo() const
{
    return d->imageInfo;
}

void AdvancedRenameListItem::setName(const QString& name)
{
    setText(0, name + '.' + d->extension);
}

QString AdvancedRenameListItem::name() const
{
    return text(0);
}

void AdvancedRenameListItem::setNewName(const QString& name)
{
    setText(1, name + '.' + d->extension);
}

QString AdvancedRenameListItem::newName() const
{
    return text(1);
}

// --------------------------------------------------------

class AdvancedRenameDialogPriv
{
public:

    AdvancedRenameDialogPriv() :
        configGroupName("Advanced Rename Dialog"),
        configLastUsedRenamePattern("Last Used Rename Pattern"),

        singleFileMode(true),
        listView(0),
        advancedRenameWidget(0)
    {}

    const QString         configGroupName;
    const QString         configLastUsedRenamePattern;

    bool                  singleFileMode;

    QTreeWidget*          listView;
    AdvancedRenameWidget* advancedRenameWidget;
    NewNamesList          newNamesList;
};

AdvancedRenameDialog::AdvancedRenameDialog(QWidget* parent)
                    : KDialog(parent), d(new AdvancedRenameDialogPriv)
{
    d->advancedRenameWidget = new AdvancedRenameWidget(this);
    d->advancedRenameWidget->setInputColumns(3);
    d->advancedRenameWidget->setTooltipAlignment(Qt::AlignCenter);

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

    QWidget* mainWidget     = new QWidget(this);
    QGridLayout* mainLayout = new QGridLayout;
    mainLayout->addWidget(d->listView,             0, 0, 1, 1);
    mainLayout->addWidget(d->advancedRenameWidget, 1, 0, 1, 1);
    mainLayout->setRowStretch(0, 10);
    mainWidget->setLayout(mainLayout);

    setMainWidget(mainWidget);
    setButtons(Help|Cancel|Ok);
    enableButton(Ok, false);
    setHelp("advancedrename.anchor", "digikam");
    initDialog();
    readSettings();

    // avoid focusing the AdvancedRenameWidget, to show the click message
    d->listView->setFocus();

    // --------------------------------------------------------

    connect(d->advancedRenameWidget, SIGNAL(signalTextChanged(const QString&)),
            this, SLOT(slotParseStringChanged(const QString&)));

    connect(this, SIGNAL(signalWindowHasMoved()),
            d->advancedRenameWidget, SLOT(slotUpdateTrackerPos()));

    connect(this, SIGNAL(signalWindowLostFocus()),
            d->advancedRenameWidget, SLOT(slotHideToolTipTracker()));
}

AdvancedRenameDialog::~AdvancedRenameDialog()
{
    writeSettings();
    delete d;
}

void AdvancedRenameDialog::slotParseStringChanged(const QString& parseString)
{
    d->newNamesList.clear();

    enableButton(Ok, !parseString.isEmpty());

    int index = 1;
    QTreeWidgetItemIterator it(d->listView);
    while (*it)
    {
        AdvancedRenameListItem* item = dynamic_cast<AdvancedRenameListItem*>((*it));
        if (item)
        {
            ParseInformation parseInfo(item->imageInfo());
            parseInfo.index = index;

            QString newName = d->advancedRenameWidget->parse(parseInfo);
            item->setNewName(newName);
            d->newNamesList << NewNameInfo(item->imageInfo(), newName);

            ++index;
        }
        ++it;
    }
    d->listView->viewport()->update();
}

void AdvancedRenameDialog::slotAddImages(const KUrl::List& urls)
{
    d->listView->clear();
    AdvancedRenameListItem* item = 0;

    int itemCount = 0;
    for (KUrl::List::const_iterator it = urls.constBegin(); it != urls.constEnd(); ++it)
    {
        ImageInfo info(*it);
        if (info.isNull())
        {
            continue;
        }
        item = new AdvancedRenameListItem(d->listView);
        item->setImageInfo(info);
        ++itemCount;
    }

    // set current filename if only one image has been added
    if (itemCount == 1)
    {
        QFileInfo info(urls.first().toLocalFile());
        d->advancedRenameWidget->setText(info.completeBaseName());
        d->advancedRenameWidget->focusLineEdit();
    }

    initDialog(itemCount);
}

void AdvancedRenameDialog::initDialog(int count)
{
    const int minSize = 450;

    QString title = i18np("Rename", "Rename (%1 images)", count);
    setWindowTitle(title);

    d->singleFileMode = (count > 1);

    // resize the dialog when only a single image is selected, it doesn't need to be so big
    // in this case.
    resize(minSize, (count > 1) ? minSize : 0);
}

NewNamesList AdvancedRenameDialog::newNames()
{
    return d->newNamesList;
}

void AdvancedRenameDialog::moveEvent(QMoveEvent* e)
{
    Q_UNUSED(e)
    emit signalWindowHasMoved();
}

bool AdvancedRenameDialog::event(QEvent* e)
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

void AdvancedRenameDialog::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    d->advancedRenameWidget->setText(group.readEntry(d->configLastUsedRenamePattern, QString()));
}

void AdvancedRenameDialog::writeSettings()
{
    if (!d->singleFileMode)
    {
        KSharedConfig::Ptr config = KGlobal::config();
        KConfigGroup group        = config->group(d->configGroupName);

        group.writeEntry(d->configLastUsedRenamePattern, d->advancedRenameWidget->text());
    }
}

}  // namespace Digikam
