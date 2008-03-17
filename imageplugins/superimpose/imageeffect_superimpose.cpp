/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-01-04
 * Description : a Digikam image editor plugin for superimpose a 
 *               template to an image.
 *
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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
 
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QPixmap>
#include <QFrame>
#include <QDir>
#include <QFile>
#include <QButtonGroup> 
#include <QGridLayout>

// KDE includes.

#include <kcursor.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kmenu.h>
#include <kstandarddirs.h>
#include <qprogressbar.h>
#include <knuminput.h>
#include <kiconloader.h>
#include <kfiledialog.h> 
#include <kapplication.h>
#include <kconfig.h>
#include <kglobalsettings.h>
#include <kglobal.h>

// Local includes.

#include "version.h"
#include "ddebug.h"
#include "dimg.h"
#include "imageiface.h"
#include "thumbbar.h"
#include "superimposewidget.h"
#include "dirselectwidget.h"
#include "imageeffect_superimpose.h"
#include "imageeffect_superimpose.moc"

namespace DigikamSuperImposeImagesPlugin
{

ImageEffect_SuperImpose::ImageEffect_SuperImpose(QWidget* parent)
                       : Digikam::ImageDlgBase(parent, i18n("Template Superimpose to Photograph"),
                                               "superimpose", false, false)
{
    QString whatsThis;
           
    // About data and help button.
    
    KAboutData* about = new KAboutData("digikam", 0,
                                       ki18n("Template Superimpose"), 
                                       digikam_version,
                                       ki18n("A digiKam image plugin to superimpose a template onto a photograph."),
                                       KAboutData::License_GPL,
                                       ki18n("(c) 2005-2006, Gilles Caulier\n"
                                       "(c) 2006-2008, Gilles Caulier and Marcel Wiesweg"), 
                                       KLocalizedString(),
                                       "http://www.digikam.org");
    
    about->addAuthor(ki18n("Gilles Caulier"), ki18n("Author and maintainer"),
                     "caulier dot gilles at gmail dot com");
    
    about->addAuthor(ki18n("Marcel Wiesweg"), ki18n("Developer"),
                     "marcel dot wiesweg at gmx dot de");

    setAboutData(about);    
    
    // -------------------------------------------------------------

    QFrame *frame = new QFrame(mainWidget());
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);

    QGridLayout* gridFrame = new QGridLayout( frame );
    m_previewWidget        = new SuperImposeWidget(400, 300, frame);
    m_previewWidget->setWhatsThis( i18n("<p>This is the preview of the template "
                                        "superimposed onto the image.") );

    // -------------------------------------------------------------

    QWidget *toolBox     = new QWidget(frame);
    QHBoxLayout *hlay    = new QHBoxLayout(toolBox);
    QButtonGroup *bGroup = new QButtonGroup(frame);

    QPushButton *zoomInButton = new QPushButton( toolBox );
    bGroup->addButton(zoomInButton, ZOOMIN);
    zoomInButton->setIcon(KIconLoader::global()->loadIcon("zoom-in", KIconLoader::Toolbar));
    zoomInButton->setCheckable(true);
    zoomInButton->setToolTip( i18n( "Zoom in" ) );

    QPushButton *zoomOutButton = new QPushButton( toolBox );
    bGroup->addButton(zoomOutButton, ZOOMOUT);
    zoomOutButton->setIcon(KIconLoader::global()->loadIcon("zoom-out", KIconLoader::Toolbar));
    zoomOutButton->setCheckable(true);
    zoomOutButton->setToolTip( i18n( "Zoom out" ) );

    QPushButton *moveButton = new QPushButton( toolBox );
    bGroup->addButton(moveButton, MOVE);
    moveButton->setIcon(KIconLoader::global()->loadIcon("move", KIconLoader::Toolbar));
    moveButton->setCheckable(true);
    moveButton->setChecked(true);
    moveButton->setToolTip( i18n( "Move" ) );

    bGroup->setExclusive(true);

    hlay->setMargin(0);
    hlay->setSpacing(0);
    hlay->addSpacing(20);
    hlay->addWidget(zoomInButton);
    hlay->addSpacing(20);
    hlay->addWidget(zoomOutButton);
    hlay->addSpacing(20);
    hlay->addWidget(moveButton);
    hlay->addSpacing(20);

    // -------------------------------------------------------------

    gridFrame->addWidget(m_previewWidget, 0, 0, 1, 3 );
    gridFrame->addWidget(toolBox, 1, 1, 1, 1);
    gridFrame->setColumnStretch(0, 10);
    gridFrame->setColumnStretch(2, 10);
    gridFrame->setRowStretch(0, 10);
    gridFrame->setMargin(spacingHint());
    gridFrame->setSpacing(spacingHint());
    
