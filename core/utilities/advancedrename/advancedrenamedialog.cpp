/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-14
 * Description : a rename dialog for the AdvancedRename utility
 *
 * Copyright (C) 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2013-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <QMenu>
#include <QApplication>
#include <QStyle>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QHeaderView>

// KDE includes

#include <klocalizedstring.h>
#include <kconfiggroup.h>

// Local includes

#include "dxmlguiwindow.h"
#include "advancedrenamewidget.h"
#include "contextmenuhelper.h"
#include "parser.h"
#include "parsesettings.h"
#include "advancedrenamemanager.h"
#include "advancedrenameprocessdialog.h"
#include "digikam_debug.h"

namespace Digikam
{

class AdvancedRenameListItem::Private
{
public:

    Private()
    {
    }

    QUrl    imageUrl;
    QString completeFileName;
};

// --------------------------------------------------------

AdvancedRenameListItem::AdvancedRenameListItem(QTreeWidget* const view)
    : QTreeWidgetItem(view),
      d(new Private)
{
}

AdvancedRenameListItem::AdvancedRenameListItem(QTreeWidget* const view, const QUrl& url)
    : QTreeWidgetItem(view),
      d(new Private)
{
    setImageUrl(url);
}

AdvancedRenameListItem:: ~AdvancedRenameListItem()
{
    delete d;
}

void AdvancedRenameListItem::setImageUrl(const QUrl& url)
{
    d->imageUrl = url;

    QFileInfo fi(d->imageUrl.toLocalFile());
    d->completeFileName  = fi.fileName();

    setName(d->completeFileName);
    setNewName(d->completeFileName);
}

QUrl AdvancedRenameListItem::imageUrl() const
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
    QColor normalText = qApp->palette().text().color();
    setForeground(OldName, invalid ? Qt::red : normalText);
    setForeground(NewName, invalid ? Qt::red : normalText);
}

bool AdvancedRenameListItem::isNameEqual() const
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
        buttons(0),
        advancedRenameManager(0),
        advancedRenameWidget(0)
    {
    }

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
    QDialogButtonBox*      buttons;

    AdvancedRenameManager* advancedRenameManager;
    AdvancedRenameWidget*  advancedRenameWidget;
    NewNamesList           newNamesList;
};

const QString AdvancedRenameDialog::Private::configGroupName(QLatin1String("AdvancedRename Dialog"));
const QString AdvancedRenameDialog::Private::configLastUsedRenamePatternEntry(QLatin1String("Last Used Rename Pattern"));
const QString AdvancedRenameDialog::Private::configDialogSizeEntry(QLatin1String("Dialog Size"));

// --------------------------------------------------------

AdvancedRenameDialog::AdvancedRenameDialog(QWidget* const parent)
    : QDialog(parent),
      d(new Private)
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

void AdvancedRenameDialog::setupWidgets()
{
    d->buttons = new QDialogButtonBox(QDialogButtonBox::Help | QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    d->buttons->button(QDialogButtonBox::Ok)->setDefault(true);

    d->advancedRenameManager = new AdvancedRenameManager();
    d->advancedRenameWidget  = new AdvancedRenameWidget(this);
    d->advancedRenameManager->setWidget(d->advancedRenameWidget);

    // --------------------------------------------------------

    d->sortActionName = new QAction(i18n("By Name"), this);
    d->sortActionDate = new QAction(i18n("By Date"), this);
    d->sortActionSize = new QAction(i18n("By File Size"), this);

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
    d->listView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    d->listView->header()->setSectionResizeMode(1, QHeaderView::Stretch);
    d->listView->setWhatsThis(i18n("This list shows the results for your renaming pattern. Red items indicate a "
                                   "name collision, either because the new name is equal to the current name, "
                                   "or because the name has already been assigned to another item."));

    // --------------------------------------------------------

    QWidget* const mainWidget     = new QWidget(this);
    QGridLayout* const mainLayout = new QGridLayout;
    mainLayout->addWidget(d->listView,             0, 0, 1, 1);
    mainLayout->addWidget(d->advancedRenameWidget, 1, 0, 1, 1);
    mainLayout->setRowStretch(0, 10);
    mainWidget->setLayout(mainLayout);

    QVBoxLayout* const vbx = new QVBoxLayout(this);
    vbx->addWidget(mainWidget);
    vbx->addWidget(d->buttons);
    setLayout(vbx);

    setMinimumWidth(d->advancedRenameWidget->minimumWidth());
}

void AdvancedRenameDialog::setupConnections()
{
    connect(d->advancedRenameWidget, SIGNAL(signalTextChanged(QString)),
            this, SLOT(slotParseStringChanged(QString)));

    connect(d->advancedRenameWidget, SIGNAL(signalReturnPressed()),
            this, SLOT(slotReturnPressed()));

    connect(d->advancedRenameManager, SIGNAL(signalSortingChanged(QList<QUrl>)),
            this, SLOT(slotAddImages(QList<QUrl>)));

    connect(d->listView, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotShowContextMenu(QPoint)));

    connect(d->sortGroupActions, SIGNAL(triggered(QAction*)),
            this, SLOT(slotSortActionTriggered(QAction*)));

    connect(d->sortGroupDirections, SIGNAL(triggered(QAction*)),
            this, SLOT(slotSortDirectionTriggered(QAction*)));

    connect(d->buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(accept()));

    connect(d->buttons->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
            this, SLOT(reject()));

    connect(d->buttons->button(QDialogButtonBox::Help), SIGNAL(clicked()),
            this, SLOT(slotHelp()));
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
        QUrl url = QUrl::fromLocalFile(file);
        new AdvancedRenameListItem(d->listView, url);
    }

    // set current filename if only one image has been added
    if (d->singleFileMode)
    {
        QFileInfo info(fileList.first());
        d->advancedRenameWidget->setParseString(info.completeBaseName());
        d->advancedRenameWidget->setParseTimerDuration(50);
        d->advancedRenameWidget->focusLineEdit();
        d->advancedRenameWidget->highlightLineEdit();
        d->singleFileModeOldFilename = info.fileName();
    }

    d->buttons->button(QDialogButtonBox::Ok)->setEnabled(checkNewNames());
}

