/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-06-23
 * Description : a tab widget to display ICC profile infos
 * 
 * Copyright 2006 by Gilles Caulier
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

#include <config.h>
 
// Qt includes.

#include <qlayout.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>
#include <qlabel.h>
#include <qmap.h>
#include <qhbox.h>
#include <qfile.h>
#include <qcombobox.h>
#include <qgroupbox.h>

// KDE includes.

#include <kdebug.h>
#include <kdialogbase.h>
#include <klocale.h>
#include <kapplication.h>

// Lcms includes.

#include LCMS_HEADER
#if LCMS_VERSION < 114
#define cmsTakeCopyright(profile) "Unknown"
#endif // LCMS_VERSION < 114

// Local includes.

#include "metadatalistview.h"
#include "cietonguewidget.h"
#include "iccprofilewidget.h"

namespace Digikam
{

static const char* ICCHumanList[] =
{
     "ColorSpace",
     "Copyright",
     "DeviceClass",
     "Name",
     "Description",
     "RenderingIntent",
     "-1"
};

static const char* ICCEntryList[] =
{
     "Header",
     "-1"
};

class ICCProfileWidgetPriv
{

public:

    ICCProfileWidgetPriv()
    {
        cieTongue = 0;
    }
    
    QStringList      tagsfilter;
    QStringList      keysFilter;
    
