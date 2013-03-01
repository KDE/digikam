/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-14
 * Description : a rename dialog for the AdvancedRename utility
 *
 * Copyright (C) 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
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

#include <QAction>
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
#include <kurl.h>
#include <kmenu.h>

// Local includes

#include "advancedrenamewidget.h"
#include "contextmenuhelper.h"
#include "parser.h"
#include "parsesettings.h"
#include "advancedrenamemanager.h"
#include "advancedrenameprocessdialog.h"

namespace Digikam
{

class AdvancedRenameListItem::Private
{
public:

    Private()
    {
    }

    KUrl    imageUrl;
    QString completeFileName;
};

// --------------------------------------------------------

AdvancedRenameListItem::AdvancedRenameListItem(QTreeWidget* view)
    : QTreeWidgetItem(view), d(new Private)
{
}

AdvancedRenameListItem::AdvancedRenameListItem(QTreeWidget* view, const KUrl& url)
    : QTreeWidgetItem(view), d(new Private)
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

    setName(d->completeFileName);
    setNewName(d->completeFileName);
}

KUrl AdvancedRenameListItem::imageUrl() const
{
    return d->imageUrl;
}

void AdvancedRenameListItem::setName(const QString& name)
{
    setText(OldName, name);
}

QString AdvancedRenameListItem::name() const
{
    return text(OldName);
}

void AdvancedRenameListItem::setNewName(const QString& name)
{
    setText(NewName, name);
}

QString AdvancedRenameListItem::newName() const
{
    return text(NewName);
}

void AdvancedRenameListItem::markInvalid(bool invalid)
{
    QColor normalText = kapp->palette().text().color();
    setForeground(OldName, invalid ? Qt::red : normalText);
    setForeground(NewName, invalid ? Qt::red : normalText);
}

bool AdvancedRenameListItem::isNameEqual()
{
    return (name() == newName());
}

// --------------------------------------------------------

class AdvancedRenameDialog::Private
{
public:

    Private() :
        singleFileMode(false),
        minSizeDialog(450),
        sortActionName(0),
        sortActionDate(0),
        sortActionSize(0),
        sortActionAscending(0),
        sortActionDescending(0),
        sortGroupActions(0),
        sortGroupDirections(0),
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

    QAction*               sortActionName;
    QAction*               sortActionDate;
    QAction*               sortActionSize;

    QAction*               sortActionAscending;
    QAction*               sortActionDescending;

    QActionGroup*          sortGroupActions;
    QActionGroup*          sortGroupDirections;

    QTreeWidget*           listView;
    AdvancedRenameManager* advancedRenameManager;
    AdvancedRenameWidget*  advancedRenameWidget;
    NewNamesList           newNamesList;
};
const QString AdvancedRenameDialog::Private::configGroupName("AdvancedRename Dialog");
const QString AdvancedRenameDialog::Private::configLastUsedRenamePatternEntry("Last Used Rename Pattern");
const QString AdvancedRenameDialog::Private::configDialogSizeEntry("Dialog Size");

// --------------------------------------------------------

