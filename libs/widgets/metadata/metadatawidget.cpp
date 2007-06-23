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

#include <qlayout.h>
#include <qmap.h>
#include <qfile.h> 
#include <qmime.h>
#include <q3header.h>

#include <qpainter.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <q3dragobject.h> 
#include <qclipboard.h>
#include <q3simplerichtext.h>
#include <q3paintdevicemetrics.h>
#include <q3stylesheet.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3Frame>
#include <Q3VBoxLayout>
#include <Q3HButtonGroup>
// KDE includes.

#include <k3listview.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kglobalsettings.h>
#include <kprinter.h>
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
    }

    Q3GridLayout            *mainLayout;

    Q3HButtonGroup          *toolButtons;
    Q3HButtonGroup          *levelButtons;

    QByteArray              metadata;

    QString                 fileName;

    MetadataListView       *view;

    DMetadata::MetaDataMap  metaDataMap;
};

MetadataWidget::MetadataWidget(QWidget* parent)
              : QWidget(parent)
{
    d = new MetadataWidgetPriv;

    d->mainLayout = new Q3GridLayout(this, 2, 4, KDialog::spacingHint(), KDialog::spacingHint());
    KIconLoader *iconLoader = KApplication::kApplication()->iconLoader();

    d->levelButtons = new Q3HButtonGroup(this);
    d->levelButtons->setInsideMargin( 0 );
    d->levelButtons->setExclusive(true);
    d->levelButtons->setFrameShape(Q3Frame::NoFrame);

    QPushButton *simpleLevel = new QPushButton( d->levelButtons );
    simpleLevel->setPixmap( iconLoader->loadIcon( "ascii", (KIcon::Group)KIcon::Toolbar ) );
    simpleLevel->setToggleButton(true);
    simpleLevel->setWhatsThis( i18n( "Toggle tags view to a simple human-readable list" ) );
    d->levelButtons->insert(simpleLevel, SIMPLE);

    QPushButton *fullLevel = new QPushButton( d->levelButtons );
    fullLevel->setPixmap( iconLoader->loadIcon( "document", (KIcon::Group)KIcon::Toolbar ) );
    fullLevel->setToggleButton(true);
    fullLevel->setWhatsThis( i18n( "Toggle tags view to a full list" ) );
    d->levelButtons->insert(fullLevel, FULL);

    d->toolButtons = new Q3HButtonGroup(this);
    d->toolButtons->setInsideMargin( 0 );
    d->toolButtons->setFrameShape(Q3Frame::NoFrame);

    QPushButton *saveMetadata = new QPushButton( d->toolButtons );
    saveMetadata->setPixmap( iconLoader->loadIcon( "filesave", (KIcon::Group)KIcon::Toolbar ) );
    saveMetadata->setWhatsThis( i18n( "Save meta-data to a binary file" ) );
    d->toolButtons->insert(saveMetadata);
    
    QPushButton *printMetadata = new QPushButton( d->toolButtons );
    printMetadata->setPixmap( iconLoader->loadIcon( "fileprint", (KIcon::Group)KIcon::Toolbar ) );
    printMetadata->setWhatsThis( i18n( "Print meta-data to printer" ) );
    d->toolButtons->insert(printMetadata);

    QPushButton *copy2ClipBoard = new QPushButton( d->toolButtons );
    copy2ClipBoard->setPixmap( iconLoader->loadIcon( "editcopy", (KIcon::Group)KIcon::Toolbar ) );
    copy2ClipBoard->setWhatsThis( i18n( "Copy meta-data to clipboard" ) );
    d->toolButtons->insert(copy2ClipBoard);

    d->mainLayout->addMultiCellWidget(d->levelButtons, 0, 0, 0, 1);
    d->mainLayout->setColStretch(3, 10);
    d->mainLayout->addMultiCellWidget(d->toolButtons, 0, 0, 4, 4);

    d->view = new MetadataListView(this);
    d->mainLayout->addMultiCellWidget(d->view, 1, 1, 0, 4);

    // -----------------------------------------------------------------
    
    connect(d->levelButtons, SIGNAL(released(int)),
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
    d->toolButtons->setEnabled(b);
}

bool MetadataWidget::setMetadata(const QByteArray& data)
{
    d->metadata = data;
    
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

const QByteArray& MetadataWidget::getMetadata()
{
    return d->metadata;
}

bool MetadataWidget::storeMetadataToFile(const KUrl& url)
{
    if( url.isEmpty() )
        return false;

    QFile file(url.path());
    if ( !file.open(QIODevice::WriteOnly) ) 
        return false;
    
    QDataStream stream( &file );
    stream.writeRawBytes(d->metadata.data(), d->metadata.size());
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

    QApplication::clipboard()->setData(new Q3TextDrag(textmetadata), QClipboard::Clipboard);
}

void MetadataWidget::slotPrintMetadata(void)
{
    QString textmetadata = i18n("<p><big><big><b>File name: %1 (%2)</b></big></big>")
                           ,d->fileName,
                           getMetadataTitle();
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

    KPrinter printer;
    printer.setFullPage( true );

    if ( printer.setup( this ) )
    {
        QPainter p( &printer );

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
            richText.draw( &p, margin, margin, view, colorGroup() );
            view.moveBy( 0, view.height() );
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
    KFileDialog fileSaveDialog(KUrl(KGlobalSettings::documentPath()),
                               QString(),
                               this,
                               false);

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
    if (d->levelButtons->selectedId() == mode)
        return;

    d->levelButtons->setButton(mode);
    buildView();
}

int MetadataWidget::getMode(void)
{
    int level = d->levelButtons->selectedId();
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

bool MetadataWidget::loadFromData(QString fileName, const QByteArray& data)
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
    Q3VBoxLayout *vLayout = new Q3VBoxLayout( KDialog::spacingHint() ); 
    vLayout->addWidget(w);
    vLayout->addStretch();
    d->mainLayout->addMultiCellLayout(vLayout, 2, 2, 0, 4);
}

}  // namespace Digikam

