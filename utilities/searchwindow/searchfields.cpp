/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-01-20
 * Description : User interface for searches
 *
 * Copyright (C) 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2011-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "searchfields.moc"

// C++ includes

#include <cmath>

// Qt includes

#include <QApplication>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QSpinBox>
#include <QTimeEdit>
#include <QTreeView>

// KDE includes

#include <kcombobox.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>
#include <ksqueezedtextlabel.h>
#include <kdebug.h>

// LibKDcraw includes

#include <libkdcraw/squeezedcombobox.h>
#include <libkdcraw/rexpanderbox.h>

// Local includes

#include "album.h"
#include "albumdb.h"
#include "albummanager.h"
#include "albummodel.h"
#include "albumselectcombobox.h"
#include "choicesearchutilities.h"
#include "dimg.h"
#include "dmetadata.h"
#include "imagescanner.h"
#include "ddateedit.h"
#include "ratingsearchutilities.h"
#include "searchfieldgroup.h"
#include "searchwindow.h"
#include "tagscache.h"
#include "colorlabelfilter.h"
#include "picklabelfilter.h"
#include "applicationsettings.h"

using namespace KDcrawIface;

namespace Digikam
{

SearchField* SearchField::createField(const QString& name, SearchFieldGroup* const parent)
{
    if (name == "albumid")
    {
        SearchFieldAlbum* const field = new SearchFieldAlbum(parent, SearchFieldAlbum::TypeAlbum);
        field->setFieldName(name);
        field->setText(i18n("Album"), i18n("Search pictures located in"));
        return field;
    }
    else if (name == "albumname")
    {
        SearchFieldText* const field = new SearchFieldText(parent);
        field->setFieldName(name);
        field->setText(i18n("Album"), i18n("The album name contains"));
        return field;
    }
    else if (name == "albumcaption")
    {
        SearchFieldText* const field = new SearchFieldText(parent);
        field->setFieldName(name);
        field->setText(i18n("Album"), i18n("The album caption contains"));
        return field;
    }
    else if (name == "albumcollection")
    {
        SearchFieldChoice* const field = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18n("Album"), i18n("The album category is"));
        ApplicationSettings* const settings = ApplicationSettings::instance();
        if (settings)
        {
            QStringList Categories = settings->getAlbumCategoryNames();
            int size = Categories.size();
            QStringList categorychoices;
            for(int i=0; i<size; i++)
            {
                categorychoices << Categories.at(i) << Categories.at(i);
            }
            field->setChoice(categorychoices);
        }
        return field;
    }
    else if (name == "tagid")
    {
        SearchFieldAlbum* const field = new SearchFieldAlbum(parent, SearchFieldAlbum::TypeTag);
        field->setFieldName(name);
        field->setText(i18n("Tags"), i18n("Return pictures with tag"));
        return field;
    }
    else if (name == "tagname")
    {
        SearchFieldText* const field = new SearchFieldText(parent);
        field->setFieldName(name);
        field->setText(i18n("Tags"), i18n("A tag of the picture contains"));
        return field;
    }
    else if (name == "notag")
    {
        /**
        * @todo Merge a "Not tagged" field into TagModel together with AND/OR control
        * for checked tags and logical connections (AND and Not Tagged checked => all other tags disabled)
        */
        SearchFieldCheckBox* const field = new SearchFieldCheckBox(parent);
        field->setFieldName(name);
        field->setText(i18n("Tags"), i18n("Image has no tags"));
        field->setLabel(i18n("Not Tagged"));
        return field;
    }
    else if (name == "filename")
    {
        SearchFieldText* const field = new SearchFieldText(parent);
        field->setFieldName(name);
        field->setText(i18n("File Name"), i18n("Return pictures whose file name contains"));
        return field;
    }
    else if (name == "modificationdate")
    {
        SearchFieldRangeDate* const field = new SearchFieldRangeDate(parent, SearchFieldRangeDate::DateTime);
        field->setFieldName(name);
        field->setText(i18n("Modification"), i18n("Return pictures modified between"));
        field->setBetweenText(i18nc("'Return pictures modified between...and...", "and"));
        return field;
    }
    else if (name == "filesize")
    {
        SearchFieldRangeDouble* const field = new SearchFieldRangeDouble(parent);
        field->setFieldName(name);
        field->setText(i18n("File Size"), i18n("Size of the file"));
        field->setBetweenText(i18nc("Size of the file ...-...", "-"));
        field->setNumberPrefixAndSuffix(QString(), "MB");
        field->setBoundary(0, 1000000, 1, 0.5);
        field->setFactor(1024 * 1024);
        return field;
    }
    else if (name == "labels")
    {
        SearchFieldLabels* const field = new SearchFieldLabels(parent);
        field->setFieldName(name);
        field->setText(i18n("Labels"), i18n("Return pictures with labels"));
        return field;
    }
    else if (name == "rating")
    {
        SearchFieldRating* const field = new SearchFieldRating(parent);
        field->setFieldName(name);
        field->setText(i18n("Rating"), i18n("Return pictures rated at least"));
        field->setBetweenText(i18nc("Return pictures rated at least...at most...", "at most"));
        return field;
    }
    else if (name == "creationdate")
    {
        SearchFieldRangeDate* const field = new SearchFieldRangeDate(parent, SearchFieldRangeDate::DateTime);
        field->setFieldName(name);
        field->setText(i18n("Date"), i18n("Return pictures created between"));
        field->setBetweenText(i18nc("'Return pictures created between...and...", "and"));
        return field;
    }
    else if (name == "digitizationdate")
    {
        SearchFieldRangeDate* const field = new SearchFieldRangeDate(parent, SearchFieldRangeDate::DateTime);
        field->setFieldName(name);
        field->setText(i18n("Digitization"), i18n("Return pictures digitized between"));
        field->setBetweenText(i18nc("'Return pictures digitized between...and...", "and"));
        return field;
    }
    else if (name == "orientation")
    {
        //choice
        SearchFieldChoice* const field = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18n("Exif Orientation"), i18n("Find pictures with orientation flag"));
        QMap<int, QString> map = DMetadata::possibleValuesForEnumField(MetadataInfo::Orientation);
        field->setChoice(map);
        return field;
    }
    else if (name == "dimension")
    {
        // "width", "height", "pixels"
    }
    else if (name == "width")
    {
        SearchFieldRangeInt* const field = new SearchFieldRangeInt(parent);
        field->setFieldName(name);
        field->setText(i18n("Width"), i18n("Find pictures with a width between"));
        field->setBetweenText(i18nc("Find pictures with a width between...and...", "and"));
        field->setNumberPrefixAndSuffix(QString(), i18nc("Pixels", "px"));
        field->setBoundary(1, 1000000, 250);
        field->setSuggestedValues(QList<int>()
                                  << 50 << 100 << 200 << 300 << 400 << 500 << 600 << 700 << 800 << 900
                                  << 1000 << 1250 << 1500 << 1750 << 2000 << 3000 << 4000
                                  << 5000 << 6000 << 7000 << 8000 << 9000 << 10000
                                 );
        field->setSuggestedInitialValue(1000);
        field->setSingleSteps(50, 1000);
        return field;
    }
    else if (name == "height")
    {
        SearchFieldRangeInt* const field = new SearchFieldRangeInt(parent);
        field->setFieldName(name);
        field->setText(i18n("Height"), i18n("Find pictures with a height between"));
        field->setBetweenText(i18nc("Find pictures with a height between...and...", "and"));
        field->setNumberPrefixAndSuffix(QString(), i18nc("Pixels", "px"));
        field->setBoundary(1, 1000000, 250);
        field->setSuggestedValues(QList<int>()
                                  << 50 << 100 << 200 << 300 << 400 << 500 << 600 << 700 << 800 << 900
                                  << 1000 << 1250 << 1500 << 1750 << 2000 << 3000 << 4000
                                  << 5000 << 6000 << 7000 << 8000 << 9000 << 10000
                                 );
        field->setSuggestedInitialValue(1000);
        field->setSingleSteps(50, 1000);
        return field;
    }
    else if (name == "pageorientation")
    {
        SearchFieldPageOrientation* const field = new SearchFieldPageOrientation(parent);
        field->setFieldName(name);
        field->setText(i18n("Orientation"), i18nc("Find pictures with any orientation / landscape / portrait orientation...",
                                                  "Find pictures with"));
        return field;
    }
    else if (name == "format")
    {
        //choice
        SearchFieldChoice* const field = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18n("File Format"), i18n("Return pictures with the image file format"));
        QStringList formats = DatabaseAccess().db()->getFormatStatistics().keys();
        formats += formats;
        formats.sort();
        field->setChoice(formats);
        return field;
    }
    else if (name == "colordepth")
    {
        //choice
        SearchFieldColorDepth* const field = new SearchFieldColorDepth(parent);
        field->setFieldName(name);
        field->setText(i18n("Color Depth"), i18nc("Find pictures with any color depth / 8 bits per channel...", "Find pictures with"));
        return field;
    }
    else if (name == "colormodel")
    {
        //choice
        SearchFieldChoice* const field = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18n("Color Model"), i18n("Find pictures with the color model"));
        QMap<int, QString> map;
        map.insert(DImg::COLORMODELUNKNOWN, DImg::colorModelToString(DImg::COLORMODELUNKNOWN));
        map.insert(DImg::RGB,               DImg::colorModelToString(DImg::RGB));
        map.insert(DImg::GRAYSCALE,         DImg::colorModelToString(DImg::GRAYSCALE));
        map.insert(DImg::MONOCHROME,        DImg::colorModelToString(DImg::MONOCHROME));
        map.insert(DImg::INDEXED,           DImg::colorModelToString(DImg::INDEXED));
        map.insert(DImg::YCBCR,             DImg::colorModelToString(DImg::YCBCR));
        map.insert(DImg::CMYK,              DImg::colorModelToString(DImg::CMYK));
        map.insert(DImg::CIELAB,            DImg::colorModelToString(DImg::CIELAB));
        map.insert(DImg::COLORMODELRAW,     DImg::colorModelToString(DImg::COLORMODELRAW));
        field->setChoice(map);
        return field;
    }

    else if (name == "make")
    {
        //string
        SearchFieldText* const field = new SearchFieldText(parent);
        field->setFieldName(name);
        field->setText(i18n("Camera"), i18n("The make of the camera"));
        return field;
    }
    else if (name == "model")
    {
        //string
        SearchFieldText* const field = new SearchFieldText(parent);
        field->setFieldName(name);
        field->setText(i18n("Camera"), i18n("The model of the camera"));
        return field;
    }
    else if (name == "aperture")
    {
        //double
        SearchFieldRangeDouble* const field = new SearchFieldRangeDouble(parent);
        field->setFieldName(name);
        field->setText(i18n("Aperture"), i18n("Lens aperture as f-number"));
        field->setBetweenText(i18nc("Lens aperture as f-number ...-...", "-"));
        field->setNoValueText("f/#");
        field->setNumberPrefixAndSuffix("f/", QString());
        field->setBoundary(0.3, 65536, 1, 0.1);
        field->setSuggestedValues(QList<double>()
                                  << 0.5 << 0.7 << 1.0 << 1.4 << 2 << 2.8 << 4 << 5.6
                                  << 8 << 11 << 16 << 22 << 32 << 45 << 64 << 90 << 128
                                 );
        field->setSuggestedInitialValue(1.0);
        field->setSingleSteps(0.1, 10);
        return field;
    }
    else if (name == "focallength")
    {
        //double
        SearchFieldRangeInt* const field = new SearchFieldRangeInt(parent);
        field->setFieldName(name);
        field->setText(i18n("Focal length"), i18n("Focal length of the lens"));
        field->setBetweenText(i18nc("Focal length of the lens ...-...", "-"));
        field->setNumberPrefixAndSuffix(QString(), "mm");
        field->setBoundary(0, 200000, 10);
        field->setSuggestedValues(QList<int>()
                                  << 10 << 15 << 20 << 25 << 30 << 40 << 50 << 60 << 70 << 80 << 90
                                  << 100 << 150 << 200 << 250 << 300 << 400 << 500 << 750 << 1000
                                 );
        field->setSuggestedInitialValue(30);
        field->setSingleSteps(2, 500);
        return field;
    }
    else if (name == "focallength35")
    {
        //double
        SearchFieldRangeInt* const field = new SearchFieldRangeInt(parent);
        field->setFieldName(name);
        field->setText(i18n("Focal length"), i18n("35mm equivalent focal length"));
        field->setBetweenText(i18nc("35mm equivalent focal length ...-...", "-"));
        field->setNumberPrefixAndSuffix(QString(), "mm");
        field->setBoundary(0, 200000, 10);
        field->setSuggestedValues(QList<int>()
                                  << 8 << 10 << 15 << 16 << 20 << 28 << 30 << 40 << 50 << 60 << 70 << 80
                                  << 90 << 100 << 150 << 200 << 250 << 300 << 400 << 500 << 750 << 1000
                                 );
        field->setSuggestedInitialValue(28);
        field->setSingleSteps(2, 500);
        return field;
    }
    else if (name == "exposuretime")
    {
        //double
        SearchFieldRangeInt* const field = new SearchFieldRangeInt(parent);
        field->setFieldName(name);
        field->setText(i18n("Exposure"), i18n("Exposure time"));
        field->setBetweenText(i18nc("Exposure time ...-...", "-"));
        field->setNumberPrefixAndSuffix(QString(), "s");
        field->enableFractionMagic("1/"); // it's 1/250, not 250 as in the spin box
        field->setBoundary(86400, -1024000, 10); // negative is 1/
        field->setSuggestedValues(QList<int>()
                                  << 30 << 15 << 8 << 4 << 2 << 1 << -2 << -4 << -8 << -15
                                  << -30 << -50 << -60 << -100 << -125 << -150 << -200
                                  << -250 << -500 << -750 << -1000 << -2000 << -4000 << -8000 << -16000
                                 );
        field->setSuggestedInitialValue(-200);
        field->setSingleSteps(2000, 5);
        return field;
    }
    else if (name == "exposureprogram")
    {
        //choice
        SearchFieldChoice* const field = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18n("Exposure"), i18n("Automatic exposure program"));
        QMap<int, QString> map = DMetadata::possibleValuesForEnumField(MetadataInfo::ExposureProgram);
        field->setChoice(map);
        return field;
    }
    else if (name == "exposuremode")
    {
        //choice
        SearchFieldChoice* const field = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18n("Exposure"), i18n("Automatic or manual exposure"));
        QMap<int, QString> map = DMetadata::possibleValuesForEnumField(MetadataInfo::ExposureMode);
        field->setChoice(map);
        return field;
    }
    else if (name == "sensitivity")
    {
        //int
        SearchFieldRangeInt* const field = new SearchFieldRangeInt(parent);
        field->setFieldName(name);
        field->setText(i18n("Sensitivity"), i18n("ISO film speed (linear scale, ASA)"));
        field->setBetweenText(i18nc("ISO film speed (linear scale, ASA) ...-...", "-"));
        field->setBoundary(0, 2000000, 50);
        field->setSuggestedValues(QList<int>()
                                  << 6 << 8 << 10 << 12 << 16 << 20 << 25 << 32 << 40 << 50 << 64
                                  << 80 << 100 << 125 << 160 << 200 << 250 << 320 << 400 << 500
                                  << 640 << 800 << 1000 << 1250 << 1600 << 2000 << 2500 << 3200
                                  << 4000 << 5000 << 6400
                                 );
        field->setSuggestedInitialValue(200);
        field->setSingleSteps(1, 400);
        return field;
    }
    else if (name == "flashmode")
    {
        //choice
        /**
        * @todo This is a bitmask, and gives some more information
        */
        SearchFieldChoice* const field = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18n("Flash"), i18n("Flash mode"));
        QMap<int, QString> map = DMetadata::possibleValuesForEnumField(MetadataInfo::FlashMode);
        field->setChoice(map);
        return field;
    }
    else if (name == "whitebalance")
    {
        //choice
        SearchFieldChoice* const field = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18n("White Balance"), i18n("Automatic or manual white balance"));
        QMap<int, QString> map = DMetadata::possibleValuesForEnumField(MetadataInfo::WhiteBalance);
        field->setChoice(map);
        return field;
    }
    else if (name == "whitebalancecolortemperature")
    {
        //int
        SearchFieldRangeInt* const field = new SearchFieldRangeInt(parent);
        field->setFieldName(name);
        field->setText(i18n("White balance"), i18n("Color temperature used for white balance"));
        field->setBetweenText(i18nc("Color temperature used for white balance ...-...", "-"));
        field->setNumberPrefixAndSuffix(QString(), "K");
        field->setBoundary(1, 100000, 100);
        return field;
    }
    else if (name == "meteringmode")
    {
        //choice
        SearchFieldChoice* const field = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18n("Metering Mode"), i18n("Method to determine the exposure"));
        QMap<int, QString> map = DMetadata::possibleValuesForEnumField(MetadataInfo::MeteringMode);
        field->setChoice(map);
        return field;
    }
    else if (name == "subjectdistance")
    {
        //double
        SearchFieldRangeDouble* const field = new SearchFieldRangeDouble(parent);
        field->setFieldName(name);
        field->setText(i18n("Subject Distance"), i18n("Distance of the subject from the lens"));
        field->setBetweenText(i18nc("Distance of the subject from the lens ...-...", "-"));
        field->setNumberPrefixAndSuffix(QString(), "m");
        field->setBoundary(0, 50000, 1, 0.1);
        return field;
    }
    else if (name == "subjectdistancecategory")
    {
        //choice
        SearchFieldChoice* const field = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18n("Subject Distance"), i18n("Macro, close or distant view"));
        QMap<int, QString> map = DMetadata::possibleValuesForEnumField(MetadataInfo::SubjectDistanceCategory);
        field->setChoice(map);
        return field;
    }

    else if (name == "latitude")
    {
    }
    else if (name == "longitude")
    {
    }
    else if (name == "altitude")
    {
        SearchFieldRangeDouble* const field = new SearchFieldRangeDouble(parent);
        field->setFieldName(name);
        field->setText(i18n("GPS"), i18n("Altitude range"));
        field->setBetweenText(i18nc("Altitude range ...-...", "-"));
        field->setNumberPrefixAndSuffix(QString(), "m");
        field->setBoundary(0, 10000, 4, 1);
        return field;
    }
    else if (name == "positionorientation")
    {
    }
    else if (name == "positiontilt")
    {
    }
    else if (name == "positionroll")
    {
    }
    else if (name == "positiondescription")
    {
    }
    else if (name == "nogps")
    {
        SearchFieldCheckBox* const field = new SearchFieldCheckBox(parent);
        field->setFieldName(name);
        field->setText(i18n("GPS"), i18n("Image has no GPS info"));
        field->setLabel(i18n("Not Geo-located"));
        return field;
    }

    else if (name == "comment")
    {
        SearchFieldText* const field = new SearchFieldText(parent);
        field->setFieldName(name);
        field->setText(i18n("Caption"), i18n("Return pictures whose comment contains"));
        return field;
    }
    else if (name == "commentauthor")
    {
        SearchFieldText* const field = new SearchFieldText(parent);
        field->setFieldName(name);
        field->setText(i18n("Author"), i18n("Return pictures commented by"));
        return field;
    }
    else if (name == "headline")
    {
        SearchFieldText* const field = new SearchFieldText(parent);
        field->setFieldName(name);
        field->setText(i18n("Headline"), i18n("Return pictures with the IPTC headline"));
        return field;
    }
    else if (name == "title")
    {
        SearchFieldText* const field = new SearchFieldText(parent);
        field->setFieldName(name);
        field->setText(i18n("Title"), i18n("Return pictures with the IPTC title"));
        return field;
    }
    else if (name == "keyword")
    {
        SearchFieldText* const field = new SearchFieldKeyword(parent);
        field->setFieldName(name);
        field->setText(QString(), i18n("Find pictures that have associated all these words:"));
        return field;
    }
    else if (name == "aspectratioimg")
    {
        SearchFieldText* const field = new SearchFieldText(parent);
        field->setFieldName(name);
        field->setText(i18n("Aspect Ratio"), i18n("Return pictures with the aspect ratio"));
        return field;
    }
    else if (name == "pixelsize")
    {
        SearchFieldRangeInt* const field = new SearchFieldRangeInt(parent);
        field->setFieldName(name);
        field->setText(i18n("Pixel Size"), i18n("Value of (Width * Height) between"));
        field->setBetweenText(i18nc("Value of (Width * Height) between...and...", "and"));
        field->setNumberPrefixAndSuffix(QString(), "px");
        field->setBoundary(1, 2000000000, 100);
        return field;
    }


    else if (name == "videoaspectratio")
    {
        SearchFieldChoice* field = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18n("Aspect Ratio"), i18n("Return video with the frame aspect ratio"));
        QStringList ratio;
        ratio << "4:3"<< "4:3";
        ratio << "3:2"<< "3:2";
        ratio << "16:9" << "16:9";
        ratio << "2:1" << "2:1";
        // TODO: add more possible aspect ratio
        field->setChoice(ratio);
        return field;
    }
    else if (name == "videoduration")
    {
        SearchFieldRangeInt* field = new SearchFieldRangeInt(parent);
        field->setFieldName(name);
        field->setText(i18n("Duration"), i18n("Length of the video"));
        field->setBetweenText(i18nc("Find video with a length between...and...", "and"));
        field->setNumberPrefixAndSuffix(QString(), i18nc("Seconds", "s"));
        field->setBoundary(1, 10000, 100);
        field->setSuggestedValues(QList<int>()
                                  << 10 << 30 << 60 << 90 << 120 << 240 << 360 << 500 << 1000 << 2000
                                  << 3000 << 4000 << 5000 << 6000 << 7000 << 8000 << 9000 << 10000
                                  // TODO : adjust default values
                                 );
        field->setSuggestedInitialValue(10);
        field->setSingleSteps(10, 100);
        return field;
    }
    else if (name == "videoframerate")
    {
        SearchFieldRangeInt* field = new SearchFieldRangeInt(parent);
        field->setFieldName(name);
        field->setText(i18n("Frame Rate"), i18n("Return video with the frame rate"));
        field->setBetweenText(i18nc("Find video with frame rate between...and...", "and"));
        field->setNumberPrefixAndSuffix(QString(), i18nc("Frames per Second", "fps"));
        field->setBoundary(10, 60, 5);
        field->setSuggestedValues(QList<int>()
                                  << 10 << 15 << 20 << 25 << 30 << 35 << 40 << 45 << 55 << 60
                                  // TODO : adjust default values
                                 );
        field->setSuggestedInitialValue(10);
        field->setSingleSteps(5, 60);
        return field;
    }
    else if (name == "videocodec")
    {
        SearchFieldChoice* field = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18n("Codec"), i18n("Return video codec"));
        QStringList codec;
        codec << "avi"  << "Audio Video Interleave";
        codec << "mov"  << "QuickTime";
        codec << "mp4"  << "MPEG Layer 4";
        codec << "3gp"  << "3GPP";
        codec << "divx" << "DivX";
        codec << "wma"  << "Windows Media Video";
        codec << "mkv"  << "Matroska";
        codec << "s263" << "H.263";
        codec << "mjpg" << "Motion JPEG";
        // TODO: add more possible codec
        field->setChoice(codec);
        return field;
    }
    else if (name == "videoaudiobitrate")
    {
        SearchFieldRangeInt* field = new SearchFieldRangeInt(parent);
        field->setFieldName(name);
        field->setText(i18n("Audio Bit Rate"), i18n("Return video audio bits rate"));
        field->setBetweenText(i18nc("Find video with audio bit rate between...and...", "and"));
        field->setNumberPrefixAndSuffix(QString(), i18nc("Bits per Second", "bps"));
        field->setBoundary(1000, 100000, 1000);
        field->setSuggestedValues(QList<int>()
                                  << 1000 << 4000 << 8000 << 12000 << 16000 << 20000 << 30000 << 40000 << 50000
                                  << 60000 << 700000 << 800000 << 900000 << 100000
                                  // TODO : adjust default values
                                 );
        field->setSuggestedInitialValue(1000);
        field->setSingleSteps(1000, 1000);
        return field;
    }
    else if (name == "videoaudiochanneltype")
    {
        SearchFieldChoice* field = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18n("Audio Channel Type"), i18n("Return video audio Channel Type"));
        QStringList type;
        type << "1" << i18n("Mono");
        type << "2" << i18n("Stereo");
        // TODO: add more possible audio channel type
        field->setChoice(type);
        return field;
    }
    else if (name == "videoaudiocompressor")
    {
        SearchFieldChoice* field = new SearchFieldChoice(parent);
        field->setFieldName(name);
        field->setText(i18n("Audio Compressor"), i18n("Return video audio Compressor"));
        QStringList type;
        type << "raw" << "RAW";
        type << "mp3" << "MPEG Layer 3";
        type << "mp4a" << "MPEG4 Audio";
        type << "samr" << "Adaptive Multi-rate Audio";
        type << "sowt" << "Apple QuickTime SWOT Little Endian PCM Audio";
        type << "Microsoft PCM" << "Microsoft PCM";
        // TODO: add more possible audio compressor
        field->setChoice(type);
        return field;
    }

    else
    {
        kWarning() << "SearchField::createField: cannot create SearchField for" << name;
    }

    return 0;
}

