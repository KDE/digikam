/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-22
 * Description : a generic widget to display metadata
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "metadatawidget.h"

// Qt includes

#include <QButtonGroup>
#include <QClipboard>
#include <QDataStream>
#include <QFile>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QMap>
#include <QMimeData>
#include <QPaintDevice>
#include <QPainter>
#include <QPointer>
#include <QPrintDialog>
#include <QPrinter>
#include <QPushButton>
#include <QTextDocument>
#include <QToolButton>
#include <QVBoxLayout>
#include <QActionGroup>
#include <QStandardPaths>
#include <QMenu>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "metadatalistview.h"
#include "metadatalistviewitem.h"
#include "mdkeylistviewitem.h"
#include "searchtextbar.h"
#include "setup.h"
#include "dfiledialog.h"

namespace Digikam
{

class MetadataWidget::Private
{

public:

    Private()
    {
        view           = 0;
        mainLayout     = 0;
        filterBtn      = 0;
        toolBtn        = 0;
        searchBar      = 0;
        optionsMenu    = 0;
        noneAction     = 0;
        photoAction    = 0;
        customAction   = 0;
        settingsAction = 0;
    }

    QAction*               noneAction;
    QAction*               photoAction;
    QAction*               customAction;
    QAction*               settingsAction;

    QGridLayout*           mainLayout;

    QToolButton*           filterBtn;
    QToolButton*           toolBtn;

    QString                fileName;

    QStringList            tagsFilter;

    QMenu*                 optionsMenu;

    MetadataListView*      view;

    SearchTextBar*         searchBar;

