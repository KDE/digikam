/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-07-17
 * Description : Metadata tags selector config panel.
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include "metadatapanel.moc"

// Qt includes

#include <QFrame>
#include <QVBoxLayout>

// KDE includes

#include <klocale.h>
#include <kdialog.h>
#include <kvbox.h>
#include <ktabwidget.h>
#include <kapplication.h>
#include <kconfig.h>

// Libkexiv2 includes

#include <libkexiv2/version.h>
#include <libkexiv2/kexiv2.h>

// Local includes

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
     "Focus"
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

class MetadataPanelPriv
{
public:

    MetadataPanelPriv()
    {
        tab                     = 0;
        exifViewerConfig        = 0;
        mknoteViewerConfig      = 0;
        iptcViewerConfig        = 0;
        xmpViewerConfig         = 0;
    }

    KTabWidget           *tab;

    MetadataSelectorView *exifViewerConfig;
    MetadataSelectorView *mknoteViewerConfig;
    MetadataSelectorView *iptcViewerConfig;
    MetadataSelectorView *xmpViewerConfig;
};

MetadataPanel::MetadataPanel(KTabWidget* tab)
             : QObject(tab), d(new MetadataPanelPriv)
{
    d->tab = tab;

    // --------------------------------------------------------

    d->exifViewerConfig   = new MetadataSelectorView(d->tab);
    d->exifViewerConfig->setDefaultFilter(ExifHumanList);
    d->tab->insertTab(1, d->exifViewerConfig, i18n("EXIF viewer"));

    d->mknoteViewerConfig = new MetadataSelectorView(d->tab);
    d->mknoteViewerConfig->setDefaultFilter(MakerNoteHumanList);
    d->tab->insertTab(2, d->mknoteViewerConfig, i18n("Makernotes viewer"));

    d->iptcViewerConfig   = new MetadataSelectorView(d->tab);
    d->iptcViewerConfig->setDefaultFilter(IptcHumanList);
    d->tab->insertTab(3, d->iptcViewerConfig, i18n("IPTC viewer"));

    d->xmpViewerConfig    = new MetadataSelectorView(d->tab);
    d->xmpViewerConfig->setDefaultFilter(XmpHumanList);
    d->tab->insertTab(4, d->xmpViewerConfig, i18n("XMP viewer"));

#if KEXIV2_VERSION < 0x010000
    d->tab->setTabBarHidden(true);
#endif

    // --------------------------------------------------------

    connect(d->tab, SIGNAL(currentChanged(int)),
            this, SLOT(slotTabChanged(int)));
}

MetadataPanel::~MetadataPanel()
{
    delete d;
}

void MetadataPanel::applySettings()
{
#if KEXIV2_VERSION >= 0x010000
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("Image Properties SideBar");

    if (d->exifViewerConfig->selector()->model()->rowCount())
        group.writeEntry("EXIF Tags Filter", d->exifViewerConfig->checkedTagsList());

    if (d->mknoteViewerConfig->selector()->model()->rowCount())
        group.writeEntry("MAKERNOTE Tags Filter", d->mknoteViewerConfig->checkedTagsList());

    if (d->iptcViewerConfig->selector()->model()->rowCount())
        group.writeEntry("IPTC Tags Filter", d->iptcViewerConfig->checkedTagsList());

    if (d->xmpViewerConfig->selector()->model()->rowCount())
        group.writeEntry("XMP Tags Filter", d->xmpViewerConfig->checkedTagsList());

    config->sync();
#endif
}

void MetadataPanel::slotTabChanged(int index)
{
    DMetadata meta;
    kapp->setOverrideCursor(Qt::WaitCursor);
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("Image Properties SideBar");

    switch(index)
    {
#if KEXIV2_VERSION >= 0x010000
        case 1:
        {
            if (!d->exifViewerConfig->selector()->model()->rowCount())
            {
                d->exifViewerConfig->selector()->setTagsMap(meta.getStdExifTagsList());
                d->exifViewerConfig->selector()->setcheckedTagsList(group.readEntry("EXIF Tags Filter", QStringList()));
            }
            break;
        }

        case 2:
        {
            if (!d->mknoteViewerConfig->selector()->model()->rowCount())
            {
                d->mknoteViewerConfig->selector()->setTagsMap(meta.getMakernoteTagsList());
                d->mknoteViewerConfig->selector()->setcheckedTagsList(group.readEntry("MAKERNOTE Tags Filter", QStringList()));
            }
            break;
        }

        case 3:
        {
            if (!d->iptcViewerConfig->selector()->model()->rowCount())
            {
                d->iptcViewerConfig->selector()->setTagsMap(meta.getIptcTagsList());
                d->iptcViewerConfig->selector()->setcheckedTagsList(group.readEntry("IPTC Tags Filter", QStringList()));
            }
            break;
        }

        case 4:
        {
            if (!d->xmpViewerConfig->selector()->model()->rowCount())
            {
                d->xmpViewerConfig->selector()->setTagsMap(meta.getXmpTagsList());
                d->xmpViewerConfig->selector()->setcheckedTagsList(group.readEntry("XMP Tags Filter", QStringList()));
            }
            break;
        }
#endif
        default:
            break;
    }
    kapp->restoreOverrideCursor();
}

}  // namespace Digikam