// -------------------------------------------------------------------------------------------

SearchField::SearchField(QObject* const parent)
    : QObject(parent)
{
    m_label                = new QLabel;
    m_detailLabel          = new QLabel;
    m_clearButton          = new AnimatedClearButton;
    m_categoryLabelVisible = true;
    m_valueIsValid         = false;
}

void SearchField::setup(QGridLayout* const layout, int line)
{
    if (line == -1)
    {
        line = layout->rowCount();
    }

    // 10px indent
    layout->setColumnMinimumWidth(0, 10);
    // set stretch for the value widget columns
    layout->setColumnStretch(3, 1);
    layout->setColumnStretch(5, 1);
    // push value widgets to the left
    layout->setColumnStretch(6, 1);

    setupLabels(layout, line);
    // value widgets can use columns 3,4,5.
    // In the case of "from ... to ..." fields, column 3 and 5 can contain spin boxes etc.,
    // and 4 can contain a label in between.
    // In other cases, a widget or sublayout spanning the three columns is recommended.
    setupValueWidgets(layout, line, 3);

    // setup the clear button that appears dynamically
    if (qApp->isLeftToRight())
    {
        m_clearButton->setPixmap(SmallIcon("edit-clear-locationbar-rtl", 0, KIconLoader::DefaultState));
    }
    else
    {
        m_clearButton->setPixmap(SmallIcon("edit-clear-locationbar-ltr", 0, KIconLoader::DefaultState));
    }

    // Important: Don't cause re-layouting when button gets hidden/shown!
    m_clearButton->stayVisibleWhenAnimatedOut(true);
    m_clearButton->setToolTip(i18n("Reset contents"));

    connect(m_clearButton, SIGNAL(clicked()),
            this, SLOT(clearButtonClicked()));

    layout->addWidget(m_clearButton, line, 7);
}