    DMetadata              metadata;
    DMetadata::MetaDataMap metaDataMap;
};

MetadataWidget::MetadataWidget(QWidget* const parent, const QString& name)
    : QWidget(parent),
      d(new Private)
{
    setObjectName(name);

    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    d->mainLayout = new QGridLayout(this);

    // -----------------------------------------------------------------

    d->filterBtn  = new QToolButton(this);
    d->filterBtn->setToolTip(i18n("Tags filter options"));
    d->filterBtn->setIcon(QIcon::fromTheme(QLatin1String("view-filter")));
    d->filterBtn->setPopupMode(QToolButton::InstantPopup);
    d->filterBtn->setWhatsThis(i18n("Apply tags filter over metadata."));

    d->optionsMenu                  = new QMenu(d->filterBtn);
    QActionGroup* const filterGroup = new QActionGroup(this);

    d->noneAction     = d->optionsMenu->addAction(i18n("No filter"));
    d->noneAction->setCheckable(true);
    filterGroup->addAction(d->noneAction);
    d->photoAction    = d->optionsMenu->addAction(i18n("Photograph"));
    d->photoAction->setCheckable(true);
    filterGroup->addAction(d->photoAction);
    d->customAction   = d->optionsMenu->addAction(i18n("Custom"));
    d->customAction->setCheckable(true);
    filterGroup->addAction(d->customAction);
    d->optionsMenu->addSeparator();
    d->settingsAction = d->optionsMenu->addAction(i18n("Settings"));
    d->settingsAction->setCheckable(false);

    filterGroup->setExclusive(true);
    d->filterBtn->setMenu(d->optionsMenu);

    // -----------------------------------------------------------------

    d->toolBtn = new QToolButton(this);
    d->toolBtn->setToolTip(i18n("Tools"));
    d->toolBtn->setIcon(QIcon::fromTheme(QLatin1String("system-run")));
    d->toolBtn->setPopupMode(QToolButton::InstantPopup);
    d->toolBtn->setWhatsThis(i18n("Run tool over metadata tags."));

    QMenu* const toolMenu         = new QMenu(d->toolBtn);
    QAction* const saveMetadata   = toolMenu->addAction(i18nc("@action:inmenu", "Save in file"));
    QAction* const printMetadata  = toolMenu->addAction(i18nc("@action:inmenu", "Print"));
    QAction* const copy2ClipBoard = toolMenu->addAction(i18nc("@action:inmenu", "Copy to Clipboard"));
    d->toolBtn->setMenu(toolMenu);

    d->view         = new MetadataListView(this);

    QString barName = name + QLatin1String("SearchBar");
    d->searchBar    = new SearchTextBar(this, barName);

    // -----------------------------------------------------------------

    d->mainLayout->addWidget(d->filterBtn, 0, 0, 1, 1);
    d->mainLayout->addWidget(d->searchBar, 0, 1, 1, 3);
    d->mainLayout->addWidget(d->toolBtn,   0, 4, 1, 1);
    d->mainLayout->addWidget(d->view,      1, 0, 1, 5);
    d->mainLayout->setColumnStretch(2, 10);
    d->mainLayout->setRowStretch(1, 10);
    d->mainLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    d->mainLayout->setSpacing(0);

    // -----------------------------------------------------------------

    connect(d->optionsMenu, SIGNAL(triggered(QAction*)),
            this, SLOT(slotFilterChanged(QAction*)));

    connect(copy2ClipBoard, SIGNAL(triggered(bool)),
            this, SLOT(slotCopy2Clipboard()));

    connect(printMetadata, SIGNAL(triggered(bool)),
            this, SLOT(slotPrintMetadata()));

    connect(saveMetadata, SIGNAL(triggered(bool)),
            this, SLOT(slotSaveMetadataToFile()));

    connect(d->searchBar, SIGNAL(signalSearchTextSettings(SearchTextSettings)),
            d->view, SLOT(slotSearchTextChanged(SearchTextSettings)));

    connect(d->view, SIGNAL(signalTextFilterMatch(bool)),
            d->searchBar, SLOT(slotSearchResult(bool)));
}

MetadataWidget::~MetadataWidget()
{
    delete d;
}

void MetadataWidget::slotFilterChanged(QAction* action)
{
    if (action == d->settingsAction)
    {
        emit signalSetupMetadataFilters();
    }
    else if (action == d->noneAction  ||
             action == d->photoAction ||
             action == d->customAction)
    {
        buildView();
    }
}

QStringList MetadataWidget::getTagsFilter() const
{
    return d->tagsFilter;
}

void MetadataWidget::setTagsFilter(const QStringList& list)
{
    d->tagsFilter = list;
    buildView();
}

MetadataListView* MetadataWidget::view() const
{
    return d->view;
}

void MetadataWidget::enabledToolButtons(bool b)
{
    d->toolBtn->setEnabled(b);
}

bool MetadataWidget::setMetadata(const DMetadata& data)
{
    d->metadata = DMetadata(data);

    // Cleanup all metadata contents.
    setMetadataMap();

    if (d->metadata.isEmpty())
    {
        setMetadataEmpty();
        return false;
    }

    // Try to decode current metadata.
    if (decodeMetadata())
    {
        enabledToolButtons(true);
    }
    else
    {
        enabledToolButtons(false);
    }

    // Refresh view using decoded metadata.
    buildView();

    return true;
}

void MetadataWidget::setMetadataEmpty()
{
    d->view->clear();
    enabledToolButtons(false);
}

const DMetadata& MetadataWidget::getMetadata()
{
    return d->metadata;
}

bool MetadataWidget::storeMetadataToFile(const QUrl& url, const QByteArray& metaData)
{
    if ( url.isEmpty() )
    {
        return false;
    }

    QFile file(url.toLocalFile());

    if ( !file.open(QIODevice::WriteOnly) )
    {
        return false;
    }

    QDataStream stream( &file );
    stream.writeRawData(metaData.data(), metaData.size());
    file.close();
    return true;
}

void MetadataWidget::setMetadataMap(const DMetadata::MetaDataMap& data)
{
    d->metaDataMap = data;
}

const DMetadata::MetaDataMap& MetadataWidget::getMetadataMap()
{
    return d->metaDataMap;
}

void MetadataWidget::setIfdList(const DMetadata::MetaDataMap& ifds, const QStringList& tagsFilter)
{
    d->view->setIfdList(ifds, tagsFilter);
}

void MetadataWidget::setIfdList(const DMetadata::MetaDataMap& ifds, const QStringList& keysFilter,
                                const QStringList& tagsFilter)
{
    d->view->setIfdList(ifds, keysFilter, tagsFilter);
}

void MetadataWidget::slotCopy2Clipboard()
{
    QString textmetadata  = i18n("File name: %1 (%2)",d->fileName,getMetadataTitle());
    int i                 = 0;
    QTreeWidgetItem* item = 0;

    do
    {
        item                            = d->view->topLevelItem(i);
        MdKeyListViewItem* const lvItem = dynamic_cast<MdKeyListViewItem*>(item);

        if (lvItem)
        {
            textmetadata.append(QLatin1String("\n\n>>> "));
            textmetadata.append(lvItem->getDecryptedKey());
            textmetadata.append(QLatin1String(" <<<\n\n"));

            int j                  = 0;
            QTreeWidgetItem* item2 = 0;

            do
            {
                item2                               = dynamic_cast<QTreeWidgetItem*>(lvItem)->child(j);
                MetadataListViewItem* const lvItem2 = dynamic_cast<MetadataListViewItem*>(item2);

                if (lvItem2)
                {
                    textmetadata.append(lvItem2->text(0));
                    textmetadata.append(QLatin1String(" : "));
                    textmetadata.append(lvItem2->text(1));
                    textmetadata.append(QLatin1String("\n"));
                }

                ++j;
            }
            while (item2);
        }

        ++i;
    }
    while (item);

    QMimeData* const mimeData = new QMimeData();
    mimeData->setText(textmetadata);
    QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);
}

