/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-22
 * Description : a generic widget to display metadata
 *
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <Q3SimpleRichText>
#include <Q3PaintDeviceMetrics>
#include <Q3StyleSheet>
#include <Q3MimeSourceFactory>
#include <QColorGroup>
#include <QMimeData>
#include <QClipboard>
#include <QMap>
#include <QFile> 
#include <QPainter>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QFrame>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QDataStream>
#include <QColorGroup>
#include <QPrinter>
#include <QPrintDialog>

// KDE includes.

#include <k3listview.h>
#include <klocale.h>
#include <kglobal.h>
#include <kfiledialog.h>
#include <kglobalsettings.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kapplication.h>

// Local includes.

#include "ddebug.h"
#include "metadatalistview.h"
#include "mdkeylistviewitem.h"
#include "metadatawidget.h"
#include "metadatawidget.moc"

namespace Digikam
{

class MetadataWidgetPriv
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
    }

    QWidget                *levelGBox;
    QWidget                *toolsGBox;

    QGridLayout            *mainLayout;

    QButtonGroup           *toolButtons;
    QButtonGroup           *levelButtons;

    QString                 fileName;

    MetadataListView       *view;

    DMetadata               metadata;
    DMetadata::MetaDataMap  metaDataMap;
};

MetadataWidget::MetadataWidget(QWidget* parent, const char* name)
              : QWidget(parent)
{
    d = new MetadataWidgetPriv;
    setObjectName(name);

    d->mainLayout           = new QGridLayout(this);
    KIconLoader* iconLoader = KIconLoader::global();

    // -----------------------------------------------------------------

    d->levelGBox       = new QWidget(this);
    d->levelButtons    = new QButtonGroup(d->levelGBox);
    QHBoxLayout *hlay1 = new QHBoxLayout(d->levelGBox);
    d->levelButtons->setExclusive(true);

    QPushButton *simpleLevel = new QPushButton( d->levelGBox );
    simpleLevel->setIcon( iconLoader->loadIcon( "user", (KIconLoader::Group)KIconLoader::Toolbar ) );
    simpleLevel->setCheckable(true);
    simpleLevel->setWhatsThis( i18n( "Toggle tags view to a simple human-readable list" ) );
    d->levelButtons->addButton(simpleLevel, SIMPLE);
    hlay1->addWidget(simpleLevel);

    QPushButton *fullLevel = new QPushButton( d->levelGBox );
    fullLevel->setIcon( iconLoader->loadIcon( "media-playlist", (KIconLoader::Group)KIconLoader::Toolbar ) );
    fullLevel->setCheckable(true);
    fullLevel->setWhatsThis( i18n( "Toggle tags view to a full list" ) );
    d->levelButtons->addButton(fullLevel, FULL);

    hlay1->addWidget(fullLevel);
    hlay1->setSpacing(0);
    hlay1->setMargin(KDialog::spacingHint());

    // -----------------------------------------------------------------

    d->toolsGBox       = new QWidget(this);
    d->toolButtons     = new QButtonGroup(d->toolsGBox);
    QHBoxLayout *hlay2 = new QHBoxLayout(d->toolsGBox);

    QPushButton *saveMetadata = new QPushButton( d->toolsGBox );
    saveMetadata->setIcon( iconLoader->loadIcon( "document-save", (KIconLoader::Group)KIconLoader::Toolbar ) );
    saveMetadata->setWhatsThis( i18n( "Save meta-data to a binary file" ) );
    d->toolButtons->addButton(saveMetadata);

    QPushButton *printMetadata = new QPushButton( d->toolsGBox );
    printMetadata->setIcon( iconLoader->loadIcon( "document-print", (KIconLoader::Group)KIconLoader::Toolbar ) );
    printMetadata->setWhatsThis( i18n( "Print meta-data to printer" ) );
    d->toolButtons->addButton(printMetadata);

    QPushButton *copy2ClipBoard = new QPushButton( d->toolsGBox );
    copy2ClipBoard->setIcon( iconLoader->loadIcon( "edit-copy", (KIconLoader::Group)KIconLoader::Toolbar ) );
    copy2ClipBoard->setWhatsThis( i18n( "Copy meta-data to clipboard" ) );
    d->toolButtons->addButton(copy2ClipBoard);

    hlay2->addWidget(saveMetadata);
    hlay2->addWidget(printMetadata);
    hlay2->addWidget(copy2ClipBoard);
    hlay2->setSpacing(0);
    hlay1->setMargin(KDialog::spacingHint());

    d->view = new MetadataListView(this);

    // -----------------------------------------------------------------

    d->mainLayout->addWidget(d->levelGBox, 0, 0, 1, 2 );
    d->mainLayout->setColumnStretch(3, 10);
    d->mainLayout->addWidget(d->toolsGBox, 0, 4, 1, 1);
    d->mainLayout->addWidget(d->view, 1, 0, 1, 5 );
    d->mainLayout->setSpacing(0);
    d->mainLayout->setMargin(0);

    // -----------------------------------------------------------------

    connect(d->levelButtons, SIGNAL(buttonReleased(int)),
            this, SLOT(slotModeChanged(int)));

    connect(copy2ClipBoard, SIGNAL(clicked()),
            this, SLOT(slotCopy2Clipboard()));

    connect(printMetadata, SIGNAL(clicked()),
            this, SLOT(slotPrintMetadata()));

    connect(saveMetadata, SIGNAL(clicked()),
            this, SLOT(slotSaveMetadataToFile()));
}

