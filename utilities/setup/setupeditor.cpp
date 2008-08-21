/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 * 
 * Date        : 2004-08-03
 * Description : setup Image Editor tab.
 *
 * Copyright (C) 2004-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

namespace Digikam
{
class SetupEditorPriv
{
public:

    SetupEditorPriv()
    {
        hideToolBar          = 0;
        themebackgroundColor = 0;
        backgroundColor      = 0;
        colorBox             = 0;
        overExposureColor    = 0;
        underExposureColor   = 0;
        useRawImportTool     = 0;
    }

    QHBox        *colorBox;

    QCheckBox    *hideToolBar;
    QCheckBox    *themebackgroundColor;
    QCheckBox    *useRawImportTool;

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

    d->themebackgroundColor = new QCheckBox(i18n("&Use theme background color"), interfaceOptionsGroup);

    QWhatsThis::add(d->themebackgroundColor, i18n("<p>Enable this option to use background theme "
                                             "color in image editor area"));

    d->colorBox = new QHBox(interfaceOptionsGroup);

    QLabel *backgroundColorlabel = new QLabel(i18n("&Background color:"), d->colorBox);

    d->backgroundColor = new KColorButton(d->colorBox);
    backgroundColorlabel->setBuddy(d->backgroundColor);
    QWhatsThis::add(d->backgroundColor, i18n("<p>Customize background color to use "
                                             "in image editor area."));

    d->hideToolBar = new QCheckBox(i18n("H&ide toolbar in fullscreen mode"), interfaceOptionsGroup);

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
    QWhatsThis::add(d->underExposureColor, i18n("<p>Customize the color used in image editor to identify "
                                                "under-exposed pixels."));

    QHBox *overExpoBox         = new QHBox(exposureOptionsGroup);
    QLabel *overExpoColorlabel = new QLabel( i18n("&Over-exposure color:"), overExpoBox);
    d->overExposureColor       = new KColorButton(overExpoBox);
    overExpoColorlabel->setBuddy(d->overExposureColor);
    QWhatsThis::add(d->overExposureColor, i18n("<p>Customize the color used in image editor to identify "
                                               "over-exposed pixels."));

    // --------------------------------------------------------

    layout->addWidget(interfaceOptionsGroup);
    layout->addWidget(exposureOptionsGroup);
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
    d->backgroundColor->setColor(config->readColorEntry("BackgroundColor", &Black));
    d->hideToolBar->setChecked(config->readBoolEntry("FullScreen Hide ToolBar", false));
    d->underExposureColor->setColor(config->readColorEntry("UnderExposureColor", &White));
    d->overExposureColor->setColor(config->readColorEntry("OverExposureColor", &Black));
    d->useRawImportTool->setChecked(config->readBoolEntry("UseRawImportTool", false));
}

void SetupEditor::applySettings()
{
    KConfig* config = kapp->config();
    config->setGroup("ImageViewer Settings");
    config->writeEntry("UseThemeBackgroundColor", d->themebackgroundColor->isChecked());
    config->writeEntry("BackgroundColor",         d->backgroundColor->color());
    config->writeEntry("FullScreen Hide ToolBar", d->hideToolBar->isChecked());
    config->writeEntry("UnderExposureColor",      d->underExposureColor->color());
    config->writeEntry("OverExposureColor",       d->overExposureColor->color());
    config->writeEntry("UseRawImportTool",        d->useRawImportTool->isChecked());
    config->sync();
}

}  // namespace Digikam
