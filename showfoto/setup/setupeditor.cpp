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

#include <QColor>
#include <QGroupBox>
#include <QLabel>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QComboBox>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>
#include <kcolorbutton.h>
#include <knuminput.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kglobal.h>
#include <kvbox.h>

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

    KHBox        *colorBox;

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
    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->setSpacing( KDialog::spacingHint() );

    // --------------------------------------------------------

    QGroupBox *interfaceOptionsGroup = new QGroupBox(i18n("Interface Options"), this);
    QVBoxLayout *gLayout1            = new QVBoxLayout();

    d->themebackgroundColor = new QCheckBox(i18n("&Use theme background color"),
                                            interfaceOptionsGroup);

    d->themebackgroundColor->setWhatsThis(i18n("<p>Enable this option to use background theme "
                                               "color in image editor area"));

    d->colorBox                  = new KHBox(interfaceOptionsGroup);
    QLabel *backgroundColorlabel = new QLabel( i18n("&Background color:"), d->colorBox);
    d->backgroundColor           = new KColorButton(d->colorBox);
    backgroundColorlabel->setBuddy(d->backgroundColor);
    d->backgroundColor->setWhatsThis(i18n("<p>Select background color to use "
                                          "for image editor area."));

    d->hideToolBar        = new QCheckBox(i18n("H&ide toolbar in fullscreen mode"), interfaceOptionsGroup);
    d->hideThumbBar       = new QCheckBox(i18n("Hide &thumbbar in fullscreen mode"), interfaceOptionsGroup);
    d->horizontalThumbBar = new QCheckBox(i18n("Use &horizontal thumbbar (need to restart showFoto)"), interfaceOptionsGroup);
    d->horizontalThumbBar->setWhatsThis( i18n("<p>If this option is enabled, the thumbnails bar will be displayed "
                                              "horizontally behind the image area. You need to restart showFoto "
                                              "for this option take effect.<p>"));
    d->useTrash   = new QCheckBox(i18n("&Deleting items should move them to trash"), interfaceOptionsGroup);
    d->showSplash = new QCheckBox(i18n("&Show splash screen at startup"), interfaceOptionsGroup);

    d->useRawImportTool = new QCheckBox(i18n("Use Raw Import Tool to handle Raw image"), interfaceOptionsGroup);
    d->useRawImportTool->setWhatsThis(i18n("<p>Set on this option to use Raw Import "
                                           "tool before to load a Raw image, "
                                           "to customize indeep decoding settings."));

    gLayout1->addWidget(d->themebackgroundColor);
    gLayout1->addWidget(d->colorBox);
    gLayout1->addWidget(d->hideToolBar);
    gLayout1->addWidget(d->hideThumbBar);
    gLayout1->addWidget(d->horizontalThumbBar);
    gLayout1->addWidget(d->useTrash);
    gLayout1->addWidget(d->showSplash);
    gLayout1->addWidget(d->useRawImportTool);
    interfaceOptionsGroup->setLayout(gLayout1);

    // --------------------------------------------------------

    QGroupBox *exposureOptionsGroup = new QGroupBox(i18n("Exposure Indicators"), this);
    QVBoxLayout *gLayout2           = new QVBoxLayout();

    KHBox *underExpoBox         = new KHBox(exposureOptionsGroup);
    QLabel *underExpoColorlabel = new QLabel( i18n("&Under-exposure color:"), underExpoBox);
    d->underExposureColor       = new KColorButton(underExpoBox);
    underExpoColorlabel->setBuddy(d->underExposureColor);
    d->underExposureColor->setWhatsThis( i18n("<p>Customize color used in image editor to identify "
                                              "under-exposed pixels.") );

    KHBox *overExpoBox         = new KHBox(exposureOptionsGroup);
    QLabel *overExpoColorlabel = new QLabel( i18n("&Over-exposure color:"), overExpoBox);
    d->overExposureColor       = new KColorButton(overExpoBox);
    overExpoColorlabel->setBuddy(d->overExposureColor);
    d->overExposureColor->setWhatsThis( i18n("<p>Customize color used in image editor to identify "
                                             "over-exposed pixels.") );

    gLayout2->addWidget(underExpoBox);
    gLayout2->addWidget(overExpoBox);
    exposureOptionsGroup->setLayout(gLayout2);

    // --------------------------------------------------------

    QGroupBox *ExifGroupOptions = new QGroupBox(i18n("EXIF Actions"), this);
    QVBoxLayout *gLayout3       = new QVBoxLayout();

    d->exifRotateBox = new QCheckBox(ExifGroupOptions);
    d->exifRotateBox->setText(i18n("Show images/thumbs &rotated according to orientation tag"));

    d->exifSetOrientationBox = new QCheckBox(ExifGroupOptions);
    d->exifSetOrientationBox->setText(i18n("Set orientation tag to normal after rotate/flip"));

    gLayout3->addWidget(d->exifRotateBox);
    gLayout3->addWidget(d->exifSetOrientationBox);
    ExifGroupOptions->setLayout(gLayout3);

    // --------------------------------------------------------

    QGroupBox *sortOptionsGroup = new QGroupBox(i18n("Sort order for images"), this);
    QVBoxLayout *gLayout4       = new QVBoxLayout();

    KHBox* sortBox       = new KHBox(sortOptionsGroup);
    new QLabel(i18n("Sort images by:"), sortBox);
    d->sortOrderComboBox = new QComboBox(sortBox);
    d->sortOrderComboBox->insertItem(0, i18n("File Date"));
    d->sortOrderComboBox->insertItem(1, i18n("File Name"));
    d->sortOrderComboBox->insertItem(2, i18n("File size"));
    d->sortOrderComboBox->setWhatsThis(i18n("<p>Here, select whether newly-loaded "
                                            "images are sorted by file-date, file-name, or file-size."));

    d->sortReverse = new QCheckBox(i18n("Reverse ordering"), sortOptionsGroup);
    d->sortReverse->setWhatsThis(i18n("<p>If this option is enabled, newly-loaded "
                                      "images will be sorted in descending order."));

    gLayout4->addWidget(sortBox);
    gLayout4->addWidget(d->sortReverse);
    sortOptionsGroup->setLayout(gLayout4);

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
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(QString("ImageViewer Settings"));
    QColor Black(Qt::black);
    QColor White(Qt::white);
    d->themebackgroundColor->setChecked(group.readEntry("UseThemeBackgroundColor", true));
    d->backgroundColor->setColor(group.readEntry("BackgroundColor", Black));
    d->hideToolBar->setChecked(group.readEntry("FullScreen Hide ToolBar", false));
    d->hideThumbBar->setChecked(group.readEntry("FullScreenHideThumbBar", true));
    d->horizontalThumbBar->setChecked(group.readEntry("HorizontalThumbbar", false));
    d->useTrash->setChecked(group.readEntry("DeleteItem2Trash", false));
    d->showSplash->setChecked(group.readEntry("ShowSplash", true));
    d->exifRotateBox->setChecked(group.readEntry("EXIF Rotate", true));
    d->exifSetOrientationBox->setChecked(group.readEntry("EXIF Set Orientation", true));
    d->underExposureColor->setColor(group.readEntry("UnderExposureColor", White));
    d->overExposureColor->setColor(group.readEntry("OverExposureColor", Black));
    d->sortOrderComboBox->setCurrentIndex(group.readEntry("SortOrder", 0));
    d->sortReverse->setChecked(group.readEntry("ReverseSort", false));
    d->useRawImportTool->setChecked(group.readEntry("UseRawImportTool", false));
}