void AdvancedRenameDialog::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    QSize s = group.readEntry(d->configDialogSizeEntry, QSize(d->minSizeDialog, d->minSizeDialog));
    resize(s);
    d->advancedRenameWidget->setParseString(group.readEntry(d->configLastUsedRenamePatternEntry, QString()));
}

void AdvancedRenameDialog::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
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

void AdvancedRenameDialog::slotReturnPressed()
{
    if (d->buttons->button(QDialogButtonBox::Ok)->isEnabled())
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
    QMenu menu(this);
    menu.addSection(i18n("Sort Images"));

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
    settings.useOriginalFileExtension = true;
    // settings.useOriginalFileExtension = d->singleFileMode ? false : true;
    d->advancedRenameManager->parseFiles(parseString, settings);

    // fill the tree widget with the updated files
    QTreeWidgetItemIterator it(d->listView);

    while (*it)
    {
        AdvancedRenameListItem* const item = dynamic_cast<AdvancedRenameListItem*>((*it));

        if (item)
        {
            QString newName = d->advancedRenameManager->newName(item->imageUrl().toLocalFile());
            item->setNewName(newName);
            d->newNamesList << NewNameInfo(item->imageUrl(), newName);
        }

        ++it;
    }

    bool enableBtn = checkNewNames() && !parseString.isEmpty();
    d->buttons->button(QDialogButtonBox::Ok)->setEnabled(enableBtn);

    d->listView->viewport()->update();

    if (!d->singleFileMode)
    {
        QApplication::restoreOverrideCursor();
    }
}

void AdvancedRenameDialog::slotAddImages(const QList<QUrl>& urls)
{
    if (urls.isEmpty())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "No item to process";
        return;
    }

    d->listView->clear();
    d->advancedRenameManager->reset();
    QList<ParseSettings> files;

    foreach(const QUrl& url, urls)
    {
        ParseSettings ps;
        ps.fileUrl = url;
        files << ps;
        qCDebug(DIGIKAM_GENERAL_LOG) << url;
    }

    d->advancedRenameManager->addFiles(files);

    initDialog();
    slotParseStringChanged(d->advancedRenameWidget->parseString());
}

NewNamesList AdvancedRenameDialog::newNames() const
{
    return filterNewNames();
}

bool AdvancedRenameDialog::checkNewNames() const
{
    int numNames      = 0;
    int numEqualNames = 0;
    bool ok           = true;

    QSet<QString> tmpNewNames;
    QTreeWidgetItemIterator it(d->listView);

    while (*it)
    {
        AdvancedRenameListItem* const item = dynamic_cast<AdvancedRenameListItem*>((*it));

        if (item)
        {
            ++numNames;
            QFileInfo fi(item->imageUrl().toLocalFile());

            QString completeNewName = fi.path();
            completeNewName.append(QLatin1Char('/'));
            completeNewName.append(item->newName());

            bool invalid = tmpNewNames.contains(completeNewName);
            tmpNewNames << completeNewName;

            item->markInvalid(invalid);
            ok &= !invalid;

            if (item->isNameEqual())
            {
                ++ numEqualNames;
            }
        }

        ++it;
    }

    return (ok && !(numNames == numEqualNames));
}

NewNamesList AdvancedRenameDialog::filterNewNames() const
{
    NewNamesList filteredNames;
    QTreeWidgetItemIterator it(d->listView);

    while (*it)
    {
        AdvancedRenameListItem* const item = dynamic_cast<AdvancedRenameListItem*>((*it));

        if (item && !item->isNameEqual())
        {
            filteredNames << NewNameInfo(item->imageUrl(), item->newName());
        }

        ++it;
    }

    return filteredNames;
}

void AdvancedRenameDialog::slotHelp()
{
    DXmlGuiWindow::openHandbook();
}

}  // namespace Digikam
