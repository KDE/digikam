/* ==================-==========================================
 *
 * Date        : 2008-02-10
 * Description : a tool to fix automatically camera lens aberrations
 *
 * Copyright (C) 2008 by Adrian Schroeter <adrian at suse dot de>
 * Copyright (C) 2008-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "lensfuncameraselector.moc"

// Qt includes

#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QString>
#include <QWidget>

// KDE includes

#include <kcombobox.h>
#include <kdialog.h>
#include <klocale.h>
#include <kdebug.h>

// LibKDcraw includes

#include <libkdcraw/rnuminput.h>
#include <libkdcraw/rcombobox.h>

using namespace KDcrawIface;

namespace Digikam
{

class LensFunCameraSelector::LensFunCameraSelectorPriv
{
public:

    LensFunCameraSelectorPriv()
    {
        metadataUsage        = 0;
        make                 = 0;
        model                = 0;
        focal                = 0;
        aperture             = 0;
        distance             = 0;
        iface                = 0;
        passiveMetadataUsage = false;
    }

    bool             passiveMetadataUsage;

    QCheckBox*       metadataUsage;

    RComboBox*       make;
    RComboBox*       model;
    RComboBox*       lens;

    RDoubleNumInput* focal;
    RDoubleNumInput* aperture;
    RDoubleNumInput* distance;

    DMetadata        metadata;

    LensFunIface*    iface;
};

LensFunCameraSelector::LensFunCameraSelector(LensFunIface* iface, QWidget* parent)
                     : QWidget(parent), d(new LensFunCameraSelectorPriv)
{
    d->iface           = iface;
    QGridLayout* grid  = new QGridLayout(this);
    d->metadataUsage   = new QCheckBox(i18n("Use Metadata"), this);

    d->make            = new RComboBox(this);
    d->make->setDefaultIndex(0);

    d->model           = new RComboBox(this);
    d->model->setDefaultIndex(0);

    d->lens            = new RComboBox(this);
    d->lens->setDefaultIndex(0);

    QLabel* makeLabel  = new QLabel(i18nc("camera make", "Make:"), this);
    QLabel* modelLabel = new QLabel(i18nc("camera model", "Model:"), this);
    QLabel* lensLabel  = new QLabel(i18nc("camera lens", "Lens:"), this);

    d->metadataUsage->setEnabled(false);
    d->metadataUsage->setCheckState(Qt::Unchecked);
    d->metadataUsage->setWhatsThis(i18n("Set this option to try to guess the right camera/lens settings "
                                   "from the image metadata (as Exif or XMP)."));

    QLabel* focalLabel = new QLabel(i18n("Focal Length:"), this);
    QLabel* aperLabel  = new QLabel(i18n("Aperture:"), this);
    QLabel* distLabel  = new QLabel(i18n("Subject Distance:"), this);

    d->focal = new RDoubleNumInput(this);
    d->focal->setDecimals(1);
    d->focal->input()->setRange(1.0, 1000.0, 0.01, true);
    d->focal->setDefaultValue(1.0);

    d->aperture = new RDoubleNumInput(this);
    d->aperture->setDecimals(1);
    d->aperture->input()->setRange(1.1, 64.0, 0.1, true);
    d->aperture->setDefaultValue(1.1);

    d->distance = new RDoubleNumInput(this);
    d->distance->setDecimals(1);
    d->distance->input()->setRange(0.0, 100.0, 0.1, true);
    d->distance->setDefaultValue(0.0);

    grid->addWidget(d->metadataUsage, 0, 0, 1, 3);
    grid->addWidget(makeLabel,    1, 0, 1, 3);
    grid->addWidget(d->make,      2, 0, 1, 3);
    grid->addWidget(modelLabel,   3, 0, 1, 3);
    grid->addWidget(d->model,     4, 0, 1, 3);
    grid->addWidget(lensLabel,    5, 0, 1, 3);
    grid->addWidget(d->lens,      6, 0, 1, 3);
    grid->addWidget(focalLabel,   7, 0, 1, 1);
    grid->addWidget(d->focal,     7, 1, 1, 2);
    grid->addWidget(aperLabel,    8, 0, 1, 1);
    grid->addWidget(d->aperture,  8, 1, 1, 2);
    grid->addWidget(distLabel,    9, 0, 1, 1);
    grid->addWidget(d->distance,  9, 1, 1, 2);
    grid->setMargin(0);
    grid->setSpacing(KDialog::spacingHint());

    connect(d->metadataUsage, SIGNAL(toggled(bool)),
            this, SLOT(slotUseMetadata(bool)));

    connect(d->make, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotUpdateCombos()));

    connect(d->model, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotUpdateLensCombo()));

    connect(d->lens, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotLensSelected()));

    connect(d->focal, SIGNAL(valueChanged(double)),
            this, SLOT(slotFocalChanged(double)));

    connect(d->aperture, SIGNAL(valueChanged(double)),
            this, SLOT(slotApertureChanged(double)));

    connect(d->distance, SIGNAL(valueChanged(double)),
            this, SLOT(slotDistanceChanged(double)));

    LensFunCameraSelector::Device firstDevice; // empty strings
//    setDevice( firstDevice );
}

LensFunCameraSelector::~LensFunCameraSelector()
{
    delete d;
}

void LensFunCameraSelector::findFromMetadata(const DMetadata& meta)
{
    d->metadata = meta;
    findFromMetadata();
}

void LensFunCameraSelector::enableUseMetadata(bool b)
{
    d->metadataUsage->setEnabled(b);
}

void LensFunCameraSelector::setUseMetadata(bool b)
{
    d->metadataUsage->setChecked(b);
}

bool LensFunCameraSelector::useMetadata() const
{
      return (d->metadataUsage->isChecked());
}

void LensFunCameraSelector::setPassiveMetadataUsage(bool b)
{
    d->passiveMetadataUsage = b;
}

void LensFunCameraSelector::slotUseMetadata(bool b)
{
    if (b)
    {
        if (d->passiveMetadataUsage)
        {
            d->make->setEnabled(false);
            d->model->setEnabled(false);
            d->lens->setEnabled(false);
            d->focal->setEnabled(false);
            d->aperture->setEnabled(false);
            d->distance->setEnabled(false);
        }
        else
        {
            findFromMetadata();
        }
    }
    else
    {
        d->make->setEnabled(true);
        d->model->setEnabled(true);
        d->lens->setEnabled(true);
        d->focal->setEnabled(true);
        d->aperture->setEnabled(true);
        d->distance->setEnabled(true);
    }
}

void LensFunCameraSelector::findFromMetadata()
{
//    LensFunCameraSelector::Device firstDevice; // empty strings
//    setDevice( firstDevice );

    if (d->metadata.isEmpty())
    {
        d->metadataUsage->setCheckState(Qt::Unchecked);
        enableUseMetadata(false);
    }
    else
    {
        d->metadataUsage->setCheckState(Qt::Checked);
        enableUseMetadata(true);
    }

    LensFunContainer settings;
    d->iface->findFromMetadata(d->metadata, settings);

    // ------------------------------------------------------------------------------------------------

    QString makeLF;
    int     makerIdx = -1;

    if (settings.usedCamera)
    {
        makeLF   = settings.usedCamera->Maker;
        makerIdx = d->make->combo()->findText(makeLF);
    }

    if (makerIdx >= 0)
    {
        d->make->setCurrentIndex(makerIdx);
        d->make->setEnabled(false);
    }

    slotUpdateCombos();

    // ------------------------------------------------------------------------------------------------

    QString modelLF;
    int     modelIdx = -1;

    if (settings.usedCamera)
    {
        modelLF  = settings.usedCamera->Model;
        modelIdx = d->model->combo()->findText(modelLF);
    }

    if (modelIdx >= 0)
    {
        d->model->setCurrentIndex(modelIdx);
        d->model->setEnabled(false);
        slotUpdateLensCombo();
    }

    // ------------------------------------------------------------------------------------------------

    QString lensLF;
    int     lensIdx = -1;

    if (settings.usedLens)
    {
        lensLF  = settings.usedLens->Model;
        lensIdx = d->lens->combo()->findText(lensLF);
    }

    if (lensIdx >= 0)
    {
        // found lens model directly, best case :)
        d->lens->setCurrentIndex(lensIdx);
        d->lens->setEnabled(false);
    }
    else
    {
        // Lens not found, try to reduce the list according to the values we have
        // FIXME: Implement removal of not matching lenses ...
        d->lens->setEnabled(true);
    }

    // ------------------------------------------------------------------------------------------------

    if (settings.focalLength != -1.0)
    {
        d->focal->setValue(settings.focalLength);
        d->focal->setEnabled(false);
    }

    if (settings.aperture != -1.0)
    {
        d->aperture->setValue(settings.aperture);
        d->aperture->setEnabled(false);
    }

    if (settings.subjectDistance != -1.0)
    {
        d->distance->setValue(settings.subjectDistance);
        d->distance->setEnabled(false);
    }
}

void LensFunCameraSelector::slotFocalChanged(double focal)
{
    d->iface->m_settings.focalLength = focal;
    emit signalLensSettingsChanged();
}

void LensFunCameraSelector::slotApertureChanged(double aperture)
{
    d->iface->m_settings.aperture = aperture;
    emit signalLensSettingsChanged();
}

void LensFunCameraSelector::slotDistanceChanged(double distance)
{
    d->iface->m_settings.subjectDistance = distance;
    emit signalLensSettingsChanged();
}

void LensFunCameraSelector::slotUpdateCombos()
{
    const lfCamera* const* it = d->iface->m_lfCameras;

    // reset box
    d->model->combo()->clear();

    bool firstRun = false;
    if ( d->make->combo()->count() == 0 )
       firstRun = true;

    while ( *it )
    {
       if ( firstRun )
       {
           // Maker DB does not change, so we fill it only once.
           if ( (*it)->Maker )
           {
                QString t( (*it)->Maker );
                if ( d->make->combo()->findText( t, Qt::MatchExactly ) < 0 )
                    d->make->addItem( t );
           }
       }

       // Fill models for current selected maker
       if ( (*it)->Model && (*it)->Maker == d->make->combo()->currentText() )
       {
            LensFunContainer::DevicePtr dev = *it;
            QVariant b                      = qVariantFromValue(dev);
            d->model->combo()->addItem( (*it)->Model, b );
       }

       ++it;
    }
    d->make->combo()->model()->sort(0, Qt::AscendingOrder);
    d->model->combo()->model()->sort(0, Qt::AscendingOrder);

    // Fill Lens list for current Maker & Model
    slotUpdateLensCombo();
}

void LensFunCameraSelector::slotUpdateLensCombo()
{
    d->lens->combo()->clear();

    QVariant v                      = d->model->combo()->itemData( d->model->currentIndex() );
    LensFunContainer::DevicePtr dev = v.value<LensFunContainer::DevicePtr>();
    if (!dev)
    {
        kDebug() << "Device is null!";
        return;
    }

    const lfLens** lenses           = d->iface->m_lfDb->FindLenses( dev, NULL, NULL );
    d->iface->m_settings.cropFactor = dev->CropFactor;

    while (lenses && *lenses)
    {
        LensFunContainer::LensPtr lens = *lenses;
        QVariant b                     = qVariantFromValue(lens);
        d->lens->combo()->addItem((*lenses)->Model, b);
        ++lenses;
    }
    d->lens->combo()->model()->sort(0, Qt::AscendingOrder);

    emit(signalLensSettingsChanged());
}

void LensFunCameraSelector::slotLensSelected()
{
    QVariant v                    = d->lens->combo()->itemData( d->lens->currentIndex() );
    d->iface->m_settings.usedLens = v.value<LensFunContainer::LensPtr>();

    if ( d->iface->m_settings.cropFactor <= 0.0 ) // this should not happen
        d->iface->m_settings.cropFactor = d->iface->m_settings.usedLens->CropFactor;

    emit(signalLensSettingsChanged());
}

void LensFunCameraSelector::setDevice(Device& /*d*/)
{
    slotUpdateCombos();
}

#if 0
LensFunCameraSelector::Device LensFunCameraSelector::getDevice()
{
}
#endif

}  // namespace Digikam
