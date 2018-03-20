/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-02-13
 * Description : slide show settings container.
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "slideshowsettings.h"

// KDE includes

#include <kconfiggroup.h>
#include <ksharedconfig.h>

// Local includes

#include "metadatasettings.h"

namespace Digikam
{

const QString SlideShowSettings::configGroupName(QLatin1String("ImageViewer Settings"));
const QString SlideShowSettings::configSlideShowStartCurrentEntry(QLatin1String("SlideShowStartCurrent"));
const QString SlideShowSettings::configSlideShowDelayEntry(QLatin1String("SlideShowDelay"));
const QString SlideShowSettings::configSlideShowLoopEntry(QLatin1String("SlideShowLoop"));
const QString SlideShowSettings::configSlideShowPrintApertureFocalEntry(QLatin1String("SlideShowPrintApertureFocal"));
const QString SlideShowSettings::configSlideShowPrintCommentEntry(QLatin1String("SlideShowPrintComment"));
const QString SlideShowSettings::configSlideShowPrintTitleEntry(QLatin1String("SlideShowPrintTitle"));
const QString SlideShowSettings::configSlideShowPrintCapIfNoTitleEntry(QLatin1String("SlideShowPrintCapIfNoTitle"));
const QString SlideShowSettings::configSlideShowPrintDateEntry(QLatin1String("SlideShowPrintDate"));
const QString SlideShowSettings::configSlideShowPrintExpoSensitivityEntry(QLatin1String("SlideShowPrintExpoSensitivity"));
const QString SlideShowSettings::configSlideShowPrintMakeModelEntry(QLatin1String("SlideShowPrintMakeModel"));
const QString SlideShowSettings::configSlideShowPrintNameEntry(QLatin1String("SlideShowPrintName"));
const QString SlideShowSettings::configSlideShowPrintTagsEntry(QLatin1String("SlideShowPrintTags"));
const QString SlideShowSettings::configSlideShowPrintLabelsEntry(QLatin1String("SlideShowPrintLabels"));
const QString SlideShowSettings::configSlideShowProgressIndicatorEntry(QLatin1String("SlideShowProgressIndicator"));
const QString SlideShowSettings::configSlideScreenEntry(QLatin1String("SlideScreen"));

SlideShowSettings::SlideShowSettings()
{
    startWithCurrent      = false;
    exifRotate            = true;
    loop                  = false;
    delay                 = 5;
    printName             = true;
    printDate             = false;
    printComment          = false;
    printTitle            = false;
    printCapIfNoTitle     = false;
    printLabels           = false;
    printApertureFocal    = false;
    printMakeModel        = false;
    printExpoSensitivity  = false;
    printTags             = false;
    showProgressIndicator = true;
    slideScreen           = -2;
    autoPlayEnabled       = true;
}

SlideShowSettings::~SlideShowSettings()
{
}

void SlideShowSettings::readFromConfig()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroupName);
    startWithCurrent          = group.readEntry(configSlideShowStartCurrentEntry,         false);
    delay                     = group.readEntry(configSlideShowDelayEntry,                5);
    loop                      = group.readEntry(configSlideShowLoopEntry,                 false);
    printName                 = group.readEntry(configSlideShowPrintNameEntry,            true);
    printDate                 = group.readEntry(configSlideShowPrintDateEntry,            false);
    printApertureFocal        = group.readEntry(configSlideShowPrintApertureFocalEntry,   false);
    printExpoSensitivity      = group.readEntry(configSlideShowPrintExpoSensitivityEntry, false);
    printMakeModel            = group.readEntry(configSlideShowPrintMakeModelEntry,       false);
    printComment              = group.readEntry(configSlideShowPrintCommentEntry,         false);
    printTitle                = group.readEntry(configSlideShowPrintTitleEntry,           false);
    printCapIfNoTitle         = group.readEntry(configSlideShowPrintCapIfNoTitleEntry,    false);
    printTags                 = group.readEntry(configSlideShowPrintTagsEntry,            false);
    printLabels               = group.readEntry(configSlideShowPrintLabelsEntry,          false);
    showProgressIndicator     = group.readEntry(configSlideShowProgressIndicatorEntry,    true);
    slideScreen               = group.readEntry(configSlideScreenEntry,                   -2);
    exifRotate                = MetadataSettings::instance()->settings().exifRotate;
}

void SlideShowSettings::writeToConfig()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroupName);
    group.writeEntry(configSlideShowStartCurrentEntry,         startWithCurrent);
    group.writeEntry(configSlideShowDelayEntry,                delay);
    group.writeEntry(configSlideShowLoopEntry,                 loop);
    group.writeEntry(configSlideShowPrintNameEntry,            printName);
    group.writeEntry(configSlideShowPrintDateEntry,            printDate);
    group.writeEntry(configSlideShowPrintApertureFocalEntry,   printApertureFocal);
    group.writeEntry(configSlideShowPrintExpoSensitivityEntry, printExpoSensitivity);
    group.writeEntry(configSlideShowPrintMakeModelEntry,       printMakeModel);
    group.writeEntry(configSlideShowPrintCommentEntry,         printComment);
    group.writeEntry(configSlideShowPrintTitleEntry,           printTitle);
    group.writeEntry(configSlideShowPrintCapIfNoTitleEntry,    printCapIfNoTitle);
    group.writeEntry(configSlideShowPrintTagsEntry,            printTags);
    group.writeEntry(configSlideShowPrintLabelsEntry,          printLabels);
    group.writeEntry(configSlideShowProgressIndicatorEntry,    showProgressIndicator);
    group.writeEntry(configSlideScreenEntry,                   slideScreen);
    group.sync();
}

int SlideShowSettings::indexOf(const QUrl& url) const
{
    return fileList.indexOf(url);
}

int SlideShowSettings::count() const
{
    return fileList.count();
}

}   // namespace Digikam
