/* ==================-==========================================
 *
 * Date        : 2008-02-10
 * Description : a plugin to fix automatically camera lens aberrations
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

Q_DECLARE_METATYPE( Digikam::LensFunCameraSelector::DevicePtr )
Q_DECLARE_METATYPE( Digikam::LensFunCameraSelector::LensPtr )

namespace Digikam
{

LensFunCameraSelector::LensFunCameraSelector(LensFunIface* iface, QWidget* parent)
                     : QWidget(parent)
{
    m_iface            = iface;
    QGridLayout* grid  = new QGridLayout(this);
    m_exifUsage        = new QCheckBox(i18n("Use Metadata"), this);

    m_make             = new RComboBox(this);
    m_make->setDefaultIndex(0);

    m_model            = new RComboBox(this);
    m_model->setDefaultIndex(0);

    m_lens             = new RComboBox(this);
    m_lens->setDefaultIndex(0);

    QLabel* makeLabel  = new QLabel(i18nc("camera make", "Make:"), this);
    QLabel* modelLabel = new QLabel(i18nc("camera model", "Model:"), this);
    QLabel* lensLabel  = new QLabel(i18nc("camera lens", "Lens:"), this);

    m_exifUsage->setEnabled(false);
    m_exifUsage->setCheckState(Qt::Unchecked);
    m_exifUsage->setWhatsThis(i18n("Set this option to try to guess the right camera/lens settings "
                                   "from the image metadata (as Exif or XMP)."));

    QLabel* focalLabel = new QLabel(i18n("Focal Length:"), this);
    QLabel* aperLabel  = new QLabel(i18n("Aperture:"), this);
    QLabel* distLabel  = new QLabel(i18n("Subject Distance:"), this);

    m_focal = new RDoubleNumInput(this);
    m_focal->setDecimals(1);
    m_focal->input()->setRange(1.0, 1000.0, 0.01, true);
    m_focal->setDefaultValue(1.0);

    m_aperture = new RDoubleNumInput(this);
    m_aperture->setDecimals(1);
    m_aperture->input()->setRange(1.1, 64.0, 0.1, true);
    m_aperture->setDefaultValue(1.1);

    m_distance = new RDoubleNumInput(this);
    m_distance->setDecimals(1);
    m_distance->input()->setRange(0.0, 100.0, 0.1, true);
    m_distance->setDefaultValue(0.0);

    grid->addWidget(m_exifUsage, 0, 0, 1, 3);
    grid->addWidget(makeLabel,   1, 0, 1, 3);
    grid->addWidget(m_make,      2, 0, 1, 3);
    grid->addWidget(modelLabel,  3, 0, 1, 3);
    grid->addWidget(m_model,     4, 0, 1, 3);
    grid->addWidget(lensLabel,   5, 0, 1, 3);
    grid->addWidget(m_lens,      6, 0, 1, 3);
    grid->addWidget(focalLabel,  7, 0, 1, 1);
    grid->addWidget(m_focal,     7, 1, 1, 2);
    grid->addWidget(aperLabel,   8, 0, 1, 1);
    grid->addWidget(m_aperture,  8, 1, 1, 2);
    grid->addWidget(distLabel,   9, 0, 1, 1);
    grid->addWidget(m_distance,  9, 1, 1, 2);
    grid->setMargin(0);
    grid->setSpacing(KDialog::spacingHint());

    connect(m_exifUsage, SIGNAL(toggled(bool)),
            this, SLOT(slotUseExif(bool)));

    connect(m_make, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotUpdateCombos()));

    connect(m_model, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotUpdateLensCombo()));

    connect(m_lens, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotLensSelected()));

    connect(m_focal, SIGNAL(valueChanged(double)),
            this, SLOT(slotFocalChanged(double)));

    connect(m_aperture, SIGNAL(valueChanged(double)),
            this, SLOT(slotApertureChanged(double)));

    connect(m_distance, SIGNAL(valueChanged(double)),
            this, SLOT(slotDistanceChanged(double)));

    LensFunCameraSelector::Device firstDevice; // empty strings
//    setDevice( firstDevice );
}

LensFunCameraSelector::~LensFunCameraSelector()
{
}

#if 0
LensFunCameraSelector::Device LensFunCameraSelector::getDevice()
{
}
#endif

void LensFunCameraSelector::findFromMetadata(const DMetadata& meta)
{
    m_metadata = meta;
    findFromMetadata();
}

void LensFunCameraSelector::findFromMetadata()
{
//    LensFunCameraSelector::Device firstDevice; // empty strings
//    setDevice( firstDevice );

    if (m_metadata.isEmpty())
    {
        m_exifUsage->setCheckState(Qt::Unchecked);
        m_exifUsage->setEnabled(false);
    }
    else
    {
        m_exifUsage->setCheckState(Qt::Checked);
        m_exifUsage->setEnabled(true);
    }

    PhotoInfoContainer photoInfo = m_metadata.getPhotographInformation();
    QString make                 = photoInfo.make;
    QString model                = photoInfo.model;
    QString lens                 = photoInfo.lens;

    // ------------------------------------------------------------------------------------------------

    int makerIdx = m_make->combo()->findText(make);
    if (makerIdx < 0)
    {
        const lfCamera** makes = m_iface->m_lfDb->FindCameras( make.toAscii(), model.toAscii() );
        QString makeLF = "";
        if (makes && *makes)
        {
            makeLF =(*makes)->Maker;
            makerIdx = m_make->combo()->findText(makeLF);
        }
    }
    if (makerIdx >= 0)
    {
        m_make->setCurrentIndex(makerIdx);
        m_make->setEnabled(false);
    }

    slotUpdateCombos();
    int modelIdx = m_model->combo()->findText(model);
    if (modelIdx < 0)
    {
        const lfCamera** makes = m_iface->m_lfDb->FindCameras( make.toAscii(), model.toAscii() );
        QString modelLF = "";
        int count = 0;
        while (makes && *makes)
        {
            modelLF =(*makes)->Model;
            ++makes;
            ++count;
        }
        if (count == 1)
        {
            modelIdx = m_model->combo()->findText(modelLF);
        }
    }
    if (modelIdx >= 0)
    {
        m_model->setCurrentIndex(modelIdx);
        m_model->setEnabled(false);
        slotUpdateLensCombo();
    }

    // The LensFun DB has the Maker before the Lens model name.
    // We use here the Camera Maker, because the Lens Maker seems not to be
    // part of the Exif data. This is of course bad for 3rd party lenses, but
    // they seem anyway not to have Exif entries usually :/
    int lensIdx = m_lens->combo()->findText(lens);
    if (lensIdx < 0)
       lensIdx = m_lens->combo()->findText(make + ' ' + lens);
    if (lensIdx < 0)
    {
        QString lensCutted = lens;
        if (lensCutted.contains("Nikon"))
        {
            // adapt exiv2 strings to lensfun strings
            lensCutted.replace("Nikon ", "");
            lensCutted.replace("Zoom-", "");
            lensCutted.replace("IF-ID", "ED-IF");
        }
        QVariant v    = m_model->combo()->itemData( m_model->currentIndex() );
        DevicePtr dev = v.value<LensFunCameraSelector::DevicePtr>();
        const lfLens** lenses = m_iface->m_lfDb->FindLenses( dev, NULL, lensCutted.toAscii().data() );
        QString lensLF = "";
        int count = 0;
        while (lenses && *lenses)
        {
            lensLF =(*lenses)->Model;
            ++lenses;
            ++count;
        }
        if (count == 1)
        {
            lensIdx = m_lens->combo()->findText(lensLF);
        }
    }

    if (lensIdx >= 0)
    {
        // found lens model directly, best case :)
        m_lens->setCurrentIndex(lensIdx);
        m_lens->setEnabled(false);
    }
    else
    {
        // Lens not found, try to reduce the list according to the values we have
        // FIXME: Implement removal of not matching lenses ...
        m_lens->setEnabled(true);
    }

    kDebug() << "Search for Lens: " << make << " :: " << lens
             << "< and found: >" << m_lens->combo()->itemText(0) + " <";

    QString temp = photoInfo.focalLength;
    if (!temp.isEmpty())
    {
        double focal = temp.mid(0, temp.length() -3).toDouble(); // HACK: strip the " mm" at the end ...
        kDebug() << "Focal Length: " << focal;
        m_focal->setValue(focal);
        m_focal->setEnabled(false);
    }

    temp = photoInfo.aperture;
    if (!temp.isEmpty())
    {
        double aperture = temp.mid(1).toDouble();
        kDebug() << "Aperture: " << aperture;
        m_aperture->setValue(aperture);
        m_aperture->setEnabled(false);
    }

    // ------------------------------------------------------------------------------------------------
    // Try to get subject distance value.

    // From standard Exif.
    temp = m_metadata.getExifTagString("Exif.Photo.SubjectDistance");
    if (temp.isEmpty())
    {
        // From standard XMP.
        temp = m_metadata.getXmpTagString("Xmp.exif.SubjectDistance");
    }
    if (temp.isEmpty())
    {
        // From Canon Makernote.
        temp = m_metadata.getExifTagString("Exif.CanonSi.SubjectDistance");
    }
    if (temp.isEmpty())
    {
        // From Nikon Makernote.
        temp = m_metadata.getExifTagString("Exif.NikonLd2.FocusDistance");
    }
    if (temp.isEmpty())
    {
        // From Nikon Makernote.
        temp = m_metadata.getExifTagString("Exif.NikonLd3.FocusDistance");
    }
    // TODO: Add here others Makernotes tags.

    if (!temp.isEmpty())
    {
        temp = temp.replace(" m", "");
        double distance = temp.toDouble();
        kDebug() << "Subject Distance: " << distance;
        m_distance->setValue(distance);
        m_distance->setEnabled(false);
    }
}

void LensFunCameraSelector::slotFocalChanged(double f)
{
    m_iface->m_focalLength = f;
    emit signalLensSettingsChanged();
}

void LensFunCameraSelector::slotApertureChanged(double a)
{
    m_iface->m_aperture = a;
    emit signalLensSettingsChanged();
}

void LensFunCameraSelector::slotDistanceChanged(double d)
{
    m_iface->m_subjectDistance = d;
    emit signalLensSettingsChanged();
}

void LensFunCameraSelector::slotUseExif(bool b)
{
    if (b)
    {
        findFromMetadata();
    }
    else
    {
        m_make->setEnabled(true);
        m_model->setEnabled(true);
        m_lens->setEnabled(true);
        m_focal->setEnabled(true);
        m_aperture->setEnabled(true);
        m_distance->setEnabled(true);
    }
}

void LensFunCameraSelector::slotUpdateCombos()
{
    const lfCamera* const* it = m_iface->m_lfCameras;

    // reset box
    m_model->combo()->clear();

    bool firstRun = false;
    if ( m_make->combo()->count() == 0 )
       firstRun = true;

    while ( *it )
    {
       if ( firstRun )
       {
           // Maker DB does not change, so we fill it only once.
           if ( (*it)->Maker )
           {
                QString t( (*it)->Maker );
                if ( m_make->combo()->findText( t, Qt::MatchExactly ) < 0 )
                    m_make->addItem( t );
           }
       }

       // Fill models for current selected maker
       if ( (*it)->Model && (*it)->Maker == m_make->combo()->currentText() )
       {
            LensFunCameraSelector::DevicePtr dev;
            dev        = *it;
            QVariant b = qVariantFromValue(dev);
            m_model->combo()->addItem( (*it)->Model, b );
       }

       ++it;
    }
    m_make->combo()->model()->sort(0, Qt::AscendingOrder);
    m_model->combo()->model()->sort(0, Qt::AscendingOrder);

    // Fill Lens list for current Maker & Model
    slotUpdateLensCombo();
}

void LensFunCameraSelector::slotUpdateLensCombo()
{
    m_lens->combo()->clear();

    QVariant v    = m_model->combo()->itemData( m_model->currentIndex() );
    DevicePtr dev = v.value<LensFunCameraSelector::DevicePtr>();
    if (!dev)
    {
        kDebug() << "Device is null!";
        return;
    }

    const lfLens** lenses = m_iface->m_lfDb->FindLenses( dev, NULL, NULL );
    m_iface->m_cropFactor = dev->CropFactor;

    while (lenses && *lenses)
    {
        LensFunCameraSelector::LensPtr lens = *lenses;
        QVariant b                          = qVariantFromValue(lens);
        m_lens->combo()->addItem((*lenses)->Model, b);
        ++lenses;
    }
    m_lens->combo()->model()->sort(0, Qt::AscendingOrder);

    emit(signalLensSettingsChanged());
}

void LensFunCameraSelector::slotLensSelected()
{
    QVariant v          = m_lens->combo()->itemData( m_lens->currentIndex() );
    m_iface->m_usedLens = v.value<LensFunCameraSelector::LensPtr>();

    if ( m_iface->m_cropFactor <= 0.0 ) // this should not happen
        m_iface->m_cropFactor = m_iface->m_usedLens->CropFactor;

    emit(signalLensSettingsChanged());
}

void LensFunCameraSelector::setDevice(Device& /*d*/)
{
    slotUpdateCombos();
}

}  // namespace Digikam
