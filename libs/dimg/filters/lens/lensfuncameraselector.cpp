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

// Local includes

#include "lensfuniface.h"

using namespace KDcrawIface;

namespace Digikam
{

class LensFunCameraSelector::LensFunCameraSelectorPriv
{
public:

    LensFunCameraSelectorPriv()
        : configUseMetadata("UseMetadata")
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

    const QString    configUseMetadata;

    RComboBox*       make;
    RComboBox*       model;
    RComboBox*       lens;

    RDoubleNumInput* focal;
    RDoubleNumInput* aperture;
    RDoubleNumInput* distance;

    DMetadata        metadata;

    LensFunIface*    iface;
};

LensFunCameraSelector::LensFunCameraSelector(QWidget* parent)
                     : QWidget(parent), d(new LensFunCameraSelectorPriv)
{
    d->iface           = new LensFunIface();

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
    grid->addWidget(makeLabel,        1, 0, 1, 3);
    grid->addWidget(d->make,          2, 0, 1, 3);
    grid->addWidget(modelLabel,       3, 0, 1, 3);
    grid->addWidget(d->model,         4, 0, 1, 3);
    grid->addWidget(lensLabel,        5, 0, 1, 3);
    grid->addWidget(d->lens,          6, 0, 1, 3);
    grid->addWidget(focalLabel,       7, 0, 1, 1);
    grid->addWidget(d->focal,         7, 1, 1, 2);
    grid->addWidget(aperLabel,        8, 0, 1, 1);
    grid->addWidget(d->aperture,      8, 1, 1, 2);
    grid->addWidget(distLabel,        9, 0, 1, 1);
    grid->addWidget(d->distance,      9, 1, 1, 2);
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
            this, SLOT(slotFocalChanged()));

    connect(d->aperture, SIGNAL(valueChanged(double)),
            this, SLOT(slotApertureChanged()));

    connect(d->distance, SIGNAL(valueChanged(double)),
            this, SLOT(slotDistanceChanged()));

    LensFunCameraSelector::Device firstDevice; // empty strings
//    setDevice( firstDevice );
}

LensFunCameraSelector::~LensFunCameraSelector()
{
    delete d->iface;
    delete d;
}

LensFunIface* LensFunCameraSelector::iface() const
{
    return d->iface;
}

LensFunContainer LensFunCameraSelector::settings()
{
    // Update settings in LensFun interface
    blockSignals(true);
    slotCameraSelected();
    slotLensSelected();
    slotFocalChanged();
    slotApertureChanged();
    slotDistanceChanged();
    blockSignals(false);
    return d->iface->settings();
}

void LensFunCameraSelector::setSettings(const LensFunContainer& settings)
{
    blockSignals(true);
    d->iface->setSettings(settings);
    refreshSettingsView(d->iface->m_settings);
    blockSignals(false);
}

void LensFunCameraSelector::readSettings(KConfigGroup& group)
{
    setUseMetadata(group.readEntry(d->configUseMetadata, true));
}

void LensFunCameraSelector::writeSettings(KConfigGroup& group)
{
    group.writeEntry(d->configUseMetadata, useMetadata());
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
            emit signalLensSettingsChanged();
        }
        else
        {
            findFromMetadata();
            emit signalLensSettingsChanged();
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
        slotUpdateCombos();
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
    refreshSettingsView(settings);
}

void LensFunCameraSelector::refreshSettingsView(const LensFunContainer& settings)
{
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
        d->make->setEnabled(d->passiveMetadataUsage);
        slotUpdateCombos();
    }

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
        d->model->setEnabled(d->passiveMetadataUsage);
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
        d->lens->setEnabled(d->passiveMetadataUsage);
    }

    // ------------------------------------------------------------------------------------------------

    if (settings.focalLength != -1.0)
    {
        d->focal->setValue(settings.focalLength);
        d->focal->setEnabled(d->passiveMetadataUsage);
    }

    if (settings.aperture != -1.0)
    {
        d->aperture->setValue(settings.aperture);
        d->aperture->setEnabled(d->passiveMetadataUsage);
    }

    if (settings.subjectDistance != -1.0)
    {
        d->distance->setValue(settings.subjectDistance);
        d->distance->setEnabled(d->passiveMetadataUsage);
    }
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
            d->model->combo()->addItem( dev->Model, b );
       }

       ++it;
    }
    d->make->combo()->model()->sort(0, Qt::AscendingOrder);
    d->model->combo()->model()->sort(0, Qt::AscendingOrder);

    slotCameraSelected();

    // Fill Lens list for current Maker & Model and fire signalLensSettingsChanged()
    slotUpdateLensCombo();
}

void LensFunCameraSelector::slotUpdateLensCombo()
{
    d->lens->combo()->clear();

    QVariant v = d->model->combo()->itemData( d->model->currentIndex() );
    if (!v.isValid() || v.isNull())
    {
        kDebug() << "Invalid variant value for device!";
        return;
    }
    kDebug() << "variant: " << v;

    LensFunContainer::DevicePtr dev = v.value<LensFunContainer::DevicePtr>();
    if (!dev)
    {
        kDebug() << "Device is null!";
        return;
    }

    kDebug() << "dev: " << dev->Maker << " :: " << dev->Model;

    const lfLens** lenses           = d->iface->m_lfDb->FindLenses( dev, NULL, NULL );
    d->iface->m_settings.cropFactor = dev ? dev->CropFactor : -1;

    while (lenses && *lenses)
    {
        LensFunContainer::LensPtr lens = *lenses;
        QVariant b                     = qVariantFromValue(lens);
        d->lens->combo()->addItem(lens->Model, b);
        ++lenses;
    }
    d->lens->combo()->model()->sort(0, Qt::AscendingOrder);

    // NOTE: signalLensSettingsChanged() is fired by this slot.
    slotLensSelected();
}

void LensFunCameraSelector::slotCameraSelected()
{
    QVariant v                      = d->model->combo()->itemData( d->model->currentIndex() );
    d->iface->m_settings.usedCamera = d->metadataUsage->isChecked() && d->passiveMetadataUsage ? 0 :
                                      v.value<LensFunContainer::DevicePtr>();
}

void LensFunCameraSelector::slotLensSelected()
{
    QVariant v                    = d->lens->combo()->itemData( d->lens->currentIndex() );
    d->iface->m_settings.usedLens = d->metadataUsage->isChecked() && d->passiveMetadataUsage ? 0 :
                                    v.value<LensFunContainer::LensPtr>();

    if (d->iface->m_settings.usedLens &&
        d->iface->m_settings.cropFactor <= 0.0) // this should not happen
        d->iface->m_settings.cropFactor = d->iface->m_settings.usedLens->CropFactor;
    else 
        d->iface->m_settings.cropFactor = -1.0;

    emit signalLensSettingsChanged();
}

void LensFunCameraSelector::slotFocalChanged()
{
    d->iface->m_settings.focalLength = d->metadataUsage->isChecked() && d->passiveMetadataUsage ? -1.0 :
                                       d->focal->value();
    emit signalLensSettingsChanged();
}

void LensFunCameraSelector::slotApertureChanged()
{
    d->iface->m_settings.aperture = d->metadataUsage->isChecked() && d->passiveMetadataUsage ? -1.0 :
                                    d->aperture->value();
    emit signalLensSettingsChanged();
}

void LensFunCameraSelector::slotDistanceChanged()
{
    d->iface->m_settings.subjectDistance = d->metadataUsage->isChecked() && d->passiveMetadataUsage ? -1.0 :
                                           d->distance->value();
    emit signalLensSettingsChanged();
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