MetadataWidget::~MetadataWidget()
{
    delete d;
}

MetadataListView* MetadataWidget::view(void)
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
        enabledToolButtons(true);
    else
        enabledToolButtons(false);

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
    if( url.isEmpty() )
        return false;

    QFile file(url.path());
    if ( !file.open(QIODevice::WriteOnly) ) 
        return false;

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

void MetadataWidget::setIfdList(const DMetadata::MetaDataMap &ifds, const QStringList& tagsFilter)
{
    d->view->setIfdList(ifds, tagsFilter);
}

void MetadataWidget::setIfdList(const DMetadata::MetaDataMap &ifds, const QStringList& keysFilter,
                                const QStringList& tagsFilter)
{
    d->view->setIfdList(ifds, keysFilter, tagsFilter);
}

void MetadataWidget::slotModeChanged(int)
{
    buildView();
}

void MetadataWidget::slotCopy2Clipboard(void)
{
    QString textmetadata = i18n("File name: %1 (%2)",d->fileName,getMetadataTitle());
    Q3ListViewItemIterator it( d->view );

    while ( it.current() )
    {
        if ( !it.current()->isSelectable() )
        {
            MdKeyListViewItem *item = dynamic_cast<MdKeyListViewItem *>(it.current());
            textmetadata.append("\n\n>>> ");
            textmetadata.append(item->getMdKey());
            textmetadata.append(" <<<\n\n");
        }
        else
        {
            Q3ListViewItem *item = it.current();
            textmetadata.append(item->text(0));
            textmetadata.append(" : ");
            textmetadata.append(item->text(1));
            textmetadata.append("\n");
        }

        ++it;
    }

    QMimeData *mimeData = new QMimeData();
    mimeData->setText(textmetadata);
    QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);
}

void MetadataWidget::slotPrintMetadata(void)
{
    QString textmetadata = i18n("<p><big><big><b>File name: %1 (%2)</b></big></big>",
                                d->fileName, getMetadataTitle());
    Q3ListViewItemIterator it( d->view );

    while ( it.current() )
    {
        if ( !it.current()->isSelectable() )
        {
            MdKeyListViewItem *item = dynamic_cast<MdKeyListViewItem *>(it.current());
            textmetadata.append("<br><br><b>");
            textmetadata.append(item->getMdKey());
            textmetadata.append("</b><br><br>");
        }
        else
        {
            Q3ListViewItem *item = it.current();
            textmetadata.append(item->text(0));
            textmetadata.append(" : <i>");
            textmetadata.append(item->text(1));
            textmetadata.append("</i><br>");
        }

        ++it;
    }

    textmetadata.append("</p>");

    QPrinter printer;
    printer.setFullPage( true );

    QPrintDialog dialog(&printer, kapp->activeWindow());
    if (dialog.exec())
    {
        QPainter p(&printer);

        if ( !p.device() ) 
            return;

        Q3PaintDeviceMetrics metrics(p.device());
        int dpiy = metrics.logicalDpiY();
        int margin = (int) ( (2/2.54)*dpiy ); // 2 cm margins
        QRect view( margin, margin, metrics.width() - 2*margin, metrics.height() - 2*margin );
        QFont font(KApplication::font());
        font.setPointSize( 10 ); // we define 10pt to be a nice base size for printing
        Q3SimpleRichText richText( textmetadata, font,
                                  QString(),
                                  Q3StyleSheet::defaultSheet(),
                                  Q3MimeSourceFactory::defaultFactory(),
                                  view.height() );
        richText.setWidth( &p, view.width() );
        int page = 1;

        do
        {
            richText.draw( &p, margin, margin, view, QColorGroup(palette()) );
            view.translate( 0, view.height() );
            p.translate( 0 , -view.height() );
            p.setFont( font );
            p.drawText( view.right() - p.fontMetrics().width( QString::number( page ) ),
                        view.bottom() + p.fontMetrics().ascent() + 5, QString::number( page ) );

            if ( view.top() - margin >= richText.height() )
                break;

            printer.newPage();
            page++;
        }
        while (true);
    }
}

KUrl MetadataWidget::saveMetadataToFile(const QString& caption, const QString& fileFilter)
{
    KFileDialog fileSaveDialog(KUrl(KGlobalSettings::documentPath()), QString(), this);
    fileSaveDialog.setOperationMode(KFileDialog::Saving);
    fileSaveDialog.setMode(KFile::File);
    fileSaveDialog.setSelection(d->fileName);
    fileSaveDialog.setCaption(caption);
    fileSaveDialog.setFilter(fileFilter);

    // Check for cancel.
    if ( fileSaveDialog.exec() == KFileDialog::Accepted )
        return fileSaveDialog.selectedUrl().path();

    return KUrl();
}

void MetadataWidget::setMode(int mode)
{
    if (d->levelButtons->checkedId() == mode)
        return;

    d->levelButtons->button(mode)->setChecked(true);
    buildView();
}

int MetadataWidget::getMode(void)
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

bool MetadataWidget::loadFromData(QString fileName, const DMetadata& data)
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

void MetadataWidget::setFileName(QString fileName)
{
    d->fileName = fileName;
}

void MetadataWidget::setUserAreaWidget(QWidget *w)
{
    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->setSpacing( KDialog::spacingHint() );
    vLayout->addWidget(w);
    vLayout->addStretch();
    d->mainLayout->addLayout(vLayout, 2, 0, 1, 5 );
}

}  // namespace Digikam
