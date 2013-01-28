/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-06-23
 * Description : a tab widget to display ICC profile infos
 *
 * Copyright (C) 2006-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "iccprofilewidget.moc"

// Qt includes

#include <QComboBox>
#include <QFile>
#include <QGroupBox>
#include <QLabel>
#include <QMap>
#include <QPushButton>

// KDE includes

#include <kapplication.h>
#include <kdialog.h>
#include <khbox.h>
#include <klocale.h>
#include <kdebug.h>

// Local includes

#include "cietonguewidget.h"
#include "metadatalistview.h"

namespace
{
static const char* ICCHumanList[] =
{
    "Icc.Header.ColorSpace",
    "Icc.Header.Copyright",
    "Icc.Header.DeviceClass",
    "Icc.Header.Name",
    "Icc.Header.Description",
    "Icc.Header.RenderingIntent",
    "-1"
};

// This entry list is only require for compatibility with MetadataWidget implementation.
static const char* ICCEntryList[] =
{
    "Header",
    "-1"
};
}

namespace Digikam
{

class ICCTagInfo
{

public:

    ICCTagInfo()
    {
    }

    ICCTagInfo(const QString& title, const QString& description)
        : m_title(title), m_description(description)
    {
    }

    QString title()       const
    {
        return m_title;
    }

    QString description() const
    {
        return m_description;
    }

private:

    QString m_title;
    QString m_description;
};

typedef QMap<QString, ICCTagInfo> ICCTagInfoMap;

// ---------------------------------------------------------------------------------------

class ICCProfileWidget::Private
{

public:

    Private()
    {
        cieTongue = 0;
    }

    IccProfile       profile;

    QStringList      keysFilter;

    CIETongueWidget* cieTongue;