void SearchField::setupLabels(QGridLayout* layout, int line)
{
    m_label->setObjectName("SearchField_MainLabel");
    m_detailLabel->setObjectName("SearchField_DetailLabel");
    layout->addWidget(m_label, line, 1);
    layout->addWidget(m_detailLabel, line, 2);
}

void SearchField::setFieldName(const QString& fieldName)
{
    m_name = fieldName;
}

void SearchField::setText(const QString& label, const QString& detailLabel)
{
    m_label->setText(label);
    m_detailLabel->setText(detailLabel);
}

bool SearchField::supportsField(const QString& fieldName)
{
    return m_name == fieldName;
}

void SearchField::setVisible(bool visible)
{
    m_label->setVisible(visible && m_categoryLabelVisible);
    m_detailLabel->setVisible(visible);
    m_clearButton->setShallBeShown(visible);
    setValueWidgetsVisible(visible);
}

bool SearchField::isVisible()
{
    // the detail label is considered representative for all widgets
    return m_detailLabel->isVisible();
}

void SearchField::setCategoryLabelVisible(bool visible)
{
    if (m_categoryLabelVisible == visible)
    {
        return;
    }

    m_categoryLabelVisible = visible;
    // update status: compare setVisible() and isVisible()
    m_label->setVisible(m_detailLabel->isVisible() && m_categoryLabelVisible);
}

void SearchField::setCategoryLabelVisibleFromPreviousField(SearchField* previousField)
{
    if (previousField->m_label->text() == m_label->text())
    {
        setCategoryLabelVisible(false);
    }
    else
    {
        setCategoryLabelVisible(true);
    }
}

QList<QRect> SearchField::widgetRects(WidgetRectType type) const
{
    QList<QRect> rects;

    if (type == LabelAndValueWidgetRects)
    {
        rects << m_label->geometry();
        rects << m_detailLabel->geometry();
    }

    rects += valueWidgetRects();
    return rects;
}

void SearchField::clearButtonClicked()
{
    reset();
}

void SearchField::setValidValueState(bool valueIsValid)
{
    if (valueIsValid != m_valueIsValid)
    {
        m_valueIsValid = valueIsValid;
        // Note: setVisible visibility is independent from animateVisible visibility!
        m_clearButton->animateVisible(m_valueIsValid);
    }
}

// -------------------------------------------------------------------------

SearchFieldText::SearchFieldText(QObject* const parent)
    : SearchField(parent), m_edit(0)
{
}

void SearchFieldText::setupValueWidgets(QGridLayout* layout, int row, int column)
{
    m_edit = new KLineEdit;
    layout->addWidget(m_edit, row, column, 1, 3);

    connect(m_edit, SIGNAL(textChanged(QString)),
            this, SLOT(valueChanged(QString)));
}

void SearchFieldText::read(SearchXmlCachingReader& reader)
{
    QString value = reader.value();
    m_edit->setText(value);
}

void SearchFieldText::write(SearchXmlWriter& writer)
{
    QString value = m_edit->text();

    if (!value.isEmpty())
    {
        writer.writeField(m_name, SearchXml::Like);
        writer.writeValue(value);
        writer.finishField();
    }
}

void SearchFieldText::reset()
{
    m_edit->setText(QString());
}

void SearchFieldText::setValueWidgetsVisible(bool visible)
{
    m_edit->setVisible(visible);
}

QList<QRect> SearchFieldText::valueWidgetRects() const
{
    QList<QRect> rects;
    rects << m_edit->geometry();
    return rects;
}

void SearchFieldText::valueChanged(const QString& text)
{
    setValidValueState(!text.isEmpty());
}

// -------------------------------------------------------------------------

SearchFieldKeyword::SearchFieldKeyword(QObject* const parent)
    : SearchFieldText(parent)
{
}

