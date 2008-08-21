/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 * 
 * Date        : 2006-06-23
 * Description : a tab widget to display ICC profile infos
 * 
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <qmap.h>

// KDE includes.

#include <kdialogbase.h>
#include <klocale.h>
#include <kapplication.h>

// Lcms includes.

#include LCMS_HEADER
#if LCMS_VERSION < 114
#define cmsTakeCopyright(profile) "Unknown"
#endif // LCMS_VERSION < 114

// Local includes.

#include "ddebug.h"
#include "metadatalistview.h"
#include "cietonguewidget.h"
#include "iccprofilewidget.h"
#include "iccprofilewidget.moc"

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

// This entry list is only require for compatibility with MetadataWidget implementation.
static const char* ICCEntryList[] =
{
     "Header",
     "-1"
};

class ICCTagInfo
{

public:

    ICCTagInfo(){}
        
    ICCTagInfo(const QString& title, const QString& description)
        : m_title(title), m_description(description){}

    QString title()       const { return m_title;       }          
    QString description() const { return m_description; }          
    
private:
    
    QString m_title;    
    QString m_description; 
};

typedef QMap<QString, ICCTagInfo> ICCTagInfoMap;    

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
    
    ICCTagInfoMap    iccTagsDescription;
};    


ICCProfileWidget::ICCProfileWidget(QWidget* parent, const char* name, int w, int h)
                : MetadataWidget(parent, name)
{
    cmsErrorAction(LCMS_ERROR_SHOW);    
    
    d = new ICCProfileWidgetPriv;

    // Set the translated ICC tags titles/descriptions list 
    d->iccTagsDescription["Icc.Header.Name"]            = ICCTagInfo(i18n("Name"),         
                                                          i18n("The ICC profile product name"));
    d->iccTagsDescription["Icc.Header.Description"]     = ICCTagInfo(i18n("Description"),  
                                                          i18n("The ICC profile product description"));
    d->iccTagsDescription["Icc.Header.Information"]     = ICCTagInfo(i18n("Information"),  
                                                          i18n("Additional ICC profile information"));
    d->iccTagsDescription["Icc.Header.Manufacturer"]    = ICCTagInfo(i18n("Manufacturer"), 
                                                          i18n("Raw information about the ICC profile manufacturer"));
    d->iccTagsDescription["Icc.Header.Model"]           = ICCTagInfo(i18n("Model"), 
                                                          i18n("Raw information about the ICC profile model"));
    d->iccTagsDescription["Icc.Header.Copyright"]       = ICCTagInfo(i18n("Copyright"), 
                                                          i18n("Raw information about the ICC profile copyright"));
    d->iccTagsDescription["Icc.Header.ProfileID"]       = ICCTagInfo(i18n("Profile ID"), 
                                                          i18n("The ICC profile ID number"));
    d->iccTagsDescription["Icc.Header.ColorSpace"]      = ICCTagInfo(i18n("Color Space"), 
                                                          i18n("The color space used by the ICC profile"));
    d->iccTagsDescription["Icc.Header.ConnectionSpace"] = ICCTagInfo(i18n("Connection Space"), 
                                                          i18n("The connection space used by the ICC profile"));
    d->iccTagsDescription["Icc.Header.DeviceClass"]     = ICCTagInfo(i18n("Device Class"), 
                                                          i18n("The ICC profile device class"));
    d->iccTagsDescription["Icc.Header.RenderingIntent"] = ICCTagInfo(i18n("Rendering Intent"), 
                                                          i18n("The ICC profile rendering intent"));
    d->iccTagsDescription["Icc.Header.ProfileVersion"]  = ICCTagInfo(i18n("Profile Version"), 
                                                          i18n("The ICC version used to record the profile"));
    d->iccTagsDescription["Icc.Header.CMMFlags"]        = ICCTagInfo(i18n("CMM Flags"), 
                                                          i18n("The ICC profile color management flags"));

    // Set the list of keys and tags filters.
    for (int i=0 ; QString(ICCEntryList[i]) != QString("-1") ; i++)
        d->keysFilter << ICCEntryList[i];
    
    for (int i=0 ; QString(ICCHumanList[i]) != QString("-1") ; i++)
        d->tagsfilter << ICCHumanList[i];

    // Add CIE tongue graph to the widget area
        
    d->cieTongue = new CIETongueWidget(w, h, this);
    QWhatsThis::add( d->cieTongue, i18n("<p>This area contains a CIE or chromaticity diagram. "
                    "A CIE diagram is a representation of all the colors "
                    "that a person with normal vision can see. This is represented "
                    "by the colored sail-shaped area. In addition you will see a "
                    "triangle that is superimposed on the diagram outlined in white. "
                    "This triangle represents the outer boundaries of the color space "
                    "of the device that is characterized by the inspected profile. "
                    "This is called the device gamut.<p>"
                    "In addition there are black dots and yellow lines on the diagram. "
                    "Each black dot represents one of the measurement points that were "
                    "used to create this profile. The yellow line represents the "
                    "amount that each point is corrected by the profile, and the "
                    "direction of this correction."));
                    
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

void ICCProfileWidget::setLoadingFailed()
{
    d->cieTongue->loadingFailed();
}

QString ICCProfileWidget::getMetadataTitle()
{
    return i18n("ICC Color Profile Information");
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
            d->cieTongue->setProfileData(iccData);
        }        
    }

    return true;
}

bool ICCProfileWidget::decodeMetadata()
{
    QByteArray iccData = getMetadata();
    if (iccData.isNull())
        return false;

    d->cieTongue->setProfileData(iccData);
    
    cmsHPROFILE hProfile = cmsOpenProfileFromMem(iccData.data(), (DWORD)iccData.size());

    if (!hProfile)
    {
        DDebug() << "Cannot parse ICC profile tags using LCMS" << endl;
        return false;
    }

    DMetadata::MetaDataMap metaDataMap;

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

    metaDataMap.insert("Icc.Header.ProfileID",      QString::number((uint)*cmsTakeProfileID(hProfile)));
    metaDataMap.insert("Icc.Header.ProfileVersion", QString::number((uint)cmsGetProfileICCversion(hProfile)));
    metaDataMap.insert("Icc.Header.CMMFlags",       QString::number((uint)cmsTakeHeaderFlags(hProfile)));
    
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
   
    cmsCloseProfile(hProfile);    
    
    // Update all metadata contents.
    setMetadataMap(metaDataMap);
    return true;
}

void ICCProfileWidget::buildView()
{    
    if (getMode() == SIMPLE)
    {
        setIfdList(getMetadataMap(), d->keysFilter, d->tagsfilter);
    }
    else
    {
        setIfdList(getMetadataMap(), d->keysFilter, QStringList());
    }

    MetadataWidget::buildView();
}

QString ICCProfileWidget::getTagTitle(const QString& key)
{
    ICCTagInfoMap::Iterator it = d->iccTagsDescription.find(key);
    if (it != d->iccTagsDescription.end())
        return(it.data().title());
    
    return key.section('.', 2, 2);
}

void ICCProfileWidget::slotSaveMetadataToFile()
{
    KURL url = saveMetadataToFile(i18n("ICC color profile File to Save"), 
                                  QString("*.icc *.icm|"+i18n("ICC Files (*.icc; *.icm)")));
    storeMetadataToFile(url);
}

QString ICCProfileWidget::getTagDescription(const QString& key)
{
    ICCTagInfoMap::Iterator it = d->iccTagsDescription.find(key);
    if (it != d->iccTagsDescription.end())
        return(it.data().description());
    
    return key.section('.', 2, 2);
}

}  // namespace Digikam
