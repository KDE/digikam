/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-27
 * Description : a digiKam image plugin for fixing dots produced by
 *               hot/stuck/dead pixels from a CCD.
 *
 * Copyright (C) 2005-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2005-2006 by Unai Garro <ugarro at users dot sourceforge dot net>
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


#include "hotpixelstool.h"
#include "hotpixelstool.moc"

// Qt includes

#include <QGridLayout>
#include <QLabel>
#include <QPolygon>
#include <QProgressBar>
#include <QPushButton>

// KDE includes

#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kimageio.h>
#include <klocale.h>
#include <kstandarddirs.h>

// LibKDcraw includes

#include <libkdcraw/rcombobox.h>

// Local includes

#include "blackframelistview.h"
#include "daboutdata.h"
#include "dimg.h"
#include "editortooliface.h"
#include "editortoolsettings.h"
#include "hotpixelfixer.h"
#include "imagedialog.h"
#include "imageiface.h"
#include "imagepanelwidget.h"
#include "version.h"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamHotPixelsImagesPlugin
{

class HotPixelsToolPriv
{
public:

    HotPixelsToolPriv()
    {
        blackFrameButton   = 0;
        progressBar        = 0;
        filterMethodCombo  = 0;
        blackFrameListView = 0;
        previewWidget      = 0;
        gboxSettings       = 0;
    }

    QPushButton*        blackFrameButton;
    QProgressBar*       progressBar;
    QList<HotPixel>     hotPixelsList;

    KUrl                blackFrameURL;

    RComboBox*          filterMethodCombo;

    BlackFrameListView* blackFrameListView;
    ImagePanelWidget*   previewWidget;
    EditorToolSettings* gboxSettings;
};

HotPixelsTool::HotPixelsTool(QObject* parent)
             : EditorToolThreaded(parent),
               d(new HotPixelsToolPriv)
{
    setObjectName("hotpixels");
    setToolName(i18n("Hot Pixels"));
    setToolIcon(SmallIcon("hotpixels"));

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings(EditorToolSettings::Ok|
                                             EditorToolSettings::Try|
                                             EditorToolSettings::Cancel,
                                             EditorToolSettings::PanIcon);
    QGridLayout* grid = new QGridLayout(d->gboxSettings->plainPage());

    QLabel *filterMethodLabel = new QLabel(i18n("Filter:"), d->gboxSettings->plainPage());
    d->filterMethodCombo      = new RComboBox(d->gboxSettings->plainPage());
    d->filterMethodCombo->addItem(i18nc("average filter mode", "Average"));
    d->filterMethodCombo->addItem(i18nc("linear filter mode", "Linear"));
    d->filterMethodCombo->addItem(i18nc("quadratic filter mode", "Quadratic"));
    d->filterMethodCombo->addItem(i18nc("cubic filter mode", "Cubic"));
    d->filterMethodCombo->setDefaultIndex(HotPixelFixer::QUADRATIC_INTERPOLATION);

    d->blackFrameButton = new QPushButton(i18n("Black Frame..."), d->gboxSettings->plainPage());
    d->blackFrameButton->setWhatsThis(i18n("Use this button to add a new black frame file which will "
                                          "be used by the hot pixels removal filter.") );

    d->blackFrameListView = new BlackFrameListView(d->gboxSettings->plainPage());

    // -------------------------------------------------------------

    grid->addWidget(filterMethodLabel,     0, 0, 1, 1);
    grid->addWidget(d->filterMethodCombo,  0, 1, 1, 1);
    grid->addWidget(d->blackFrameButton,   0, 2, 1, 1);
    grid->addWidget(d->blackFrameListView, 1, 0, 2, 3);
    grid->setRowStretch(3, 10);
    grid->setMargin(d->gboxSettings->spacingHint());
    grid->setSpacing(d->gboxSettings->spacingHint());

    setToolSettings(d->gboxSettings);

    // -------------------------------------------------------------

    d->previewWidget = new ImagePanelWidget(470, 350, "hotpixels Tool", d->gboxSettings->panIconView(),
                                            0, ImagePanelWidget::SeparateViewDuplicate);

    setToolView(d->previewWidget);
    init();

    // -------------------------------------------------------------

    connect(d->filterMethodCombo, SIGNAL(activated(int)),
            this, SLOT(slotEffect()));

    connect(d->blackFrameButton, SIGNAL(clicked()),
            this, SLOT(slotAddBlackFrame()));

    connect(d->blackFrameListView, SIGNAL(blackFrameSelected(QList<HotPixel>, const KUrl&)),
            this, SLOT(slotBlackFrame(QList<HotPixel>, const KUrl&)));
}

HotPixelsTool::~HotPixelsTool()
{
    delete d;
}

void HotPixelsTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("hotpixels Tool");
    d->blackFrameURL = KUrl(group.readEntry("Last Black Frame File", QString()));
    d->filterMethodCombo->setCurrentIndex(group.readEntry("Filter Method", d->filterMethodCombo->defaultIndex()));

    if (d->blackFrameURL.isValid())
    {
        EditorToolIface::editorToolIface()->setToolStartProgress(i18n("Loading: "));
        BlackFrameListViewItem *item = new BlackFrameListViewItem(d->blackFrameListView, d->blackFrameURL);

        connect(item, SIGNAL(signalLoadingProgress(float)),
                this, SLOT(slotLoadingProgress(float)));

        connect(item, SIGNAL(signalLoadingComplete()),
                this, SLOT(slotLoadingComplete()));
    }
}

