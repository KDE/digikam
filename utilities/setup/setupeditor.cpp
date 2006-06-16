/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2004-08-03
 * Description : setup Image Editor tab.
 *
 * Copyright 2004-2006 by Gilles Caulier
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

// QT includes.

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
#include <klistview.h>
#include <ktrader.h>

// Local includes.

#include "setupeditor.h"

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
    }

    QHBox        *colorBox;

    QCheckBox    *hideToolBar;
    QCheckBox    *themebackgroundColor;

    KColorButton *backgroundColor;
};

SetupEditor::SetupEditor(QWidget* parent )
           : QWidget(parent)
{
    d = new SetupEditorPriv;
    QVBoxLayout *layout = new QVBoxLayout( parent );

    // --------------------------------------------------------

    QVGroupBox *interfaceOptionsGroup = new QVGroupBox(i18n("Interface Options"),
                                                       parent);

    d->themebackgroundColor = new QCheckBox(i18n("&Use theme background color"),
                                            interfaceOptionsGroup);

    QWhatsThis::add( d->themebackgroundColor, i18n("<p>Enable this option to use the background theme "
                                              "color into image editor area") );

    d->colorBox = new QHBox(interfaceOptionsGroup);

    QLabel *backgroundColorlabel = new QLabel( i18n("&Background color:"),
                                               d->colorBox );

    d->backgroundColor = new KColorButton(d->colorBox);
    backgroundColorlabel->setBuddy(d->backgroundColor);
    QWhatsThis::add( d->backgroundColor, i18n("<p>Customize here the background color to use "
                                              "into image editor area.") );
    backgroundColorlabel->setBuddy( d->backgroundColor );

    d->hideToolBar = new QCheckBox(i18n("H&ide toolbar in fullscreen mode"),
                                   interfaceOptionsGroup);

    layout->addWidget(interfaceOptionsGroup);

    // --------------------------------------------------------

    connect(d->themebackgroundColor, SIGNAL(toggled(bool)),
            this, SLOT(slotThemeBackgroundColor(bool)));

    layout->addStretch();

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

void SetupEditor::applySettings()
{
    KConfig* config = kapp->config();

    config->setGroup("ImageViewer Settings");
    config->writeEntry("UseThemeBackgroundColor", d->themebackgroundColor->isChecked());
    config->writeEntry("BackgroundColor", d->backgroundColor->color());
    config->writeEntry("FullScreen Hide ToolBar", d->hideToolBar->isChecked());
    config->sync();
}

void SetupEditor::readSettings()
{
    KConfig* config = kapp->config();
    QColor *Black = new QColor(Qt::black);

    config->setGroup("ImageViewer Settings");
    d->themebackgroundColor->setChecked(config->readBoolEntry("UseThemeBackgroundColor", true));
    d->backgroundColor->setColor( config->readColorEntry("BackgroundColor", Black ) );
    d->hideToolBar->setChecked(config->readBoolEntry("FullScreen Hide ToolBar", false));

    delete Black;
}

}  // namespace Digikam

#include "setupeditor.moc"
