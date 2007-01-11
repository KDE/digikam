/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2007-11-01
 * Description : a settings widget to handle pure white and 
 *               black color alert.
 * 
 * Copyright 2007 Gilles Caulier
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

#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qwidget.h>
#include <qtimer.h>
#include <qcheckbox.h>

// KDE includes.

#include <kapplication.h>
#include <kdialog.h>
#include <klocale.h>
#include <kcolorbutton.h>
#include <kconfig.h>

// Local includes.

#include "ddebug.h"
#include "coloralertwidget.h"
#include "coloralertwidget.moc"

namespace Digikam
{

class ColorAlertWidgetPriv
{
public:

    ColorAlertWidgetPriv()
    {
        whiteAlertBox   = 0;
        blackAlertBox   = 0;
        whiteAlertColor = 0;
        blackAlertColor = 0;
    }

    QString       settingsSection;

    QCheckBox    *whiteAlertBox;
    QCheckBox    *blackAlertBox;

    KColorButton *whiteAlertColor;
    KColorButton *blackAlertColor;
};
    
ColorAlertWidget::ColorAlertWidget(const QString& settingsSection, QWidget *parent)
                : QHBox(parent, 0, Qt::WDestructiveClose)
{
    d = new ColorAlertWidgetPriv;
    d->settingsSection = settingsSection;

    setSpacing(KDialog::spacingHint());

    d->whiteAlertBox   = new QCheckBox(i18n("White!"), this);
    QWhatsThis::add(d->whiteAlertBox, i18n("<p>Set on this option to display pure white "
                                           "over-colored on preview. This will help you to avoid "
                                           "overexposing the image. This will not have an effect on the "
                                           "final rendering."));
    d->whiteAlertColor = new KColorButton(this);
    QWhatsThis::add(d->whiteAlertColor, i18n("<p>Customize here the pure white alert color.") );

    d->blackAlertBox   = new QCheckBox(i18n("Black!"), this);
    QWhatsThis::add(d->blackAlertBox, i18n("<p>Set on this option to display pure black "
                                           "over-colored on preview. This will help you to avoid "
                                           "underexposing the image. This will not have an effect on the "
                                           "final rendering."));
    d->blackAlertColor = new KColorButton(this);
    QWhatsThis::add(d->blackAlertColor, i18n("<p>Customize here the pure white alert color.") );

    QWidget *space = new QWidget(this);
    setStretchFactor(space, 10);
    
    // -------------------------------------------------------------
    
    connect(d->whiteAlertBox, SIGNAL(toggled(bool)),
            d->whiteAlertColor, SLOT(setEnabled(bool)));

    connect(d->blackAlertBox, SIGNAL(toggled(bool)),
            d->blackAlertColor, SLOT(setEnabled(bool)));

    connect(d->whiteAlertBox, SIGNAL(toggled(bool)),
            this, SIGNAL(signalWhiteAlertToggled(bool)));

    connect(d->blackAlertBox, SIGNAL(toggled(bool)),
            this, SIGNAL(signalBlackAlertToggled(bool)));

    connect(d->whiteAlertColor, SIGNAL(changed(const QColor &)),
            this, SIGNAL(signalWhiteAlertColorChanged(const QColor &)));

    connect(d->blackAlertColor, SIGNAL(changed(const QColor &)),
            this, SIGNAL(signalBlackAlertColorChanged(const QColor &)));

    // -------------------------------------------------------------

    readSettings(); 
}

ColorAlertWidget::~ColorAlertWidget()
{
    writeSettings();
    delete d;
}

bool ColorAlertWidget::whiteAlertIsChecked()
{
    return d->whiteAlertBox->isChecked();
}

bool ColorAlertWidget::blackAlertIsChecked()
{
    return d->blackAlertBox->isChecked();
}

QColor ColorAlertWidget::whiteAlertColor()
{
    return d->whiteAlertColor->color();
}

QColor ColorAlertWidget::blackAlertColor()
{
    return d->blackAlertColor->color();
}

void ColorAlertWidget::readSettings(void)
{
    KConfig *config = kapp->config();
    config->setGroup(d->settingsSection);

    QColor black(Qt::black);
    QColor white(Qt::white);
    d->whiteAlertColor->setColor(config->readColorEntry("White Alert Color", &black));
    d->blackAlertColor->setColor(config->readColorEntry("Black Alert Color", &white));
    d->whiteAlertBox->setChecked(config->readBoolEntry("White Alert", false));
    d->blackAlertBox->setChecked(config->readBoolEntry("Black Alert", false));

    d->whiteAlertColor->setEnabled(whiteAlertIsChecked());
    d->blackAlertColor->setEnabled(blackAlertIsChecked());
}
    
void ColorAlertWidget::writeSettings(void)
{
    KConfig *config = kapp->config();
    config->setGroup(d->settingsSection);
    config->writeEntry( "White Alert Color", d->whiteAlertColor->color() );
    config->writeEntry( "Black Alert Color", d->blackAlertColor->color() );
    config->writeEntry( "White Alert", whiteAlertIsChecked() );
    config->writeEntry( "Black Alert", blackAlertIsChecked() );
    config->sync();
}

}  // NameSpace Digikam

