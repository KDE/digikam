/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-02-22
 * Description : a generic widget to display metadata
 *
 * Copyright 2006 by Gilles Caulier
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
#include <qcombobox.h>
#include <qpainter.h>
#include <qhbuttongroup.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qdragobject.h> 
#include <qclipboard.h>
#include <qsimplerichtext.h>
#include <qpaintdevicemetrics.h>
#include <qstylesheet.h>

// KDE includes.

#include <kdialogbase.h>
#include <klistview.h>
#include <klocale.h>
#include <kprinter.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kapplication.h>

// Local includes.

#include "metadatalistview.h"
#include "mdkeylistviewitem.h"
#include "metadatawidget.h"

namespace Digikam
{

class MetadataWidgetPriv
{

public:

    MetadataWidgetPriv()
    {
        modeCombo  = 0;
        view       = 0;
        mainLayout = 0;
    }

    QGridLayout                 *mainLayout;

    QByteArray                   metadata;

    QComboBox                   *modeCombo;
    
    QString                      fileName;
    
    MetadataListView            *view;
    
    MetadataWidget::MetaDataMap  metaDataMap;
};

MetadataWidget::MetadataWidget(QWidget* parent, const char* name)
              : QWidget(parent, name)
{
    d = new MetadataWidgetPriv;

    d->mainLayout = new QGridLayout(this, 2, 4, KDialog::marginHint(), KDialog::spacingHint());

    QLabel* modeLabel = new QLabel(i18n("Level of detail:"), this);
    d->modeCombo      = new QComboBox(this);
    d->modeCombo->insertItem(i18n("Simple"));
    d->modeCombo->insertItem(i18n("Full"));
    d->mainLayout->addMultiCellWidget(modeLabel, 0, 0, 0, 1);
    d->mainLayout->addMultiCellWidget(d->modeCombo, 0, 0, 2, 2);

    QWhatsThis::add( d->modeCombo, i18n("<p>Select here the information level to display<p>"
                                        "<b>Simple</b>: display general information about the photograph "
                                        " (default).<p>"
                                        "<b>Full</b>: display all sections.") );
    d->mainLayout->setColStretch(3, 10);

    KIconLoader *iconLoader = KApplication::kApplication()->iconLoader();
    QHButtonGroup *toolButtons = new QHButtonGroup(this);
    toolButtons->setInsideMargin( 0 );
    toolButtons->setFrameShape(QFrame::NoFrame);

    QPushButton *printMetadata = new QPushButton( toolButtons );
    printMetadata->setPixmap( iconLoader->loadIcon( "fileprint", (KIcon::Group)KIcon::Toolbar ) );
    QWhatsThis::add( printMetadata, i18n( "Print metadata to printer" ) );
    toolButtons->insert(printMetadata);
    
    QPushButton *copy2ClipBoard = new QPushButton( toolButtons );
    copy2ClipBoard->setPixmap( iconLoader->loadIcon( "editcopy", (KIcon::Group)KIcon::Toolbar ) );
    QWhatsThis::add( copy2ClipBoard, i18n( "Copy metadata to clipboard" ) );
    toolButtons->insert(copy2ClipBoard);

    d->mainLayout->addMultiCellWidget(toolButtons, 0, 0, 4, 4);

    d->view = new MetadataListView(this);
    d->mainLayout->addMultiCellWidget(d->view, 1, 1, 0, 4);

    connect(d->modeCombo, SIGNAL(activated(int)),
            this, SLOT(slotModeChanged(int)));

    connect(copy2ClipBoard, SIGNAL(clicked()),
            this, SLOT(slotCopy2Clipboard()));

    connect(printMetadata, SIGNAL(clicked()),
            this, SLOT(slotPrintMetadata()));
}

MetadataWidget::~MetadataWidget()
{
    delete d;
}

MetadataListView* MetadataWidget::view(void)
{
    return d->view;
}

bool MetadataWidget::setMetadata(const QByteArray& data)
{
    d->metadata = data;

    if (d->metadata.isEmpty())
    {
        d->view->clear();
        return false;
    }
    
    // Cleanup all metadata contents and try to decode current metadata.
    setMetadataMap();
    decodeMetadata();

    // Refresh view using decoded metadata.
    buildView();
    
    return true;
}

const QByteArray& MetadataWidget::getMetadata()
{
    return d->metadata;
}

void MetadataWidget::setMetadataMap(const MetaDataMap& data)
{
    d->metaDataMap = data;
}

const MetadataWidget::MetaDataMap& MetadataWidget::getMetadataMap()
{
    return d->metaDataMap;
}

void MetadataWidget::setIfdList(const MetaDataMap &ifds, const QStringList& tagsFilter)
{
    d->view->setIfdList(ifds, tagsFilter);
}

void MetadataWidget::setIfdList(const MetaDataMap &ifds, const QStringList& keysFilter,
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

void MetadataWidget::slotPrintMetadata(void)
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
                                  QString::null,
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

void MetadataWidget::setMode(int mode)
{
    if (d->modeCombo->currentItem() == mode)
        return;

    d->modeCombo->setCurrentItem(mode);
    buildView();
}

int MetadataWidget::getMode(void)
{
    return d->modeCombo->currentItem();
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
    return (QString::null);
}

QString MetadataWidget::getTagDescription(const QString&)
{
    return (QString::null);
}

void MetadataWidget::setFileName(QString fileName)
{
    d->fileName = fileName;
}

void MetadataWidget::setUserAreaWidget(QWidget *w)
{
    QVBoxLayout *vLayout = new QVBoxLayout( KDialog::spacingHint() ); 
    vLayout->addWidget(w);
    vLayout->addStretch();
    d->mainLayout->addMultiCellLayout(vLayout, 2, 2, 0, 4);
}
}  // namespace Digikam

#include "metadatawidget.moc"
