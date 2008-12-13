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
#include <kglobalsettings.h>

// Local includes.

#include "daboutdata.h"
#include "ddebug.h"
#include "dimg.h"
#include "imageiface.h"
#include "editortoolsettings.h"
#include "thumbbar.h"
#include "superimposewidget.h"
#include "dirselectwidget.h"
#include "superimposetool.h"
#include "superimposetool.moc"

using namespace Digikam;

namespace DigikamSuperImposeImagesPlugin
{

SuperImposeTool::SuperImposeTool(QObject* parent)
               : EditorTool(parent)
{
    setName("superimpose");
    setToolName(i18n("Template Superimpose"));
    setToolIcon(SmallIcon("superimpose"));

    // -------------------------------------------------------------

    QFrame *frame = new QFrame(0);
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);

    QGridLayout* gridFrame = new QGridLayout(frame, 1, 2);
    m_previewWidget        = new SuperImposeWidget(400, 300, frame);
    QWhatsThis::add( m_previewWidget, i18n("<p>This is the preview of the template "
                                           "superimposed onto the image.") );

    // -------------------------------------------------------------

    QHButtonGroup *bGroup = new QHButtonGroup(frame);
    KIconLoader icon;
    bGroup->addSpace(0);
    QPushButton *zoomInButton = new QPushButton( bGroup );
    bGroup->insert(zoomInButton, ZOOMIN);
    zoomInButton->setPixmap( icon.loadIcon( "viewmag+", (KIcon::Group)KIcon::Toolbar ) );
    zoomInButton->setToggleButton(true);
    QToolTip::add( zoomInButton, i18n( "Zoom in" ) );
    bGroup->addSpace(20);
    QPushButton *zoomOutButton = new QPushButton( bGroup );
    bGroup->insert(zoomOutButton, ZOOMOUT);
    zoomOutButton->setPixmap( icon.loadIcon( "viewmag-", (KIcon::Group)KIcon::Toolbar ) );
    zoomOutButton->setToggleButton(true);
    QToolTip::add( zoomOutButton, i18n( "Zoom out" ) );
    bGroup->addSpace(20);
    QPushButton *moveButton = new QPushButton( bGroup );
    bGroup->insert(moveButton, MOVE);
    moveButton->setPixmap( icon.loadIcon( "move", (KIcon::Group)KIcon::Toolbar ) );
    moveButton->setToggleButton(true);
    moveButton->setOn(true);
    QToolTip::add( moveButton, i18n( "Move" ) );
    bGroup->addSpace(20);
    bGroup->setExclusive(true);
    bGroup->setFrameShape(QFrame::NoFrame);

    gridFrame->addMultiCellWidget(m_previewWidget, 0, 0, 0, 2);
    gridFrame->addMultiCellWidget(bGroup,          1, 1, 1, 1);
    gridFrame->setRowStretch(0, 10);
    gridFrame->setColStretch(0, 10);
    gridFrame->setColStretch(2, 10);
    gridFrame->setMargin(0);
    gridFrame->setSpacing(0);

    setToolView(frame);

    // -------------------------------------------------------------

    m_gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                            EditorToolSettings::Ok|
                                            EditorToolSettings::Cancel);
    QGridLayout* grid = new QGridLayout(m_gboxSettings->plainPage(), 1, 1);

    m_thumbnailsBar = new ThumbBarView(m_gboxSettings->plainPage());
    m_dirSelect     = new DirSelectWidget(m_gboxSettings->plainPage());
    QPushButton *templateDirButton = new QPushButton( i18n("Root Directory..."), m_gboxSettings->plainPage() );
    QWhatsThis::add( templateDirButton, i18n("<p>Set here the current templates' root directory.") );

    grid->addMultiCellWidget(m_thumbnailsBar,   0, 1, 0, 0);
    grid->addMultiCellWidget(m_dirSelect,       0, 0, 1, 1);
    grid->addMultiCellWidget(templateDirButton, 1, 1, 1, 1);
    grid->setMargin(0);
    grid->setSpacing(m_gboxSettings->spacingHint());
    grid->setColStretch(1, 10);

    setToolSettings(m_gboxSettings);
    init();

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

SuperImposeTool::~SuperImposeTool()
{
}

void SuperImposeTool::populateTemplates()
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
        new ThumbBarItem( m_thumbnailsBar, KURL(fi->filePath()) );
        ++it;
    }
}

void SuperImposeTool::readSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("Album Settings");
    KURL albumDBUrl( config->readPathEntry("Album Path", KGlobalSettings::documentPath()) );
    config->setGroup("superimpose Tool");
    config->setGroup("Template Superimpose Tool Settings");
    m_templatesRootUrl.setPath( config->readEntry("Templates Root URL", albumDBUrl.path()) );
    m_templatesUrl.setPath( config->readEntry("Templates URL", albumDBUrl.path()) );
    m_dirSelect->setRootPath(m_templatesRootUrl, m_templatesUrl);
}

void SuperImposeTool::writeSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("superimpose Tool");
    config->writeEntry( "Templates Root URL", m_dirSelect->rootPath().path() );
    config->writeEntry( "Templates URL", m_templatesUrl.path() );
    config->sync();
}

void SuperImposeTool::slotResetSettings()
{
    m_previewWidget->resetEdit();
}

void SuperImposeTool::slotRootTemplateDirChanged()
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

void SuperImposeTool::slotTemplateDirChanged(const KURL& url)
{
    if( url.isValid() )
    {
        m_templatesUrl = url;
        populateTemplates();
    }
}

void SuperImposeTool::finalRendering()
{
    kapp->setOverrideCursor(KCursor::waitCursor());
    m_previewWidget->setEnabled(false);
    m_dirSelect->setEnabled(false);
    m_thumbnailsBar->setEnabled(false);

    ImageIface iface(0, 0);
    DImg img = m_previewWidget->makeSuperImpose();
    iface.putOriginalImage(i18n("Super Impose"), img.bits(),
                           img.width(), img.height() );

    m_previewWidget->setEnabled(true);
    m_dirSelect->setEnabled(true);
    m_thumbnailsBar->setEnabled(true);
    kapp->restoreOverrideCursor();
}

}  // NameSpace DigikamSuperImposeImagesPlugin
