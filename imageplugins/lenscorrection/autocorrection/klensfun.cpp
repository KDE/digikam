/* ============================================================
 *
 * Date        : 2008-02-10
 * Description : a plugin to fix automaticaly camera lens aberrations
 *
 * Copyright (C) 2008 by Adrian Schroeter <adrian at suse dot de>
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <QByteArray>
#include <QGridLayout>
#include <QString>
#include <QCheckBox>
#include <QWidget>
#include <QLabel>

// KDE includes.

#include <kdialog.h>
#include <klocale.h>
#include <kcombobox.h>
#include <knuminput.h>

// Local includes.

#include "ddebug.h"
#include "klensfun.h"
#include "klensfun.moc"

Q_DECLARE_METATYPE( DigikamAutoCorrectionImagesPlugin::KLFDeviceSelector::DevicePtr );
Q_DECLARE_METATYPE( DigikamAutoCorrectionImagesPlugin::KLFDeviceSelector::LensPtr );

namespace DigikamAutoCorrectionImagesPlugin
{

KLensFun::KLensFun()
{
    m_init = false;
    init();
}

KLensFun::~KLensFun()
{
    if ( m_init )
    {
    }
}

bool KLensFun::init()
{
    m_lfDb = lf_db_new();
    m_lfDb->Load();
    m_lfCameras  = m_lfDb->GetCameras();
    m_init       = true;
    m_usedLens   = NULL;
    m_filterCCA  = true;
    m_filterVig  = true;
    m_filterDist = true;

    return true;
}

void KLensFun::setCorrection(bool CCA, bool Vig, bool CCI, bool Dist, bool Geom)
{
    m_filterCCA  = CCA;
    m_filterVig  = Vig;
    m_filterCCI  = CCI;
    m_filterDist = Dist;
    m_filterGeom = Geom;
}

bool KLensFun::supportsDistortion()
{
    if (m_usedLens == NULL) return false;

    lfLensCalibDistortion res;
    return m_usedLens->InterpolateDistortion(m_focalLength, res);
}

bool KLensFun::supportsCCA()
{
    if (m_usedLens == NULL) return false;

    lfLensCalibTCA res;
    return m_usedLens->InterpolateTCA(m_focalLength, res);
}

bool KLensFun::supportsVig()
{
    if (m_usedLens == NULL) return false;

    lfLensCalibVignetting res;
    return m_usedLens->InterpolateVignetting(m_focalLength, m_aperture, m_subjectDistance, res);
}

#if 0
KLensFun::correctionData KLensFun::getCorrectionData()
{
}
#endif

// -------------------------------------------------------------------

KLFDeviceSelector::KLFDeviceSelector(QWidget *parent) 
                 : QWidget(parent)
{
    m_klf              = new KLensFun();
    QGridLayout* grid  = new QGridLayout(this);
    m_exifUsage        = new QCheckBox(i18n("Use Exif Data"), this);
    m_make             = new KComboBox(this);
    m_model            = new KComboBox(this);
    m_lens             = new KComboBox(this);
    QLabel *makeLabel  = new QLabel(i18n("Make:"), this);
    QLabel *modelLabel = new QLabel(i18n("Model:"), this);
    QLabel *lensLabel  = new QLabel(i18n("Lens:"), this);

    m_exifUsage->setEnabled(false);
    m_exifUsage->setCheckState(Qt::Unchecked);
    m_make->setInsertPolicy(QComboBox::InsertAlphabetically);
    m_model->setInsertPolicy(QComboBox::InsertAlphabetically);
    m_lens->setInsertPolicy(QComboBox::InsertAlphabetically);

    QLabel *focalLabel = new QLabel(i18n("Focal Length:"), this);
    QLabel *aperLabel  = new QLabel(i18n("Aperture:"), this);
    QLabel *distLabel  = new QLabel(i18n("Subject Distance:"), this);

    m_focal = new KDoubleNumInput(this);
    m_focal->setDecimals(1);
    m_focal->setRange(1.0, 1000.0, 0.01, true);

    m_aperture = new KDoubleNumInput(this);
    m_aperture->setDecimals(1);
    m_aperture->setRange(1.1, 64.0, 0.1, true);

    m_distance = new KDoubleNumInput(this);
    m_distance->setDecimals(1);
    m_distance->setRange(0.0, 100.0, 0.1, true);

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

    connect(m_exifUsage, SIGNAL(stateChanged(int)), 
            this, SLOT(slotUseExif(int)));

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

    KLFDeviceSelector::Device firstDevice; // empty strings
//    setDevice( firstDevice );
}

KLFDeviceSelector::~KLFDeviceSelector()
{
    delete m_klf;
}

#if 0
KLFDeviceSelector::Device KLFDeviceSelector::getDevice()
{
}
#endif

void KLFDeviceSelector::findFromMetadata(const Digikam::DMetadata& meta)
{
    m_metadata = meta;
    findFromMetadata();
}

void KLFDeviceSelector::findFromMetadata()
{
//    KLFDeviceSelector::Device firstDevice; // empty strings
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

    Digikam::PhotoInfoContainer photoInfo = m_metadata.getPhotographInformations();

    QString make  = photoInfo.make;
    QString model = photoInfo.model;

    // ------------------------------------------------------------------------------------------------
    // Try to get Lens Data informations from makernote.
    // NOTE: Marcel, it's a non-standarized way, maybe such a thing should go to DMetadata instead
    // to share code with digiKam database interface to host Lens informations.

    // Canon Cameras.
    QString lens = m_metadata.getExifTagString("Exif.Canon.0x0095");
    if (lens.isEmpty())
    {
        // Nikon Cameras.
        lens = m_metadata.getExifTagString("Exif.Nikon3.LensData");
        if (lens.isEmpty())
        {
            // Minolta Cameras.
            lens = m_metadata.getExifTagString("Exif.Minolta.LensID");
            if (lens.isEmpty())
            {
                // Pentax Cameras.
                lens = m_metadata.getExifTagString("Exif.Pentax.LensType");
                if (lens.isEmpty())
                {
                    // Panasonic Cameras.
                    lens = m_metadata.getExifTagString("Exif.Panasonic.0x0310");
                    if (lens.isEmpty())
                    {
                        // Sigma Cameras.
                        lens = m_metadata.getExifTagString("Exif.Sigma.LensRange");
                        if (lens.isEmpty())
                        {
                            // TODO : add Fuji, Olympus, Sony Cameras before XMP parsing.

                            // XMP aux tags.
                            lens = m_metadata.getXmpTagString("Xmp.aux.Lens");
                            if (lens.isEmpty())
                            {
                                // XMP M$ tags (Lens Maker + Lens Model to be compatible with LensFun Database).
                                lens = m_metadata.getXmpTagString("Xmp.MicrosoftPhoto.LensManufacturer");
                                if (!lens.isEmpty())
                                    lens.append(" ");

                                lens.append(m_metadata.getXmpTagString("Xmp.MicrosoftPhoto.LensModel"));
                            }
                        }
                    }
                }
            }
        }
    }

    // ------------------------------------------------------------------------------------------------

    int makerIdx = m_make->findText(make);
    if (makerIdx >= 0) 
    {
        m_make->setCurrentIndex(makerIdx);
        m_make->setEnabled(false);
    }

    slotUpdateCombos();
    int modelIdx = m_model->findText(model);
    if (modelIdx >= 0) 
    {
        m_model->setCurrentIndex(modelIdx);
        m_model->setEnabled(false);
        slotUpdateLensCombo();
    }

    // The LensFun DB has the Maker in front of the Lens model name.
    // We use here the Camera Maker, because the Lens Maker seems not to be
    // part of the Exif data. This is of course bad for 3rd party lenses, but
    // they seem anyway not to have Exif entrys ususally :/
    int lensIdx = m_lens->findText(lens); 
    if (lensIdx < 0)
       lensIdx = m_lens->findText(make + " " + lens); 

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

    DDebug() << "Search for Lens: " << make << " :: " << lens 
             << "< and found: >" << m_lens->itemText(0) + "<";

    QString temp = photoInfo.focalLength;
    if (!temp.isEmpty())
    {
        double focal = temp.mid(0, temp.length() -3).toDouble(); // HACK: strip the " mm" at the end ...
        DDebug() << "Focal Length: " << focal << endl; 
        m_focal->setValue(focal);
        m_focal->setEnabled(false);
    }

    temp = photoInfo.aperture;
    if (!temp.isEmpty())
    {
        double aperture = temp.mid(1).toDouble();
        DDebug() << "Aperture: " << aperture << endl; 
        m_aperture->setValue(aperture);
        m_aperture->setEnabled(false);
    }

    // ------------------------------------------------------------------------------------------------
    // Try to get subject distance value.

    // From standard Exif.
    temp = m_metadata.getExifTagString("Exif.Photo.SubjectDistance");
    if (temp.isEmpty())
    {
        // From standard Xmp.
        temp = m_metadata.getExifTagString("Xmp.exif.SubjectDistance");
        if (temp.isEmpty())
        {
            // From Canon Makernote.
            temp = m_metadata.getExifTagString("Exif.CanonSi.SubjectDistance");

            // TODO: Add here others Makernotes tags.
        }
    }

    if (!temp.isEmpty())
    {
        double distance = temp.toDouble();
        DDebug() << "Subject Distance: " << distance << endl; 
        m_distance->setValue(distance);
        m_distance->setEnabled(false);
    }
}

void KLFDeviceSelector::slotFocalChanged(double f)
{
    m_klf->m_focalLength = f;
    emit signalLensSettingsChanged();
}

void KLFDeviceSelector::slotApertureChanged(double a)
{
    m_klf->m_aperture = a;
    emit signalLensSettingsChanged();
}

void KLFDeviceSelector::slotDistanceChanged(double d)
{
    m_klf->m_subjectDistance = d;
    emit signalLensSettingsChanged();
}

void KLFDeviceSelector::slotUseExif(int mode)
{
    if (mode == Qt::Checked)
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

void KLFDeviceSelector::slotUpdateCombos()
{
    const lfCamera* const *it = m_klf->m_lfCameras;

    // reset box
    m_model->clear();

    bool firstRun = false;
    if ( m_make->count() == 0 )
       firstRun = true;

    while ( *it ) 
    {
       if ( firstRun )
       {
           // Maker DB does not change, so we fill it only once.
           if ( (*it)->Maker ) 
           {
                QString t( (*it)->Maker );
                if ( m_make->findText( t, Qt::MatchExactly ) < 0 )
                    m_make->addItem( t );
           }
       }

       // Fill models for current selected maker
       if ( (*it)->Model && (*it)->Maker == m_make->currentText() ) 
       {
            KLFDeviceSelector::DevicePtr dev;
            dev        = *it;
            QVariant b = qVariantFromValue(dev);
            m_model->addItem( (*it)->Model, b );
       }

       it++;
    }

    // Fill Lens list for current Maker & Model
    slotUpdateLensCombo();
}

void KLFDeviceSelector::slotUpdateLensCombo()
{
    m_lens->clear();

    QVariant v    = m_model->itemData( m_model->currentIndex() );
    DevicePtr dev = v.value<KLFDeviceSelector::DevicePtr>();
    if (!dev)
    {
        DDebug() << "slotUpdateLensCombo() => Device is null!" << endl;
        return;
    }

    const lfLens **lenses = m_klf->m_lfDb->FindLenses( dev, NULL, NULL );
    m_klf->m_cropFactor   = dev->CropFactor;

    while (lenses && *lenses) 
    {
        KLFDeviceSelector::LensPtr lens = *lenses;
        QVariant b                      = qVariantFromValue(lens);
        m_lens->addItem((*lenses)->Model, b);
        lenses++;
    }

    emit(signalLensSettingsChanged());
}

void KLFDeviceSelector::slotLensSelected()
{
    QVariant v        = m_lens->itemData( m_lens->currentIndex() );
    m_klf->m_usedLens = v.value<KLFDeviceSelector::LensPtr>();

    if ( m_klf->m_cropFactor <= 0.0 ) // this should not happend
        m_klf->m_cropFactor = m_klf->m_usedLens->CropFactor;

    emit(signalLensSettingsChanged());
}

void KLFDeviceSelector::setDevice( Device &/*d*/ )
{
    slotUpdateCombos();
}

