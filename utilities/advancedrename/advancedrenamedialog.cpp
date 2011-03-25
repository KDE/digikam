/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-14
 * Description : a rename dialog for the AdvancedRename utility
 *
 * Copyright (C) 2009-2010 by Andi Clemens <andi dot clemens at gmx dot net>
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

#include "advancedrenamedialog.moc"

// Qt includes

#include <QEvent>
#include <QFileInfo>
#include <QGridLayout>
#include <QHeaderView>
#include <QMoveEvent>
#include <QSet>
#include <QString>
#include <QTreeWidget>

// KDE includes

#include <kapplication.h>
#include <klocale.h>

// Local includes

#include "advancedrenamewidget.h"
#include "parser.h"
#include "parsesettings.h"
#include "advancedrenamemanager.h"
#include "advancedrenameprocessdialog.h"

namespace Digikam
{

class AdvancedRenameListItem::AdvancedRenameListItemPriv
{
public:

    AdvancedRenameListItemPriv()
    {
    }

    KUrl    imageUrl;
    QString completeFileName;
};

// --------------------------------------------------------

AdvancedRenameListItem::AdvancedRenameListItem(QTreeWidget* view)
    : QTreeWidgetItem(view), d(new AdvancedRenameListItemPriv)
{
}

AdvancedRenameListItem::AdvancedRenameListItem(QTreeWidget* view, const KUrl& url)
    : QTreeWidgetItem(view), d(new AdvancedRenameListItemPriv)
{
    setImageUrl(url);
}

AdvancedRenameListItem:: ~AdvancedRenameListItem()
{
    delete d;
}

void AdvancedRenameListItem::setImageUrl(const KUrl& url)
{
    d->imageUrl = url;

    QFileInfo fi(d->imageUrl.toLocalFile());
    d->completeFileName  = fi.fileName();

    setName(d->completeFileName,    false);
    setNewName(d->completeFileName, false);
}

KUrl AdvancedRenameListItem::imageUrl() const
{
    return d->imageUrl;
}

void AdvancedRenameListItem::setName(const QString& name, bool check)
{
    setText(OldName, name);

    if (check)
    {
        markInvalid(isInvalid());
    }
}

QString AdvancedRenameListItem::name() const
{
    return text(OldName);
}

void AdvancedRenameListItem::setNewName(const QString& name, bool check)
{
    setText(NewName, name);

    if (check)
    {
        markInvalid(isInvalid());
    }
}

QString AdvancedRenameListItem::newName() const
{
    return text(NewName);
}

void AdvancedRenameListItem::markInvalid(bool invalid)
{
    QColor normalText = kapp->palette().text().color();
    setTextColor(OldName, invalid ? Qt::red : normalText);
    setTextColor(NewName, invalid ? Qt::red : normalText);
}

bool AdvancedRenameListItem::isInvalid()
{
    return ( name() == newName() );
}

// --------------------------------------------------------

class AdvancedRenameDialog::AdvancedRenameDialogPriv
{
public:

    AdvancedRenameDialogPriv() :
        singleFileMode(false),
        minSizeDialog(450),
        listView(0),
        advancedRenameManager(0),
        advancedRenameWidget(0)
    {}

    static const QString   configGroupName;
    static const QString   configLastUsedRenamePatternEntry;
    static const QString   configDialogSizeEntry;

    QString                singleFileModeOldFilename;

    bool                   singleFileMode;
    int                    minSizeDialog;