    setPreviewAreaWidget(frame);     
    
    // -------------------------------------------------------------
    
    QWidget *gbox2    = new QWidget(mainWidget());
    QGridLayout* grid = new QGridLayout( gbox2 );
    
    m_thumbnailsBar = new Digikam::ThumbBarView(gbox2);
    m_dirSelect     = new DirSelectWidget(gbox2);
    QPushButton *templateDirButton = new QPushButton( i18n("Root Directory..."), gbox2 );
    templateDirButton->setWhatsThis( i18n("<p>Set here the current templates' root directory.") );

    // -------------------------------------------------------------

    grid->addWidget(m_thumbnailsBar, 0, 0, 2, 1);
    grid->addWidget(m_dirSelect, 0, 1, 1, 1);    
    grid->addWidget(templateDirButton, 1, 1, 1, 1);    
    grid->setColumnStretch(1, 10);
    grid->setMargin(spacingHint());
    grid->setSpacing(spacingHint());

    setUserAreaWidget(gbox2);
    
    // -------------------------------------------------------------
    
    connect(bGroup, SIGNAL(released(int)),
            m_previewWidget, SLOT(slotEditModeChanged(int)));
    
    connect(m_thumbnailsBar, SIGNAL(signalUrlSelected(const KUrl&)),
            m_previewWidget, SLOT(slotSetCurrentTemplate(const KUrl&)));            

    connect(m_dirSelect, SIGNAL(folderItemSelected(const KUrl &)),
            this, SLOT(slotTemplateDirChanged(const KUrl &)));
    
    connect(templateDirButton, SIGNAL(clicked()),
            this, SLOT(slotRootTemplateDirChanged()));
                                    
    // -------------------------------------------------------------
    
    populateTemplates();
}

ImageEffect_SuperImpose::~ImageEffect_SuperImpose()
{
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

    QFileInfoList fileinfolist = dir.entryInfoList();
    if (fileinfolist.isEmpty())
       return;
    
    QFileInfoList::const_iterator fi;

    for (fi = fileinfolist.constBegin(); fi != fileinfolist.constEnd(); ++fi)
    {
        new Digikam::ThumbBarItem( m_thumbnailsBar, KUrl(fi->filePath()) );
    }
}

void ImageEffect_SuperImpose::readUserSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("Album Settings");
    KUrl albumDBUrl( group.readEntry("Album Path", KGlobalSettings::documentPath()) );
    group = config->group("superimpose Tool Dialog");
    group = config->group("Template Superimpose Tool Settings");
    m_templatesRootUrl.setPath( group.readEntry("Templates Root URL", albumDBUrl.path()) );
    m_templatesUrl.setPath( group.readEntry("Templates URL", albumDBUrl.path()) );
    m_dirSelect->setRootPath(m_templatesRootUrl, m_templatesUrl);
}

void ImageEffect_SuperImpose::writeUserSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("superimpose Tool Dialog");
    group.writeEntry( "Templates Root URL", m_dirSelect->rootPath().path() );
    group.writeEntry( "Templates URL", m_templatesUrl.path() );
    group.sync();
}

void ImageEffect_SuperImpose::resetValues()
{
    m_previewWidget->resetEdit();
} 

void ImageEffect_SuperImpose::slotRootTemplateDirChanged(void)
{
    KUrl url = KFileDialog::getExistingDirectory(m_templatesRootUrl.path(), kapp->activeWindow(),
                                                 i18n("Select Template Root Directory to Use"));

    if( url.isValid() )
    {
        m_dirSelect->setRootPath(url);
        m_templatesRootUrl = url;
        m_templatesUrl = url;
        populateTemplates();
    }
}

void ImageEffect_SuperImpose::slotTemplateDirChanged(const KUrl& url)
{
    if( url.isValid() )
    {
        m_templatesUrl = url;
        populateTemplates();
    }
}

void ImageEffect_SuperImpose::finalRendering()
{
    setCursor(Qt::WaitCursor);
    m_previewWidget->setEnabled(false);
    m_dirSelect->setEnabled(false);
    m_thumbnailsBar->setEnabled(false);

    Digikam::ImageIface iface(0, 0);
    Digikam::DImg img = m_previewWidget->makeSuperImpose();
    iface.putOriginalImage(i18n("Super Impose"), img.bits(),
                           img.width(), img.height() );

    m_previewWidget->setEnabled(true);
    m_dirSelect->setEnabled(true);
    m_thumbnailsBar->setEnabled(true);
    unsetCursor();
    accept();
}

}  // NameSpace DigikamSuperImposeImagesPlugin
