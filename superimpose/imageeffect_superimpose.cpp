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

// C++ includes.

#include <cmath>
#include <cstdio>
#include <cstdlib>
 
// Qt includes. 
 
#include <qvgroupbox.h>
#include <qgroupbox.h>
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
#include "superimposewidget.h"
#include "dirselectwidget.h"
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
           
    setButtonWhatsThis ( User1, i18n("<p>Reset template composition to the default settings.") );
    
    // Read settings.
    
    KConfig *config = kapp->config();
    config->setGroup("Album Settings");
    KURL albumDBUrl( config->readPathEntry("Album Path", QString::null) );
    config->setGroup("Template Superimpose Tool Settings");
    m_templatesRootUrl.setPath( config->readPathEntry("Templates Root URL", albumDBUrl.path()) );
    m_templatesUrl.setPath( config->readPathEntry("Templates URL", albumDBUrl.path()) );
    
    // About data and help button.
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Template Superimpose"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A digiKam image plugin to superimpose a template onto an image."),
                                       KAboutData::License_GPL,
                                       "(c) 2005, Gilles Caulier", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
    
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
    QWhatsThis::add( m_previewWidget, i18n("<p>This is the preview of the template superimposed onto the image.") );

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
    
    QGroupBox *gbox2 = new QGroupBox(i18n("Templates"), plainPage());
    QGridLayout* grid = new QGridLayout( gbox2, 2, 3, 20, spacingHint());
    
    m_thumbnailsBar = new Digikam::ThumbBarView(gbox2);
    m_dirSelect = new DirSelectWidget(m_templatesRootUrl, m_templatesUrl, gbox2);
    QPushButton *templateDirButton = new QPushButton( i18n("Root Directory..."), gbox2 );
    QWhatsThis::add( templateDirButton, i18n("<p>Change here the current templates' root directory.") );

    grid->addMultiCellWidget(m_thumbnailsBar, 0, 1, 0, 0);
    grid->addMultiCellWidget(m_dirSelect, 0, 0, 1, 2);    
    grid->addMultiCellWidget(templateDirButton, 1, 1, 1, 1);    
    
    hlayout->addWidget(gbox2);
    
    // -------------------------------------------------------------
    
    adjustSize();
    disableResize();

    // -------------------------------------------------------------
    
    connect(bGroup, SIGNAL(released(int)),
            m_previewWidget, SLOT(slotEditModeChanged(int)));
    
    connect(m_thumbnailsBar, SIGNAL(signalURLSelected(const KURL&)),
            m_previewWidget, SLOT(slotSetCurrentTemplate(const KURL&)));            

    connect(m_dirSelect, SIGNAL(folderItemSelected(const KURL &)),
            this, SLOT(slotTemplateDirChanged(const KURL &)));
    
    connect(templateDirButton, SIGNAL(clicked()),
            this, SLOT(slotRootTemplateDirChanged()));
                                    
    // -------------------------------------------------------------
    
    populateTemplates();
}

ImageEffect_SuperImpose::~ImageEffect_SuperImpose()
{
    KConfig *config = kapp->config();
    config->setGroup("Template Superimpose Tool Settings");
    config->writePathEntry( "Templates Root URL", m_dirSelect->rootPath().path() );
    config->writePathEntry( "Templates URL", m_templatesUrl.path() );
    config->sync();
}

void ImageEffect_SuperImpose::populateTemplates(void)
{
    m_thumbnailsBar->clear(true);
    
    if (!m_templatesUrl.isValid() || !m_templatesUrl.isLocalFile())
       return;
       
    QDir dir(m_templatesUrl.path(), "*.png *.PNG");
    
    if (!dir.exists())
       return;
       
    dir.setFilter ( QDir::Files | QDir::NoSymLinks );

    const QFileInfoList* fileinfolist = dir.entryInfoList();
    if (!fileinfolist)
       return;
    
    QFileInfoListIterator it(*fileinfolist);
    QFileInfo* fi;

    while( (fi = it.current() ) )
        {
        new Digikam::ThumbBarItem( m_thumbnailsBar, KURL::KURL(fi->filePath()) );
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

void ImageEffect_SuperImpose::slotRootTemplateDirChanged(void)
{
    KURL url = KFileDialog::getExistingDirectory(m_templatesRootUrl.path(), kapp->activeWindow(),
                                                 i18n("Select Template Root Directory to Use"));
    
    if( url.isValid() )
       {
       m_dirSelect->setRootPath(url);
       m_templatesRootUrl = url;
       m_templatesUrl = url;
       populateTemplates();
       }
}

void ImageEffect_SuperImpose::slotTemplateDirChanged(const KURL& url)
{
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
    iface.putOriginalData(i18n("Super Impose"), (uint*)img.bits(),
                           m_previewWidget->getTemplateSize().width(),
                           m_previewWidget->getTemplateSize().height() );   
    
    m_parent->setCursor( KCursor::arrowCursor() );
    accept();       
}

}  // NameSpace DigikamSuperImposeImagesPlugin

#include "imageeffect_superimpose.moc"
