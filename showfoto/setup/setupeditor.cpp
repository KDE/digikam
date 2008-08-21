/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-02
 * Description : setup showfoto tab.
 *
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008 by Arnd Baecker <arnd dot baecker at web dot de>
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

// Qt includes.

#include <qlayout.h>
#include <qcolor.h>
#include <qhbox.h>
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qcheckbox.h>
#include <qcombobox.h>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>
#include <kcolorbutton.h>
#include <knuminput.h>
#include <kconfig.h>
#include <kapplication.h>

// Local includes.

#include "setupeditor.h"
#include "setupeditor.moc"

namespace ShowFoto
{

class SetupEditorPriv
{
public:

    SetupEditorPriv()
    {
        backgroundColor       = 0;
        hideToolBar           = 0;
        hideThumbBar          = 0;
        horizontalThumbBar    = 0;
        showSplash            = 0;
        useTrash              = 0;
        exifRotateBox         = 0;
        exifSetOrientationBox = 0;
        overExposureColor     = 0;
        underExposureColor    = 0;
        themebackgroundColor  = 0;
        colorBox              = 0;
        sortOrderComboBox     = 0;
        sortReverse           = 0;
        useRawImportTool      = 0;
    }

    QHBox        *colorBox;

    QCheckBox    *sortReverse;
    QCheckBox    *hideToolBar;
    QCheckBox    *hideThumbBar;
    QCheckBox    *horizontalThumbBar;
    QCheckBox    *showSplash;
    QCheckBox    *useTrash;
    QCheckBox    *exifRotateBox;
    QCheckBox    *exifSetOrientationBox;
    QCheckBox    *themebackgroundColor;
    QCheckBox    *useRawImportTool;

    QComboBox    *sortOrderComboBox;