    CIETongueWidget *cieTongue;
};

ICCProfileWidget::ICCProfileWidget(QWidget* parent, const char* name)
                : MetadataWidget(parent, name)
{
    d = new ICCProfileWidgetPriv;
    
    for (int i=0 ; QString(ICCEntryList[i]) != QString("-1") ; i++)
        d->keysFilter << ICCEntryList[i];
    
    for (int i=0 ; QString(ICCHumanList[i]) != QString("-1") ; i++)
        d->tagsfilter << ICCHumanList[i];

    cmsErrorAction(LCMS_ERROR_SHOW);    

    d->cieTongue = new CIETongueWidget(256, 256, this);
    QWhatsThis::add( d->cieTongue, i18n("<p>This area contains a CIE or chromaticity diagram. "
                    "A CIE diagram is a representation of all of the colors "
                    "that a person with normal vision can see. This is represented "
                    "by the colored sail shaped area. In addition you will see a "
                    "triangle that is superimposed on the diagram outlined in white. "
                    "This triangle represents that outer boundries of the color space "
                    "of the device that is characterized by the profile being inspected. "
                    "This is called the device gamut.<p>"
                    "In addition there are black dots and yellow lines on the diagram. "
                    "Each black dot represents one of the measurement points that was "
                    "used to create this profile. The yellow line represents the "
                    "amount that each point is corrected by the profile and the "
                    "direction of the correction."));
                    
    setUserAreaWidget(d->cieTongue);
    decodeMetadata();
}

ICCProfileWidget::~ICCProfileWidget()
{
    delete d;
}

void ICCProfileWidget::setDataLoading()
{
    d->cieTongue->loadingStarted();
}

void ICCProfileWidget::setLoadingComplete(bool b)
{
    d->cieTongue->loadingComplete(b);
}

QString ICCProfileWidget::getMetadataTitle(void)
{
    return i18n("ICC Color Profile Informations");
}

bool ICCProfileWidget::loadFromURL(const KURL& url)
{
    setFileName(url.path());
    
    if (url.isEmpty())
    {
        setMetadata();
        d->cieTongue->setProfileData();
        return false;
    }
    else
    {    
        
        QFile file(url.path());
        if ( !file.open(IO_ReadOnly) ) 
        {
            setMetadata();
            d->cieTongue->setProfileData();
            return false;
        }
            
        QByteArray iccData(file.size());
        QDataStream stream( &file );
        stream.readRawBytes(iccData.data(), iccData.size());
        file.close();
        
        if (iccData.isEmpty())
        {
            setMetadata();
            d->cieTongue->setProfileData();
            return false;
        }
        else
        {
            setMetadata(iccData);
            d->cieTongue->setProfileData(&iccData);
        }        
    }

    return true;
}

bool ICCProfileWidget::decodeMetadata()
{
    QByteArray iccData = getMetadata();
    if (iccData.isNull())
        return false;
        
    cmsHPROFILE hProfile = cmsOpenProfileFromMem(iccData.data(), (DWORD)iccData.size());

    if (!hProfile)
    {
        kdDebug() << "Cannot parse ICC profile tags using LCMS" << endl;
        return false;
    }

    MetaDataMap metaDataMap;

    if ( !QString(cmsTakeProductName(hProfile)).isEmpty() )
        metaDataMap.insert("Icc.Header.Name", QString(cmsTakeProductName(hProfile)).replace("\n", " "));

    if ( !QString(cmsTakeProductDesc(hProfile)).isEmpty() )
        metaDataMap.insert("Icc.Header.Description", QString(cmsTakeProductDesc(hProfile)).replace("\n", " "));

    if ( !QString(cmsTakeProductInfo(hProfile)).isEmpty() )
        metaDataMap.insert("Icc.Header.Information", QString(cmsTakeProductInfo(hProfile)).replace("\n", " "));

    if ( !QString(cmsTakeManufacturer(hProfile)).isEmpty() )
        metaDataMap.insert("Icc.Header.Manufacturer", QString(cmsTakeManufacturer(hProfile)).replace("\n", " "));

    if ( !QString(cmsTakeModel(hProfile)).isEmpty() )
        metaDataMap.insert("Icc.Header.Model", QString(cmsTakeModel(hProfile)).replace("\n", " "));

    if ( !QString(cmsTakeCopyright(hProfile)).isEmpty() )
        metaDataMap.insert("Icc.Header.Copyright", QString(cmsTakeCopyright(hProfile)).replace("\n", " "));

    metaDataMap.insert("Icc.Header.ProfileID", QString::number((uint)*cmsTakeProfileID(hProfile)));
    
    QString colorSpace;        
    switch (cmsGetColorSpace(hProfile))
    {
        case icSigLabData:
            colorSpace = i18n("Lab");
            break;
        case icSigLuvData:
            colorSpace = i18n("Luv");
            break;
        case icSigRgbData:
            colorSpace = i18n("RGB");
            break;
        case icSigGrayData:
            colorSpace = i18n("GRAY");
            break;
        case icSigHsvData:
            colorSpace = i18n("HSV");
            break;
        case icSigHlsData:
            colorSpace = i18n("HLS");
            break;
        case icSigCmykData:
            colorSpace = i18n("CMYK");
            break;
        case icSigCmyData:
            colorSpace= i18n("CMY");
            break;
        default:
            colorSpace = i18n("Unknown");
            break;
    }
    metaDataMap.insert("Icc.Header.ColorSpace", colorSpace);

    QString connectionSpace;        
    switch (cmsGetPCS(hProfile))
    {
        case icSigLabData:
            connectionSpace = i18n("Lab");
            break;
        case icSigLuvData:
            connectionSpace = i18n("Luv");
            break;
        case icSigRgbData:
            connectionSpace = i18n("RGB");
            break;
        case icSigGrayData:
            connectionSpace = i18n("GRAY");
            break;
        case icSigHsvData:
            connectionSpace = i18n("HSV");
            break;
        case icSigHlsData:
            connectionSpace = i18n("HLS");
            break;
        case icSigCmykData:
            connectionSpace = i18n("CMYK");
            break;
        case icSigCmyData:
            connectionSpace= i18n("CMY");
            break;
        default:
            connectionSpace = i18n("Unknown");
            break;
    }
    metaDataMap.insert("Icc.Header.ConnectionSpace", connectionSpace);
    
    QString device;        
    switch ((int)cmsGetDeviceClass(hProfile))
    {
        case icSigInputClass:
            device = i18n("Input device");
            break;
        case icSigDisplayClass:
            device = i18n("Display device");
            break;
        case icSigOutputClass:
            device = i18n("Output device");
            break;
        case icSigColorSpaceClass:
            device = i18n("Color space");
            break;
        case icSigLinkClass:
            device = i18n("Link device");
            break;
        case icSigAbstractClass:
            device = i18n("Abstract");
            break;
        case icSigNamedColorClass:
            device = i18n("Named color");
            break;
        default:
            device = i18n("Unknown");
            break;
    }
    metaDataMap.insert("Icc.Header.DeviceClass", device);

    QString intent;        
    switch (cmsTakeRenderingIntent(hProfile))
    {
        case 0:
            intent = i18n("Perceptual");
            break;
        case 1:
            intent = i18n("Relative Colorimetric");
            break;
        case 2:
            intent = i18n("Saturation");
            break;
        case 3:
            intent = i18n("Absolute Colorimetric");
            break;
        default:
            intent = i18n("Unknown");
            break;
    }    
    metaDataMap.insert("Icc.Header.RenderingIntent", intent);
    
    d->cieTongue->setProfileHandler(hProfile);
    
    cmsCloseProfile(hProfile);    
    
    // Update all metadata contents.
    setMetadataMap(metaDataMap);
    return true;
}

void ICCProfileWidget::buildView(void)
{    
    if (getMode() == SIMPLE)
    {
        setIfdList(getMetadataMap(), d->keysFilter, d->tagsfilter);
    }
    else
    {
        setIfdList(getMetadataMap(), d->keysFilter, QStringList());
    }
}

QString ICCProfileWidget::getTagTitle(const QString& key)
{
    return key.section('.', 2, 2);
}

QString ICCProfileWidget::getTagDescription(const QString& /*key*/)
{
    // TODO
    return QString();
}

}  // namespace Digikam

#include "iccprofilewidget.moc"