void SearchFieldKeyword::read(SearchXmlCachingReader& reader)
{
    QString keyword = reader.value();
    m_edit->setText(KeywordSearch::merge(m_edit->text(), keyword));
}

void SearchFieldKeyword::write(SearchXmlWriter& writer)
{
    QStringList keywordList = KeywordSearch::split(m_edit->text());

    foreach(const QString& keyword, keywordList)
    {
        if (!keyword.isEmpty())
        {
            writer.writeField(m_name, SearchXml::Like);
            writer.writeValue(keyword);
            writer.finishField();
        }
    }
}

// -------------------------------------------------------------------------

SearchFieldRangeDate::SearchFieldRangeDate(QObject* const parent, Type type)
    : SearchField(parent),
      m_firstTimeEdit(0),
      m_firstDateEdit(0),
      m_secondTimeEdit(0),
      m_secondDateEdit(0),
      m_type(type)
{
    m_betweenLabel = new QLabel;
}

void SearchFieldRangeDate::setupValueWidgets(QGridLayout* layout, int row, int column)
{
    m_firstDateEdit  = new DDateEdit;
    m_secondDateEdit = new DDateEdit;

    if (m_type == DateOnly)
    {
        layout->addWidget(m_firstDateEdit,  row, column);
        layout->addWidget(m_betweenLabel,   row, column + 1, Qt::AlignHCenter);
        layout->addWidget(m_secondDateEdit, row, column + 2);
    }
    else
    {
        QHBoxLayout* const hbox1 = new QHBoxLayout;
        QHBoxLayout* const hbox2 = new QHBoxLayout;

        m_firstTimeEdit  = new QTimeEdit;
        m_secondTimeEdit = new QTimeEdit;

        hbox1->addWidget(m_firstDateEdit);
        hbox1->addWidget(m_firstTimeEdit);
        hbox2->addWidget(m_secondDateEdit);
        hbox2->addWidget(m_secondTimeEdit);

        layout->addLayout(hbox1,          row, column);
        layout->addWidget(m_betweenLabel, row, column + 1, Qt::AlignHCenter);
        layout->addLayout(hbox2,          row, column + 2);
    }

    connect(m_firstDateEdit, SIGNAL(dateChanged(QDate)),
            this, SLOT(valueChanged()));

    connect(m_secondDateEdit, SIGNAL(dateChanged(QDate)),
            this, SLOT(valueChanged()));
}

void SearchFieldRangeDate::setBetweenText(const QString& between)
{
    m_betweenLabel->setText(between);
}

void SearchFieldRangeDate::read(SearchXmlCachingReader& reader)
{
    SearchXml::Relation relation = reader.fieldRelation();

    if (relation == SearchXml::Interval || relation == SearchXml::IntervalOpen)
    {
        QList<QDateTime> dates = reader.valueToDateTimeList();

        if (dates.size() != 2)
        {
            return;
        }

        if (m_type == DateTime)
        {
            m_firstDateEdit->setDate(dates.first().date());
            m_firstTimeEdit->setTime(dates.first().time());
            m_secondDateEdit->setDate(dates.last().date());
            m_secondTimeEdit->setTime(dates.last().time());
        }
        else
        {
            if (relation == SearchXml::Interval)
            {
                dates.last() = dates.last().addDays(-1);
            }

            m_firstDateEdit->setDate(dates.first().date());
            m_secondDateEdit->setDate(dates.last().date());
        }
    }
    else
    {
        QDateTime dt = reader.valueToDateTime();

        if (m_type == DateTime)
        {
            if (relation == SearchXml::Equal)
            {
                m_firstDateEdit->setDate(dt.date());
                m_firstTimeEdit->setTime(dt.time());
                m_secondDateEdit->setDate(dt.date());
                m_secondTimeEdit->setTime(dt.time());
            }
            else if (relation == SearchXml::GreaterThanOrEqual || relation == SearchXml::GreaterThan)
            {
                m_firstDateEdit->setDate(dt.date());
                m_firstTimeEdit->setTime(dt.time());
            }

            {
                m_secondDateEdit->setDate(dt.date());
                m_secondTimeEdit->setTime(dt.time());
            }
        }
        else
        {
            // In DateOnly mode, we always assume dealing with the beginning of the day, QTime(0,0,0).
            // In the UI, we show the date only (including the whole day).
            // The difference between ...Than and ...ThanOrEqual is only one second, ignored.

            if (relation == SearchXml::Equal)
            {
                m_firstDateEdit->setDate(dt.date());
                m_secondDateEdit->setDate(dt.date());
            }
            else if (relation == SearchXml::GreaterThanOrEqual || relation == SearchXml::GreaterThan)
            {
                m_firstDateEdit->setDate(dt.date());
            }
            else if (relation == SearchXml::LessThanOrEqual || relation == SearchXml::LessThan)
            {
                dt = dt.addDays(-1);
                m_secondDateEdit->setDate(dt.date());
            }
        }
    }

    valueChanged();
}

void SearchFieldRangeDate::write(SearchXmlWriter& writer)
{
    if (m_firstDateEdit->date().isValid() && m_secondDateEdit->date().isValid())
    {
        QDateTime firstDate(m_firstDateEdit->date());

        if (m_type == DateTime)
        {
            firstDate.setTime(m_firstTimeEdit->time());
        }

        QDateTime secondDate(m_secondDateEdit->date());

        if (m_type == DateTime)
        {
            secondDate.setTime(m_secondTimeEdit->time());
        }

        if (firstDate == secondDate)
        {
            writer.writeField(m_name, SearchXml::Equal);
            writer.writeValue(firstDate);
            writer.finishField();
        }
        else
        {
            if (m_type == DateOnly)
            {
                secondDate = secondDate.addDays(1);
            }

            writer.writeField(m_name, SearchXml::Interval);
            writer.writeValue(QList<QDateTime>() << firstDate << secondDate);
            writer.finishField();
        }
    }
    else
    {
        QDate date = m_firstDateEdit->date();

        if (date.isValid())
        {
            writer.writeField(m_name, SearchXml::GreaterThanOrEqual);
            QDateTime dt(date);

            if (m_type == DateTime)
            {
                dt.setTime(m_firstTimeEdit->time());
            }

            writer.writeValue(dt);
            writer.finishField();
        }

        date = m_secondDateEdit->date();

        if (date.isValid())
        {
            writer.writeField(m_name, SearchXml::LessThan);
            QDateTime dt(date);

            if (m_type == DateTime)
            {
                dt.setTime(m_secondTimeEdit->time());
            }
            else
            {
                dt = dt.addDays(1);    // include whole day
            }

            writer.writeValue(dt);
            writer.finishField();
        }
    }
}

void SearchFieldRangeDate::reset()
{
    m_firstDateEdit->setDate(QDate());

    if (m_type == DateTime)
    {
        m_firstTimeEdit->setTime(QTime(0, 0, 0, 0));
    }

    m_secondDateEdit->setDate(QDate());

    if (m_type == DateTime)
    {
        m_secondTimeEdit->setTime(QTime(0, 0, 0, 0));
    }

    valueChanged();
}

void SearchFieldRangeDate::setBoundary(const QDateTime& min, const QDateTime& max)
{
    //something here?
    Q_UNUSED(min);
    Q_UNUSED(max);
}

void SearchFieldRangeDate::setValueWidgetsVisible(bool visible)
{
    m_firstDateEdit->setVisible(visible);

    if (m_firstTimeEdit)
    {
        m_firstTimeEdit->setVisible(visible);
    }

    m_secondDateEdit->setVisible(visible);

    if (m_secondTimeEdit)
    {
        m_secondTimeEdit->setVisible(visible);
    }

    m_betweenLabel->setVisible(visible);
}

QList<QRect> SearchFieldRangeDate::valueWidgetRects() const
{
    QList<QRect> rects;
    rects << m_firstDateEdit->geometry();

    if (m_firstTimeEdit)
    {
        rects << m_firstTimeEdit->geometry();
    }

    rects << m_secondDateEdit->geometry();

    if (m_secondTimeEdit)
    {
        rects << m_secondTimeEdit->geometry();
    }

    return rects;
}

void SearchFieldRangeDate::valueChanged()
{
    setValidValueState(m_firstDateEdit->date().isValid() || m_secondDateEdit->date().isValid());
}

// -------------------------------------------------------------------------

SearchFieldRangeInt::SearchFieldRangeInt(QObject* const parent)
    : SearchField(parent),
      m_min(0),
      m_max(100),
      m_reciprocal(false),
      m_firstBox(0),
      m_secondBox(0)
{
    m_betweenLabel = new QLabel;
    m_firstBox     = new CustomStepsIntSpinBox;
    m_secondBox    = new CustomStepsIntSpinBox;
}

void SearchFieldRangeInt::setupValueWidgets(QGridLayout* layout, int row, int column)
{
    //     QHBoxLayout *hbox = new QHBoxLayout;
    //     layout->addLayout(hbox, row, column);

    m_firstBox->setSpecialValueText(" ");
    m_secondBox->setSpecialValueText(" ");

    //     hbox->addWidget(m_firstBox);
    //     hbox->addWidget(m_betweenLabel);
    //     hbox->addWidget(m_secondBox);
    //     hbox->addStretch(1);
    layout->addWidget(m_firstBox, row, column);
    layout->addWidget(m_betweenLabel, row, column + 1, Qt::AlignHCenter);
    layout->addWidget(m_secondBox, row, column + 2);

    connect(m_firstBox, SIGNAL(valueChanged(int)),
            this, SLOT(valueChanged()));

    connect(m_secondBox, SIGNAL(valueChanged(int)),
            this, SLOT(valueChanged()));
}

