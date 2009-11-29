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
#include "parsesettings.h"
#include "parser.h"

namespace Digikam
{

class AdvancedRenameListItemPriv
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

class AdvancedRenameDialogPriv
{
public:

    AdvancedRenameDialogPriv() :
        configGroupName("AdvancedRename Dialog"),
        configLastUsedRenamePatternEntry("Last Used Rename Pattern"),
        configDialogSizeEntry("Dialog Size"),

        singleFileMode(false),
        minSizeDialog(450),
        listView(0),
        advancedRenameWidget(0)
    {}

    const QString         configGroupName;
    const QString         configLastUsedRenamePatternEntry;
    const QString         configDialogSizeEntry;

    QString               singleFileModeOldFilename;

    bool                  singleFileMode;
    int                   minSizeDialog;

    QTreeWidget*          listView;
    AdvancedRenameWidget* advancedRenameWidget;
    NewNamesList          newNamesList;
};

AdvancedRenameDialog::AdvancedRenameDialog(QWidget* parent)
                    : KDialog(parent), d(new AdvancedRenameDialogPriv)
{
    d->advancedRenameWidget = new AdvancedRenameWidget(this);
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

void AdvancedRenameDialog::slotReturnPressed()
{
    if (isButtonEnabled(Ok))
    {
        accept();
    }
}

void AdvancedRenameDialog::slotParseStringChanged(const QString& parseString)
{
    d->newNamesList.clear();
    d->advancedRenameWidget->parser()->init();

    QTreeWidgetItemIterator it(d->listView);
    while (*it)
    {
        AdvancedRenameListItem* item = dynamic_cast<AdvancedRenameListItem*>((*it));
        if (item)
        {
            ParseSettings settings(ImageInfo(item->imageUrl()));

            QString newName = d->advancedRenameWidget->parse(settings);
            item->setNewName(newName);
            d->newNamesList << NewNameInfo(item->imageUrl(), newName);
        }
        ++it;
    }

    d->advancedRenameWidget->parser()->reset();

    bool enableBtn = !parseString.isEmpty() && checkNewNames();
    enableButton(Ok, enableBtn);

    d->listView->viewport()->update();
}

void AdvancedRenameDialog::slotAddImages(const KUrl::List& urls)
{
    d->listView->clear();
    AdvancedRenameListItem* item = 0;

    int itemCount = 0;
    for (KUrl::List::const_iterator it = urls.constBegin(); it != urls.constEnd(); ++it)
    {
        item = new AdvancedRenameListItem(d->listView);
        item->setImageUrl(*it);
        ++itemCount;
    }

    // set current filename if only one image has been added
    if (itemCount == 1)
    {
        QFileInfo info(urls.first().toLocalFile());
        d->advancedRenameWidget->setText(info.baseName());
        d->advancedRenameWidget->focusLineEdit();
        d->singleFileModeOldFilename = info.fileName();
    }
    d->singleFileMode = (itemCount <= 1);

    enableButton(Ok, checkNewNames());
    initDialog(itemCount);
    slotParseStringChanged(d->advancedRenameWidget->text());
}

void AdvancedRenameDialog::initDialog(int count)
{
    QString title = i18np("Rename", "Rename (%1 images)", count);
    setWindowTitle(title);
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

    QSize s = group.readEntry(d->configDialogSizeEntry, QSize(d->minSizeDialog, d->minSizeDialog));
    resize(s);
    d->advancedRenameWidget->setText(group.readEntry(d->configLastUsedRenamePatternEntry, QString()));
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
        group.writeEntry(d->configLastUsedRenamePatternEntry, d->advancedRenameWidget->text());
    }
}

bool AdvancedRenameDialog::checkNewNames()
{
    QSet<QString> tmpNewNames;
    bool ok = true;

    QTreeWidgetItemIterator it(d->listView);
    while (*it)
    {
        AdvancedRenameListItem* item = dynamic_cast<AdvancedRenameListItem*>((*it));
        if (item)
        {
            bool valid = !item->isInvalid() && ( !tmpNewNames.contains(item->newName()) );
            ok         = ok && valid;
            item->markInvalid(!valid);
            tmpNewNames << item->newName();
        }
        ++it;
    }

    return ok;
}

}  // namespace Digikam
