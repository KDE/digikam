/* ============================================================
 *
 * Date        : 2008-02-10
 * Description : a plugin to fix automatically camera lens aberrations
 *
 * Copyright (C) 2008 by Adrian Schroeter <adrian at suse dot de>
 * Copyright (C) 2008-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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


#include "autocorrectiontool.moc"

// Qt includes

#include <QBitmap>
#include <QBrush>
#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QTimer>

// KDE includes

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kseparator.h>
#include <kstandarddirs.h>

// Local includes

#include "daboutdata.h"
#include "dmetadata.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "klensfun.h"
#include "version.h"

using namespace Digikam;

namespace DigikamAutoCorrectionImagesPlugin
{

class AutoCorrectionToolPriv
{
public:

    AutoCorrectionToolPriv() :
        configGroupName("Lens Auto-Correction Tool"),
        configCCAEntry("CCA"),
        configVignettingEntry("Vignetting"),
        configCCIEntry("CCI"),
        configDistortionEntry("Distortion"),
        configGeometryEntry("Geometry"),

        maskPreviewLabel(0),
        showGrid(0),
        filterCCA(0),
        filterVig(0),
        filterCCI(0),
        filterDist(0),
        filterGeom(0),
        cameraSelector(0),
        previewWidget(0),
        gboxSettings(0)
        {}

    const QString       configGroupName;
    const QString       configCCAEntry;
    const QString       configVignettingEntry;
    const QString       configCCIEntry;
    const QString       configDistortionEntry;
    const QString       configGeometryEntry;

    QLabel*             maskPreviewLabel;

    QCheckBox*          showGrid;
    QCheckBox*          filterCCA;
    QCheckBox*          filterVig;
    QCheckBox*          filterCCI;
    QCheckBox*          filterDist;
    QCheckBox*          filterGeom;

    KLFDeviceSelector*  cameraSelector;

    ImageWidget*        previewWidget;
    EditorToolSettings* gboxSettings;
};

AutoCorrectionTool::AutoCorrectionTool(QObject* parent)
                  : EditorToolThreaded(parent),
                    d(new AutoCorrectionToolPriv)
{
    setObjectName("lensautocorrection");
    setToolName(i18n("Lens Auto-Correction"));
    setToolIcon(SmallIcon("lensdistortion"));

    d->previewWidget = new ImageWidget("antivignetting Tool", 0, QString(),
                                       true, ImageGuideWidget::HVGuideMode, true);

    setToolView(d->previewWidget);

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;

    QGridLayout *grid  = new QGridLayout(d->gboxSettings->plainPage());
    d->cameraSelector  = new KLFDeviceSelector(d->gboxSettings->plainPage());
    KSeparator *line   = new KSeparator(Qt::Horizontal, d->gboxSettings->plainPage());

    // -------------------------------------------------------------

    d->showGrid   = new QCheckBox(i18n("Show grid"), d->gboxSettings->plainPage());
    d->showGrid->setWhatsThis(i18n("Set this option to visualize the correction grid to be applied."));

    d->filterCCA  = new QCheckBox(i18n("Chromatic Aberration"), d->gboxSettings->plainPage());
    d->filterCCA->setWhatsThis(i18n("Chromatic aberration is easily recognized as color fringes "
                                    "towards the image corners. CA is due to a varying lens focus "
                                    "for different colors."));
    d->filterVig  = new QCheckBox(i18n("Vignetting"), d->gboxSettings->plainPage());
    d->filterVig->setWhatsThis(i18n("Vignetting refers to an image darkening, mostly in the corners. "
                                    "Optical and natural vignetting can be canceled out with this option, "
                                    "whereas mechanical vignetting will not be cured."));
    d->filterCCI  = new QCheckBox(i18n("Color Correction"), d->gboxSettings->plainPage());
    d->filterCCI->setWhatsThis(i18n("All lenses have a slight color tinge to them, "
                                    "mostly due to the anti-reflective coating. "
                                    "The tinge can be canceled when the respective data is known for the lens."));
    d->filterDist = new QCheckBox(i18n("Distortion"), d->gboxSettings->plainPage());
    d->filterDist->setWhatsThis(i18n("Distortion refers to an image deformation, which is most pronounced "
                                     "towards the corners. These Seidel aberrations are known as pincushion "
                                     "and barrel distortions."));
    d->filterGeom = new QCheckBox(i18n("Geometry"), d->gboxSettings->plainPage());
    d->filterGeom->setWhatsThis(i18n("Four geometries are handled here: Rectilinear (99 percent of all lenses), "
                                     "Fisheye, Cylindrical, Equirectangular."));

    grid->addWidget(d->showGrid,       0, 0, 1, 2);
    grid->addWidget(d->cameraSelector, 1, 0, 1, 2);
    grid->addWidget(line,              2, 0, 1, 2);
    grid->addWidget(d->filterCCA,      3, 0, 1, 2);
    grid->addWidget(d->filterVig,      4, 0, 1, 2);
    grid->addWidget(d->filterCCI,      5, 0, 1, 2);
    grid->addWidget(d->filterDist,     6, 0, 1, 2);
    grid->addWidget(d->filterGeom,     7, 0, 1, 2);
    grid->setRowStretch(8, 10);
    grid->setMargin(d->gboxSettings->spacingHint());
    grid->setSpacing(d->gboxSettings->spacingHint());

    setToolSettings(d->gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(d->cameraSelector, SIGNAL(signalLensSettingsChanged()),
            this, SLOT(slotLensChanged()));

    connect(d->showGrid, SIGNAL(stateChanged(int)),
            this, SLOT(slotSetFilters()));

    connect(d->filterCCA, SIGNAL(stateChanged(int)),
            this, SLOT(slotSetFilters()));

    connect(d->filterVig, SIGNAL(stateChanged(int)),
            this, SLOT(slotSetFilters()));

    connect(d->filterCCI, SIGNAL(stateChanged(int)),
            this, SLOT(slotSetFilters()));

    connect(d->filterDist, SIGNAL(stateChanged(int)),
            this, SLOT(slotSetFilters()));

    connect(d->filterGeom, SIGNAL(stateChanged(int)),
            this, SLOT(slotSetFilters()));

    QTimer::singleShot(0, this, SLOT(slotResetSettings()));
}

AutoCorrectionTool::~AutoCorrectionTool()
{
    delete d;
}

void AutoCorrectionTool::slotLensChanged()
{
    d->filterCCA->setEnabled(d->cameraSelector->getKLFObject()->supportsCCA());
    d->filterVig->setEnabled(d->cameraSelector->getKLFObject()->supportsVig());
    d->filterCCI->setEnabled(d->cameraSelector->getKLFObject()->supportsVig());
    d->filterDist->setEnabled(d->cameraSelector->getKLFObject()->supportsDistortion());
    d->filterGeom->setEnabled(d->cameraSelector->getKLFObject()->supportsDistortion());
    slotSetFilters();
}

void AutoCorrectionTool::slotSetFilters()
{
    d->cameraSelector->getKLFObject()->setCorrection(
        (d->filterCCA->checkState()  == Qt::Checked && d->filterCCA->isEnabled())  ? true : false,
        (d->filterVig->checkState()  == Qt::Checked && d->filterVig->isEnabled())  ? true : false,
        (d->filterCCI->checkState()  == Qt::Checked && d->filterCCI->isEnabled())  ? true : false,
        (d->filterDist->checkState() == Qt::Checked && d->filterDist->isEnabled()) ? true : false,
        (d->filterGeom->checkState() == Qt::Checked && d->filterGeom->isEnabled()) ? true : false
     );

    slotTimer();
}

void AutoCorrectionTool::readSettings()
{
    d->gboxSettings->blockSignals(true);
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    d->filterCCA->setCheckState(group.readEntry(d->configCCAEntry, true)         ? Qt::Checked : Qt::Unchecked);
    d->filterVig->setCheckState(group.readEntry(d->configVignettingEntry, true)  ? Qt::Checked : Qt::Unchecked);
    d->filterCCI->setCheckState(group.readEntry(d->configCCIEntry, true)         ? Qt::Checked : Qt::Unchecked);
    d->filterDist->setCheckState(group.readEntry(d->configDistortionEntry, true) ? Qt::Checked : Qt::Unchecked);
    d->filterGeom->setCheckState(group.readEntry(d->configGeometryEntry, true)   ? Qt::Checked : Qt::Unchecked);

    d->gboxSettings->blockSignals(false);
    slotSetFilters();
}

void AutoCorrectionTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    if ( d->filterCCA->isEnabled() )
        group.writeEntry(d->configCCAEntry,        (d->filterCCA->checkState() == Qt::Checked)  ? true : false);
    if ( d->filterVig->isEnabled() )
        group.writeEntry(d->configVignettingEntry, (d->filterVig->checkState() == Qt::Checked)  ? true : false);
    if ( d->filterCCI->isEnabled() )
        group.writeEntry(d->configCCIEntry,        (d->filterCCI->checkState() == Qt::Checked)  ? true : false);
    if ( d->filterDist->isEnabled() )
        group.writeEntry(d->configDistortionEntry, (d->filterDist->checkState() == Qt::Checked) ? true : false);
    if ( d->filterGeom->isEnabled() )
        group.writeEntry(d->configGeometryEntry,   (d->filterGeom->checkState() == Qt::Checked) ? true : false);

    d->previewWidget->writeSettings();
    group.sync();
}

void AutoCorrectionTool::slotResetSettings()
{
    d->gboxSettings->blockSignals(true);

    // Read Exif information ...
    DImg      *img = d->previewWidget->imageIface()->getOriginalImg();
    DMetadata  meta;
    meta.setExif(img->getExif());
    meta.setIptc(img->getIptc());
    meta.setXmp(img->getXmp());
    d->cameraSelector->findFromMetadata(meta);

    d->gboxSettings->blockSignals(false);
}

void AutoCorrectionTool::prepareEffect()
{
    d->gboxSettings->setEnabled(false);

    ImageIface* iface          = d->previewWidget->imageIface();
    uchar *data                = iface->getPreviewImage();
    int w                      = iface->previewWidth();
    int h                      = iface->previewHeight();
    bool sb                    = iface->previewSixteenBit();

    DImg image(w, h, sb, false, data);

    if (d->showGrid->isChecked())
    {
        QBitmap pattern(9, 9);
        pattern.clear();
        QPainter p1(&pattern);
        p1.setPen(QPen(Qt::black, 1));
        p1.drawLine(5, 0, 5, 9);
        p1.drawLine(0, 5, 9, 5);
        p1.end();

        QPixmap pix(w, h);
        pix.fill(Qt::transparent);
        QPainter p2(&pix);
        p2.setPen(QPen(Qt::gray, 1));
        p2.fillRect(0, 0, pix.width(), pix.height(), QBrush(pattern));
        p2.end();
        DImg grid(pix.toImage());

        DColorComposer *composer = DColorComposer::getComposer(DColorComposer::PorterDuffNone);
        DColorComposer::MultiplicationFlags flags = DColorComposer::NoMultiplication;

        // Do alpha blending of template on dest image
        image.bitBlendImage(composer, &grid, 0, 0, w, h, 0, 0, flags);
    }

    setFilter(dynamic_cast<DImgThreadedFilter *>
                       (new KLensFunFilter(&image, this, d->cameraSelector->getKLFObject())));

}

void AutoCorrectionTool::prepareFinal()
{
    d->gboxSettings->setEnabled(false);

    ImageIface iface(0, 0);
    uchar *data = iface.getOriginalImage();
    DImg originalImage(iface.originalWidth(), iface.originalHeight(),
                                iface.originalSixteenBit(), iface.originalHasAlpha(), data);

    setFilter(dynamic_cast<DImgThreadedFilter *>
                       (new KLensFunFilter( &originalImage, this, d->cameraSelector->getKLFObject())));

    delete [] data;
}

void AutoCorrectionTool::putPreviewData()
{
    DImg imDest = filter()->getTargetImage();
    d->previewWidget->imageIface()->putPreviewImage(imDest.bits());
    d->previewWidget->updatePreview();
}

void AutoCorrectionTool::putFinalData()
{
    ImageIface iface(0, 0);
    iface.putOriginalImage(i18n("Lens Auto-Correction"), filter()->getTargetImage().bits());
}

void AutoCorrectionTool::renderingFinished()
{
    d->gboxSettings->setEnabled(true);
}

}  // namespace DigikamAutoCorrectionImagesPlugin