    QTreeWidget*           listView;
    AdvancedRenameManager* advancedRenameManager;
    AdvancedRenameWidget*  advancedRenameWidget;
    NewNamesList           newNamesList;
};
const QString AdvancedRenameDialog::AdvancedRenameDialogPriv::configGroupName("AdvancedRename Dialog");
const QString AdvancedRenameDialog::AdvancedRenameDialogPriv::configLastUsedRenamePatternEntry("Last Used Rename Pattern");
const QString AdvancedRenameDialog::AdvancedRenameDialogPriv::configDialogSizeEntry("Dialog Size");

// --------------------------------------------------------

AdvancedRenameDialog::AdvancedRenameDialog(QWidget* parent)
    : KDialog(parent), d(new AdvancedRenameDialogPriv)
{
    d->advancedRenameManager  = new AdvancedRenameManager();
    d->advancedRenameWidget   = new AdvancedRenameWidget(this);
    d->advancedRenameManager->setWidget(d->advancedRenameWidget);

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
    d->listView->setWhatsThis(i18n("This list shows the results for your renaming pattern. Red items indicate a "
                                   "a name collision, either because the new name is equal to the current name, "
                                   "or because the name has already been assigned to another item."));

    // --------------------------------------------------------

    QWidget* mainWidget     = new QWidget(this);
    QGridLayout* mainLayout = new QGridLayout;
    mainLayout->addWidget(d->listView,             0, 0, 1, 1);
    mainLayout->addWidget(d->advancedRenameWidget, 1, 0, 1, 1);
    mainLayout->setRowStretch(0, 10);
    mainWidget->setLayout(mainLayout);

    setMainWidget(mainWidget);

    setMinimumWidth(d->advancedRenameWidget->minimumWidth());

    setButtons(Help|Cancel|Ok);
    enableButton(Ok, false);
    setHelp("advancedrename.anchor", "digikam");

    initDialog();
    readSettings();

    // --------------------------------------------------------

    connect(d->advancedRenameWidget, SIGNAL(signalTextChanged(const QString&)),
            this, SLOT(slotParseStringChanged(const QString&)));

    connect(d->advancedRenameWidget, SIGNAL(signalReturnPressed()),
            this, SLOT(slotReturnPressed()));
}

AdvancedRenameDialog::~AdvancedRenameDialog()
{
    writeSettings();
    delete d->advancedRenameManager;
    delete d;
}

void AdvancedRenameDialog::slotReturnPressed()
{
    if (isButtonEnabled(Ok))
    {
        accept();
    }
}

void AdvancedRenameDialog::slotParseStringChanged(const QString& parseString)
{
    if (!d->advancedRenameManager)
    {
        return;
    }

    d->newNamesList.clear();

    // generate new file names
    ParseSettings settings;
    settings.useOriginalFileExtension = d->singleFileMode ? false : true;
    d->advancedRenameManager->parseFiles(parseString, settings);

    // fill the tree widget with the updated files
    QTreeWidgetItemIterator it(d->listView);

    while (*it)
    {
        AdvancedRenameListItem* item = dynamic_cast<AdvancedRenameListItem*>((*it));

        if (item)
        {
            QString newName = d->advancedRenameManager->newName(item->imageUrl().toLocalFile());
            item->setNewName(newName);
            d->newNamesList << NewNameInfo(item->imageUrl(), newName);
        }

        ++it;
    }

    bool enableBtn = !parseString.isEmpty() && checkNewNames();
    enableButton(Ok, enableBtn);

    d->listView->viewport()->update();
}

void AdvancedRenameDialog::slotAddImages(const KUrl::List& urls)
{
    if (urls.isEmpty())
    {
        return;
    }

    d->listView->clear();
    d->advancedRenameManager->reset();
    QList<ParseSettings> files;
    foreach (const KUrl& url, urls)
    {
        ParseSettings ps;
        ps.fileUrl = url;
        files << ps;
    }
    d->advancedRenameManager->addFiles(files, AdvancedRenameManager::SortAscending);

    initDialog();
    slotParseStringChanged(d->advancedRenameWidget->parseString());
}

void AdvancedRenameDialog::initDialog()
{
    int count = d->advancedRenameManager->fileList().size();

    QString title = i18np("Rename", "Rename (%1 images)", count);
    setWindowTitle(title);

    if (count < 1)
    {
        d->listView->clear();
        return;
    }

    d->singleFileMode = count == 1;

    AdvancedRenameListItem* item = 0;
    foreach (const QString& file, d->advancedRenameManager->fileList())
    {
        item = new AdvancedRenameListItem(d->listView);
        KUrl url(file);
        item->setImageUrl(url);
    }

    // set current filename if only one image has been added
    if (d->singleFileMode)
    {
        QFileInfo info(d->advancedRenameManager->fileList().first());
        d->advancedRenameWidget->setParseString(info.fileName());
        d->advancedRenameWidget->focusLineEdit();
        d->advancedRenameWidget->highlightLineEdit(info.completeBaseName());
        d->advancedRenameWidget->setParseTimerDuration(50);
        d->singleFileModeOldFilename = info.fileName();
    }

    enableButton(Ok, checkNewNames());
}

NewNamesList AdvancedRenameDialog::newNames()
{
    return filterNewNames();
}

void AdvancedRenameDialog::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    QSize s = group.readEntry(d->configDialogSizeEntry, QSize(d->minSizeDialog, d->minSizeDialog));
    resize(s);
    d->advancedRenameWidget->setParseString(group.readEntry(d->configLastUsedRenamePatternEntry, QString()));
}

void AdvancedRenameDialog::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configDialogSizeEntry, size());

    if (d->singleFileMode)
    {
        d->advancedRenameWidget->clear();
    }
    else
    {
        group.writeEntry(d->configLastUsedRenamePatternEntry, d->advancedRenameWidget->parseString());
    }
}

bool AdvancedRenameDialog::checkNewNames()
{
    int numNames        = 0;
    int numInvalidNames = 0;

    QTreeWidgetItemIterator it(d->listView);

    while (*it)
    {
        AdvancedRenameListItem* item = dynamic_cast<AdvancedRenameListItem*>((*it));

        if (item)
        {
            ++numNames;
            bool invalid = item->isInvalid();
            item->markInvalid(invalid);
            if (invalid)
            {
                ++ numInvalidNames;
            }
        }

        ++it;
    }

    return !(numNames == numInvalidNames);
}

NewNamesList AdvancedRenameDialog::filterNewNames()
{
    NewNamesList filteredNames;
    QTreeWidgetItemIterator it(d->listView);

    while (*it)
    {
        AdvancedRenameListItem* item = dynamic_cast<AdvancedRenameListItem*>((*it));

        if (item && !item->isInvalid())
        {
            filteredNames << NewNameInfo(item->imageUrl(), item->newName());
        }

        ++it;
    }

    return filteredNames;
}

}  // namespace Digikam
