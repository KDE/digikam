/* ============================================================
 * Author: Caulier Gilles <caulier dot gilles at free.fr>
 * Date  : 2004-11-17
 * Description :
 *
 * Copyright 2003 by Gilles Caulier
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
#include <qcolor.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qvgroupbox.h>
#include <qevent.h> 
#include <qtextedit.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qwhatsthis.h>
#include <qtooltip.h>
#include <qdatetime.h>
#include <qsize.h>
#include <qrect.h>
#include <qcheckbox.h>

// KDE includes.

#include <kmimetype.h>
#include <kfileitem.h>
#include <klocale.h>
#include <kselect.h>
#include <kdialogbase.h>
#include <kdeversion.h>
#include <kapplication.h>
#include <kcursor.h>
#include <kseparator.h> 
#include <ksqueezedtextlabel.h>
#include <kglobal.h>
#include <kfilemetainfo.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>

// LibKexif includes.

#include <libkexif/kexififd.h>
#include <libkexif/kexifentry.h>
#include <libkexif/kexifdata.h>
#include <libkexif/kexiflistview.h>
#include <libkexif/kexiflistviewitem.h>
#include <libkexif/kexif.h>
#include <libkexif/kexifutils.h>

// Local includes.

#include "imagehistogram.h"
#include "histogramwidget.h"
#include "colorgradientwidget.h"

#include "albumfolderview.h"
#include "albumfolderitem.h"
#include "albumiconview.h"
#include "albumiconitem.h"
#include "albumlister.h"
#include "albummanager.h"
#include "albumdb.h"
#include "album.h"
#include "albumsettings.h"

#include "imageproperties.h"


ExifThumbLabel::ExifThumbLabel(QWidget * parent)
               :QLabel(parent)
{
    m_popmenu = new KPopupMenu(this);
    m_popmenu->setCheckable(true);
    m_popmenu->insertTitle(i18n("Correct Exif Orientation Tag"));
    m_popmenu->insertItem(i18n("Normal"), 11);
    m_popmenu->insertItem(i18n("Flipped Horizontally"), 12);
    m_popmenu->insertItem(i18n("Rotated 180 Degrees"), 13);
    m_popmenu->insertItem(i18n("Flipped Vertically"), 14);
    m_popmenu->insertItem(i18n("Rotated 90 Degrees / Horiz. Flipped"), 15);
    m_popmenu->insertItem(i18n("Rotated 90 Degrees"), 16);
    m_popmenu->insertItem(i18n("Rotated 90 Degrees / Vert. Flipped"), 17);
    m_popmenu->insertItem(i18n("Rotated 270 Degrees"), 18);
}

ExifThumbLabel::~ExifThumbLabel()
{
    delete m_popmenu;
}

void ExifThumbLabel::setOrientationMenu(KExifData *currExifData, KURL currentUrl)
{
    m_currentUrl = currentUrl;
    
    for (int i = 11 ; i <= 18 ; ++i)
        m_popmenu->setItemChecked(i, false); 
    
    int orient = currExifData->getImageOrientation();
    m_popmenu->setItemChecked(orient + 10, true);
}

void ExifThumbLabel::mousePressEvent( QMouseEvent * e)
{
    if (e->button() !=  Qt::RightButton) return;

    int orientation = m_popmenu->exec(QCursor::pos());
    
    if (orientation == -1)
       return;
    
    orientation = orientation - 10;
          
    kdDebug() << "Setting Exif Orientation to " << orientation << endl;

    KExifData::ImageOrientation o = (KExifData::ImageOrientation)orientation;

    if (!KExifUtils::writeOrientation(m_currentUrl.path(), o))
        {
        KMessageBox::sorry(0, i18n("Failed to correct Exif orientation for file %1.")
                          .arg(m_currentUrl.filename()));
        return;
        }
        
    for (int i = 11 ; i <= 18 ; ++i)
        m_popmenu->setItemChecked(i, false);            
                
    m_popmenu->setItemChecked(orientation + 10, true);
}

////////////////////////////////////////////////////////////////////////////////////////

// Constructor with AlbumIconView and AlbumIconItem instance.

ImageProperties::ImageProperties(AlbumIconView* view, AlbumIconItem* currItem, 
                                 QWidget *parent, QRect *selectionArea)
               : KDialogBase(Tabbed, QString::null, 
                             Help|User1|User2|Stretch|Close,
                             Close, parent, 0, true, true, 
                             KStdGuiItem::guiItem(KStdGuiItem::Forward), 
                             KStdGuiItem::guiItem(KStdGuiItem::Back))
{
    m_view          = view;                          // Needed for PAlbum using.
    m_currItem      = currItem;
    
    m_IEcurrentURL  = currItem->fileItem()->url();   // With Image Editor mode, save current idem url (used if 
                                                     // an image selection area is passed in constructor).
    
    m_currfileURL   = m_currItem->fileItem()->url();    

    m_selectionArea = selectionArea;
    
    setupGui();
}
    
// Constructor without AlbumIconView and AlbumIconItem instance and with KURL list (Stand Alone mode).

ImageProperties::ImageProperties(KURL::List filesList, KURL currentFile, 
                                 QWidget *parent, QRect *selectionArea)
               : KDialogBase(Tabbed, QString::null, 
                             Help|User1|User2|Stretch|Close,
                             Close, parent, 0, true, true, 
                             KStdGuiItem::guiItem(KStdGuiItem::Forward), 
                             KStdGuiItem::guiItem(KStdGuiItem::Back))
{
    m_view          = 0L;
    m_currItem      = 0L;
    
    m_currfileURL   = currentFile;    
    m_urlList       = filesList; 
    m_urlListIt     = m_urlList.find(m_currfileURL);   

    m_selectionArea = selectionArea;
    
    setupGui();
}

void ImageProperties::setupGui(void)
{
    parentWidget()->setCursor( KCursor::waitCursor() );
    setHelp("propertiesmetadatahistogram.anchor", "digikam");        
    
    //General tab init.
    
    setupGeneralTab();
    
    // Exif Viewer init.
    
    m_ExifData = 0L;
    
    setupExifViewer();
        
    // HistoGramViewer init.
    
    m_histogramWidget = 0L;
    m_hGradient       = 0L;

    setupHistogramViewer();

    // Read config.
    
    kapp->config()->setGroup("Image Properties Dialog");
    showPage(kapp->config()->readNumEntry("Tab Actived", 0));                             // General tab.
    m_levelExifCB->setCurrentItem(kapp->config()->readNumEntry("Exif Level", 0));         // General Exif level.
    m_embeddedThumbBox->setChecked(kapp->config()->readBoolEntry("Show Exif Thumb",
                                                                 true));                  // Exif thumb on.
    m_currentGeneralExifItemName = kapp->config()->readEntry("General Exif Item",
                                                             QString::null);
    m_currentExtendedExifItemName = kapp->config()->readEntry("Extended Exif Item",
                                                              QString::null);
    m_currentAllExifItemName = kapp->config()->readEntry("All Exif Item",
                                                         QString::null);
    m_channelCB->setCurrentItem(kapp->config()->readNumEntry("Histogram Channel", 0));    // Luminosity.
    m_scaleCB->setCurrentItem(kapp->config()->readNumEntry("Histogram Scale", 0));        // Linear.
    m_colorsCB->setCurrentItem(kapp->config()->readNumEntry("Histogram Color", 0));       // Red.
    m_renderingCB->setCurrentItem(kapp->config()->readNumEntry("Histogram Rendering", 0));// Full image.

    // Init all info data.
    
    slotItemChanged();
    
    resize(configDialogSize("Image Properties Dialog"));
    
    parentWidget()->setCursor( KCursor::arrowCursor() );       
}

ImageProperties::~ImageProperties()
{
    // If there is a currently histogram computation when dialog is closed,
    // stop it before the m_image data are deleted automaticly!
    m_histogramWidget->stopHistogramComputation();
   
    // Save config.
    getCurrentExifItem();
    
    kapp->config()->setGroup("Image Properties Dialog");
    kapp->config()->writeEntry("Tab Actived", activePageIndex());
    kapp->config()->writeEntry("Exif Level", m_levelExifCB->currentItem());
    kapp->config()->writeEntry("Show Exif Thumb", m_embeddedThumbBox->isChecked());
    kapp->config()->writeEntry("General Exif Item", m_currentGeneralExifItemName);
    kapp->config()->writeEntry("Extended Exif Item", m_currentExtendedExifItemName);
    kapp->config()->writeEntry("All Exif Item", m_currentAllExifItemName);
    kapp->config()->writeEntry("Histogram Channel", m_channelCB->currentItem());
    kapp->config()->writeEntry("Histogram Scale", m_scaleCB->currentItem());
    kapp->config()->writeEntry("Histogram Color", m_colorsCB->currentItem());
    kapp->config()->writeEntry("Histogram Rendering", m_renderingCB->currentItem());

    saveDialogSize("Image Properties Dialog");
    
    // For Exif viewer.
    
    if (m_ExifData)
        delete m_ExifData;
    
    // For histogram viever.
    
    if ( m_histogramWidget )
       delete m_histogramWidget;
    
    if ( m_hGradient )        
       delete m_hGradient;
}

void ImageProperties::slotUser1()
{
    if (m_view)             // Digikam embeded mode.
       {
       if (!m_currItem)
          return;

       m_currItem = dynamic_cast<AlbumIconItem*>(m_currItem->nextItem());
       m_currfileURL = m_currItem->fileItem()->url();    
       slotItemChanged();
       }
    else                    // Stand Alone mode.
       {
       m_currfileURL = *(m_urlListIt++);   
       slotItemChanged();
       }
}

void ImageProperties::slotUser2()
{
    if (m_view)             // Digikam embeded mode.
       {
       if (!m_currItem)
           return;
    
       m_currItem = dynamic_cast<AlbumIconItem*>(m_currItem->prevItem());
       m_currfileURL = m_currItem->fileItem()->url();    
       slotItemChanged();
       }
    else                    // Stand Alone mode.
       {
       m_currfileURL = *(m_urlListIt--);    
       slotItemChanged();
       }
}

void ImageProperties::slotItemChanged()
{
    if (m_view)             // Digikam embeded mode.
       {
       if (!m_currItem)
          return;
       }
    else                    // Stand Alone mode.
       {
       if (!m_currfileURL.isValid())
          return;
       }
       
    setCursor( KCursor::waitCursor() );
    
    if (!m_HistogramThumbJob.isNull())
        m_HistogramThumbJob->kill();

    if (!m_HistogramThumbJob.isNull())
        delete m_HistogramThumbJob;

    if (!m_generalThumbJob.isNull())
        m_generalThumbJob->kill();

    if (!m_generalThumbJob.isNull())
        delete m_generalThumbJob;

    setCaption(i18n("Properties for \"%1\"").arg(m_currfileURL.fileName()));
    
    // -------------------------------------------------------------                                         
    // Update General tab.
    
    m_filename->clear();
    m_filetype->clear();
    m_filedim->clear();
    m_filepath->clear();
    m_filedate->clear();
    m_filesize->clear();
    m_fileowner->clear();
    m_filepermissions->clear();
    m_filealbum->clear();
    m_filecomments->clear();
    m_filetags->clear();
    
    m_generalThumbJob = new ThumbnailJob(m_currfileURL, 128);
    
    connect(m_generalThumbJob,
            SIGNAL(signalThumbnailMetaInfo(const KURL&,
                                           const QPixmap&,
                                           const KFileMetaInfo*)),
            SLOT(slotGotGeneralThumbnail(const KURL&,
                                         const QPixmap&,
                                         const KFileMetaInfo*)));

    connect(m_generalThumbJob,
            SIGNAL(signalFailed(const KURL&)),
            SLOT(slotFailedGeneralThumbnail(const KURL&)));       

    // File system informations
    
    KFileItem* fi = new KFileItem(KFileItem::Unknown,
                                  KFileItem::Unknown,
                                  m_currfileURL);

    m_filename->setText( m_currfileURL.fileName() );
    m_filetype->setText( KMimeType::findByURL(m_currfileURL)->name() );
#if KDE_VERSION >= 0x30200
    KFileMetaInfo meta(m_currfileURL);
#else
    KFileMetaInfo meta(m_currfileURL.path());
#endif

    if (meta.isValid())
        {
        QSize dims;
        
        if (meta.containsGroup("Jpeg EXIF Data"))
            dims = meta.group("Jpeg EXIF Data").item("Dimensions").value().toSize();
        else if (meta.containsGroup("General"))
            dims = meta.group("General").item("Dimensions").value().toSize();
        else if (meta.containsGroup("Technical"))
            dims = meta.group("Technical").item("Dimensions").value().toSize();
        
        m_filedim->setText( QString("%1x%2 %3").arg(dims.width())
                            .arg(dims.height()).arg(i18n("pixels")) );
        }
   
    if (m_view)             // Digikam embeded mode.
       {           
       m_pathLabel->hide();
       m_filepath->hide();
       }
    else                    // Stand Alone mode.
       {
       m_pathLabel->show();
       m_filepath->show();
       m_filepath->setText( m_currfileURL.path() );
       }
    
    QDateTime dateurl;
    dateurl.setTime_t(fi->time(KIO::UDS_MODIFICATION_TIME));
    m_filedate->setText( KGlobal::locale()->formatDateTime(dateurl, true, true) );
    m_filesize->setText( i18n("%1 (%2)").arg(KIO::convertSize(fi->size()))
                                        .arg(KGlobal::locale()->formatNumber(fi->size(), 0)) );
    m_fileowner->setText( i18n("%1 - %2").arg(fi->user()).arg(fi->group()) );
    m_filepermissions->setText( fi->permissionsString() );

    // Digikam informations (only available with embedded mode)
    
    if (m_view)             // Digikam embeded mode.
       {           
       m_filealbum->show();
       m_filecomments->show();
       m_filetags->show();
       m_albumLabel->show();
       m_commentsLabel->show();
       m_tagsLabel->show();       
       
       PAlbum* palbum = m_view->albumLister()->findParentAlbum(fi);
    
       if (palbum)
           m_filealbum->setText( palbum->getURL().remove(0, 1) );
    
       m_filecomments->setText( m_view->itemComments(m_currItem) );
       QStringList tagPaths(m_view->itemTagPaths(m_currItem));
    
       for (QStringList::iterator it = tagPaths.begin(); it != tagPaths.end(); ++it)
          (*it).remove(0,1);
    
       m_filetags->setText( tagPaths.join(", "));
       }
    else                    // Stand Alone mode.
       {
       m_filealbum->hide();
       m_filecomments->hide();
       m_filetags->hide();
       m_albumLabel->hide();
       m_commentsLabel->hide();
       m_tagsLabel->hide();       
       }
       
    // -------------------------------------------------------------                                         
    // Update Exif Viewer tab.

    m_listview->clear();
    
    if (m_ExifData)
        delete m_ExifData;
    
    m_ExifData = new KExifData;
    
    if (m_ExifData->readFromFile(m_currfileURL.path()))
        {
        slotLevelExifChanged(m_levelExifCB->currentItem());
    
        // Exif embedded thumbnails creation.
    
        QImage thumbnail(m_ExifData->getThumbnail());
        
        if (!thumbnail.isNull()) 
            {
            m_exifThumb->setFixedSize(thumbnail.size());
            m_exifThumb->setPixmap(QPixmap(thumbnail));
            m_exifThumb->setOrientationMenu(m_ExifData, m_currfileURL.path());
            }        
        else
            {
            m_exifThumb->setFixedSize(0, 0);
            m_exifThumb->setPixmap(QPixmap::QPixmap(0, 0));
            }

        QToolTip::add( m_listview, i18n("Select an item to see its description"));
        }
    
    slotToogleEmbeddedThumb(m_embeddedThumbBox->isChecked());
    
    // -------------------------------------------------------------                                         
    // Update Histogram Viewer tab.
    
    m_HistogramThumbJob = new ThumbnailJob(m_currfileURL, 48);
    
    connect(m_HistogramThumbJob,
            SIGNAL(signalThumbnailMetaInfo(const KURL&,
                                           const QPixmap&,
                                           const KFileMetaInfo*)),
            SLOT(slotGotHistogramThumbnail(const KURL&,
                                           const QPixmap&,
                                           const KFileMetaInfo*)));
   
    connect(m_HistogramThumbJob,
            SIGNAL(signalFailed(const KURL&)),
            SLOT(slotFailedHistogramThumbnail(const KURL&)));     

    // This is necessary to stop computation because m_image.bits() is currently used by
    // threaded histogram algorithm.
    
    m_histogramWidget->stopHistogramComputation();
        
    if ( m_image.load(m_currfileURL.path()) )
       {
        if(m_image.depth() < 32)                 // we works always with 32bpp.
            m_image = m_image.convertDepth(32);
       
        m_image.setAlphaBuffer(true);
        
        // If a selection area is done in Image Editor and if the current image is the same 
        // in Image Editor, then compute too the histogram for this selection.
        
        if (m_selectionArea && m_IEcurrentURL == m_currfileURL)
           {
           m_imageSelection = m_image.copy(*m_selectionArea);
           m_histogramWidget->updateData((uint *)m_image.bits(), m_image.width(), m_image.height(),
                                         (uint *)m_imageSelection.bits(), m_imageSelection.width(),
                                         m_imageSelection.height());
           m_renderingLabel->show();                                         
           m_renderingCB->show();                                         
           }
        else 
           {
           m_histogramWidget->updateData((uint *)m_image.bits(), m_image.width(), m_image.height());
           m_renderingLabel->hide();                                         
           m_renderingCB->hide();
           }
        }
    else 
        {
        m_imageSelection.reset();
        m_image.reset();
        m_histogramWidget->updateData(0L, 0, 0);
        }
               
    // Setup buttons.
    
    if (m_view)             // Digikam embeded mode.
       {           
       enableButton(User1, m_currItem->nextItem() != 0);
       enableButton(User2, m_currItem->prevItem() != 0);
       }
    else                    // Stand Alone mode.
       {
       enableButton(User1, m_urlListIt != m_urlList.end());
       enableButton(User2, m_urlListIt != m_urlList.begin());
       }    
    
    setCursor( KCursor::arrowCursor() );     
}


//-----------------------------------------------------------------------------------------------------------
// Histogram Viewer implementation methods

void ImageProperties::setupHistogramViewer(void)
{
    QFrame *page = addPage( i18n("&Histogram"));
   
    QVBoxLayout *topLayout = new QVBoxLayout( page, 0, spacingHint() );

    // Setup Histogram infos tab.
                                              
    QHBoxLayout *hlay = new QHBoxLayout(topLayout);
    
    m_histogramThumb = new QLabel( page );
    m_histogramThumb->setFixedHeight( 48 );
    hlay->addWidget(m_histogramThumb);
    
    QGridLayout *grid = new QGridLayout(hlay, 2, 4);
    
    QLabel *label1 = new QLabel(i18n("Channel:"), page);
    label1->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );
    m_channelCB = new QComboBox( false, page );
    m_channelCB->insertItem( i18n("Luminosity") );
    m_channelCB->insertItem( i18n("Red") );
    m_channelCB->insertItem( i18n("Green") );
    m_channelCB->insertItem( i18n("Blue") );
    m_channelCB->insertItem( i18n("Alpha") );
    m_channelCB->insertItem( i18n("Colors") );
    QWhatsThis::add( m_channelCB, i18n("<p>Select here the histogram channel to display:<p>"
                                       "<b>Luminosity</b>: drawing the image luminosity values.<p>"
                                       "<b>Red</b>: drawing the red image channel values.<p>"
                                       "<b>Green</b>: drawing the green image channel values.<p>"
                                       "<b>Blue</b>: drawing the blue image channel values.<p>"
                                       "<b>Alpha</b>: drawing the alpha image channel values. " 
                                       "This channel corresponding to the transparency value and "
                                       "is supported by some image formats such as PNG or GIF.<p>"
                                       "<b>Colors</b>: drawing all color channels values at the same time."));
    
    QLabel *label2 = new QLabel(i18n("Scale:"), page);
    label2->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_scaleCB = new QComboBox( false, page );
    m_scaleCB->insertItem( i18n("Linear") );
    m_scaleCB->insertItem( i18n("Logarithmic") );
    QWhatsThis::add( m_scaleCB, i18n("<p>Select here the histogram scale.<p>"
                                     "If the image maximal counts is small, you can use the linear scale.<p>"
                                     "Logarithmic scale can be used when the maximal counts is big. "
                                     "Like this all values (small and big) will be visible on the graph."));
    
    QLabel *label10 = new QLabel(i18n("Colors:"), page);
    label10->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );
    m_colorsCB = new QComboBox( false, page );
    m_colorsCB->insertItem( i18n("Red") );
    m_colorsCB->insertItem( i18n("Green") );
    m_colorsCB->insertItem( i18n("Blue") );
    m_colorsCB->setEnabled( false );
    QWhatsThis::add( m_colorsCB, i18n("<p>Select here the main color displayed with Colors Channel mode:<p>"
                                       "<b>Red</b>: drawing the red image channel on the foreground.<p>"
                                       "<b>Green</b>: drawing the green image channel on the foreground.<p>"
                                       "<b>Blue</b>: drawing the blue image channel on the foreground.<p>"));
                                       
    m_renderingLabel = new QLabel(i18n("Rendering:"), page);
    m_renderingLabel->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );
    m_renderingCB = new QComboBox( false, page );
    m_renderingCB->insertItem( i18n("Full Image") );
    m_renderingCB->insertItem( i18n("Selection") );
    
    QWhatsThis::add( m_renderingCB, i18n("<p>Select here the histogram rendering method:<p>"
                     "<b>Full Image</b>: drawing histogram using the full image.<p>"
                     "<b>Selection</b>: drawing histogram using the current image selection."));  
                                                                            
    grid->addWidget(label1, 0, 0);
    grid->addWidget(m_channelCB, 0, 1);
    grid->addWidget(label2, 0, 2);
    grid->addWidget(m_scaleCB, 0, 3);
    grid->addWidget(label10, 1, 0);
    grid->addWidget(m_colorsCB, 1, 1);
    grid->addWidget(m_renderingLabel, 1, 2);
    grid->addWidget(m_renderingCB, 1, 3);
    
    // -------------------------------------------------------------
    
    QFrame *frame = new QFrame( page );
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l = new QVBoxLayout(frame, 5, 0);
    
    m_histogramWidget = new Digikam::HistogramWidget(256, 140, frame);
    QWhatsThis::add( m_histogramWidget, i18n("<p>This is the histogram drawing of the "
                                             "selected image channel"));
    l->addWidget(m_histogramWidget, 0);
        
    m_hGradient = new Digikam::ColorGradientWidget( KSelector::Horizontal, 20, page );
    m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );
    topLayout->addWidget(frame, 4);
    topLayout->addWidget(m_hGradient, 0);

    // -------------------------------------------------------------

    QHBoxLayout *hlay2 = new QHBoxLayout(topLayout);
    QLabel *label3 = new QLabel(i18n("Intensity range:"), page);
    label3->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    m_minInterv = new QSpinBox(0, 255, 1, page);
    m_minInterv->setValue(0);
    QWhatsThis::add( m_minInterv, i18n("<p>Select here the minimal intensity "
                                       "value of the histogram selection."));    
    m_maxInterv = new QSpinBox(0, 255, 1, page);
    m_maxInterv->setValue(255);
    QWhatsThis::add( m_minInterv, i18n("<p>Select here the maximal intensity value "
                                       "of the histogram selection."));
    hlay2->addWidget(label3);
    hlay2->addWidget(m_minInterv);
    hlay2->addWidget(m_maxInterv);
    
    // -------------------------------------------------------------
    
    QGroupBox *gbox = new QGroupBox(4, Qt::Horizontal, i18n("Statistics"), page);
    QWhatsThis::add( gbox, i18n("<p>You can see here the statistic results calculated with the "
                                "selected histogram part. These values are available for all "
                                "channels."));
                                
    QLabel *label4 = new QLabel(i18n("Mean:"), gbox);
    label4->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_labelMeanValue = new QLabel(gbox);
    m_labelMeanValue->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    
    QLabel *label5 = new QLabel(i18n("Pixels:"), gbox);
    label5->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_labelPixelsValue = new QLabel(gbox);
    m_labelPixelsValue->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    
    QLabel *label6 = new QLabel(i18n("Standard deviation:"), gbox);
    label6->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_labelStdDevValue = new QLabel(gbox);
    m_labelStdDevValue->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);

    QLabel *label7 = new QLabel(i18n("Count:"), gbox);
    label7->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_labelCountValue = new QLabel(gbox);
    m_labelCountValue->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    
    QLabel *label8 = new QLabel(i18n("Median:"), gbox);
    label8->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_labelMedianValue = new QLabel(gbox);
    m_labelMedianValue->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    
    QLabel *label9 = new QLabel(i18n("Percentile:"), gbox);
    label9->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_labelPercentileValue = new QLabel(gbox);
    m_labelPercentileValue->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    
    topLayout->addWidget(gbox);
    topLayout->addStretch();

    // -------------------------------------------------------------
    
    connect(m_channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged(int)));
    
    connect(m_scaleCB, SIGNAL(activated(int)),
            this, SLOT(slotScaleChanged(int)));
    
    connect(m_colorsCB, SIGNAL(activated(int)),
            this, SLOT(slotColorsChanged(int)));     
                   
    connect(m_renderingCB, SIGNAL(activated(int)),
            this, SLOT(slotRenderingChanged(int)));       
             
    connect(m_histogramWidget, SIGNAL(signalMousePressed( int )),
            this, SLOT(slotUpdateMinInterv(int)));
       
    connect(m_histogramWidget, SIGNAL(signalMouseReleased( int )),
            this, SLOT(slotUpdateMaxInterv(int)));

    connect(m_histogramWidget, SIGNAL(signalHistogramComputationDone()),
            this, SLOT(slotRefreshOptions()));
            
    connect(m_minInterv, SIGNAL(valueChanged (int)),
            m_histogramWidget, SLOT(slotMinValueChanged(int)));

    connect(m_minInterv, SIGNAL(valueChanged (int)),
            this, SLOT(slotIntervChanged(int)));

    connect(m_maxInterv, SIGNAL(valueChanged (int)),
            m_histogramWidget, SLOT(slotMaxValueChanged(int)));

    connect(m_maxInterv, SIGNAL(valueChanged (int)),
            this, SLOT(slotIntervChanged(int)));
}

void ImageProperties::slotRefreshOptions(void)
{
    slotChannelChanged(m_channelCB->currentItem());
    slotScaleChanged(m_scaleCB->currentItem());
    slotColorsChanged(m_colorsCB->currentItem());
    
    if (m_selectionArea && m_IEcurrentURL == m_currfileURL)
       slotRenderingChanged(m_renderingCB->currentItem());
}

void ImageProperties::slotChannelChanged(int channel)
{
    switch(channel)
       {
       case 1:           // Red.
          m_histogramWidget->m_channelType = Digikam::HistogramWidget::RedChannelHistogram;
          m_hGradient->setColors( QColor( "black" ), QColor( "red" ) );
          m_colorsCB->setEnabled(false);
          break;
       
       case 2:           // Green.
          m_histogramWidget->m_channelType = Digikam::HistogramWidget::GreenChannelHistogram;
          m_hGradient->setColors( QColor( "black" ), QColor( "green" ) );
          m_colorsCB->setEnabled(false);
          break;
          
       case 3:           // Blue.
          m_histogramWidget->m_channelType = Digikam::HistogramWidget::BlueChannelHistogram;
          m_hGradient->setColors( QColor( "black" ), QColor( "blue" ) );
          m_colorsCB->setEnabled(false);
          break;

       case 4:           // Alpha.
          m_histogramWidget->m_channelType = Digikam::HistogramWidget::AlphaChannelHistogram;
          m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );
          m_colorsCB->setEnabled(false);
          break;
          
       case 5:           // All color channels.
          m_histogramWidget->m_channelType = Digikam::HistogramWidget::ColorChannelsHistogram;
          m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );
          m_colorsCB->setEnabled(true);
          break;
                              
       default:          // Luminosity.
          m_histogramWidget->m_channelType = Digikam::HistogramWidget::ValueHistogram;
          m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );
          m_colorsCB->setEnabled(false);
          break;
       }
   
    m_histogramWidget->repaint(false);
    updateInformations();
}

void ImageProperties::slotScaleChanged(int scale)
{
    switch(scale)
       {
       case 1:           // Log.
          m_histogramWidget->m_scaleType = Digikam::HistogramWidget::LogScaleHistogram;
          break;
          
       default:          // Lin.
          m_histogramWidget->m_scaleType = Digikam::HistogramWidget::LinScaleHistogram;
          break;
       }
   
    m_histogramWidget->repaint(false);
}

void ImageProperties::slotColorsChanged(int color)
{
    switch(color)
       {
       case 1:           // Green.
          m_histogramWidget->m_colorType = Digikam::HistogramWidget::GreenColor;
          break;
       
       case 2:           // Blue.
          m_histogramWidget->m_colorType = Digikam::HistogramWidget::BlueColor;
          break;

       default:          // Red.
          m_histogramWidget->m_colorType = Digikam::HistogramWidget::RedColor;
          break;
       }

    m_histogramWidget->repaint(false);
    updateInformations();
}

void ImageProperties::slotRenderingChanged(int rendering)
{
    switch(rendering)
       {
       case 1:           // Image Selection.
          m_histogramWidget->m_renderingType = Digikam::HistogramWidget::ImageSelectionHistogram;
          break;
       
       default:          // Full Image.
          m_histogramWidget->m_renderingType = Digikam::HistogramWidget::FullImageHistogram;
          break;
       }

    m_histogramWidget->repaint(false);
    updateInformations();
}

void ImageProperties::slotUpdateMinInterv(int min)
{
    m_minInterv->setValue(min);
}

void ImageProperties::slotUpdateMaxInterv(int max)
{
    m_maxInterv->setValue(max);
    updateInformations();
}

void ImageProperties::slotIntervChanged(int)
{
    m_maxInterv->setMinValue(m_minInterv->value());
    m_minInterv->setMaxValue(m_maxInterv->value());
    updateInformations();
}

void ImageProperties::updateInformations()
{
    QString value;
    int min = m_minInterv->value();
    int max = m_maxInterv->value();
    int channel = m_channelCB->currentItem();

    if ( channel == Digikam::HistogramWidget::ColorChannelsHistogram )
       channel = m_colorsCB->currentItem()+1;
               
    double mean = m_histogramWidget->m_imageHistogram->getMean(channel, min, max);
    m_labelMeanValue->setText(value.setNum(mean, 'f', 1));
    
    double pixels = m_histogramWidget->m_imageHistogram->getPixels();
    m_labelPixelsValue->setText(value.setNum((float)pixels, 'f', 0));
    
    double stddev = m_histogramWidget->m_imageHistogram->getStdDev(channel, min, max);
    m_labelStdDevValue->setText(value.setNum(stddev, 'f', 1));
      
    double counts = m_histogramWidget->m_imageHistogram->getCount(channel, min, max);
    m_labelCountValue->setText(value.setNum((float)counts, 'f', 0));
    
    double median = m_histogramWidget->m_imageHistogram->getMedian(channel, min, max);
    m_labelMedianValue->setText(value.setNum(median, 'f', 1)); 

    double percentile = (pixels > 0 ? (100.0 * counts / pixels) : 0.0);
    m_labelPercentileValue->setText(value.setNum(percentile, 'f', 1));
}


void ImageProperties::slotGotHistogramThumbnail(const KURL&, const QPixmap& pix,
                                                const KFileMetaInfo*)
{
    m_histogramThumb->clear();
    m_mainExifThumb->clear();
    
    if (!pix.isNull())
       {
       m_histogramThumb->setPixmap(pix);
       m_mainExifThumb->setPixmap(pix);
       }
}

void ImageProperties::slotFailedHistogramThumbnail(const KURL&)
{
    m_histogramThumb->clear();
    m_mainExifThumb->clear();
}

//-----------------------------------------------------------------------------------------------------------
// Exif Viewer implementation methods

void ImageProperties::setupExifViewer(void)
{
    QFrame *page = addPage( i18n("Exif") );
    QGridLayout* layout = new QGridLayout(page);
    
    // Setup options header.
    
    QHBoxLayout *hlay = new QHBoxLayout(layout);
    
    m_mainExifThumb = new QLabel( page );
    m_mainExifThumb->setFixedHeight( 48 );

    QLabel *label1 = new QLabel(i18n("Level: "), page);
    label1->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );
    m_levelExifCB = new QComboBox( false, page );
    m_levelExifCB->insertItem( i18n("General") );
    m_levelExifCB->insertItem( i18n("Extended") );
    m_levelExifCB->insertItem( i18n("All") );
    QWhatsThis::add( m_levelExifCB, i18n("<p>Select here the Exif information level to display<p>"
                                         "<b>General</b>: display general information about the photograph "
                                         " (default).<p>"
                                         "<b>Extended</b>: display extended information about the "
                                         "photograph.<p>"
                                         "<b>All</b>: display all Exif sections."));  
                                         
    m_embeddedThumbBox = new QCheckBox(i18n("Show Exif thumbnail"), page);
                                                                           
    hlay->addWidget(m_mainExifThumb);
    hlay->addStretch();
    hlay->addWidget(label1);
    hlay->addWidget(m_levelExifCB);
    hlay->addStretch();
    hlay->addWidget(m_embeddedThumbBox);

    // Setup Exif infos tab.                                         
    
    m_listview = new KExifListView(page, true);
    
    m_embeddedThumbnail = new QVGroupBox(i18n("Embedded Exif thumbnail"), page);
    m_exifThumb = new ExifThumbLabel(m_embeddedThumbnail);
    QWhatsThis::add( m_exifThumb, i18n("<p>You can see here the Exif thumbnail embedded in image.<p>"
                                       "If you press under with right mouse button, you can corrected the "
                                       "Exif orientation tag by a popup menu."));
                                       
    layout->addWidget(m_listview, 1, 0);
    layout->addWidget(m_embeddedThumbnail, 2, 0);
    
    // Setup slots connections.
    
    connect(m_levelExifCB, SIGNAL(activated(int)),
            this, SLOT(slotLevelExifChanged(int)));
            
    connect(m_embeddedThumbBox, SIGNAL(toggled(bool)),
            this, SLOT(slotToogleEmbeddedThumb(bool)));            
}

void ImageProperties::slotLevelExifChanged(int level)
{
    m_listview->clear();
    QPtrList<KExifIfd> ifdList(m_ExifData->ifdList());
    
    switch(level)
       {
       case 1:           // Extended.
          for (KExifIfd* ifd = ifdList.first(); ifd; ifd = ifdList.next())
              {
              if (ifd->getName() == "EXIF")
                 {
                 m_listview->addItems(ifd->entryList());
                 setCurrentExifItem();       
                 return;
                 }
              }
          break;
       
       case 2:           // All.
          for (KExifIfd* ifd = ifdList.first(); ifd; ifd = ifdList.next())
              {
              KExifListViewItem *item = new KExifListViewItem(m_listview, m_listview->lastItem(),
                                                              ifd->getName());
              m_listview->addItems(ifd->entryList(), item );
              }
          setCurrentExifItem();       
          break;

       default:          // General.
          for (KExifIfd* ifd = ifdList.first(); ifd; ifd = ifdList.next())
              {
              if (ifd->getName() == "0")
                 {
                 m_listview->addItems(ifd->entryList());
                 setCurrentExifItem();       
                 return;
                 }
              }
          break;
       }
}

void ImageProperties::slotToogleEmbeddedThumb(bool toogle)
{
    if (toogle)
       m_embeddedThumbnail->show();
    else
       m_embeddedThumbnail->hide();
}

void ImageProperties::getCurrentExifItem(void)
{    
    switch(m_levelExifCB->currentItem())
       {
       case 1:           // Extended.
          m_currentExtendedExifItemName = m_listview->getCurrentItemName();
          break;
       
       case 2:           // All.
          m_currentAllExifItemName = m_listview->getCurrentItemName();
          break;

       default:          // General.
          m_currentGeneralExifItemName = m_listview->getCurrentItemName();
          break;
       }
}

void ImageProperties::setCurrentExifItem(void)
{    
    switch(m_levelExifCB->currentItem())
       {
       case 1:           // Extended.
          m_listview->setCurrentItem(m_currentExtendedExifItemName);
          break;
       
       case 2:           // All.
          m_listview->setCurrentItem(m_currentAllExifItemName);
        break;

       default:          // General.
          m_listview->setCurrentItem(m_currentGeneralExifItemName);
          break;
       }
}

//-----------------------------------------------------------------------------------------------------------
// General Tab implementation methods

void ImageProperties::setupGeneralTab()
{
    QFrame *page = addPage( i18n("General") );
    QVBoxLayout *vlay = new QVBoxLayout( page, 0, spacingHint() );
    
    // Setup thumbnail.
    
    m_generalThumb = new QLabel( page );
    m_generalThumb->setFixedHeight( 128 );
    vlay->addWidget(m_generalThumb, 0, Qt::AlignHCenter);
    
    // Setup File properties infos.                                         
                                         
    KSeparator *sep1 = new KSeparator (Horizontal, page);
    vlay->addWidget(sep1);
    
    QGridLayout *hlay1 = new QGridLayout(8, 3);
    vlay->addLayout( hlay1 );
    
    QLabel *name = new QLabel( i18n("Name:"), page);
    m_filename = new KSqueezedTextLabel(page);
    name->setBuddy( m_filename );
    hlay1->addMultiCellWidget( name, 0, 0, 0, 0 );
    hlay1->addMultiCellWidget( m_filename, 0, 0, 1, 2  );
    
    QLabel *type = new QLabel( i18n("Type:"), page);
    m_filetype = new KSqueezedTextLabel(page);
    type->setBuddy( m_filetype );
    hlay1->addMultiCellWidget( type, 1, 1, 0, 0 );
    hlay1->addMultiCellWidget( m_filetype, 1, 1, 1, 2  );
    
    QLabel *dim = new QLabel( i18n("Dimensions:"), page);
    m_filedim = new KSqueezedTextLabel(page);
    dim->setBuddy( m_filedim );
    hlay1->addMultiCellWidget( dim, 2, 2, 0, 0 );
    hlay1->addMultiCellWidget( m_filedim, 2, 2, 1, 2  );

    m_pathLabel = new QLabel( i18n("Location:"), page);
    m_filepath = new KSqueezedTextLabel(page);
    m_pathLabel->setBuddy( m_filepath );
    hlay1->addMultiCellWidget( m_pathLabel, 3, 3, 0, 0 );
    hlay1->addMultiCellWidget( m_filepath, 3, 3, 1, 2  );
    
    QLabel *date = new QLabel( i18n("Modification Date:"), page);
    m_filedate = new KSqueezedTextLabel(page);
    date->setBuddy( m_filedate );
    hlay1->addMultiCellWidget( date, 4, 4, 0, 0 );
    hlay1->addMultiCellWidget( m_filedate, 4, 4, 1, 2  );
    
    QLabel *size = new QLabel( i18n("Size:"), page);
    m_filesize = new KSqueezedTextLabel(page);
    size->setBuddy( m_filesize );
    hlay1->addMultiCellWidget( size, 5, 5, 0, 0 );
    hlay1->addMultiCellWidget( m_filesize, 5, 5, 1, 2  );

    QLabel *owner = new QLabel( i18n("Owner:"), page);
    m_fileowner = new KSqueezedTextLabel(page);
    owner->setBuddy( m_fileowner );
    hlay1->addMultiCellWidget( owner, 6, 6, 0, 0 );
    hlay1->addMultiCellWidget( m_fileowner, 6, 6, 1, 2  );

    QLabel *permissions = new QLabel( i18n("Permissions:"), page);
    m_filepermissions = new KSqueezedTextLabel(page);
    permissions->setBuddy( m_filepermissions );
    hlay1->addMultiCellWidget( permissions, 7, 7, 0, 0 );
    hlay1->addMultiCellWidget( m_filepermissions, 7, 7, 1, 2  );
        
    KSeparator *sep2 = new KSeparator (Horizontal, page);
    vlay->addWidget(sep2);

    // Setup Digikam infos.
    
    QGridLayout *hlay2 = new QGridLayout(3, 3);
    vlay->addLayout( hlay2 );
    
    m_albumLabel = new QLabel( i18n("Album:"), page);
    m_filealbum = new KSqueezedTextLabel(page);
    m_albumLabel->setBuddy( m_filealbum );
    hlay2->addMultiCellWidget( m_albumLabel, 0, 0, 0, 0 );
    hlay2->addMultiCellWidget( m_filealbum, 0, 0, 1, 2  );

    m_commentsLabel = new QLabel( i18n("Comments:"), page);
    m_filecomments = new KSqueezedTextLabel(page);
    m_commentsLabel->setBuddy( m_filecomments );
    hlay2->addMultiCellWidget( m_commentsLabel, 1, 1, 0, 0 );
    hlay2->addMultiCellWidget( m_filecomments, 1, 1, 1, 2  );
    
    m_tagsLabel = new QLabel( i18n("Tags:"), page);
    m_filetags = new KSqueezedTextLabel(page);
    m_tagsLabel->setBuddy( m_filetags );
    hlay2->addMultiCellWidget( m_tagsLabel, 2, 2, 0, 0 );
    hlay2->addMultiCellWidget( m_filetags, 2, 2, 1, 2  );
                    
    vlay->addStretch(1);
}

void ImageProperties::slotGotGeneralThumbnail(const KURL&, const QPixmap& pix,
                                              const KFileMetaInfo*)
{
    m_generalThumb->clear();

    if (!pix.isNull())
       m_generalThumb->setPixmap(pix);
}

void ImageProperties::slotFailedGeneralThumbnail(const KURL&)
{
    m_generalThumb->clear();
    m_generalThumb->setText(i18n("Thumnail unavailable"));
}

#include "imageproperties.moc"

