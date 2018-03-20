/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-07-17
 * Description : Metadata tags selector config panel.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "metadatapanel.h"

// Qt includes

#include <QFrame>
#include <QVBoxLayout>
#include <QList>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "metaengine.h"
#include "metadataselector.h"
#include "dmetadata.h"

namespace Digikam
{

static const char* ExifHumanList[] =
{
    "Make",
    "Model",
    "DateTime",
    "ImageDescription",
    "Copyright",
    "ShutterSpeedValue",
    "ApertureValue",
    "ExposureProgram",
    "ExposureMode",
    "ExposureBiasValue",
    "ExposureTime",
    "WhiteBalance",
    "ISOSpeedRatings",
    "FocalLength",
    "SubjectDistance",
    "MeteringMode",
    "Contrast",
    "Saturation",
    "Sharpness",
    "LightSource",
    "Flash",
    "FNumber",
    "GPSLatitude",
    "GPSLongitude",
    "GPSAltitude",
    "-1"
};

// This list mix different tags name used by camera makers.
static const char* MakerNoteHumanList[] =
{
    "AFFocusPos",
    "AFMode",
    "AFPoint",
    "AutofocusMode",
    "ColorMode",
    "ColorTemperature",
    "Contrast",
    "DigitalZoom",
    "ExposureMode",
    "ExposureProgram",
    "ExposureCompensation",
    "ExposureManualBias",
    "Flash",
    "FlashBias",
    "FlashMode",
    "FlashType",
    "FlashDevice",
    "FNumber",
    "Focus",
    "FocusDistance",
    "FocusMode",
    "FocusSetting",
    "FocusType",
    "Hue",
    "HueAdjustment",
    "ImageStabilizer",
    "ImageStabilization",
    "InternalFlash",
    "ISOSelection",
    "ISOSpeed",
    "Lens",
    "LensType",
    "LensRange",
    "Macro",
    "MacroFocus",
    "MeteringMode",
    "NoiseReduction",
    "OwnerName",
    "Quality",
    "Tone",
    "ToneComp",
    "Saturation",
    "Sharpness",
    "ShootingMode",
    "ShutterSpeedValue",
    "SpotMode",
    "SubjectDistance",
    "WhiteBalance",
    "WhiteBalanceBias",
    "-1"
};

static const char* IptcHumanList[] =
{
    "Caption",
    "City",
    "Contact",
    "Copyright",
    "Credit",
    "DateCreated",
    "Headline",
    "Keywords",
    "ProvinceState",
    "Source",
    "Urgency",
    "Writer",
    "-1"
};

static const char* XmpHumanList[] =
{
    "Description",
    "City",
    "Relation",
    "Rights",
    "Publisher",
    "CreateDate",
    "Title",
    "Identifier",
    "State",
    "Source",
    "Rating",
    "Advisory",
    "-1"
};

class MetadataPanel::Private
{
public:

    Private()
    {
        tab                = 0;
        exifViewerConfig   = 0;
        mknoteViewerConfig = 0;
        iptcViewerConfig   = 0;
        xmpViewerConfig    = 0;

        setDefaultFilter(ExifHumanList,      defaultExifFilter);
        setDefaultFilter(MakerNoteHumanList, defaultMknoteFilter);
        setDefaultFilter(IptcHumanList,      defaultIptcFilter);
        setDefaultFilter(XmpHumanList,       defaultXmpFilter);
    };

    void setDefaultFilter(const char** const list, QStringList& filter)
    {
        for (int i=0 ; QLatin1String(list[i]) != QLatin1String("-1") ; ++i)
        {
            filter << QLatin1String(list[i]);
        }
    };

public:

    QTabWidget*           tab;

    QStringList           defaultExifFilter;
    QStringList           defaultMknoteFilter;
    QStringList           defaultIptcFilter;
    QStringList           defaultXmpFilter;

