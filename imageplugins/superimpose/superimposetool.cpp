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


#include "superimposetool.h"
#include "superimposetool.moc"

// Qt includes

#include <QButtonGroup>
#include <QDir>
#include <QFile>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QToolButton>

// KDE includes

#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmenu.h>
#include <kstandarddirs.h>
#include <qprogressbar.h>

// Local includes

#include "daboutdata.h"
#include "dimg.h"
#include "dirselectwidget.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "superimposewidget.h"
#include "thumbbar.h"
#include "version.h"

using namespace Digikam;

namespace DigikamSuperImposeImagesPlugin
{

SuperImposeTool::SuperImposeTool(QObject* parent)
               : EditorTool(parent)
{
    setObjectName("superimpose");
    setToolName(i18n("Template Superimpose"));
    setToolIcon(SmallIcon("superimpose"));

    // -------------------------------------------------------------

    QFrame *frame = new QFrame(0);
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);

    QGridLayout* gridFrame = new QGridLayout(frame);
    m_previewWidget        = new SuperImposeWidget(400, 300, frame);
    m_previewWidget->setWhatsThis( i18n("This previews the template superimposed onto the image."));

    // -------------------------------------------------------------

    QWidget *toolBox     = new QWidget(frame);
    QHBoxLayout *hlay    = new QHBoxLayout(toolBox);
    QButtonGroup *bGroup = new QButtonGroup(frame);

    QToolButton *zoomInButton = new QToolButton(toolBox);
    bGroup->addButton(zoomInButton, ZOOMIN);
    zoomInButton->setIcon(KIcon("zoom-in"));
    zoomInButton->setCheckable(true);
    zoomInButton->setToolTip(i18n("Zoom in"));

    QToolButton *zoomOutButton = new QToolButton(toolBox);
    bGroup->addButton(zoomOutButton, ZOOMOUT);
    zoomOutButton->setIcon(KIcon("zoom-out"));
    zoomOutButton->setCheckable(true);
    zoomOutButton->setToolTip(i18n("Zoom out"));

    QToolButton *moveButton = new QToolButton(toolBox);
    bGroup->addButton(moveButton, MOVE);
    moveButton->setIcon(KIcon("transform-move"));
    moveButton->setCheckable(true);
    moveButton->setChecked(true);
    moveButton->setToolTip(i18n("Move"));

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

    gridFrame->addWidget(m_previewWidget,   0, 0, 1, 3);
    gridFrame->addWidget(toolBox,           1, 1, 1, 1);
    gridFrame->setColumnStretch(0, 10);
    gridFrame->setColumnStretch(2, 10);
    gridFrame->setRowStretch(0, 10);
    gridFrame->setMargin(0);
    gridFrame->setSpacing(0);

    setToolView(frame);

    // -------------------------------------------------------------

    m_gboxSettings    = new EditorToolSettings(EditorToolSettings::Default|
                                               EditorToolSettings::Ok|
                                               EditorToolSettings::Cancel);

    QGridLayout* grid = new QGridLayout(m_gboxSettings->plainPage());

    m_thumbnailsBar   = new ThumbBarView(m_gboxSettings->plainPage());
    m_thumbnailsBar->setToolTip(new ThumbBarToolTip(m_thumbnailsBar));
    m_dirSelect       = new DirSelectWidget(m_gboxSettings->plainPage());
    QPushButton *templateDirButton = new QPushButton(i18n("Root Directory..."), m_gboxSettings->plainPage());
    templateDirButton->setWhatsThis(i18n("Set here the current templates' root directory."));

    // -------------------------------------------------------------

    grid->addWidget(m_thumbnailsBar,    0, 0, 2, 1);
    grid->addWidget(m_dirSelect,        0, 1, 1, 1);
    grid->addWidget(templateDirButton,  1, 1, 1, 1);
    grid->setColumnStretch(1, 10);
    grid->setMargin(m_gboxSettings->spacingHint());
    grid->setSpacing(m_gboxSettings->spacingHint());

    setToolSettings(m_gboxSettings);
    init();

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

SuperImposeTool::~SuperImposeTool()
{
}

void SuperImposeTool::populateTemplates(void)
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
        new ThumbBarItem( m_thumbnailsBar, KUrl(fi->filePath()) );
    }
}

void SuperImposeTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("Album Settings");
    KUrl albumDBUrl( group.readEntry("Album Path", KGlobalSettings::documentPath()) );
    group = config->group("superimpose Tool");
    group = config->group("Template Superimpose Tool Settings");
    m_templatesRootUrl.setPath( group.readEntry("Templates Root URL", albumDBUrl.path()) );
    m_templatesUrl.setPath( group.readEntry("Templates URL", albumDBUrl.path()) );
    m_dirSelect->setRootPath(m_templatesRootUrl, m_templatesUrl);
}

void SuperImposeTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("superimpose Tool");
    group.writeEntry( "Templates Root URL", m_dirSelect->rootPath().path() );
    group.writeEntry( "Templates URL", m_templatesUrl.path() );
    group.sync();
}

void SuperImposeTool::slotResetSettings()
{
    m_previewWidget->resetEdit();
}

void SuperImposeTool::slotRootTemplateDirChanged(void)
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

void SuperImposeTool::slotTemplateDirChanged(const KUrl& url)
{
    if (url.isValid())
    {
        m_templatesUrl = url;
        populateTemplates();
    }
}

void SuperImposeTool::finalRendering()
{
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
}

}  // namespace DigikamSuperImposeImagesPlugin