void SearchFieldRangeInt::read(SearchXmlCachingReader& reader)
{
    SearchXml::Relation relation = reader.fieldRelation();

    if (m_reciprocal)
    {
        switch (relation)
        {
            case SearchXml::LessThanOrEqual:
            case SearchXml::LessThan:
                m_firstBox->setFractionMagicValue(reader.valueToDouble());
                break;

            case SearchXml::GreaterThanOrEqual:
            case SearchXml::GreaterThan:
                m_secondBox->setFractionMagicValue(reader.valueToDouble());
                break;

            case SearchXml::Equal:
                m_firstBox->setFractionMagicValue(reader.valueToDouble());
                m_secondBox->setFractionMagicValue(reader.valueToDouble());
                break;

            case SearchXml::Interval:
            case SearchXml::IntervalOpen:
            {
                QList<double> list = reader.valueToDoubleList();

                if (list.size() != 2)
                {
                    return;
                }

                m_secondBox->setFractionMagicValue(list.first());
                m_firstBox->setFractionMagicValue(list.last());
                break;
            }

            default:
                break;
        }
    }
    else
    {
        switch (relation)
        {
            case SearchXml::GreaterThanOrEqual:
                m_firstBox->setValue(reader.valueToInt());
                break;

            case SearchXml::GreaterThan:
                m_firstBox->setValue(reader.valueToInt() - 1);
                break;

            case SearchXml::LessThanOrEqual:
                m_secondBox->setValue(reader.valueToInt());
                break;

            case SearchXml::LessThan:
                m_secondBox->setValue(reader.valueToInt() + 1);
                break;

            case SearchXml::Equal:
                m_firstBox->setValue(reader.valueToInt());
                m_secondBox->setValue(reader.valueToInt());
                break;

            case SearchXml::Interval:
            case SearchXml::IntervalOpen:
            {
                QList<int> list = reader.valueToIntList();

                if (list.size() != 2)
                {
                    return;
                }

                m_firstBox->setValue(list.first());
                m_secondBox->setValue(list.last());
                break;
            }

            default:
                break;
        }
    }
}

void SearchFieldRangeInt::write(SearchXmlWriter& writer)
{
    if (m_firstBox->value() != m_firstBox->minimum() && m_secondBox->value() != m_secondBox->minimum())
    {
        if (m_firstBox->value() != m_secondBox->value())
        {
            writer.writeField(m_name, SearchXml::Interval);

            if (m_reciprocal)
            {
                writer.writeValue(QList<float>() << m_secondBox->fractionMagicValue() << m_firstBox->fractionMagicValue());
            }
            else
            {
                writer.writeValue(QList<int>() << m_firstBox->value() << m_secondBox->value());
            }

            writer.finishField();
        }
        else
        {
            /**
            * @todo : This condition is never met.
            * Right value is either displayed empty (minimum, greater than left)
            * or one step larger than left
            */
            writer.writeField(m_name, SearchXml::Equal);

            if (m_reciprocal)
            {
                writer.writeValue(m_firstBox->fractionMagicValue());
            }
            else
            {
                writer.writeValue(m_firstBox->value());
            }

            writer.finishField();
        }
    }
    else
    {
        if (m_firstBox->value() != m_firstBox->minimum())
        {
            if (m_reciprocal)
            {
                writer.writeField(m_name, SearchXml::LessThanOrEqual);
                writer.writeValue(m_firstBox->fractionMagicValue());
            }
            else
            {
                writer.writeField(m_name, SearchXml::GreaterThanOrEqual);
                writer.writeValue(m_firstBox->value());
            }

            writer.finishField();
        }

        if (m_secondBox->value() != m_secondBox->minimum())
        {
            if (m_reciprocal)
            {
                writer.writeField(m_name, SearchXml::GreaterThanOrEqual);
                writer.writeValue(m_secondBox->fractionMagicValue());
            }
            else
            {
                writer.writeField(m_name, SearchXml::LessThanOrEqual);
                writer.writeValue(m_secondBox->value());
            }

            writer.finishField();
        }
    }
}

void SearchFieldRangeInt::setBetweenText(const QString& text)
{
    m_betweenLabel->setText(text);
}

void SearchFieldRangeInt::setNumberPrefixAndSuffix(const QString& prefix, const QString& suffix)
{
    m_firstBox->setPrefix(prefix);
    m_secondBox->setPrefix(prefix);
    m_firstBox->setSuffix(suffix);
    m_secondBox->setSuffix(suffix);
}

void SearchFieldRangeInt::setBoundary(int min, int max, int step)
{
    if (m_reciprocal)
    {
        m_min = max;
        m_max = min;
    }
    else
    {
        m_min = min;
        m_max = max;
    }

    m_firstBox->setRange(m_min, m_max);
    m_firstBox->setSingleStep(step);
    m_firstBox->setValue(m_min);

    m_secondBox->setRange(m_min, m_max);
    m_secondBox->setSingleStep(step);
    m_secondBox->setValue(m_min);
}

void SearchFieldRangeInt::enableFractionMagic(const QString& prefix)
{
    m_reciprocal = true;

    m_firstBox->enableFractionMagic(prefix);
    m_firstBox->setInvertStepping(true);

    m_secondBox->enableFractionMagic(prefix);
    m_secondBox->setInvertStepping(true);
}

void SearchFieldRangeInt::setSuggestedValues(const QList<int>& values)
{
    m_firstBox->setSuggestedValues(values);
    m_secondBox->setSuggestedValues(values);
}

void SearchFieldRangeInt::setSuggestedInitialValue(int value)
{
    m_firstBox->setSuggestedInitialValue(value);
    m_secondBox->setSuggestedInitialValue(value);
}

void SearchFieldRangeInt::setSingleSteps(int smaller, int larger)
{
    m_firstBox->setSingleSteps(smaller, larger);
    m_secondBox->setSingleSteps(smaller, larger);
}

void SearchFieldRangeInt::setInvertStepping(bool invert)
{
    m_firstBox->setInvertStepping(invert);
    m_secondBox->setInvertStepping(invert);
}

void SearchFieldRangeInt::valueChanged()
{
    bool validValue = false;

    if (m_reciprocal)
    {
        bool firstAtMinimum  = (m_firstBox->value()  == m_firstBox->minimum());
        bool secondAtMinimum = (m_secondBox->value() == m_secondBox->minimum());

        if (!secondAtMinimum)
        {
            m_firstBox->setRange(m_secondBox->value(), m_max);
            validValue = true;
        }

        if (!firstAtMinimum)
        {
            m_secondBox->setRange(m_min - 1, m_firstBox->value());

            if (secondAtMinimum)
            {
                m_firstBox->setRange(m_min, m_max);
                m_secondBox->setValue(m_secondBox->minimum());
            }

            validValue = true;
        }

        if (firstAtMinimum && secondAtMinimum)
        {
            m_firstBox->setRange(m_min, m_max);
            m_secondBox->setRange(m_min, m_max);
        }
    }
    else
    {
        bool firstAtMinimum  = (m_firstBox->value()  == m_firstBox->minimum());
        bool secondAtMinimum = (m_secondBox->value() == m_secondBox->minimum());

        if (!secondAtMinimum)
        {
            m_firstBox->setRange(m_min, m_secondBox->value());
            validValue = true;
        }

        if (!firstAtMinimum)
        {
            m_secondBox->setRange(m_firstBox->value(), m_max);

            if (secondAtMinimum)
            {
                m_firstBox->setRange(m_min, m_max);
                m_secondBox->setValue(m_secondBox->minimum());
            }

            validValue = true;
        }

        if (firstAtMinimum && secondAtMinimum)
        {
            m_firstBox->setRange(m_min, m_max);
            m_secondBox->setRange(m_min, m_max);
        }
    }

    setValidValueState(validValue);
}

void SearchFieldRangeInt::reset()
{
    m_firstBox->setRange(m_min, m_max);
    m_secondBox->setRange(m_min, m_max);
    m_firstBox->reset();
    m_secondBox->reset();
}

void SearchFieldRangeInt::setValueWidgetsVisible(bool visible)
{
    m_firstBox->setVisible(visible);
    m_secondBox->setVisible(visible);
    m_betweenLabel->setVisible(visible);
}

QList<QRect> SearchFieldRangeInt::valueWidgetRects() const
{
    QList<QRect> rects;
    rects << m_firstBox->geometry();
    rects << m_secondBox->geometry();
    return rects;
}

// -------------------------------------------------------------------------

SearchFieldRangeDouble::SearchFieldRangeDouble(QObject* const parent)
    : SearchField(parent),
      m_min(0),
      m_max(100),
      m_factor(1),
      m_firstBox(0),
      m_secondBox(0)
{
    m_betweenLabel = new QLabel;
    m_firstBox     = new CustomStepsDoubleSpinBox;
    m_secondBox    = new CustomStepsDoubleSpinBox;
}

void SearchFieldRangeDouble::setupValueWidgets(QGridLayout* layout, int row, int column)
{
    //     QHBoxLayout *hbox = new QHBoxLayout;
    //     layout->addLayout(hbox, row, column);

    m_firstBox->setSpecialValueText(" ");
    m_secondBox->setSpecialValueText(" ");

    /*    hbox->addWidget(m_firstBox);
        hbox->addWidget(m_betweenLabel);
        hbox->addWidget(m_secondBox);
        hbox->addStretch(1);*/
    layout->addWidget(m_firstBox,     row, column);
    layout->addWidget(m_betweenLabel, row, column + 1, Qt::AlignHCenter);
    layout->addWidget(m_secondBox,    row, column + 2);

    connect(m_firstBox, SIGNAL(valueChanged(double)),
            this, SLOT(valueChanged()));

    connect(m_secondBox, SIGNAL(valueChanged(double)),
            this, SLOT(valueChanged()));
}

void SearchFieldRangeDouble::read(SearchXmlCachingReader& reader)
{
    SearchXml::Relation relation = reader.fieldRelation();

    if (relation == SearchXml::GreaterThanOrEqual || relation == SearchXml::GreaterThan)
    {
        m_firstBox->setValue(reader.valueToDouble() / m_factor);
    }
    else if (relation == SearchXml::LessThanOrEqual || relation == SearchXml::LessThan)
    {
        m_secondBox->setValue(reader.valueToDouble() / m_factor);
    }
    else if (relation == SearchXml::Interval || relation == SearchXml::IntervalOpen)
    {
        QList<double> list = reader.valueToDoubleList();

        if (list.size() != 2)
        {
            return;
        }

        m_firstBox->setValue(list.first() / m_factor);
        m_secondBox->setValue(list.last() / m_factor);
    }
}

void SearchFieldRangeDouble::write(SearchXmlWriter& writer)
{
    if (m_firstBox->value() != m_firstBox->minimum() && m_secondBox->value() != m_secondBox->minimum())
    {
        if (m_firstBox->value() != m_secondBox->value())
        {
            writer.writeField(m_name, SearchXml::Interval);
            writer.writeValue(QList<double>() << (m_firstBox->value() * m_factor) << (m_secondBox->value() * m_factor));
            writer.finishField();
        }
        else
        {
            //TODO: See SearchFieldRangeInt
            writer.writeField(m_name, SearchXml::Equal);
            writer.writeValue(m_firstBox->value() * m_factor);
            writer.finishField();
        }
    }
    else
    {
        if (m_firstBox->value() != m_firstBox->minimum())
        {
            writer.writeField(m_name, SearchXml::GreaterThanOrEqual);
            writer.writeValue(m_firstBox->value() * m_factor);
            writer.finishField();
        }

        if (m_secondBox->value() != m_secondBox->minimum())
        {
            writer.writeField(m_name, SearchXml::LessThanOrEqual);
            writer.writeValue(m_secondBox->value() * m_factor);
            writer.finishField();
        }
    }
}

