/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-22
 * Description : a generic widget to display metadata
 *
 * Copyright (C) 2006-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "metadatawidget.moc"

// Qt includes

#include <QButtonGroup>
#include <QClipboard>
#include <QColorGroup>
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

// KDE includes

#include <kapplication.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>

// Libkexiv2 includes

#include <libkexiv2/version.h>

// Local includes

#include "metadatalistview.h"
#include "metadatalistviewitem.h"
#include "mdkeylistviewitem.h"
#include "searchtextbar.h"

namespace Digikam
{

class MetadataWidget::MetadataWidgetPriv
{

public:

    MetadataWidgetPriv()
    {
        toolButtons  = 0;
        levelButtons = 0;
        view         = 0;
        mainLayout   = 0;
        toolsGBox    = 0;
        levelGBox    = 0;
        searchBar    = 0;
    }

    QWidget*               levelGBox;
    QWidget*               toolsGBox;

    QGridLayout*           mainLayout;

    QButtonGroup*          toolButtons;
    QButtonGroup*          levelButtons;

    QString                fileName;

    QStringList            tagsFilter;

    MetadataListView*      view;

    SearchTextBar*         searchBar;

    DMetadata              metadata;
    DMetadata::MetaDataMap metaDataMap;
};

MetadataWidget::MetadataWidget(QWidget* parent, const char* name)
    : QWidget(parent), d(new MetadataWidgetPriv)
{
    setObjectName(name);

    d->mainLayout           = new QGridLayout(this);
    KIconLoader* iconLoader = KIconLoader::global();

    // -----------------------------------------------------------------

    d->levelGBox       = new QWidget(this);
    d->levelButtons    = new QButtonGroup(d->levelGBox);
    QHBoxLayout* hlay1 = new QHBoxLayout(d->levelGBox);
    d->levelButtons->setExclusive(true);

    QToolButton* simpleLevel = new QToolButton(d->levelGBox);
    simpleLevel->setIcon(iconLoader->loadIcon("user-identity", (KIconLoader::Group)KIconLoader::Toolbar));
    simpleLevel->setCheckable(true);
#if KEXIV2_VERSION >= 0x010000
    simpleLevel->setWhatsThis(i18n("Switch the tags view to a custom human-readable list. "
                                   "To customize the tag filter list, go to the Metadata configuration panel."));
    simpleLevel->setToolTip(i18n("Custom list"));
#else
    simpleLevel->setWhatsThis(i18n("Switch the tags view to a human-readable list"));
    simpleLevel->setToolTip(i18n("Human-readable list"));
#endif
    d->levelButtons->addButton(simpleLevel, CUSTOM);

    QToolButton* fullLevel = new QToolButton(d->levelGBox);
    fullLevel->setIcon(iconLoader->loadIcon("view-media-playlist", (KIconLoader::Group)KIconLoader::Toolbar));
    fullLevel->setCheckable(true);
    fullLevel->setWhatsThis(i18n("Switch the tags view to a full list"));
    fullLevel->setToolTip(i18n("Full list"));
    d->levelButtons->addButton(fullLevel, FULL);

    hlay1->addWidget(simpleLevel);
    hlay1->addWidget(fullLevel);
    hlay1->setSpacing(KDialog::spacingHint());
    hlay1->setMargin(0);

    // -----------------------------------------------------------------

    d->toolsGBox       = new QWidget(this);
    d->toolButtons     = new QButtonGroup(d->toolsGBox);
    QHBoxLayout* hlay2 = new QHBoxLayout(d->toolsGBox);

    QToolButton* saveMetadata = new QToolButton(d->toolsGBox);
    saveMetadata->setIcon(iconLoader->loadIcon("document-save", (KIconLoader::Group)KIconLoader::Toolbar));
    saveMetadata->setWhatsThis(i18n("Save metadata to a binary file"));
    saveMetadata->setToolTip(i18n("Save metadata"));
    d->toolButtons->addButton(saveMetadata);

    QToolButton* printMetadata = new QToolButton(d->toolsGBox);
    printMetadata->setIcon(iconLoader->loadIcon("document-print", (KIconLoader::Group)KIconLoader::Toolbar));
    printMetadata->setWhatsThis(i18n("Print metadata to printer"));
    printMetadata->setToolTip(i18n("Print metadata"));
    d->toolButtons->addButton(printMetadata);

    QToolButton* copy2ClipBoard = new QToolButton(d->toolsGBox);
    copy2ClipBoard->setIcon( iconLoader->loadIcon("edit-copy", (KIconLoader::Group)KIconLoader::Toolbar));
    copy2ClipBoard->setWhatsThis(i18n("Copy metadata to clipboard"));
    copy2ClipBoard->setToolTip(i18n("Copy metadata to clipboard"));
    d->toolButtons->addButton(copy2ClipBoard);

    hlay2->addWidget(saveMetadata);
    hlay2->addWidget(printMetadata);
    hlay2->addWidget(copy2ClipBoard);
    hlay2->setSpacing(KDialog::spacingHint());
    hlay1->setMargin(0);

    d->view         = new MetadataListView(this);
    QString barName = QString(name) + "SearchBar";
    d->searchBar    = new SearchTextBar(this, barName.toAscii());

    // -----------------------------------------------------------------

    d->mainLayout->addWidget(d->levelGBox, 0, 0, 1, 2);
    d->mainLayout->addWidget(d->toolsGBox, 0, 4, 1, 1);
    d->mainLayout->addWidget(d->view,      1, 0, 1, 5);
    d->mainLayout->addWidget(d->searchBar, 2, 0, 1, 5);
    d->mainLayout->setColumnStretch(3, 10);
    d->mainLayout->setRowStretch(1, 10);
    d->mainLayout->setSpacing(0);
    d->mainLayout->setMargin(KDialog::spacingHint());

    // -----------------------------------------------------------------

    connect(d->levelButtons, SIGNAL(buttonReleased(int)),
            this, SLOT(slotModeChanged(int)));

    connect(copy2ClipBoard, SIGNAL(clicked()),
            this, SLOT(slotCopy2Clipboard()));

    connect(printMetadata, SIGNAL(clicked()),
            this, SLOT(slotPrintMetadata()));

    connect(saveMetadata, SIGNAL(clicked()),
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
    d->toolsGBox->setEnabled(b);
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

bool MetadataWidget::storeMetadataToFile(const KUrl& url, const QByteArray& metaData)
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

void MetadataWidget::slotModeChanged(int)
{
    buildView();
}

void MetadataWidget::slotCopy2Clipboard()
{
    QString textmetadata  = i18n("File name: %1 (%2)",d->fileName,getMetadataTitle());
    int i                 = 0;
    QTreeWidgetItem* item = 0;

    do
    {
        item                      = d->view->topLevelItem(i);
        MdKeyListViewItem* lvItem = dynamic_cast<MdKeyListViewItem*>(item);

        if (lvItem)
        {
            textmetadata.append("\n\n>>> ");
            textmetadata.append(lvItem->getDecryptedKey());
            textmetadata.append(" <<<\n\n");

            int j                  = 0;
            QTreeWidgetItem* item2 = 0;

            do
            {
                item2                         = dynamic_cast<QTreeWidgetItem*>(lvItem)->child(j);
                MetadataListViewItem* lvItem2 = dynamic_cast<MetadataListViewItem*>(item2);

                if (lvItem2)
                {
                    textmetadata.append(lvItem2->text(0));
                    textmetadata.append(" : ");
                    textmetadata.append(lvItem2->text(1));
                    textmetadata.append("\n");
                }

                ++j;
            }
            while (item2);
        }

        ++i;
    }
    while (item);

    QMimeData* mimeData = new QMimeData();
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
        item                      = d->view->topLevelItem(i);
        MdKeyListViewItem* lvItem = dynamic_cast<MdKeyListViewItem*>(item);

        if (lvItem)
        {
            textmetadata.append("<br/><br/><b>");
            textmetadata.append(lvItem->getDecryptedKey());
            textmetadata.append("</b><br/><br/>");

            int j                  = 0;
            QTreeWidgetItem* item2 = 0;

            do
            {
                item2                         = dynamic_cast<QTreeWidgetItem*>(lvItem)->child(j);
                MetadataListViewItem* lvItem2 = dynamic_cast<MetadataListViewItem*>(item2);

                if (lvItem2)
                {
                    textmetadata.append(lvItem2->text(0));
                    textmetadata.append(" : <i>");
                    textmetadata.append(lvItem2->text(1));
                    textmetadata.append("</i><br/>");
                }

                ++j;
            }
            while (item2);
        }

        ++i;
    }
    while (item);

    textmetadata.append("</p>");

    QPrinter printer;
    printer.setFullPage(true);

    QPointer<QPrintDialog> dialog = new QPrintDialog(&printer, kapp->activeWindow());

    if (dialog->exec())
    {
        QTextDocument doc;
        doc.setHtml(textmetadata);
        QFont font(KApplication::font());
        font.setPointSize(10);                // we define 10pt to be a nice base size for printing.
        doc.setDefaultFont(font);
        doc.print(&printer);
    }

    delete dialog;
}

KUrl MetadataWidget::saveMetadataToFile(const QString& caption, const QString& fileFilter)
{
    QPointer<KFileDialog> fileSaveDialog = new KFileDialog(KUrl(KGlobalSettings::documentPath()),
            QString(), this);
    fileSaveDialog->setOperationMode(KFileDialog::Saving);
    fileSaveDialog->setMode(KFile::File);
    fileSaveDialog->setSelection(d->fileName);
    fileSaveDialog->setCaption(caption);
    fileSaveDialog->setFilter(fileFilter);

    // Check for cancel.
    if ( fileSaveDialog->exec() == KFileDialog::Accepted )
    {
        KUrl selUrl = fileSaveDialog->selectedUrl();
        delete fileSaveDialog;
        return selUrl;
    }

    delete fileSaveDialog;
    return KUrl();
}

void MetadataWidget::setMode(int mode)
{
    if (d->levelButtons->checkedId() == mode)
    {
        return;
    }

    d->levelButtons->button(mode)->setChecked(true);
    buildView();
}

int MetadataWidget::getMode()
{
    int level = d->levelButtons->checkedId();
    return level;
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

void MetadataWidget::setUserAreaWidget(QWidget* w)
{
    QVBoxLayout* vLayout = new QVBoxLayout();
    vLayout->setSpacing(KDialog::spacingHint());
    vLayout->addWidget(w);
    vLayout->addStretch();
    d->mainLayout->addLayout(vLayout, 3, 0, 1, 5);
}

void MetadataWidget::buildView()
{
    d->view->slotSearchTextChanged(d->searchBar->searchTextSettings());
}

}  // namespace Digikam