void MetadataWidget::slotPrintMetadata()
{
    QString textmetadata = i18n("<p><big><big><b>File name: %1 (%2)</b></big></big>",
                                d->fileName, getMetadataTitle());

    int i                 = 0;
    QTreeWidgetItem* item = 0;

    do
    {
        item                            = d->view->topLevelItem(i);
        MdKeyListViewItem* const lvItem = dynamic_cast<MdKeyListViewItem*>(item);

        if (lvItem)
        {
            textmetadata.append(QLatin1String("<br/><br/><b>"));
            textmetadata.append(lvItem->getDecryptedKey());
            textmetadata.append(QLatin1String("</b><br/><br/>"));

            int j                  = 0;
            QTreeWidgetItem* item2 = 0;

            do
            {
                item2                               = dynamic_cast<QTreeWidgetItem*>(lvItem)->child(j);
                MetadataListViewItem* const lvItem2 = dynamic_cast<MetadataListViewItem*>(item2);

                if (lvItem2)
                {
                    textmetadata.append(lvItem2->text(0));
                    textmetadata.append(QLatin1String(" : <i>"));
                    textmetadata.append(lvItem2->text(1));
                    textmetadata.append(QLatin1String("</i><br/>"));
                }

                ++j;
            }
            while (item2);
        }

        ++i;
    }
    while (item);

    textmetadata.append(QLatin1String("</p>"));

    QPrinter printer;
    printer.setFullPage(true);

    QPointer<QPrintDialog> dialog = new QPrintDialog(&printer, qApp->activeWindow());

    if (dialog->exec())
    {
        QTextDocument doc;
        doc.setHtml(textmetadata);
        QFont font(QApplication::font());
        font.setPointSize(10);                // we define 10pt to be a nice base size for printing.
        doc.setDefaultFont(font);
        doc.print(&printer);
    }

    delete dialog;
}

QUrl MetadataWidget::saveMetadataToFile(const QString& caption, const QString& fileFilter)
{
    QPointer<DFileDialog> fileSaveDialog = new DFileDialog(this, caption, QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    fileSaveDialog->setAcceptMode(DFileDialog::AcceptSave);
    fileSaveDialog->setFileMode(DFileDialog::AnyFile);
    fileSaveDialog->selectFile(d->fileName);
    fileSaveDialog->setNameFilter(fileFilter);

    QList<QUrl> urls;

    // Check for cancel.
    if (fileSaveDialog->exec() == QDialog::Accepted)
    {
        urls = fileSaveDialog->selectedUrls();
    }

    delete fileSaveDialog;
    return (!urls.isEmpty() ? urls[0] : QUrl());
}

void MetadataWidget::setMode(int mode)
{
    if (getMode() == mode)
    {
        return;
    }

    if (mode == NONE)
        d->noneAction->setChecked(true);
    else if (mode == PHOTO)
        d->photoAction->setChecked(true);
    else
        d->customAction->setChecked(true);

    buildView();
}

int MetadataWidget::getMode() const
{
    if (d->noneAction->isChecked())
        return NONE;
    else if (d->photoAction->isChecked())
        return PHOTO;

    return CUSTOM;
}

QString MetadataWidget::getCurrentItemKey() const
{
    return d->view->getCurrentItemKey();
}

void MetadataWidget::setCurrentItemByKey(const QString& itemKey)
{
    d->view->setCurrentItemByKey(itemKey);
}

bool MetadataWidget::loadFromData(const QString& fileName, const DMetadata& data)
{
    setFileName(fileName);
    return(setMetadata(data));
}

QString MetadataWidget::getTagTitle(const QString&)
{
    return QString();
}

QString MetadataWidget::getTagDescription(const QString&)
{
    return QString();
}

void MetadataWidget::setFileName(const QString& fileName)
{
    d->fileName = fileName;
}

void MetadataWidget::setUserAreaWidget(QWidget* const w)
{
    QVBoxLayout* const vLayout = new QVBoxLayout();
    vLayout->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    vLayout->addWidget(w);
    vLayout->addStretch();
    d->mainLayout->addLayout(vLayout, 3, 0, 1, 5);
}

void MetadataWidget::buildView()
{
    d->view->slotSearchTextChanged(d->searchBar->searchTextSettings());
}

}  // namespace Digikam
