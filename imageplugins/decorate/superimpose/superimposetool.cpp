/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-01-04
 * Description : a Digikam image editor plugin for superimpose a
 *               template to an image.
 *
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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
#include <QProgressBar>

// KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmenu.h>
#include <kstandarddirs.h>

// Local includes

#include "dimg.h"
#include "dirselectwidget.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "superimposewidget.h"
#include "thumbbar.h"

namespace DigikamDecorateImagePlugin
{

class SuperImposeTool::SuperImposeToolPriv
{
public:

    SuperImposeToolPriv() :
        thumbnailsBar(0),
        gboxSettings(0),
        previewWidget(0),
        dirSelect(0)
    {}

    static const QString configGroupName;
    static const QString configTemplatesRootURLEntry;
    static const QString configTemplatesURLEntry;

    KUrl                 templatesUrl;
    KUrl                 templatesRootUrl;

    ThumbBarView*        thumbnailsBar;
    EditorToolSettings*  gboxSettings;
    SuperImposeWidget*   previewWidget;
    DirSelectWidget*     dirSelect;
};
const QString SuperImposeTool::SuperImposeToolPriv::configGroupName("superimpose Tool");
const QString SuperImposeTool::SuperImposeToolPriv::configTemplatesRootURLEntry("Templates Root URL");
const QString SuperImposeTool::SuperImposeToolPriv::configTemplatesURLEntry("Templates URL");

// --------------------------------------------------------

SuperImposeTool::SuperImposeTool(QObject* parent)
    : EditorTool(parent),
      d(new SuperImposeToolPriv)
{
    setObjectName("superimpose");
    setToolName(i18n("Template Superimpose"));
    setToolIcon(SmallIcon("superimpose"));

    // -------------------------------------------------------------

    QFrame* frame = new QFrame(0);
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);

    QGridLayout* gridFrame = new QGridLayout(frame);
    d->previewWidget       = new SuperImposeWidget(400, 300, frame);
    d->previewWidget->setWhatsThis( i18n("This previews the template superimposed onto the image."));

    // -------------------------------------------------------------

    QWidget* toolBox     = new QWidget(frame);
    QHBoxLayout* hlay    = new QHBoxLayout(toolBox);
    QButtonGroup* bGroup = new QButtonGroup(frame);

    QToolButton* zoomInButton = new QToolButton(toolBox);
    bGroup->addButton(zoomInButton, ZOOMIN);
    zoomInButton->setIcon(KIcon("zoom-in"));
    zoomInButton->setCheckable(true);
    zoomInButton->setToolTip(i18n("Zoom in"));

    QToolButton* zoomOutButton = new QToolButton(toolBox);
    bGroup->addButton(zoomOutButton, ZOOMOUT);
    zoomOutButton->setIcon(KIcon("zoom-out"));
    zoomOutButton->setCheckable(true);
    zoomOutButton->setToolTip(i18n("Zoom out"));

    QToolButton* moveButton = new QToolButton(toolBox);
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

    gridFrame->addWidget(d->previewWidget, 0, 0, 1, 3);
    gridFrame->addWidget(toolBox,          1, 1, 1, 1);
    gridFrame->setColumnStretch(0, 10);
    gridFrame->setColumnStretch(2, 10);
    gridFrame->setRowStretch(0, 10);
    gridFrame->setMargin(0);
    gridFrame->setSpacing(0);

    setToolView(frame);

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;

    QGridLayout* grid = new QGridLayout(d->gboxSettings->plainPage());

    d->thumbnailsBar = new ThumbBarView(d->gboxSettings->plainPage());
    d->thumbnailsBar->setToolTip(new ThumbBarToolTip(d->thumbnailsBar));

    d->dirSelect = new DirSelectWidget(d->gboxSettings->plainPage());
    QPushButton* templateDirButton = new QPushButton(i18n("Root Directory..."), d->gboxSettings->plainPage());
    templateDirButton->setWhatsThis(i18n("Set here the current templates' root directory."));

    // -------------------------------------------------------------