void SearchFieldRangeDouble::setBetweenText(const QString& text)
{
    m_betweenLabel->setText(text);
}

void SearchFieldRangeDouble::setNoValueText(const QString& text)
{
    m_firstBox->setSpecialValueText(text);
    m_secondBox->setSpecialValueText(text);
}

void SearchFieldRangeDouble::setNumberPrefixAndSuffix(const QString& prefix, const QString& suffix)
{
    m_firstBox->setPrefix(prefix);
    m_secondBox->setPrefix(prefix);
    m_firstBox->setSuffix(suffix);
    m_secondBox->setSuffix(suffix);
}

void SearchFieldRangeDouble::setBoundary(double min, double max, int decimals, double step)
{
    m_min = min;
    m_max = max;

    m_firstBox->setRange(min, max);
    m_firstBox->setSingleStep(step);
    m_firstBox->setDecimals(decimals);
    m_firstBox->setValue(min);

    m_secondBox->setRange(min, max);
    m_secondBox->setSingleStep(step);
    m_secondBox->setDecimals(decimals);
    m_secondBox->setValue(min);
}

void SearchFieldRangeDouble::setFactor(double factor)
{
    m_factor = factor;
}

void SearchFieldRangeDouble::setSuggestedValues(const QList<double>& values)
{
    m_firstBox->setSuggestedValues(values);
    m_secondBox->setSuggestedValues(values);
}

void SearchFieldRangeDouble::setSuggestedInitialValue(double value)
{
    m_firstBox->setSuggestedInitialValue(value);
    m_secondBox->setSuggestedInitialValue(value);
}

void SearchFieldRangeDouble::setSingleSteps(double smaller, double larger)
{
    m_firstBox->setSingleSteps(smaller, larger);
    m_secondBox->setSingleSteps(smaller, larger);
}

void SearchFieldRangeDouble::setInvertStepping(bool invert)
{
    m_firstBox->setInvertStepping(invert);
    m_secondBox->setInvertStepping(invert);
}

void SearchFieldRangeDouble::valueChanged()
{
    bool validValue      = false;
    bool firstAtMinimum  = (m_firstBox->value()  == m_firstBox->minimum());
    bool secondAtMinimum = (m_secondBox->value() == m_secondBox->minimum());

    if (!secondAtMinimum)
    {
        m_firstBox->setRange(m_min, m_secondBox->value());
        validValue = true;
    }

    if (!firstAtMinimum)
    {
        m_secondBox->setRange(m_firstBox->value(), m_max);

        if (secondAtMinimum)
        {
            m_firstBox->setRange(m_min, m_max);
            m_secondBox->setValue(m_secondBox->minimum());
        }

        validValue = true;
    }

    if (firstAtMinimum && secondAtMinimum)
    {
        m_firstBox->setRange(m_min, m_max);
        m_secondBox->setRange(m_min, m_max);
    }

    setValidValueState(validValue);
}

void SearchFieldRangeDouble::reset()
{
    m_firstBox->setRange(m_min, m_max);
    m_secondBox->setRange(m_min, m_max);
    m_firstBox->reset();
    m_secondBox->reset();
}

void SearchFieldRangeDouble::setValueWidgetsVisible(bool visible)
{
    m_firstBox->setVisible(visible);
    m_secondBox->setVisible(visible);
    m_betweenLabel->setVisible(visible);
}

QList<QRect> SearchFieldRangeDouble::valueWidgetRects() const
{
    QList<QRect> rects;
    rects << m_firstBox->geometry();
    rects << m_secondBox->geometry();
    return rects;
}

// -------------------------------------------------------------------------

SearchFieldChoice::SearchFieldChoice(QObject* const parent)
    : SearchField(parent),
      m_comboBox(0), m_type(QVariant::Invalid)
{
    m_model = new ChoiceSearchModel(this);
    m_anyText = i18n("Any");
}

void SearchFieldChoice::setupValueWidgets(QGridLayout* layout, int row, int column)
{
    m_comboBox = new ChoiceSearchComboBox;
    m_comboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    connect(m_model, SIGNAL(checkStateChanged(QVariant,bool)),
            this, SLOT(checkStateChanged()));

    m_comboBox->setModel(m_model);
    // set object name for style sheet
    m_comboBox->setObjectName("SearchFieldChoice_ComboBox");
    // label is created only after setting the model
    m_comboBox->label()->setObjectName("SearchFieldChoice_ClickLabel");
    updateComboText();

    layout->addWidget(m_comboBox, row, column, 1, 3);
}

void SearchFieldChoice::setChoice(const QMap<int, QString>& map)
{
    m_type = QVariant::Int;
    m_model->setChoice(map);
}

void SearchFieldChoice::setChoice(const QStringList& choice)
{
    m_type = QVariant::String;
    m_model->setChoice(choice);
}

void SearchFieldChoice::setAnyText(const QString& anyText)
{
    m_anyText = anyText;
}

void SearchFieldChoice::checkStateChanged()
{
    updateComboText();
}

void SearchFieldChoice::updateComboText()
{
    QStringList checkedChoices = m_model->checkedDisplayTexts();

    if (checkedChoices.isEmpty())
    {
        m_comboBox->setLabelText(m_anyText);
        setValidValueState(false);
    }
    else if (checkedChoices.count() == 1)
    {
        m_comboBox->setLabelText(checkedChoices.first());
        setValidValueState(true);
    }
    else
    {
        m_comboBox->setLabelText(i18n("Any of: %1", checkedChoices.join(", ")));
        setValidValueState(true);
    }
}

void SearchFieldChoice::read(SearchXmlCachingReader& reader)
{
    SearchXml::Relation relation = reader.fieldRelation();
    QList<int> values;

    if (relation == SearchXml::OneOf)
    {
        if (m_type == QVariant::Int)
        {
            m_model->setChecked<int>(reader.valueToIntList());
        }
        else if (m_type == QVariant::String)
        {
            m_model->setChecked<QString>(reader.valueToStringList());
        }
    }
    else
    {
        if (m_type == QVariant::Int)
        {
            m_model->setChecked<int>(reader.valueToInt(), relation);
        }
        else if (m_type == QVariant::String)
        {
            // The testRelation magic only really makes sense for integers. "Like" is not implemented.
            //m_model->setChecked<QString>(reader.value(), relation);
            m_model->setChecked<QString>(reader.value());
        }
    }
}

void SearchFieldChoice::write(SearchXmlWriter& writer)
{
    if (m_type == QVariant::Int)
    {
        QList<int> v = m_model->checkedKeys<int>();

        if (!v.isEmpty())
        {
            if (v.size() == 1)
            {
                writer.writeField(m_name, SearchXml::Equal);
                writer.writeValue(v.first());
                writer.finishField();
            }
            else
            {
                writer.writeField(m_name, SearchXml::OneOf);
                writer.writeValue(v);
                writer.finishField();
            }
        }
    }
    else if (m_type == QVariant::String)
    {
        QList<QString> v = m_model->checkedKeys<QString>();

        if (!v.isEmpty())
        {
            if (v.size() == 1)
            {
                // For choice string fields, we have the possibility to specify the wildcard
                // position with the position of *.
                if (v.first().contains("*"))
                {
                    writer.writeField(m_name, SearchXml::Like);
                }
                else
                {
                    writer.writeField(m_name, SearchXml::Equal);
                }

                writer.writeValue(v.first());
                writer.finishField();
            }
            else
            {
                // OneOf handles wildcards automatically
                writer.writeField(m_name, SearchXml::OneOf);
                writer.writeValue(v);
                writer.finishField();
            }
        }
    }
}

void SearchFieldChoice::reset()
{
    m_model->resetChecked();
}

void SearchFieldChoice::setValueWidgetsVisible(bool visible)
{
    m_comboBox->setVisible(visible);
}

QList<QRect> SearchFieldChoice::valueWidgetRects() const
{
    QList<QRect> rects;
    rects << m_comboBox->geometry();
    return rects;
}