void SetupEditor::applySettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(QString("ImageViewer Settings"));
    group.writeEntry("UseThemeBackgroundColor", d->themebackgroundColor->isChecked());
    group.writeEntry("BackgroundColor",         d->backgroundColor->color());
    group.writeEntry("FullScreen Hide ToolBar", d->hideToolBar->isChecked());
    group.writeEntry("FullScreenHideThumbBar",  d->hideThumbBar->isChecked());
    group.writeEntry("HorizontalThumbbar",      d->horizontalThumbBar->isChecked());
    group.writeEntry("DeleteItem2Trash",        d->useTrash->isChecked());
    group.writeEntry("ShowSplash",              d->showSplash->isChecked());
    group.writeEntry("EXIF Rotate",             d->exifRotateBox->isChecked());
    group.writeEntry("EXIF Set Orientation",    d->exifSetOrientationBox->isChecked());
    group.writeEntry("UnderExposureColor",      d->underExposureColor->color());
    group.writeEntry("OverExposureColor",       d->overExposureColor->color());
    group.writeEntry("SortOrder",               d->sortOrderComboBox->currentIndex());
    group.writeEntry("ReverseSort",             d->sortReverse->isChecked());
    group.writeEntry("UseRawImportTool",        d->useRawImportTool->isChecked());
    config->sync();
}

}   // namespace ShowFoto
