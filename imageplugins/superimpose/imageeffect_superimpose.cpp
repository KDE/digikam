/* ============================================================
 * File  : imageeffect_superimpose.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-01-04
 * Description : a Digikam image editor plugin for superimpose a 
 *               template to an image.
 * 
 * Copyright 2005 by Gilles Caulier
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

// Imlib2 include.

#define X_DISPLAY_MISSING 1
#include <Imlib2.h>

// C++ includes.

#include <cmath>
#include <cstdio>
#include <cstdlib>
 
// Qt includes. 
 
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qpixmap.h>
#include <qwhatsthis.h>
#include <qtooltip.h>
#include <qlayout.h>
#include <qframe.h>
#include <qdir.h>
#include <qfile.h>
#include <qhbuttongroup.h> 

// KDE includes.

#include <kcursor.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <kprogress.h>
#include <knuminput.h>
#include <kiconloader.h>
#include <kfiledialog.h> 
#include <kapplication.h>
#include <kconfig.h>

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "version.h"
#include "thumbbar.h"
#include "superimposewidget.h"
#include "imageeffect_superimpose.h"

namespace DigikamSuperImposeImagesPlugin
{

ImageEffect_SuperImpose::ImageEffect_SuperImpose(QWidget* parent)
                       : KDialogBase(Plain, i18n("Template Superimpose"),
                                     Help|User1|Ok|Cancel, Ok,
                                     parent, 0, true, true,
                                     i18n("&Reset Values")),
                         m_parent(parent)
{
    QString whatsThis;
        
    setButtonWhatsThis ( User1, i18n("<p>Reset edition mode to the default settings.") );
    
    // About data and help button.
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Template Superimpose"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A Digikam image plugin to superimpose a template to an image."),
                                       KAboutData::License_GPL,
                                       "(c) 2005, Gilles Caulier", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins.php");
    
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");
    
    m_helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Template Superimpose Handbook"), this, SLOT(slotHelp()), 0, -1, 0);
    m_helpButton->setPopup( helpMenu->menu() );
    
    // -------------------------------------------------------------

    QVBoxLayout *topLayout = new QVBoxLayout( plainPage(), 0, spacingHint());

    QFrame *headerFrame = new QFrame( plainPage() );
    headerFrame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QHBoxLayout* layout = new QHBoxLayout( headerFrame );
    layout->setMargin( 2 ); // to make sure the frame gets displayed
    layout->setSpacing( 0 );
    QLabel *pixmapLabelLeft = new QLabel( headerFrame, "pixmapLabelLeft" );
    pixmapLabelLeft->setScaledContents( false );
    layout->addWidget( pixmapLabelLeft );
    QLabel *labelTitle = new QLabel( i18n("Template Superimpose"), headerFrame, "labelTitle" );
    layout->addWidget( labelTitle );
    layout->setStretchFactor( labelTitle, 1 );
    topLayout->addWidget(headerFrame);
    
    QString directory;
    KGlobal::dirs()->addResourceType("digikamimageplugins_banner_left", KGlobal::dirs()->kde_default("data") +
                                                                        "digikamimageplugins/data");
    directory = KGlobal::dirs()->findResourceDir("digikamimageplugins_banner_left",
                                                 "digikamimageplugins_banner_left.png");
    
    pixmapLabelLeft->setPaletteBackgroundColor( QColor(201, 208, 255) );
    pixmapLabelLeft->setPixmap( QPixmap( directory + "digikamimageplugins_banner_left.png" ) );
    labelTitle->setPaletteBackgroundColor( QColor(201, 208, 255) );
    
    // -------------------------------------------------------------
    
    QHBoxLayout *hlayout = new QHBoxLayout( topLayout );

    QVGroupBox *gbox = new QVGroupBox(i18n("Preview"), plainPage());
    QFrame *frame = new QFrame(gbox);
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l = new QVBoxLayout(frame, 5, 0);
    m_previewWidget = new SuperImposeWidget(400, 300, frame);
    QWhatsThis::add( m_previewWidget, i18n("<p>This is the template superimposing preview on the image.") );

    l->addWidget(m_previewWidget, 0, Qt::AlignCenter);
                                                                                      
    QHButtonGroup *bGroup = new QHButtonGroup(gbox);
    KIconLoader icon;
    bGroup->addSpace(0);
    QPushButton *zoomInButton = new QPushButton( bGroup );
    bGroup->insert(zoomInButton, ZOOMIN);
    zoomInButton->setPixmap( icon.loadIcon( "viewmag+", (KIcon::Group)KIcon::Toolbar ) );
    zoomInButton->setToggleButton(true);
    QToolTip::add( zoomInButton, i18n( "Zoom in" ) );
    bGroup->addSpace(0);
    QPushButton *zoomOutButton = new QPushButton( bGroup );
    bGroup->insert(zoomOutButton, ZOOMOUT);
    zoomOutButton->setPixmap( icon.loadIcon( "viewmag-", (KIcon::Group)KIcon::Toolbar ) );
    zoomOutButton->setToggleButton(true);
    QToolTip::add( zoomOutButton, i18n( "Zoom out" ) );
    bGroup->addSpace(0);
    QPushButton *moveButton = new QPushButton( bGroup );
    bGroup->insert(moveButton, MOVE);
    moveButton->setPixmap( icon.loadIcon( "move", (KIcon::Group)KIcon::Toolbar ) );
    moveButton->setToggleButton(true);
    moveButton->setOn(true);
    QToolTip::add( moveButton, i18n( "Move" ) );
    bGroup->addSpace(0);
    bGroup->setExclusive(true);
    bGroup->setFrameShape(QFrame::NoFrame);
    
    hlayout->addWidget(gbox);
    
    // -------------------------------------------------------------
    
    QVGroupBox *gbox2 = new QVGroupBox(i18n("Templates"), plainPage());
    m_thumbnailsBar = new ThumbBarView(gbox2);
    QPushButton *templateDirButton = new QPushButton( i18n("Browse..."), gbox2 );
    QWhatsThis::add( templateDirButton, i18n("<p>Change the current templates directory.") );
    hlayout->addWidget(gbox2);
    
    // -------------------------------------------------------------
    
    adjustSize();
    disableResize();

    // -------------------------------------------------------------
    
    connect(bGroup, SIGNAL(released(int)),
            m_previewWidget, SLOT(slotEditModeChanged(int)));
    
    connect(m_thumbnailsBar, SIGNAL(signalURLSelected(const KURL&)),
            m_previewWidget, SLOT(slotSetCurrentTemplate(const KURL&)));            

    connect(templateDirButton, SIGNAL(clicked()),
            this, SLOT(slotTemplateDirChanged()));
                                    
    // -------------------------------------------------------------
    
    // Read templates url settings.
    
    KConfig *config = kapp->config();
    config->setGroup("ImageViewer Settings");
    m_templatesUrl.setPath( config->readPathEntry("Templates URL", "/") );
    populateTemplates();
}

ImageEffect_SuperImpose::~ImageEffect_SuperImpose()
{
    KConfig *config = kapp->config();
    config->setGroup("ImageViewer Settings");
    config->writePathEntry( "Templates URL", m_templatesUrl.path() );
    config->sync();
}

void ImageEffect_SuperImpose::populateTemplates(void)
{
    m_thumbnailsBar->clear(true);
    QDir *dir = new QDir(m_templatesUrl.path(), "*.png");
    dir->setFilter ( QDir::Files | QDir::NoSymLinks );

    const QFileInfoList* fileinfolist = dir->entryInfoList();
    QFileInfoListIterator it(*fileinfolist);
    QFileInfo* fi;

    while( (fi = it.current() ) )
        {
        new ThumbBarItem( m_thumbnailsBar, KURL::KURL(fi->filePath()) );
        ++it;
        }
}

void ImageEffect_SuperImpose::slotUser1()
{
    m_previewWidget->resetEdit();
} 

void ImageEffect_SuperImpose::slotHelp()
{
    KApplication::kApplication()->invokeHelp("superimpose",
                                             "digikamimageplugins");
}

void ImageEffect_SuperImpose::slotTemplateDirChanged(void)
{
    KURL url = KFileDialog::getExistingDirectory(m_templatesUrl.path(), 0,
                                                i18n("Select Template Directory to Browse..."));
    
    if( url.isValid() )
       {
       m_templatesUrl = url;
       populateTemplates();
       }
}

void ImageEffect_SuperImpose::slotOk()
{
    m_parent->setCursor( KCursor::waitCursor() );
    
    Digikam::ImageIface iface(0, 0);
    QImage img = m_previewWidget->makeSuperImpose();
    img.setAlphaBuffer(true);
    iface.putOriginalData( (uint*)img.bits(),
                           m_previewWidget->getTemplateSize().width(),
                           m_previewWidget->getTemplateSize().height() );   
    
    m_parent->setCursor( KCursor::arrowCursor() );
    accept();       
}

}  // NameSpace DigikamSuperImposeImagesPlugin

#include "imageeffect_superimpose.moc"