/*
class SearchFieldChoice : public SearchField
{
    // Note: Someone added this space on purpose (Marcel?)
    // It seems that automoc4 is not recognizing this macro to be in a comment
    // block and therefore will fail when parsing this file. Adding a space to the
    // macro name will fix this issue. When uncommenting this block again, make sure to
    // fix the macro name of course.

    Q_OBJ ECT

public:

    SearchFieldChoice(SearchFieldGroup *parent);

    virtual void read(SearchXmlCachingReader &reader);
    virtual void write(SearchXmlWriter &writer);
    virtual void reset();

    void setChoice(const QMap<int, QString> &map);
    void setAnyText(const QString &string);

    virtual void setupValueWidgets(QGridLayout *layout, int row, int column);
    virtual void setValueWidgetsVisible(bool visible);

protected Q_SLOTS:

    void slotClicked();
    void slotUpdateLabel();

protected:

    void setValues(const QList<int> &values);
    void setValues(int value, SearchXml::Relation relation);

    QList<int> values() const;
    QString valueText() const;

    virtual void setupChoiceWidgets();

protected:

    QString                    m_anyText;
    SqueezedClickLabel        *m_label;
    QVBoxLayout               *m_vbox;
    QMap<int, QString>         m_choiceMap;
    QMap<QCheckBox*, int>      m_widgetMap;
    VisibilityController      *m_controller;
};

SearchFieldChoice::SearchFieldChoice(SearchFieldGroup *parent)
    : SearchField(parent), m_vbox(0)
{
    m_anyText = i18n("Any");
    m_label = new SqueezedClickLabel;
    m_label->setObjectName("SearchFieldChoice_ClickLabel");
    m_controller = new VisibilityController(this);
    m_controller->setContainerWidget(parent);
}

void SearchFieldChoice::setupValueWidgets(QGridLayout *layout, int row, int column)
{
    m_vbox = new QVBoxLayout;
    layout->addLayout(m_vbox, row, column, 1, 3);

    m_label->setTextElideMode(Qt::ElideRight);
    m_vbox->addWidget(m_label);

    connect(m_label, SIGNAL(activated()),
            this, SLOT(slotClicked()));

    setupChoiceWidgets();
    slotUpdateLabel();
}

void SearchFieldChoice::slotClicked()
{
    m_controller->triggerVisibility();
}

void SearchFieldChoice::slotUpdateLabel()
{
    QString text = valueText();
    if (text.isNull())
        text = m_anyText;
    m_label->setText(text);
}

void SearchFieldChoice::setValueWidgetsVisible(bool visible)
{
    m_label->setVisible(visible);
    if (!visible)
        m_controller->hide();
}

void SearchFieldChoice::setupChoiceWidgets()
{
    QGroupBox *groupbox = new QGroupBox;
    m_vbox->addWidget(groupbox);
    m_controller->addWidget(groupbox);
    QVBoxLayout *vbox = new QVBoxLayout;

    QMap<int, QString>::const_iterator it;
    for (it = m_choiceMap.begin(); it != m_choiceMap.end(); ++it)
    {
        QCheckBox *box = new QCheckBox;
        box->setText(it.value());
        vbox->addWidget(box);
        m_controller->addWidget(box);
        m_widgetMap[box] = it.key();

        connect(box, SIGNAL(stateChanged(int)),
                this, SLOT(slotUpdateLabel()));
    }

    groupbox->setLayout(vbox);
}

QString SearchFieldChoice::valueText() const
{
    QStringList list;
    QMap<QCheckBox*, int>::const_iterator it;
    for (it = m_widgetMap.begin(); it != m_widgetMap.end(); ++it)
    {
        if (it.key()->isChecked())
            list << it.key()->text();
    }
    if (list.isEmpty())
        return QString();
    else if (list.size() == 1)
    {
        return list.first();
    }
    else
    {
        return i18n("Either of: %1", list.join(", "));
    }
}

void SearchFieldChoice::read(SearchXmlCachingReader& reader)
{
    SearchXml::Relation relation = reader.fieldRelation();
    QList<int> values;
    if (relation == SearchXml::OneOf)
    {
        setValues(reader.valueToIntList());
    }
    else
    {
        setValues(reader.valueToInt(), relation);
    }
}

void SearchFieldChoice::write(SearchXmlWriter& writer)
{
    QList<int> v = values();
    if (!v.isEmpty())
    {
        if (v.size() == 1)
        {
            writer.writeField(m_name, SearchXml::Equal);
            writer.writeValue(v.first());
            writer.finishField();
        }
        else
        {
            writer.writeField(m_name, SearchXml::OneOf);
            writer.writeValue(v);
            writer.finishField();
        }
    }
}

void SearchFieldChoice::reset()
{
    setValues(QList<int>());
}

void SearchFieldChoice::setChoice(const QMap<int, QString>& map)
{
    m_choiceMap = map;
}

void SearchFieldChoice::setValues(const QList<int>& values)
{
    QMap<QCheckBox*, int>::const_iterator it;
    for (it = m_widgetMap.begin(); it != m_widgetMap.end(); ++it)
    {
        it.key()->setChecked(values.contains(it.value()));
    }
}

void SearchFieldChoice::setValues(int value, SearchXml::Relation relation)
{
    QMap<QCheckBox*, int>::const_iterator it;
    for (it = m_widgetMap.begin(); it != m_widgetMap.end(); ++it)
    {
        it.key()->setChecked(SearchXml::testRelation(it.value(), value, relation));
    }
}

QList<int> SearchFieldChoice::values() const
{
    QList<int> list;
    QMap<QCheckBox*, int>::const_iterator it;
    for (it = m_widgetMap.begin(); it != m_widgetMap.end(); ++it)
    {
        if (it.key()->isChecked())
            list << it.value();
    }
    return list;
}
*/

// -------------------------------------------------------------------------

SearchFieldAlbum::SearchFieldAlbum(QObject* const parent, Type type)
    : SearchField(parent),
      m_comboBox(0),
      m_type(type),
      m_model(0)
{
}

void SearchFieldAlbum::setupValueWidgets(QGridLayout* layout, int row, int column)
{
    m_comboBox = new AlbumSelectComboBox;
    m_comboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    if (m_type == TypeAlbum)
    {
        m_comboBox->setDefaultAlbumModel();
        m_comboBox->setNoSelectionText(i18n("Any Album"));
    }
    else if (m_type == TypeTag)
    {
        m_comboBox->setDefaultTagModel();
        m_comboBox->setNoSelectionText(i18n("Any Tag"));
    }

    m_model = m_comboBox->model();

    connect(m_model, SIGNAL(checkStateChanged(Album*,Qt::CheckState)),
            this, SLOT(updateState()));

    updateState();

    layout->addWidget(m_comboBox, row, column, 1, 3);
}

void SearchFieldAlbum::updateState()
{
    setValidValueState(!m_model->checkedAlbums().isEmpty());
}

void SearchFieldAlbum::read(SearchXmlCachingReader& reader)
{
    QList<int> ids = reader.valueToIntOrIntList();

    foreach(int id, ids)
    {
        Album* a = 0;

        if (m_type == TypeAlbum)
        {
            a = AlbumManager::instance()->findPAlbum(id);
        }
        else if (m_type == TypeTag)
        {
            a = AlbumManager::instance()->findTAlbum(id);

            // Ignore internal tags here.
            if (a && TagsCache::instance()->isInternalTag(a->id()))
            {
                a = 0;
            }
        }

        if (!a)
        {
            kDebug() << "Search: Did not find album for ID" << id << "given in Search XML";
            return;
        }

        m_model->setChecked(a, true);
    }
}

void SearchFieldAlbum::write(SearchXmlWriter& writer)
{
    AlbumList checkedAlbums = m_model->checkedAlbums();

    if (checkedAlbums.isEmpty())
    {
        return;
    }

    QList<int> albumIds;

    foreach(Album* const album, checkedAlbums)
    {
        albumIds << album->id();
    }

    writer.writeField(m_name, SearchXml::InTree);

    if (albumIds.size() > 1)
    {
        writer.writeValue(albumIds);
    }
    else
    {
        writer.writeValue(albumIds.first());
    }

    writer.finishField();
}

void SearchFieldAlbum::reset()
{
    m_model->resetCheckedAlbums();
}

void SearchFieldAlbum::setValueWidgetsVisible(bool visible)
{
    m_comboBox->setVisible(visible);
}

QList<QRect> SearchFieldAlbum::valueWidgetRects() const
{
    QList<QRect> rects;
    rects << m_comboBox->geometry();
    return rects;
}

// -------------------------------------------------------------------------

SearchFieldRating::SearchFieldRating(QObject* const parent)
    : SearchField(parent)
{
    m_betweenLabel = new QLabel;
    m_firstBox     = new RatingComboBox;
    m_secondBox    = new RatingComboBox;
}

void SearchFieldRating::setupValueWidgets(QGridLayout* layout, int row, int column)
{
    layout->addWidget(m_firstBox,     row, column);
    layout->addWidget(m_betweenLabel, row, column + 1, Qt::AlignHCenter);
    layout->addWidget(m_secondBox,    row, column + 2);

    connect(m_firstBox, SIGNAL(ratingValueChanged(int)),
            this, SLOT(firstValueChanged()));

    connect(m_secondBox, SIGNAL(ratingValueChanged(int)),
            this, SLOT(secondValueChanged()));
}

void SearchFieldRating::read(SearchXmlCachingReader& reader)
{
    SearchXml::Relation relation = reader.fieldRelation();

    switch (relation)
    {
        case SearchXml::GreaterThanOrEqual:
            m_firstBox->setRatingValue((RatingComboBox::RatingValue)reader.valueToInt());
            break;

        case SearchXml::GreaterThan:
            m_firstBox->setRatingValue((RatingComboBox::RatingValue)(reader.valueToInt() - 1));
            break;

        case SearchXml::LessThanOrEqual:
            m_secondBox->setRatingValue((RatingComboBox::RatingValue)reader.valueToInt());
            break;

        case SearchXml::LessThan:
            m_secondBox->setRatingValue((RatingComboBox::RatingValue)(reader.valueToInt() + 1));
            break;

        case SearchXml::Equal:
            m_firstBox->setRatingValue((RatingComboBox::RatingValue)reader.valueToInt());
            m_secondBox->setRatingValue((RatingComboBox::RatingValue)reader.valueToInt());
            break;

        case SearchXml::Interval:
        case SearchXml::IntervalOpen:
        {
            QList<int> list = reader.valueToIntList();

            if (list.size() != 2)
            {
                return;
            }

            m_firstBox->setRatingValue((RatingComboBox::RatingValue)list.first());
            m_secondBox->setRatingValue((RatingComboBox::RatingValue)list.last());
            break;
        }

        default:
            break;
    }
}

void SearchFieldRating::write(SearchXmlWriter& writer)
{
    RatingComboBox::RatingValue first  = m_firstBox->ratingValue();
    RatingComboBox::RatingValue second = m_secondBox->ratingValue();

    if (first == RatingComboBox::NoRating)
    {
        writer.writeField(m_name, SearchXml::Equal);
        writer.writeValue(-1);
        writer.finishField();
    }
    else if (first != RatingComboBox::Null && first == second)
    {
        writer.writeField(m_name, SearchXml::Equal);
        writer.writeValue(first);
        writer.finishField();
    }
    else if (first != RatingComboBox::Null && second != RatingComboBox::Null)
    {
        writer.writeField(m_name, SearchXml::Interval);
        writer.writeValue(QList<int>() << first << second);
        writer.finishField();
    }
    else
    {
        if (first != RatingComboBox::Null)
        {
            writer.writeField(m_name, SearchXml::GreaterThanOrEqual);
            writer.writeValue(first);
            writer.finishField();
        }

        if (second != RatingComboBox::Null)
        {
            writer.writeField(m_name, SearchXml::LessThanOrEqual);
            writer.writeValue(second);
            writer.finishField();
        }
    }
}

void SearchFieldRating::setBetweenText(const QString& text)
{
    m_betweenLabel->setText(text);
}