    MetadataSelectorView* exifViewerConfig;
    MetadataSelectorView* mknoteViewerConfig;
    MetadataSelectorView* iptcViewerConfig;
    MetadataSelectorView* xmpViewerConfig;
};

MetadataPanel::MetadataPanel(QTabWidget* const tab)
    : QObject(tab),
      d(new Private)
{
    d->tab = tab;

    // --------------------------------------------------------

    d->exifViewerConfig   = new MetadataSelectorView(d->tab);
    d->exifViewerConfig->setDefaultFilter(d->defaultExifFilter);
    d->tab->addTab(d->exifViewerConfig, i18n("EXIF viewer"));

    d->mknoteViewerConfig = new MetadataSelectorView(d->tab);
    d->mknoteViewerConfig->setDefaultFilter(d->defaultMknoteFilter);
    d->tab->addTab(d->mknoteViewerConfig, i18n("Makernotes viewer"));

    d->iptcViewerConfig   = new MetadataSelectorView(d->tab);
    d->iptcViewerConfig->setDefaultFilter(d->defaultIptcFilter);
    d->tab->addTab(d->iptcViewerConfig, i18n("IPTC viewer"));

    d->xmpViewerConfig    = new MetadataSelectorView(d->tab);
    d->xmpViewerConfig->setDefaultFilter(d->defaultXmpFilter);
    d->tab->addTab(d->xmpViewerConfig, i18n("XMP viewer"));

    slotTabChanged(d->tab->currentIndex());

    // --------------------------------------------------------

    connect(d->tab, SIGNAL(currentChanged(int)),
            this, SLOT(slotTabChanged(int)));
}

MetadataPanel::~MetadataPanel()
{
    delete d;
}

QStringList MetadataPanel::defaultExifFilter()
{
    Private d;
    return d.defaultExifFilter;
}

QStringList MetadataPanel::defaultMknoteFilter()
{
    Private d;
    return d.defaultMknoteFilter;
}

QStringList MetadataPanel::defaultIptcFilter()
{
    Private d;
    return d.defaultIptcFilter;
}

QStringList MetadataPanel::defaultXmpFilter()
{
    Private d;
    return d.defaultXmpFilter;
}

void MetadataPanel::applySettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group("Image Properties SideBar");

    if (d->exifViewerConfig->itemsCount())
    {
        group.writeEntry("EXIF Tags Filter", d->exifViewerConfig->checkedTagsList());
    }

    if (d->mknoteViewerConfig->itemsCount())
    {
        group.writeEntry("MAKERNOTE Tags Filter", d->mknoteViewerConfig->checkedTagsList());
    }

    if (d->iptcViewerConfig->itemsCount())
    {
        group.writeEntry("IPTC Tags Filter", d->iptcViewerConfig->checkedTagsList());
    }

    if (d->xmpViewerConfig->itemsCount())
    {
        group.writeEntry("XMP Tags Filter", d->xmpViewerConfig->checkedTagsList());
    }

    config->sync();
}

void MetadataPanel::slotTabChanged(int)
{
    DMetadata meta;
    qApp->setOverrideCursor(Qt::WaitCursor);
    qApp->processEvents();
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group("Image Properties SideBar");

    QWidget* const tab = d->tab->currentWidget();

    if (tab == d->exifViewerConfig)
    {
        if (!d->exifViewerConfig->itemsCount())
        {
            d->exifViewerConfig->setTagsMap(meta.getStdExifTagsList());
            d->exifViewerConfig->setcheckedTagsList(group.readEntry("EXIF Tags Filter", d->exifViewerConfig->defaultFilter()));
        }
    }
    else if (tab == d->mknoteViewerConfig)
    {
        if (!d->mknoteViewerConfig->itemsCount())
        {
            d->mknoteViewerConfig->setTagsMap(meta.getMakernoteTagsList());
            d->mknoteViewerConfig->setcheckedTagsList(group.readEntry("MAKERNOTE Tags Filter", d->mknoteViewerConfig->defaultFilter()));
        }
    }
    else if (tab == d->iptcViewerConfig)
    {
        if (!d->iptcViewerConfig->itemsCount())
        {
            d->iptcViewerConfig->setTagsMap(meta.getIptcTagsList());
            d->iptcViewerConfig->setcheckedTagsList(group.readEntry("IPTC Tags Filter", d->iptcViewerConfig->defaultFilter()));
        }
    }
    else if (tab == d->xmpViewerConfig)
    {
        if (!d->xmpViewerConfig->itemsCount())
        {
            d->xmpViewerConfig->setTagsMap(meta.getXmpTagsList());
            d->xmpViewerConfig->setcheckedTagsList(group.readEntry("XMP Tags Filter", d->xmpViewerConfig->defaultFilter()));
        }
    }

    qApp->restoreOverrideCursor();
}

QStringList MetadataPanel::getAllCheckedTags()
{
    QStringList checkedTags;
    checkedTags
            << d->exifViewerConfig->checkedTagsList()
            << d->iptcViewerConfig->checkedTagsList()
            << d->mknoteViewerConfig->checkedTagsList()
            << d->xmpViewerConfig->checkedTagsList()
            ;

    return checkedTags;
}

QList<MetadataSelectorView*> MetadataPanel::viewers()
{
    QList<MetadataSelectorView*> viewers;
    viewers
            << d->exifViewerConfig
            << d->iptcViewerConfig
            << d->mknoteViewerConfig
            << d->xmpViewerConfig
            ;

    return viewers;
}

}  // namespace Digikam