    grid->addWidget(d->thumbnailsBar,   0, 0, 2, 1);
    grid->addWidget(d->dirSelect,       0, 1, 1, 1);
    grid->addWidget(templateDirButton,  1, 1, 1, 1);
    grid->setColumnStretch(1, 10);
    grid->setMargin(d->gboxSettings->spacingHint());
    grid->setSpacing(d->gboxSettings->spacingHint());

    setToolSettings(d->gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(bGroup, SIGNAL(buttonReleased(int)),
            d->previewWidget, SLOT(slotEditModeChanged(int)));

    connect(d->thumbnailsBar, SIGNAL(signalUrlSelected(const KUrl&)),
            d->previewWidget, SLOT(slotSetCurrentTemplate(const KUrl&)));

    connect(d->dirSelect, SIGNAL(folderItemSelected(const KUrl&)),
            this, SLOT(slotTemplateDirChanged(const KUrl&)));

    connect(templateDirButton, SIGNAL(clicked()),
            this, SLOT(slotRootTemplateDirChanged()));

    // -------------------------------------------------------------

    populateTemplates();
}

SuperImposeTool::~SuperImposeTool()
{
    delete d;
}

void SuperImposeTool::populateTemplates(void)
{
    d->thumbnailsBar->clear(true);

    if (!d->templatesUrl.isValid() || !d->templatesUrl.isLocalFile())
    {
        return;
    }

    QDir dir(d->templatesUrl.toLocalFile(), "*.png *.PNG");

    if (!dir.exists())
    {
        return;
    }

    dir.setFilter ( QDir::Files | QDir::NoSymLinks );

    QFileInfoList fileinfolist = dir.entryInfoList();

    if (fileinfolist.isEmpty())
    {
        return;
    }

    QFileInfoList::const_iterator fi;

    for (fi = fileinfolist.constBegin(); fi != fileinfolist.constEnd(); ++fi)
    {
        new ThumbBarItem( d->thumbnailsBar, KUrl(fi->filePath()) );
    }
}

void SuperImposeTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("Album Settings");
    KUrl albumDBUrl( group.readEntry("Album Path", KGlobalSettings::documentPath()) );

    group = config->group(d->configGroupName);
    d->templatesRootUrl.setPath( group.readEntry(d->configTemplatesRootURLEntry, albumDBUrl.toLocalFile()) );
    d->templatesUrl.setPath( group.readEntry(d->configTemplatesURLEntry,         albumDBUrl.toLocalFile()) );
    d->dirSelect->setRootPath(d->templatesRootUrl, d->templatesUrl);
}

void SuperImposeTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group(d->configGroupName);
    group.writeEntry( d->configTemplatesRootURLEntry, d->dirSelect->rootPath().toLocalFile() );
    group.writeEntry( d->configTemplatesURLEntry,     d->templatesUrl.toLocalFile() );
    group.sync();
}

void SuperImposeTool::slotResetSettings()
{
    d->previewWidget->resetEdit();
}

void SuperImposeTool::slotRootTemplateDirChanged(void)
{
    KUrl url = KFileDialog::getExistingDirectory(d->templatesRootUrl.toLocalFile(), kapp->activeWindow(),
               i18n("Select Template Root Directory to Use"));

    if ( url.isValid() )
    {
        d->dirSelect->setRootPath(url);
        d->templatesRootUrl = url;
        d->templatesUrl = url;
        populateTemplates();
    }
}

void SuperImposeTool::slotTemplateDirChanged(const KUrl& url)
{
    if (url.isValid())
    {
        d->templatesUrl = url;
        populateTemplates();
    }
}

void SuperImposeTool::finalRendering()
{
    d->previewWidget->setEnabled(false);
    d->dirSelect->setEnabled(false);
    d->thumbnailsBar->setEnabled(false);

    ImageIface iface(0, 0);
    DImg img = d->previewWidget->makeSuperImpose();
    //TODO: Make separate filter
    FilterAction action(i18n("Super Impose Tool"), 1, FilterAction::DocumentedHistory);
    iface.putOriginalImage(i18n("Super Impose"), action, img.bits(), img.width(), img.height() );

    d->previewWidget->setEnabled(true);
    d->dirSelect->setEnabled(true);
    d->thumbnailsBar->setEnabled(true);
}

}  // namespace DigikamDecorateImagePlugin