void SearchFieldRating::firstValueChanged()
{
    RatingComboBox::RatingValue first  = m_firstBox->ratingValue();
    RatingComboBox::RatingValue second = m_secondBox->ratingValue();

    if (first == RatingComboBox::NoRating)
    {
        m_secondBox->setRatingValue(RatingComboBox::Null);
        m_secondBox->setEnabled(false);
    }
    else
    {
        m_secondBox->setEnabled(true);
    }

    if (first >= RatingComboBox::Rating0 && first <= RatingComboBox::Rating5)
    {
        if (first > second)
        {
            m_secondBox->setRatingValue(RatingComboBox::Null);
        }
    }

    setValidValueState(first != RatingComboBox::Null || second != RatingComboBox::Null);
}

void SearchFieldRating::secondValueChanged()
{
    RatingComboBox::RatingValue first  = m_firstBox->ratingValue();
    RatingComboBox::RatingValue second = m_secondBox->ratingValue();

    // NoRating is not possible for the second box

    if (second >= RatingComboBox::Rating0 && second <= RatingComboBox::Rating5)
    {
        if (first > second)
        {
            m_firstBox->setRatingValue(second);
        }
    }

    setValidValueState(first != RatingComboBox::Null || second != RatingComboBox::Null);
}

void SearchFieldRating::reset()
{
    m_firstBox->setRatingValue(RatingComboBox::Null);
    m_secondBox->setRatingValue(RatingComboBox::Null);
}

void SearchFieldRating::setValueWidgetsVisible(bool visible)
{
    m_firstBox->setVisible(visible);
    m_secondBox->setVisible(visible);
    m_betweenLabel->setVisible(visible);
}

QList<QRect> SearchFieldRating::valueWidgetRects() const
{
    QList<QRect> rects;
    rects << m_firstBox->geometry();
    rects << m_secondBox->geometry();
    return rects;
}

// -------------------------------------------------------------------------

SearchFieldComboBox::SearchFieldComboBox(QObject* const parent)
    : SearchField(parent),
      m_comboBox(0)
{
}

void SearchFieldComboBox::setupValueWidgets(QGridLayout* layout, int row, int column)
{
    m_comboBox = new KComboBox;
    m_comboBox->setEditable(false);
    layout->addWidget(m_comboBox, row, column, 1, 3);

    connect(m_comboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(indexChanged(int)));
}

void SearchFieldComboBox::write(SearchXmlWriter& writer)
{
    int index = m_comboBox->currentIndex();

    if (index != -1)
    {
        QVariant bits = m_comboBox->itemData(index);

        if (!bits.isNull())
        {
            writer.writeField(m_name, SearchXml::Equal);
            writer.writeValue(bits.toInt());
            writer.finishField();
        }
    }
}

void SearchFieldComboBox::setValueWidgetsVisible(bool visible)
{
    m_comboBox->setVisible(visible);
}

void SearchFieldComboBox::reset()
{
    m_comboBox->setCurrentIndex(0);
}

QList<QRect> SearchFieldComboBox::valueWidgetRects() const
{
    QList<QRect> rects;
    rects << m_comboBox->geometry();
    return rects;
}

void SearchFieldComboBox::indexChanged(int index)
{
    setValidValueState(index != 0);
}

// -------------------------------------------------------------------------

SearchFieldCheckBox::SearchFieldCheckBox(QObject* const parent)
    : SearchField(parent),
      m_checkBox(0)
{
}

void SearchFieldCheckBox::setupValueWidgets(QGridLayout* layout, int row, int column)
{
    m_checkBox = new QCheckBox(m_text);
    layout->addWidget(m_checkBox, row, column, 1, 3);

    connect(m_checkBox, SIGNAL(toggled(bool)),
            this, SLOT(slotToggled(bool)));
}

void SearchFieldCheckBox::setLabel(const QString& text)
{
    m_text = text;

    if (m_checkBox)
    {
        m_checkBox->setText(m_text);
    }
}

void SearchFieldCheckBox::write(SearchXmlWriter& writer)
{
    if (m_checkBox->isChecked())
    {
        writer.writeField(m_name, SearchXml::Equal);
        writer.finishField();
    }
}

void SearchFieldCheckBox::read(SearchXmlCachingReader& reader)
{
    SearchXml::Relation relation = reader.fieldRelation();
    reader.readToEndOfElement();

    if (relation == SearchXml::Equal)
    {
        m_checkBox->setChecked(true);
    }
}

void SearchFieldCheckBox::setValueWidgetsVisible(bool visible)
{
    m_checkBox->setVisible(visible);
}

void SearchFieldCheckBox::reset()
{
    m_checkBox->setChecked(false);
}

QList<QRect> SearchFieldCheckBox::valueWidgetRects() const
{
    QList<QRect> rects;
    rects << m_checkBox->geometry();
    return rects;
}

void SearchFieldCheckBox::slotToggled(bool checked)
{
    setValidValueState(checked);
}

// -------------------------------------------------------------------------

SearchFieldColorDepth::SearchFieldColorDepth(QObject* const parent)
    : SearchFieldComboBox(parent)
{
}

void SearchFieldColorDepth::setupValueWidgets(QGridLayout* layout, int row, int column)
{
    SearchFieldComboBox::setupValueWidgets(layout, row, column);
    m_comboBox->addItem(i18n("any color depth"));
    m_comboBox->addItem(i18n("8 bits per channel"), 8);
    m_comboBox->addItem(i18n("16 bits per channel"), 16);

    m_comboBox->setCurrentIndex(0);
}

void SearchFieldColorDepth::read(SearchXmlCachingReader& reader)
{
    SearchXml::Relation relation = reader.fieldRelation();

    if (relation == SearchXml::Equal)
    {
        int bits = reader.valueToInt();

        if (bits == 8)
        {
            m_comboBox->setCurrentIndex(1);
        }
        else if (bits == 16)
        {
            m_comboBox->setCurrentIndex(2);
        }
    }
}

// -------------------------------------------------------------------------

SearchFieldPageOrientation::SearchFieldPageOrientation(QObject* const parent)
    : SearchFieldComboBox(parent)
{
}

void SearchFieldPageOrientation::setupValueWidgets(QGridLayout* layout, int row, int column)
{
    SearchFieldComboBox::setupValueWidgets(layout, row, column);
    m_comboBox->addItem(i18n("Any Orientation"));
    m_comboBox->addItem(i18n("Landscape Orientation"), 1);
    m_comboBox->addItem(i18n("Portrait orientation"), 2);

    m_comboBox->setCurrentIndex(0);
}

void SearchFieldPageOrientation::read(SearchXmlCachingReader& reader)
{
    SearchXml::Relation relation = reader.fieldRelation();

    if (relation == SearchXml::Equal)
    {
        int value = reader.valueToInt();

        if (value == 1)
        {
            m_comboBox->setCurrentIndex(1);
        }
        else if (value == 2)
        {
            m_comboBox->setCurrentIndex(2);
        }
    }
}

// -------------------------------------------------------------------------

SearchFieldLabels::SearchFieldLabels(QObject* const parent)
    : SearchField(parent),
      m_pickLabelFilter(0),
      m_colorLabelFilter(0)
{
}

void SearchFieldLabels::setupValueWidgets(QGridLayout* layout, int row, int column)
{
    QHBoxLayout* hbox  = new QHBoxLayout;
    m_pickLabelFilter  = new PickLabelFilter;
    m_colorLabelFilter = new ColorLabelFilter;
    hbox->addWidget(m_pickLabelFilter);
    hbox->addStretch(10);
    hbox->addWidget(m_colorLabelFilter);

    connect(m_pickLabelFilter, SIGNAL(signalPickLabelSelectionChanged(QList<PickLabel>)),
            this, SLOT(updateState()));

    connect(m_colorLabelFilter, SIGNAL(signalColorLabelSelectionChanged(QList<ColorLabel>)),
            this, SLOT(updateState()));

    updateState();

    layout->addLayout(hbox, row, column, 1, 3);
}

void SearchFieldLabels::updateState()
{
    setValidValueState(!m_colorLabelFilter->colorLabels().isEmpty());
}

void SearchFieldLabels::read(SearchXmlCachingReader& reader)
{
    TAlbum* a      = 0;
    QList<int> ids = reader.valueToIntOrIntList();
    QList<ColorLabel> clabels;
    QList<PickLabel>  plabels;

    foreach(int id, ids)
    {
        a = AlbumManager::instance()->findTAlbum(id);

        if (!a)
        {
            kDebug() << "Search: Did not find Label album for ID" << id << "given in Search XML";
        }
        else
        {
            int cl = TagsCache::instance()->colorLabelForTag(a->id());

            if (cl != -1)
            {
                clabels.append((ColorLabel)cl);
            }
            else
            {
                int pl = TagsCache::instance()->pickLabelForTag(a->id());

                if (pl != -1)
                {
                    plabels.append((PickLabel)pl);
                }
            }
        }
    }

    m_colorLabelFilter->setColorLabels(clabels);
    m_pickLabelFilter->setPickLabels(plabels);
}

void SearchFieldLabels::write(SearchXmlWriter& writer)
{
    QList<int>     albumIds;
    QList<TAlbum*> clAlbums = m_colorLabelFilter->getCheckedColorLabelTags();

    if (!clAlbums.isEmpty())
    {
        foreach(TAlbum* const album, clAlbums)
        {
            albumIds << album->id();
        }
    }

    QList<TAlbum*> plAlbums = m_pickLabelFilter->getCheckedPickLabelTags();

    if (!plAlbums.isEmpty())
    {
        foreach(TAlbum* const album, plAlbums)
        {
            albumIds << album->id();
        }
    }

    if (albumIds.isEmpty())
    {
        return;
    }

    // NOTE: there is no XML database query rule for Color Labels in ImageQueryBuilder::buildField()
    //       As Color Labels are internal tags, we trig database on "tagid".
    writer.writeField("tagid", SearchXml::InTree);

    if (albumIds.size() > 1)
    {
        writer.writeValue(albumIds);
    }
    else
    {
        writer.writeValue(albumIds.first());
    }

    writer.finishField();
}

void SearchFieldLabels::reset()
{
    m_colorLabelFilter->reset();
    m_pickLabelFilter->reset();
}

void SearchFieldLabels::setValueWidgetsVisible(bool visible)
{
    m_colorLabelFilter->setVisible(visible);
    m_pickLabelFilter->setVisible(visible);
}

QList<QRect> SearchFieldLabels::valueWidgetRects() const
{
    QList<QRect> rects;
    rects << m_pickLabelFilter->geometry();
    rects << m_colorLabelFilter->geometry();
    return rects;
}

} // namespace Digikam
