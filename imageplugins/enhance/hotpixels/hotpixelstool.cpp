/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-27
 * Description : a digiKam image plugin for fixing dots produced by
 *               hot/stuck/dead pixels from a CCD.
 *
 * Copyright (C) 2005-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include "dimgfiltermanager.h"
#include "editortooliface.h"
#include "editortoolsettings.h"
#include "hotpixelfixer.h"
#include "imagedialog.h"
#include "imageiface.h"
#include "imageregionwidget.h"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamEnhanceImagePlugin
{

class HotPixelsTool::Private
{
public:

    Private() :
        blackFrameButton(0),
        progressBar(0),
        filterMethodCombo(0),
        blackFrameListView(0),
        previewWidget(0),
        gboxSettings(0)
    {}

    static const QString configGroupName;
    static const QString configLastBlackFrameFileEntry;
    static const QString configFilterMethodEntry;

    QPushButton*         blackFrameButton;
    QProgressBar*        progressBar;
    QList<HotPixel>      hotPixelsList;

    KUrl                 blackFrameURL;

    RComboBox*           filterMethodCombo;

    BlackFrameListView*  blackFrameListView;
    ImageRegionWidget*   previewWidget;
    EditorToolSettings*  gboxSettings;
};

const QString HotPixelsTool::Private::configGroupName("hotpixels Tool");
const QString HotPixelsTool::Private::configLastBlackFrameFileEntry("Last Black Frame File");
const QString HotPixelsTool::Private::configFilterMethodEntry("Filter Method");

// --------------------------------------------------------

void HotPixelsTool::registerFilter()
{
    Digikam::DImgFilterManager::instance()->addGenerator(new Digikam::BasicDImgFilterGenerator<HotPixelFixer>());
}

HotPixelsTool::HotPixelsTool(QObject* const parent)
    : EditorToolThreaded(parent),
      d(new Private)
{
    setObjectName("hotpixels");
    setToolName(i18n("Hot Pixels"));
    setToolIcon(SmallIcon("hotpixels"));

    // -------------------------------------------------------------

    d->gboxSettings   = new EditorToolSettings;
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Cancel|
                                EditorToolSettings::Try);

    QGridLayout* grid = new QGridLayout(d->gboxSettings->plainPage());

    QLabel* filterMethodLabel = new QLabel(i18n("Filter:"), d->gboxSettings->plainPage());
    d->filterMethodCombo      = new RComboBox(d->gboxSettings->plainPage());
    d->filterMethodCombo->addItem(i18nc("average filter mode", "Average"));
    d->filterMethodCombo->addItem(i18nc("linear filter mode", "Linear"));
    d->filterMethodCombo->addItem(i18nc("quadratic filter mode", "Quadratic"));
    d->filterMethodCombo->addItem(i18nc("cubic filter mode", "Cubic"));
    d->filterMethodCombo->setDefaultIndex(HotPixelFixer::QUADRATIC_INTERPOLATION);

    d->blackFrameButton = new QPushButton(i18n("Black Frame..."), d->gboxSettings->plainPage());
    d->blackFrameButton->setIcon(KIcon("document-open"));
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

    // -------------------------------------------------------------

    d->previewWidget = new ImageRegionWidget;

    setToolSettings(d->gboxSettings);
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::AllPreviewModes);

    // -------------------------------------------------------------

    connect(d->filterMethodCombo, SIGNAL(activated(int)),
            this, SLOT(slotPreview()));

    connect(d->blackFrameButton, SIGNAL(clicked()),
            this, SLOT(slotAddBlackFrame()));

    connect(d->blackFrameListView, SIGNAL(signalBlackFrameSelected(QList<HotPixel>,KUrl)),
            this, SLOT(slotBlackFrame(QList<HotPixel>,KUrl)));
}

HotPixelsTool::~HotPixelsTool()
{
    delete d;
}

void HotPixelsTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    d->blackFrameURL          = KUrl(group.readEntry(d->configLastBlackFrameFileEntry, QString()));
    d->filterMethodCombo->setCurrentIndex(group.readEntry(d->configFilterMethodEntry,  d->filterMethodCombo->defaultIndex()));

    if (d->blackFrameURL.isValid())
    {
        EditorToolIface::editorToolIface()->setToolStartProgress(i18n("Loading: "));
        BlackFrameListViewItem* item = new BlackFrameListViewItem(d->blackFrameListView, d->blackFrameURL);

        connect(item, SIGNAL(signalLoadingProgress(float)),
                this, SLOT(slotLoadingProgress(float)));

        connect(item, SIGNAL(signalLoadingComplete()),
                this, SLOT(slotLoadingComplete()));
    }
}

void HotPixelsTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    group.writeEntry(d->configLastBlackFrameFileEntry, d->blackFrameURL.url());
    group.writeEntry(d->configFilterMethodEntry,       d->filterMethodCombo->currentIndex());
    group.sync();
}

void HotPixelsTool::slotLoadingProgress(float v)
{
    EditorToolIface::editorToolIface()->setToolProgress((int)(v*100));
}

void HotPixelsTool::slotLoadingComplete()
{
    EditorToolIface::editorToolIface()->setToolStopProgress();
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
        BlackFrameListViewItem* item = new BlackFrameListViewItem(d->blackFrameListView, d->blackFrameURL);

        connect(item, SIGNAL(signalLoadingProgress(float)),
                this, SLOT(slotLoadingProgress(float)));

        connect(item, SIGNAL(signalLoadingComplete()),
                this, SLOT(slotLoadingComplete()));
    }
}

void HotPixelsTool::preparePreview()
{
    DImg image              = d->previewWidget->getOriginalRegionImage();
    int interpolationMethod = d->filterMethodCombo->currentIndex();

    QList<HotPixel> hotPixelsRegion;
    QRect area = d->previewWidget->getOriginalImageRegionToRender();

    for (QList<HotPixel>::const_iterator it = d->hotPixelsList.constBegin() ; it != d->hotPixelsList.constEnd() ; ++it)
    {
        HotPixel hp = (*it);

        if (area.contains( hp.rect ))
        {
            hp.rect.moveTopLeft(QPoint( hp.rect.x()-area.x(), hp.rect.y()-area.y() ));
            hotPixelsRegion.append(hp);
        }
    }

    setFilter(dynamic_cast<DImgThreadedFilter*>(new HotPixelFixer(&image, this, hotPixelsRegion, interpolationMethod)));
}

void HotPixelsTool::prepareFinal()
{
    int interpolationMethod = d->filterMethodCombo->currentIndex();

    ImageIface iface;
    setFilter(dynamic_cast<DImgThreadedFilter*>(new HotPixelFixer(iface.original(), this, d->hotPixelsList, interpolationMethod)));
}

void HotPixelsTool::setPreviewImage()
{
    d->previewWidget->setPreviewImage(filter()->getTargetImage());
}

void HotPixelsTool::setFinalImage()
{
    ImageIface iface;
    iface.setOriginal(i18n("Hot Pixels Correction"), filter()->filterAction(), filter()->getTargetImage());
}

void HotPixelsTool::slotBlackFrame(const QList<HotPixel>& hpList, const KUrl& blackFrameURL)
{
    d->blackFrameURL = blackFrameURL;
    d->hotPixelsList = hpList;

    QPolygon pointList(d->hotPixelsList.size());
    QList <HotPixel>::const_iterator it;
    int i = 0;

    for (it = d->hotPixelsList.constBegin() ; it != d->hotPixelsList.constEnd() ; ++it, ++i)
    {
        pointList.setPoint(i, (*it).rect.center());
    }

    d->previewWidget->setHighLightPoints(pointList);

    slotPreview();
}

}  // namespace DigikamEnhanceImagePlugin