void HotPixelsTool::slotLoadingProgress(float v)
{
    EditorToolIface::editorToolIface()->setToolProgress((int)(v*100));
}

void HotPixelsTool::slotLoadingComplete()
{
    EditorToolIface::editorToolIface()->setToolStopProgress();
}

void HotPixelsTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("hotpixels Tool");
    group.writeEntry("Last Black Frame File", d->blackFrameURL.url());
    group.writeEntry("Filter Method", d->filterMethodCombo->currentIndex());
    d->previewWidget->writeSettings();
    group.sync();
}

void HotPixelsTool::slotResetSettings()
{
    d->filterMethodCombo->blockSignals(true);
    d->filterMethodCombo->slotReset();
    d->filterMethodCombo->blockSignals(false);
}

void HotPixelsTool::slotAddBlackFrame()
{
    KUrl url = ImageDialog::getImageURL(kapp->activeWindow(), d->blackFrameURL, i18n("Select Black Frame Image"));
    if (!url.isEmpty())
    {
        // Load the selected file and insert into the list.

        d->blackFrameURL = url;
        d->blackFrameListView->clear();
        BlackFrameListViewItem *item = new BlackFrameListViewItem(d->blackFrameListView, d->blackFrameURL);

        connect(item, SIGNAL(signalLoadingProgress(float)),
                this, SLOT(slotLoadingProgress(float)));

        connect(item, SIGNAL(signalLoadingComplete()),
                this, SLOT(slotLoadingComplete()));
    }
}

void HotPixelsTool::renderingFinished()
{
    d->filterMethodCombo->setEnabled(true);
    d->blackFrameListView->setEnabled(true);
}

void HotPixelsTool::prepareEffect()
{
    d->filterMethodCombo->setEnabled(false);
    d->blackFrameListView->setEnabled(false);

    DImg image              = d->previewWidget->getOriginalRegionImage();
    int interpolationMethod = d->filterMethodCombo->currentIndex();

    QList<HotPixel> hotPixelsRegion;
    QRect area = d->previewWidget->getOriginalImageRegionToRender();
    QList<HotPixel>::Iterator end(d->hotPixelsList.end());

    for (QList<HotPixel>::Iterator it = d->hotPixelsList.begin() ; it != end ; ++it )
    {
        HotPixel hp = (*it);

        if ( area.contains( hp.rect ) )
        {
           hp.rect.moveTopLeft(QPoint( hp.rect.x()-area.x(), hp.rect.y()-area.y() ));
           hotPixelsRegion.append(hp);
        }
    }

    setFilter(dynamic_cast<DImgThreadedFilter*>(new HotPixelFixer(&image, this, hotPixelsRegion, interpolationMethod)));
}

void HotPixelsTool::prepareFinal()
{
    d->filterMethodCombo->setEnabled(false);
    d->blackFrameListView->setEnabled(false);

    int interpolationMethod = d->filterMethodCombo->currentIndex();

    ImageIface iface(0, 0);
    setFilter(dynamic_cast<DImgThreadedFilter*>(new HotPixelFixer(iface.getOriginalImg(), this, d->hotPixelsList, interpolationMethod)));
}

void HotPixelsTool::putPreviewData()
{
    d->previewWidget->setPreviewImage(filter()->getTargetImage());
}

void HotPixelsTool::putFinalData()
{
    ImageIface iface(0, 0);
    iface.putOriginalImage(i18n("Hot Pixels Correction"), filter()->getTargetImage().bits());
}

void HotPixelsTool::slotBlackFrame(QList<HotPixel> hpList, const KUrl& blackFrameURL)
{
    d->blackFrameURL = blackFrameURL;
    d->hotPixelsList = hpList;

    QPolygon pointList(d->hotPixelsList.size());
    QList <HotPixel>::Iterator it;
    int i = 0;
    QList <HotPixel>::Iterator end(d->hotPixelsList.end());

    for (it = d->hotPixelsList.begin() ; it != end ; ++it, ++i)
       pointList.setPoint(i, (*it).rect.center());

    d->previewWidget->setPanIconHighLightPoints(pointList);

    slotEffect();
}

}  // namespace DigikamHotPixelsImagesPlugin
