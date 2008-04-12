/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-22
 * Description : a generic widget to display metadata
 *
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <qheader.h>
#include <qwhatsthis.h>
#include <qpainter.h>
#include <qhbuttongroup.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qdragobject.h> 
#include <qclipboard.h>
#include <qsimplerichtext.h>
#include <qpaintdevicemetrics.h>
#include <qstylesheet.h>
#include <qlistview.h>
#include <qtooltip.h>

// KDE includes.

#include <kdialogbase.h>
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
#include "searchtextbar.h"
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
        searchBar    = 0;
    }

    QGridLayout            *mainLayout;

    QHButtonGroup          *toolButtons;
    QHButtonGroup          *levelButtons;

    QByteArray              metadata;

    QString                 fileName;

    MetadataListView       *view;

    SearchTextBar          *searchBar;

    DMetadata::MetaDataMap  metaDataMap;
};

MetadataWidget::MetadataWidget(QWidget* parent, const char* name)
              : QWidget(parent, name)
{
    d = new MetadataWidgetPriv;

    d->mainLayout = new QGridLayout(this, 3, 4, KDialog::spacingHint(), KDialog::spacingHint());
    KIconLoader *iconLoader = KApplication::kApplication()->iconLoader();

    d->levelButtons = new QHButtonGroup(this);
    d->levelButtons->setInsideMargin( 0 );
    d->levelButtons->setExclusive(true);
    d->levelButtons->setFrameShape(QFrame::NoFrame);

    QPushButton *simpleLevel = new QPushButton( d->levelButtons );
    simpleLevel->setPixmap( iconLoader->loadIcon( "ascii", (KIcon::Group)KIcon::Toolbar ) );
    simpleLevel->setToggleButton(true);
    QWhatsThis::add( simpleLevel, i18n( "Switch the tags view to a simple human-readable list" ) );
    QToolTip::add( simpleLevel, i18n( "Simple list" ));
    d->levelButtons->insert(simpleLevel, SIMPLE);

    QPushButton *fullLevel = new QPushButton( d->levelButtons );
    fullLevel->setPixmap( iconLoader->loadIcon( "document", (KIcon::Group)KIcon::Toolbar ) );
    fullLevel->setToggleButton(true);
    QWhatsThis::add( fullLevel, i18n( "Switch the tags view to a full list" ) );
    QToolTip::add( fullLevel, i18n( "Full list" ));
    d->levelButtons->insert(fullLevel, FULL);

    d->toolButtons = new QHButtonGroup(this);
    d->toolButtons->setInsideMargin( 0 );
    d->toolButtons->setFrameShape(QFrame::NoFrame);

    QPushButton *saveMetadata = new QPushButton( d->toolButtons );
    saveMetadata->setPixmap( iconLoader->loadIcon( "filesave", (KIcon::Group)KIcon::Toolbar ) );
    QWhatsThis::add( saveMetadata, i18n( "Save metadata to a binary file" ) );
    QToolTip::add( saveMetadata, i18n( "Save metadata" ));
    d->toolButtons->insert(saveMetadata);
    
    QPushButton *printMetadata = new QPushButton( d->toolButtons );
    printMetadata->setPixmap( iconLoader->loadIcon( "fileprint", (KIcon::Group)KIcon::Toolbar ) );
    QWhatsThis::add( printMetadata, i18n( "Print metadata to printer" ) );
    QToolTip::add( printMetadata, i18n( "Print metadata" ));
    d->toolButtons->insert(printMetadata);

    QPushButton *copy2ClipBoard = new QPushButton( d->toolButtons );
    copy2ClipBoard->setPixmap( iconLoader->loadIcon( "editcopy", (KIcon::Group)KIcon::Toolbar ) );
    QWhatsThis::add( copy2ClipBoard, i18n( "Copy metadata to clipboard" ) );
    QToolTip::add( copy2ClipBoard, i18n( "Copy metadata to clipboard" ));
    d->toolButtons->insert(copy2ClipBoard);

    d->view         = new MetadataListView(this);
    QString barName = QString(name) + "SearchBar";
    d->searchBar    = new SearchTextBar(this, barName.ascii());

    // -----------------------------------------------------------------

    d->mainLayout->addMultiCellWidget(d->levelButtons, 0, 0, 0, 1);
    d->mainLayout->addMultiCellWidget(d->toolButtons,  0, 0, 4, 4);
    d->mainLayout->addMultiCellWidget(d->view,         1, 1, 0, 4);
    d->mainLayout->addMultiCellWidget(d->searchBar,    2, 2, 0, 4);
    d->mainLayout->setRowStretch(1, 10);
    d->mainLayout->setColStretch(3, 10);

    // -----------------------------------------------------------------
    
    connect(d->levelButtons, SIGNAL(released(int)),
            this, SLOT(slotModeChanged(int)));

    connect(copy2ClipBoard, SIGNAL(clicked()),
            this, SLOT(slotCopy2Clipboard()));

    connect(printMetadata, SIGNAL(clicked()),
            this, SLOT(slotPrintMetadata()));

    connect(saveMetadata, SIGNAL(clicked()),
            this, SLOT(slotSaveMetadataToFile()));     

    connect(d->searchBar, SIGNAL(signalTextChanged(const QString&)),
            d->view, SLOT(slotSearchTextChanged(const QString&)));

    connect(d->view, SIGNAL(signalTextFilterMatch(bool)),
            d->searchBar, SLOT(slotSearchResult(bool)));                   
}