    KColorButton *backgroundColor;
    KColorButton *underExposureColor;
    KColorButton *overExposureColor;
};

SetupEditor::SetupEditor(QWidget* parent )
           : QWidget(parent)
{
    d = new SetupEditorPriv;
    QVBoxLayout *layout = new QVBoxLayout( parent, 0, KDialog::spacingHint() );

    // --------------------------------------------------------

    QVGroupBox *interfaceOptionsGroup = new QVGroupBox(i18n("Interface Options"), parent);

    d->themebackgroundColor = new QCheckBox(i18n("&Use theme background color"),
                                            interfaceOptionsGroup);

    QWhatsThis::add( d->themebackgroundColor, i18n("<p>Enable this option to use background theme "
                                              "color in image editor area") );

    d->colorBox                  = new QHBox(interfaceOptionsGroup);
    QLabel *backgroundColorlabel = new QLabel( i18n("&Background color:"), d->colorBox);
    d->backgroundColor           = new KColorButton(d->colorBox);
    backgroundColorlabel->setBuddy(d->backgroundColor);
    QWhatsThis::add( d->backgroundColor, i18n("<p>Select background color to use "
                                              "for image editor area.") );

    d->hideToolBar        = new QCheckBox(i18n("H&ide toolbar in fullscreen mode"), interfaceOptionsGroup);
    d->hideThumbBar       = new QCheckBox(i18n("Hide &thumbbar in fullscreen mode"), interfaceOptionsGroup);
    d->horizontalThumbBar = new QCheckBox(i18n("Use &horizontal thumbbar (need to restart showFoto)"), interfaceOptionsGroup);
    QWhatsThis::add( d->horizontalThumbBar, i18n("<p>If this option is enabled, the thumbnails bar will be displayed horizontally behind "
                                                 "the image area. You need to restart showFoto for this option take effect.<p>"));
    d->useTrash   = new QCheckBox(i18n("&Deleting items should move them to trash"), interfaceOptionsGroup);
    d->showSplash = new QCheckBox(i18n("&Show splash screen at startup"), interfaceOptionsGroup);

    d->useRawImportTool = new QCheckBox(i18n("Use Raw Import Tool to handle Raw image"), interfaceOptionsGroup);
    QWhatsThis::add(d->useRawImportTool, i18n("<p>Set on this option to use Raw Import "
                                              "tool before to load a Raw image, "
                                              "to customize indeep decoding settings."));

    // --------------------------------------------------------

    QVGroupBox *exposureOptionsGroup = new QVGroupBox(i18n("Exposure Indicators"), parent);

    QHBox *underExpoBox         = new QHBox(exposureOptionsGroup);
    QLabel *underExpoColorlabel = new QLabel( i18n("&Under-exposure color:"), underExpoBox);
    d->underExposureColor       = new KColorButton(underExpoBox);
    underExpoColorlabel->setBuddy(d->underExposureColor);
    QWhatsThis::add( d->underExposureColor, i18n("<p>Customize color used in image editor to identify "
                                                 "under-exposed pixels.") );

    QHBox *overExpoBox         = new QHBox(exposureOptionsGroup);
    QLabel *overExpoColorlabel = new QLabel( i18n("&Over-exposure color:"), overExpoBox);
    d->overExposureColor       = new KColorButton(overExpoBox);
    overExpoColorlabel->setBuddy(d->overExposureColor);
    QWhatsThis::add( d->overExposureColor, i18n("<p>Customize color used in image editor to identify "
                                                "over-exposed pixels.") );

    // --------------------------------------------------------

    QVGroupBox *ExifGroupOptions = new QVGroupBox(i18n("EXIF Actions"), parent);

    d->exifRotateBox = new QCheckBox(ExifGroupOptions);
    d->exifRotateBox->setText(i18n("Show images/thumbs &rotated according to orientation tag"));

    d->exifSetOrientationBox = new QCheckBox(ExifGroupOptions);
    d->exifSetOrientationBox->setText(i18n("Set orientation tag to normal after rotate/flip"));

     // --------------------------------------------------------

    QVGroupBox *sortOptionsGroup = new QVGroupBox(i18n("Sort order for images"), parent);

    QHBox* sortBox       = new QHBox(sortOptionsGroup);
    new QLabel(i18n("Sort images by:"), sortBox);
    d->sortOrderComboBox = new QComboBox(false, sortBox);
    d->sortOrderComboBox->insertItem(i18n("File Date"), 0);
    d->sortOrderComboBox->insertItem(i18n("File Name"), 1);
    d->sortOrderComboBox->insertItem(i18n("File size"), 2);
    QWhatsThis::add(d->sortOrderComboBox, i18n("<p>Here, select whether newly-loaded "
                                               "images are sorted by file-date, file-name, or file-size."));

    d->sortReverse = new QCheckBox(i18n("Reverse ordering"), sortOptionsGroup);
    QWhatsThis::add(d->sortReverse, i18n("<p>If this option is enabled, newly-loaded "
                                         "images will be sorted in descending order."));

    // --------------------------------------------------------

    layout->addWidget(interfaceOptionsGroup);
    layout->addWidget(exposureOptionsGroup);
    layout->addWidget(ExifGroupOptions);
    layout->addWidget(sortOptionsGroup);
    layout->addStretch();

    // --------------------------------------------------------

    connect(d->themebackgroundColor, SIGNAL(toggled(bool)),
            this, SLOT(slotThemeBackgroundColor(bool)));

    readSettings();
}

SetupEditor::~SetupEditor()
{
    delete d;
}

void SetupEditor::slotThemeBackgroundColor(bool e)
{
    d->colorBox->setEnabled(!e);
}

void SetupEditor::readSettings()
{
    KConfig* config = kapp->config();
    QColor Black(Qt::black);
    QColor White(Qt::white);
    config->setGroup("ImageViewer Settings");
    d->themebackgroundColor->setChecked(config->readBoolEntry("UseThemeBackgroundColor", true));
    d->backgroundColor->setColor( config->readColorEntry("BackgroundColor", &Black ) );
    d->hideToolBar->setChecked(config->readBoolEntry("FullScreen Hide ToolBar", false));
    d->hideThumbBar->setChecked(config->readBoolEntry("FullScreenHideThumbBar", true));
    d->horizontalThumbBar->setChecked(config->readBoolEntry("HorizontalThumbbar", false));
    d->useTrash->setChecked(config->readBoolEntry("DeleteItem2Trash", false));
    d->showSplash->setChecked(config->readBoolEntry("ShowSplash", true));
    d->exifRotateBox->setChecked(config->readBoolEntry("EXIF Rotate", true));
    d->exifSetOrientationBox->setChecked(config->readBoolEntry("EXIF Set Orientation", true));
    d->underExposureColor->setColor(config->readColorEntry("UnderExposureColor", &White));
    d->overExposureColor->setColor(config->readColorEntry("OverExposureColor", &Black));
    d->sortOrderComboBox->setCurrentItem(config->readNumEntry("SortOrder", 0));
    d->sortReverse->setChecked(config->readBoolEntry("ReverseSort", false));
    d->useRawImportTool->setChecked(config->readBoolEntry("UseRawImportTool", false));
}

void SetupEditor::applySettings()
{
    KConfig* config = kapp->config();
    config->setGroup("ImageViewer Settings");
    config->writeEntry("UseThemeBackgroundColor", d->themebackgroundColor->isChecked());
    config->writeEntry("BackgroundColor",         d->backgroundColor->color());
    config->writeEntry("FullScreen Hide ToolBar", d->hideToolBar->isChecked());
    config->writeEntry("FullScreenHideThumbBar",  d->hideThumbBar->isChecked());
    config->writeEntry("HorizontalThumbbar",      d->horizontalThumbBar->isChecked());
    config->writeEntry("DeleteItem2Trash",        d->useTrash->isChecked());
    config->writeEntry("ShowSplash",              d->showSplash->isChecked());
    config->writeEntry("EXIF Rotate",             d->exifRotateBox->isChecked());
    config->writeEntry("EXIF Set Orientation",    d->exifSetOrientationBox->isChecked());
    config->writeEntry("UnderExposureColor",      d->underExposureColor->color());
    config->writeEntry("OverExposureColor",       d->overExposureColor->color());
    config->writeEntry("SortOrder",               d->sortOrderComboBox->currentItem());
    config->writeEntry("ReverseSort",             d->sortReverse->isChecked());
    config->writeEntry("UseRawImportTool",        d->useRawImportTool->isChecked());
    config->sync();
}

}   // namespace ShowFoto