AdvancedRenameDialog::AdvancedRenameDialog(QWidget* parent)
    : KDialog(parent), d(new Private)
{
    setupWidgets();
    setupConnections();

    initDialog();
    readSettings();
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

void AdvancedRenameDialog::slotSortActionTriggered(QAction* action)
{
    if (!action)
    {
        d->advancedRenameManager->setSortAction(AdvancedRenameManager::SortCustom);
    }
    else if (action == d->sortActionName)
    {
        d->advancedRenameManager->setSortAction(AdvancedRenameManager::SortName);
    }
    else if (action == d->sortActionDate)
    {
        d->advancedRenameManager->setSortAction(AdvancedRenameManager::SortDate);
    }
    else if (action == d->sortActionSize)
    {
        d->advancedRenameManager->setSortAction(AdvancedRenameManager::SortSize);
    }
}

void AdvancedRenameDialog::slotSortDirectionTriggered(QAction* direction)
{
    if (direction == d->sortActionAscending)
    {
        d->advancedRenameManager->setSortDirection(AdvancedRenameManager::SortAscending);
    }
    else if (direction == d->sortActionDescending)
    {
        d->advancedRenameManager->setSortDirection(AdvancedRenameManager::SortDescending);
    }
}

void AdvancedRenameDialog::slotShowContextMenu(const QPoint& pos)
{
    KMenu menu(this);
    menu.addTitle(i18n("Sort Images"));

    ContextMenuHelper cmhelper(&menu);
    cmhelper.addAction(d->sortActionName);
    cmhelper.addAction(d->sortActionDate);
    cmhelper.addAction(d->sortActionSize);
    cmhelper.addSeparator();
    cmhelper.addAction(d->sortActionAscending);
    cmhelper.addAction(d->sortActionDescending);

    cmhelper.exec(d->listView->viewport()->mapToGlobal(pos));
}

void AdvancedRenameDialog::slotParseStringChanged(const QString& parseString)
{
    if (!d->advancedRenameManager)
    {
        return;
    }

    if (!d->singleFileMode)
    {
        QApplication::setOverrideCursor(Qt::WaitCursor);
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

    bool enableBtn = checkNewNames() && !parseString.isEmpty();
    enableButton(Ok, enableBtn);

    d->listView->viewport()->update();

    if (!d->singleFileMode)
    {
        QApplication::restoreOverrideCursor();
    }
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
    foreach(const KUrl& url, urls)
    {
        ParseSettings ps;
        ps.fileUrl = url;
        files << ps;
    }
    d->advancedRenameManager->addFiles(files);

    initDialog();
    slotParseStringChanged(d->advancedRenameWidget->parseString());
}

void AdvancedRenameDialog::initDialog()
{
    QStringList fileList = d->advancedRenameManager->fileList();
    int count            = fileList.size();

    QString title = i18np("Rename", "Rename (%1 images)", count);
    setWindowTitle(title);

    if (count < 1)
    {
        d->listView->clear();
        return;
    }

    d->singleFileMode = count == 1;

    foreach(const QString& file, fileList)
    {
        KUrl url(file);
        new AdvancedRenameListItem(d->listView, url);
    }

    // set current filename if only one image has been added
    if (d->singleFileMode)
    {
        QFileInfo info(fileList.first());
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
    int numNames      = 0;
    int numEqualNames = 0;
    bool ok           = true;

    QSet<QString> tmpNewNames;
    QTreeWidgetItemIterator it(d->listView);

    while (*it)
    {
        AdvancedRenameListItem* item = dynamic_cast<AdvancedRenameListItem*>((*it));

        if (item)
        {
            ++numNames;
            QFileInfo fi(item->imageUrl().toLocalFile());

            QString completeNewName = fi.path();
            completeNewName.append('/');
            completeNewName.append(item->newName());

            bool invalid = tmpNewNames.contains(completeNewName);
            tmpNewNames << completeNewName;

            item->markInvalid(invalid);
            ok = !invalid;

            if (item->isNameEqual())
            {
                ++ numEqualNames;
            }
        }

        ++it;
    }

    return (ok && !(numNames == numEqualNames));
}

NewNamesList AdvancedRenameDialog::filterNewNames()
{
    NewNamesList filteredNames;
    QTreeWidgetItemIterator it(d->listView);

    while (*it)
    {
        AdvancedRenameListItem* item = dynamic_cast<AdvancedRenameListItem*>((*it));

        if (item && !item->isNameEqual())
        {
            filteredNames << NewNameInfo(item->imageUrl(), item->newName());
        }

        ++it;
    }

    return filteredNames;
}

void AdvancedRenameDialog::setupWidgets()
{
    d->advancedRenameManager  = new AdvancedRenameManager();
    d->advancedRenameWidget   = new AdvancedRenameWidget(this);
    d->advancedRenameManager->setWidget(d->advancedRenameWidget);

    // --------------------------------------------------------

    d->sortActionName  = new QAction(i18n("By Name"), this);
    d->sortActionDate  = new QAction(i18n("By Date"), this);
    d->sortActionSize  = new QAction(i18n("By File Size"), this);

    d->sortActionName->setCheckable(true);
    d->sortActionDate->setCheckable(true);
    d->sortActionSize->setCheckable(true);

    // --------------------------------------------------------

    d->sortActionAscending  = new QAction(i18n("Ascending"), this);
    d->sortActionDescending = new QAction(i18n("Descending"), this);

    d->sortActionAscending->setCheckable(true);
    d->sortActionDescending->setCheckable(true);

    d->sortActionAscending->setChecked(true);

    // --------------------------------------------------------

    d->sortGroupActions     = new QActionGroup(this);
    d->sortGroupDirections  = new QActionGroup(this);

    d->sortGroupActions->addAction(d->sortActionName);
    d->sortGroupActions->addAction(d->sortActionDate);
    d->sortGroupActions->addAction(d->sortActionSize);

    d->sortGroupDirections->addAction(d->sortActionAscending);
    d->sortGroupDirections->addAction(d->sortActionDescending);

    // --------------------------------------------------------

    d->listView = new QTreeWidget(this);
    d->listView->setRootIsDecorated(false);
    d->listView->setSelectionMode(QAbstractItemView::NoSelection);
    d->listView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->listView->setAllColumnsShowFocus(true);
    d->listView->setSortingEnabled(false);
    d->listView->setColumnCount(2);
    d->listView->setHeaderLabels(QStringList() << i18n("Current Name") << i18n("New Name"));
    d->listView->setContextMenuPolicy(Qt::CustomContextMenu);
    d->listView->header()->setResizeMode(0, QHeaderView::Stretch);
    d->listView->header()->setResizeMode(1, QHeaderView::Stretch);
    d->listView->setWhatsThis(i18n("This list shows the results for your renaming pattern. Red items indicate a "
                                   "name collision, either because the new name is equal to the current name, "
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

    setButtons(Help | Cancel | Ok);
    enableButton(Ok, false);
    setHelp("advancedrename.anchor", "digikam");
}

void AdvancedRenameDialog::setupConnections()
{
    connect(d->advancedRenameWidget, SIGNAL(signalTextChanged(QString)),
            this, SLOT(slotParseStringChanged(QString)));

    connect(d->advancedRenameWidget, SIGNAL(signalReturnPressed()),
            this, SLOT(slotReturnPressed()));

    connect(d->advancedRenameManager, SIGNAL(signalSortingChanged(KUrl::List)),
            this, SLOT(slotAddImages(KUrl::List)));

    connect(d->listView, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotShowContextMenu(QPoint)));

    connect(d->sortGroupActions, SIGNAL(triggered(QAction*)),
            this, SLOT(slotSortActionTriggered(QAction*)));

    connect(d->sortGroupDirections, SIGNAL(triggered(QAction*)),
            this, SLOT(slotSortDirectionTriggered(QAction*)));
}

}  // namespace Digikam