MetadataWidget::~MetadataWidget()
{
    delete d;
}

MetadataListView* MetadataWidget::view()
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

bool MetadataWidget::storeMetadataToFile(const KURL& url)
{
    if( url.isEmpty() )
        return false;

    QFile file(url.path());
    if ( !file.open(IO_WriteOnly) ) 
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

void MetadataWidget::slotCopy2Clipboard()
{
    QString textmetadata = i18n("File name: %1 (%2)").arg(d->fileName).arg(getMetadataTitle());
    QListViewItemIterator it( d->view );

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
            QListViewItem *item = it.current();
            textmetadata.append(item->text(0));
            textmetadata.append(" : ");
            textmetadata.append(item->text(1));
            textmetadata.append("\n");
        }

        ++it;
    }

    QApplication::clipboard()->setData(new QTextDrag(textmetadata), QClipboard::Clipboard);
}

void MetadataWidget::slotPrintMetadata()
{
    QString textmetadata = i18n("<p><big><big><b>File name: %1 (%2)</b></big></big>")
                           .arg(d->fileName)
                           .arg(getMetadataTitle());
    QListViewItemIterator it( d->view );

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
            QListViewItem *item = it.current();
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

        QPaintDeviceMetrics metrics(p.device());
        int dpiy = metrics.logicalDpiY();
        int margin = (int) ( (2/2.54)*dpiy ); // 2 cm margins
        QRect view( margin, margin, metrics.width() - 2*margin, metrics.height() - 2*margin );
        QFont font(KApplication::font());
        font.setPointSize( 10 ); // we define 10pt to be a nice base size for printing
        QSimpleRichText richText( textmetadata, font,
                                  QString(),
                                  QStyleSheet::defaultSheet(),
                                  QMimeSourceFactory::defaultFactory(),
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

KURL MetadataWidget::saveMetadataToFile(const QString& caption, const QString& fileFilter)
{
    KFileDialog fileSaveDialog(KGlobalSettings::documentPath(),
                               QString(),
                               this,
                               "MetadataFileSaveDialog",
                               false);

    fileSaveDialog.setOperationMode(KFileDialog::Saving);
    fileSaveDialog.setMode(KFile::File);
    fileSaveDialog.setSelection(d->fileName);
    fileSaveDialog.setCaption(caption);
    fileSaveDialog.setFilter(fileFilter);

    // Check for cancel.
    if ( fileSaveDialog.exec() == KFileDialog::Accepted )
        return fileSaveDialog.selectedURL().path();
    
    return KURL();
}

void MetadataWidget::setMode(int mode)
{
    if (d->levelButtons->selectedId() == mode)
        return;

    d->levelButtons->setButton(mode);
    buildView();
}

int MetadataWidget::getMode()
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

bool MetadataWidget::loadFromData(const QString& fileName, const QByteArray& data)
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

void MetadataWidget::setUserAreaWidget(QWidget *w)
{
    QVBoxLayout *vLayout = new QVBoxLayout( KDialog::spacingHint() ); 
    vLayout->addWidget(w);
    vLayout->addStretch();
    d->mainLayout->addMultiCellLayout(vLayout, 3, 3, 0, 4);
}

void MetadataWidget::buildView()
{
    d->view->slotSearchTextChanged(d->searchBar->text());
}

}  // namespace Digikam
