/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-27
 * Description : a digiKam image plugin for fixing dots produced by
 *               hot/stuck/dead pixels from a CCD.
 *
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qwhatsthis.h>
#include <qpushbutton.h>
#include <qpointarray.h>

// KDE includes.

#include <klocale.h>
#include <kconfig.h>
#include <kimageio.h>
#include <kaboutdata.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kfiledialog.h>
#include <kprogress.h>
#include <kiconloader.h>
#include <kpushbutton.h>

// LibKDcraw includes.

#include <libkdcraw/rcombobox.h>

// Local includes.

#include "daboutdata.h"
#include "ddebug.h"
#include "dimg.h"
#include "imageiface.h"
#include "imagepanelwidget.h"
#include "editortooliface.h"
#include "editortoolsettings.h"
#include "imagedialog.h"
#include "blackframelistview.h"
#include "hotpixelstool.h"
#include "hotpixelstool.moc"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamHotPixelsImagesPlugin
{

HotPixelsTool::HotPixelsTool(QObject* parent)
             : EditorToolThreaded(parent)
{
    setName("hotpixels");
    setToolName(i18n("Hot Pixels"));
    setToolIcon(SmallIcon("hotpixels"));

    // -------------------------------------------------------------

    m_gboxSettings = new EditorToolSettings(EditorToolSettings::Ok|
                                            EditorToolSettings::Try|
                                            EditorToolSettings::Cancel,
                                            EditorToolSettings::PanIcon);

    QGridLayout* grid = new QGridLayout(m_gboxSettings->plainPage(), 3, 2);

    QLabel *filterMethodLabel = new QLabel(i18n("Filter:"), m_gboxSettings->plainPage());
    m_filterMethodCombo       = new RComboBox(m_gboxSettings->plainPage());
    m_filterMethodCombo->insertItem(i18n("Average"));
    m_filterMethodCombo->insertItem(i18n("Linear"));
    m_filterMethodCombo->insertItem(i18n("Quadratic"));
    m_filterMethodCombo->insertItem(i18n("Cubic"));
    m_filterMethodCombo->setDefaultItem(HotPixelFixer::QUADRATIC_INTERPOLATION);

    m_blackFrameButton = new QPushButton(i18n("Black Frame..."), m_gboxSettings->plainPage());
    QWhatsThis::add(m_blackFrameButton, i18n("<p>Use this button to "
                    "add a new black frame file which will be used by the hot pixels removal filter."));

    m_blackFrameListView = new BlackFrameListView(m_gboxSettings->plainPage());

    grid->addMultiCellWidget(filterMethodLabel,    0, 0, 0, 0);
    grid->addMultiCellWidget(m_filterMethodCombo,  0, 0, 1, 1);
    grid->addMultiCellWidget(m_blackFrameButton,   0, 0, 2, 2);
    grid->addMultiCellWidget(m_blackFrameListView, 1, 2, 0, 2);
    grid->setRowStretch(3, 10);
    grid->setMargin(m_gboxSettings->spacingHint());
    grid->setSpacing(m_gboxSettings->spacingHint());

    setToolSettings(m_gboxSettings);

    // -------------------------------------------------------------

    m_previewWidget = new ImagePanelWidget(470, 350, "hotpixels Tool", m_gboxSettings->panIconView(),
                                           0, ImagePanelWidget::SeparateViewDuplicate);

    setToolView(m_previewWidget);
    init();

    // -------------------------------------------------------------

    connect(m_filterMethodCombo, SIGNAL(activated(int)),
            this, SLOT(slotEffect()));

    connect(m_blackFrameButton, SIGNAL(clicked()),
            this, SLOT(slotAddBlackFrame()));

    connect(m_blackFrameListView, SIGNAL(blackFrameSelected(QValueList<HotPixel>, const KURL&)),
            this, SLOT(slotBlackFrame(QValueList<HotPixel>, const KURL&)));
}

HotPixelsTool::~HotPixelsTool()
{
}

void HotPixelsTool::readSettings()
{
    KConfig *config = kapp->config();
    config->setGroup("hotpixels Tool");
    m_blackFrameURL = KURL(config->readEntry("Last Black Frame File", QString()));
    m_filterMethodCombo->setCurrentItem(config->readNumEntry("Filter Method",
                                        m_filterMethodCombo->defaultItem()));

    if (m_blackFrameURL.isValid())
    {
        EditorToolIface::editorToolIface()->setToolStartProgress(i18n("Loading: "));
        BlackFrameListViewItem *item = new BlackFrameListViewItem(m_blackFrameListView, m_blackFrameURL);

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
    KConfig *config = kapp->config();
    config->setGroup("hotpixels Tool");
    config->writeEntry("Last Black Frame File", m_blackFrameURL.url());
    config->writeEntry("Filter Method", m_filterMethodCombo->currentItem());
    m_previewWidget->writeSettings();
    config->sync();
}

void HotPixelsTool::slotResetSettings()
{
    m_filterMethodCombo->blockSignals(true);
    m_filterMethodCombo->slotReset();
    m_filterMethodCombo->blockSignals(false);
}

void HotPixelsTool::slotAddBlackFrame()
{
    KURL url = ImageDialog::getImageURL(kapp->activeWindow(), m_blackFrameURL, i18n("Select Black Frame Image"));

    if (!url.isEmpty())
    {
        // Load the selected file and insert into the list.

        m_blackFrameURL = url;
        m_blackFrameListView->clear();
        BlackFrameListViewItem *item = new BlackFrameListViewItem(m_blackFrameListView, m_blackFrameURL);

        connect(item, SIGNAL(signalLoadingProgress(float)),
                this, SLOT(slotLoadingProgress(float)));

        connect(item, SIGNAL(signalLoadingComplete()),
                this, SLOT(slotLoadingComplete()));
    }
}

void HotPixelsTool::renderingFinished()
{
    m_filterMethodCombo->setEnabled(true);
    m_blackFrameListView->setEnabled(true);
}

void HotPixelsTool::prepareEffect()
{
    m_filterMethodCombo->setEnabled(false);
    m_blackFrameListView->setEnabled(false);

    DImg image              = m_previewWidget->getOriginalRegionImage();
    int interpolationMethod = m_filterMethodCombo->currentItem();

    QValueList<HotPixel> hotPixelsRegion;
    QRect area = m_previewWidget->getOriginalImageRegionToRender();
    QValueList<HotPixel>::Iterator end(m_hotPixelsList.end());

    for (QValueList<HotPixel>::Iterator it = m_hotPixelsList.begin() ; it != end ; ++it )
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
    m_filterMethodCombo->setEnabled(false);
    m_blackFrameListView->setEnabled(false);

    int interpolationMethod = m_filterMethodCombo->currentItem();

    ImageIface iface(0, 0);
    setFilter(dynamic_cast<DImgThreadedFilter*>(new HotPixelFixer(iface.getOriginalImg(), this,m_hotPixelsList,interpolationMethod)));
}

void HotPixelsTool::putPreviewData()
{
    m_previewWidget->setPreviewImage(filter()->getTargetImage());
}

void HotPixelsTool::putFinalData()
{
    ImageIface iface(0, 0);
    iface.putOriginalImage(i18n("Hot Pixels Correction"), filter()->getTargetImage().bits());
}

void HotPixelsTool::slotBlackFrame(QValueList<HotPixel> hpList, const KURL& blackFrameURL)
{
    m_blackFrameURL = blackFrameURL;
    m_hotPixelsList = hpList;

    QPointArray pointList(m_hotPixelsList.size());
    QValueList <HotPixel>::Iterator it;
    int i = 0;
    QValueList <HotPixel>::Iterator end(m_hotPixelsList.end());

    for (it = m_hotPixelsList.begin() ; it != end ; ++it, i++)
       pointList.setPoint(i, (*it).rect.center());

    m_previewWidget->setPanIconHighLightPoints(pointList);

    slotEffect();
}

}  // NameSpace DigikamHotPixelsImagesPlugin
