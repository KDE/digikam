/* ============================================================
 *
 * Date        : 2008-02-10
 * Description : a plugin to fix lens errors
 *
 * Copyright (C) 2008 Adrian Schroeter
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

#include <lensfun.h>
#include <libkexiv2/kexiv2.h>

#include "klensfun_export.h"


class KLENSFUN_EXPORT KLensFun
{
    friend class KLFDeviceSelector;
    friend class KLensFunFilter;

public:
    KLensFun();
    virtual ~KLensFun();

//    typedef QMap<QString, QString> correctionData;
    void setCorrection( bool CCA, bool Vignettation, bool CCI, bool Distortion, bool Geometry );
//    correctionData getCorrectionData();

    bool supportsDistortion();
    bool supportsGeometry(){return supportsDistortion();};
    bool supportsCCA();
    bool supportsVig();
    bool supportsCCI(){return supportsVig();};

protected:
    bool init();

private:
    // my configuration
    bool m_init;
    bool m_filterCCA;
    bool m_filterVig;
    bool m_filterCCI;
    bool m_filterDist;
    bool m_filterGeom;

    // Database items
    lfDatabase *m_lfDb;
    const lfCamera * const *m_lfCameras;
    const lfLens **m_lfLenses;
    const lfMount *m_lfMounts;

    // To be used for modification
    const lfLens *m_usedLens;
    float m_cropFactor;
    float m_focalLength;
    float m_aperature;
    float m_subjectDistance;
};

#include "dimgthreadedfilter.h"

class KLensFunFilter : public Digikam::DImgThreadedFilter
{

public:
    KLensFunFilter(Digikam::DImg *origImage, QObject *parent,
                   KLensFun *);

    ~KLensFunFilter(){};

private:
    virtual void filterImage(void);

private:
    KLensFun *m_klf;
    lfModifier *m_lfModifier;
    QObject *m_parent;
};


#include <QWidget>

class QCheckBox;
class KComboBox;

class KLENSFUN_EXPORT KLFDeviceSelector : public QWidget
{

    Q_OBJECT

public:
    typedef QMap<QString,QString> Device;
    typedef const lfCamera* DevicePtr;
    typedef const lfLens* LensPtr;

    KLFDeviceSelector( QWidget *parent );
    virtual ~KLFDeviceSelector();

//    Device getDevice();
    void setDevice(Device&);

    KLensFun *getKLFObject(){ return m_klf; };

public slots:
    void findFromExif( KExiv2Iface::KExiv2& );

protected slots:
    void updateCombos();
    void updateLensCombo();
    void exifUsageSlot(int);
    void selectLens();

protected:
    void findFromExif();

signals :
    void lensSelected();

private:
    KLensFun *m_klf;
    KExiv2Iface::KExiv2 m_ExivMeta;

    QCheckBox *m_ExifUsage;
    KComboBox *m_Maker;
    KComboBox *m_Model;
    KComboBox *m_Lens;
};