// -------------------------------------------------------------------

KLensFunFilter::KLensFunFilter(Digikam::DImg *orgImage, QObject *parent, KLensFun *klf)
              : Digikam::DImgThreadedFilter(orgImage, parent, "LensCorrection")
{
    m_klf    = klf;
    m_parent = parent;

    initFilter();
}

void KLensFunFilter::filterImage()
{
#if 0
    if (!opts.Crop)
        opts.Crop = lens->CropFactor;
    if (!opts.Focal)
        opts.Focal = lens->MinFocal;
    if (!opts.Aperture)
        opts.Aperture = lens->MinAperture;
#endif

    int modifyFlags = 0;
    if ( m_klf->m_filterDist )
       modifyFlags |= LF_MODIFY_DISTORTION;
    if ( m_klf->m_filterGeom )
       modifyFlags |= LF_MODIFY_GEOMETRY;
    if ( m_klf->m_filterCCA )
       modifyFlags |= LF_MODIFY_TCA;
    if ( m_klf->m_filterVig )
       modifyFlags |= LF_MODIFY_VIGNETTING;
    if ( m_klf->m_filterCCI )
       modifyFlags |= LF_MODIFY_CCI;

    // Init lensfun lib, we are working on the full image.

    lfPixelFormat colorDepth = m_orgImage.bytesDepth() == 4 ? LF_PF_U8 : LF_PF_U16;

    m_lfModifier = lfModifier::Create(m_klf->m_usedLens, 
                                      m_klf->m_cropFactor,
                                      m_orgImage.width(), 
                                      m_orgImage.height());

    int modflags = m_lfModifier->Initialize(m_klf->m_usedLens, 
                                            colorDepth, 
                                            m_klf->m_focalLength, 
                                            m_klf->m_aperture, 
                                            m_klf->m_subjectDistance,
                                            m_klf->m_cropFactor, 
                                            LF_RECTILINEAR, 
                                            modifyFlags, 
                                            0/*no inverse*/);

    if (!m_lfModifier)
    {
        DError() << "ERROR: cannot initialize LensFun Modifier.";
        return;
    }

    // Calc necessary steps for progress bar

    int steps = m_klf->m_filterCCA                             ? 1 : 0 + 
                ( m_klf->m_filterVig || m_klf->m_filterCCI )   ? 1 : 0 +
                ( m_klf->m_filterDist || m_klf->m_filterGeom ) ? 1 : 0;

    DDebug() << "Modifier Flags: " << modflags << "  Steps:" << steps << endl;

    if ( steps < 1 )
       return;

    // The real correction to do

    int loop   = 0;
    int lwidth = m_orgImage.width() * 2 * 3;
    float *pos = new float[lwidth];

    // Stage 1: TCA correction 

    if ( m_klf->m_filterCCA ) 
    {
        for (unsigned int y=0; !m_cancel && (y < m_orgImage.height()); y++)
        {
            if (m_lfModifier->ApplySubpixelDistortion(0.0, y, m_orgImage.width(), 1, pos))
            {
                float *src = pos;
                for (unsigned x = 0; !m_cancel && (x < m_destImage.width()); x++)
                {
                    Digikam::DColor destPixel;

                    destPixel.setRed  (m_orgImage.getPixelColor(src[0], src[1]).red()   );
                    destPixel.setGreen(m_orgImage.getPixelColor(src[2], src[3]).green() );
                    destPixel.setBlue (m_orgImage.getPixelColor(src[4], src[5]).blue()  );

                    m_destImage.setPixelColor(x, y, destPixel);
                    src += 2 * 3;
                }
                loop++;
            }

            // Update progress bar in dialog.
            int progress = (int)(((double)y * 100.0) / m_orgImage.height());
            if (m_parent && progress%5 == 0)
                postProgress(progress/steps);
        }

        DDebug() << "Applying TCA correction... (loop: " << loop << ")" << endl;
    } 
    else
    {
        m_destImage.bitBltImage(&m_orgImage, 0, 0);
    }

    // Stage 2: Color Correction: Vignetting and CCI

    uchar *data = m_destImage.bits();
    if ( m_klf->m_filterVig || m_klf->m_filterCCI ) 
    {
        loop         = 0;
        float offset = 0.0;

        if ( steps == 3 )
            offset = 33.3;
        else if (steps == 2 && m_klf->m_filterCCA)
            offset = 50.0;

        for (unsigned int y=0; !m_cancel && (y < m_destImage.height()); y++)
        {
            if (m_lfModifier->ApplyColorModification(data, 0.0, y, m_destImage.width(), 
                                                     1, m_destImage.bytesDepth(), 0))
            {
                data += m_destImage.height() * m_destImage.bytesDepth();
                loop++;
            }

            // Update progress bar in dialog.
            int progress = (int)(((double)y * 100.0) / m_destImage.height());
            if (m_parent && progress%5 == 0)
                postProgress(progress/steps + offset);
        }

        DDebug() << "Applying Color Correction: Vignetting and CCI. (loop: " << loop << ")" << endl;
    }

    // Stage 3: Distortion and Geometry

    if ( m_klf->m_filterDist || m_klf->m_filterGeom ) 
    {
        loop = 0;

        // we need a deep copy first 
        Digikam::DImg tempImage(m_destImage.width(), m_destImage.height(), m_destImage.sixteenBit(), m_destImage.hasAlpha());

        for (unsigned long y=0; !m_cancel && (y < tempImage.height()); y++)
        {
            if (m_lfModifier->ApplyGeometryDistortion(0.0, y, tempImage.width(), 1, pos))
            {
                float *src = pos;
                for (unsigned long x = 0; !m_cancel && (x < tempImage.width()); x++, loop++)
                {
                    //qDebug (" ZZ %f %f %i %i", src[0], src[1], (int)src[0], (int)src[1]);

                    tempImage.setPixelColor(x, y, m_destImage.getPixelColor((int)src[0], (int)src[1]));
                    src += 2;
                }
            }

            // Update progress bar in dialog.
            int progress = (int)(((double)y * 100.0) / tempImage.height());
            if (m_parent && progress%5 == 0)
                postProgress(progress/steps + 33.3*(steps-1));
        }

        /*qDebug (" for %f %f %i %i", tempImage.height(), tempImage.width(), 
                                      tempImage.height(), tempImage.width());*/
        DDebug() << "Applying Distortion and Geometry Correction. (loop: " << loop << ")" << endl;

        m_destImage = tempImage;
    }

    // clean up

    delete [] pos;
    m_lfModifier->Destroy();
}

}  // NameSpace DigikamAutoCorrectionImagesPlugin