    ICCTagInfoMap    iccTagsDescription;
};

ICCProfileWidget::ICCProfileWidget(QWidget* const parent, int w, int h)
    : MetadataWidget(parent), d(new Private)
{
    dkCmsErrorAction(LCMS_ERROR_SHOW);

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
    for (int i=0 ; QString(ICCEntryList[i]) != QString("-1") ; ++i)
    {
        d->keysFilter << ICCEntryList[i];
    }

    QStringList tagsFilter;

    for (int i=0 ; QString(ICCHumanList[i]) != QString("-1") ; ++i)
    {
        tagsFilter << ICCHumanList[i];
    }

    setTagsFilter(tagsFilter);

    // Add CIE tongue graph to the widget area

    d->cieTongue = new CIETongueWidget(w, h, this);
    d->cieTongue->setWhatsThis( i18n("<p>This area contains a CIE or chromaticity diagram. "
                                     "A CIE diagram is a representation of all the colors "
                                     "that a person with normal vision can see. This is represented "
                                     "by the colored sail-shaped area. In addition you will see a "
                                     "triangle that is superimposed on the diagram outlined in white. "
                                     "This triangle represents the outer boundaries of the color space "
                                     "of the device that is characterized by the inspected profile. "
                                     "This is called the device gamut.</p>"
                                     "<p>In addition there are black dots and yellow lines on the diagram. "
                                     "Each black dot represents one of the measurement points that were "
                                     "used to create this profile. The yellow line represents the "
                                     "amount that each point is corrected by the profile, and the "
                                     "direction of this correction.</p>"));

    setUserAreaWidget(d->cieTongue);
    decodeMetadata();
}

ICCProfileWidget::~ICCProfileWidget()
{
    delete d;
}

bool ICCProfileWidget::setProfile(const IccProfile& profile)
{
    // Cleanup all metadata contents.
    setMetadataMap();

    d->profile = profile;

    if (!d->profile.open())
    {
        setMetadataEmpty();
        d->cieTongue->setProfileData();
        d->profile = IccProfile();
        return false;
    }

    // Try to decode current metadata.
    enabledToolButtons(decodeMetadata());

    // Refresh view using decoded metadata.
    buildView();
    return true;
}

IccProfile ICCProfileWidget::getProfile() const
{
    return d->profile;
}

void ICCProfileWidget::setDataLoading()
{
    d->cieTongue->loadingStarted();
}

void ICCProfileWidget::setLoadingFailed()
{
    d->cieTongue->loadingFailed();
}

void ICCProfileWidget::setUncalibratedColor()
{
    d->cieTongue->uncalibratedColor();
}

QString ICCProfileWidget::getMetadataTitle()
{
    return i18n("ICC Color Profile Information");
}

bool ICCProfileWidget::loadFromURL(const KUrl& url)
{
    setFileName(url.toLocalFile());

    if (url.isEmpty())
    {
        setProfile();
        d->cieTongue->setProfileData();
        return false;
    }
    else
    {
        IccProfile profile(url.toLocalFile());

        if (!setProfile(profile))
        {
            setProfile();
            d->cieTongue->setProfileData();
            return false;
        }
    }

    return true;
}

bool ICCProfileWidget::loadFromProfileData(const QString& fileName, const QByteArray& data)
{
    setFileName(fileName);
    return(setProfile(data));
}

bool ICCProfileWidget::loadProfile(const QString& fileName, const IccProfile& profile)
{
    setFileName(fileName);
    return(setProfile(profile));
}

bool ICCProfileWidget::decodeMetadata()
{
    if (!d->profile.isOpen())
    {
        return false;
    }

    d->cieTongue->setProfileData(d->profile.data());

    LcmsLock lock;
    cmsHPROFILE hProfile = d->profile.handle();

    if (!hProfile)
    {
        kDebug() << "Cannot parse ICC profile tags using LCMS";
        return false;
    }

    DMetadata::MetaDataMap metaDataMap;

    if ( !QString(dkCmsTakeProductName(hProfile)).isEmpty() )
    {
        metaDataMap.insert("Icc.Header.Name", QString(dkCmsTakeProductName(hProfile)).replace('\n', ' '));
    }

    if ( !QString(dkCmsTakeProductDesc(hProfile)).isEmpty() )
    {
        metaDataMap.insert("Icc.Header.Description", QString(dkCmsTakeProductDesc(hProfile)).replace('\n', ' '));
    }

    if ( !QString(dkCmsTakeProductInfo(hProfile)).isEmpty() )
    {
        metaDataMap.insert("Icc.Header.Information", QString(dkCmsTakeProductInfo(hProfile)).replace('\n', ' '));
    }

    if ( !QString(dkCmsTakeManufacturer(hProfile)).isEmpty() )
    {
        metaDataMap.insert("Icc.Header.Manufacturer", QString(dkCmsTakeManufacturer(hProfile)).replace('\n', ' '));
    }

    if ( !QString(dkCmsTakeModel(hProfile)).isEmpty() )
    {
        metaDataMap.insert("Icc.Header.Model", QString(dkCmsTakeModel(hProfile)).replace('\n', ' '));
    }

    if ( !QString(dkCmsTakeCopyright(hProfile)).isEmpty() )
    {
        metaDataMap.insert("Icc.Header.Copyright", QString(dkCmsTakeCopyright(hProfile)).replace('\n', ' '));
    }

    metaDataMap.insert("Icc.Header.ProfileID",      QString::number((uint)*dkCmsTakeProfileID(hProfile)));
    metaDataMap.insert("Icc.Header.ProfileVersion", QString::number((uint)dkCmsGetProfileICCversion(hProfile)));
    metaDataMap.insert("Icc.Header.CMMFlags",       QString::number((uint)dkCmsTakeHeaderFlags(hProfile)));

    QString colorSpace;

    switch (dkCmsGetColorSpace(hProfile))
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

    switch (dkCmsGetPCS(hProfile))
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

    switch ((int)dkCmsGetDeviceClass(hProfile))
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

    switch (dkCmsTakeRenderingIntent(hProfile))
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

    // Update all metadata contents.
    setMetadataMap(metaDataMap);
    return true;
}

void ICCProfileWidget::buildView()
{
    if (getMode() == CUSTOM)
    {
        setIfdList(getMetadataMap(), d->keysFilter, getTagsFilter());
    }
    else
    {
        setIfdList(getMetadataMap(), d->keysFilter, QStringList() << QString("FULL"));
    }

    MetadataWidget::buildView();
}

QString ICCProfileWidget::getTagTitle(const QString& key)
{
    ICCTagInfoMap::const_iterator it = d->iccTagsDescription.constFind(key);

    if (it != d->iccTagsDescription.constEnd())
    {
        return(it.value().title());
    }

    return key.section('.', 2, 2);
}

void ICCProfileWidget::slotSaveMetadataToFile()
{
    KUrl url = saveMetadataToFile(i18n("ICC color profile File to Save"),
                                  QString("*.icc *.icm|"+i18n("ICC Files (*.icc; *.icm)")));
    storeMetadataToFile(url, d->profile.data());
}

QString ICCProfileWidget::getTagDescription(const QString& key)
{
    ICCTagInfoMap::const_iterator it = d->iccTagsDescription.constFind(key);

    if (it != d->iccTagsDescription.constEnd())
    {
        return(it.value().description());
    }

    return key.section('.', 2, 2);
}

}  // namespace Digikam
