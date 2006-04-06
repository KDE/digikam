/* ============================================================
 * Author: Francisco J. Cruz <fj.cruz@supercable.es>
 * Date  : 2006-01-12
 * Description :a widget to display ICC profiles metada in file
 * dialog preview.
 * 
 * Copyright 2006 by Francisco J. Cruz
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

#include <config.h>
#include LCMS_HEADER

// Qt includes

#include <qlayout.h>
#include <qlabel.h>
#include <qvgroupbox.h>
#include <qhgroupbox.h>
#include <qdir.h>

// KDE includes

#include <klocale.h>
#include <kdialog.h>
#include <ksqueezedtextlabel.h>
#include <kdebug.h>

// Local includes.

#include "iccpreviewwidget.h"

namespace Digikam 
{

class ICCPreviewWidgetPriv
{
public:

    ICCPreviewWidgetPriv()
    {
        name            = 0;
        description     = 0;
        colorSpace      = 0;
        deviceClass     = 0;
        renderingIntent = 0;
    }

    KSqueezedTextLabel *name;
    KSqueezedTextLabel *description;
    KSqueezedTextLabel *colorSpace;
    KSqueezedTextLabel *deviceClass;
    KSqueezedTextLabel *renderingIntent;

    KURL                currentURL;
};

ICCPreviewWidget::ICCPreviewWidget(QWidget *parent)
                : KPreviewWidgetBase( parent )
{
    d = new ICCPreviewWidgetPriv;

    QVBoxLayout *layout  = new QVBoxLayout(this, 0,  KDialog::spacingHint());
    QVGroupBox *metaData = new QVGroupBox(this);
    
    QHGroupBox *name = new QHGroupBox(metaData);
    name->setFrameStyle(QFrame::NoFrame);
    QLabel *label1 = new QLabel(name);
    label1->setText(i18n("Name: "));
    d->name = new KSqueezedTextLabel(QString::null, name);

    QHGroupBox *description = new QHGroupBox(metaData);
    description->setFrameStyle(QFrame::NoFrame);
    QLabel *label2 = new QLabel(description);
    label2->setText(i18n("Description: "));
    d->description = new KSqueezedTextLabel(QString::null, description);

    QHGroupBox *colorSpace = new QHGroupBox(metaData);
    colorSpace->setFrameStyle(QFrame::NoFrame);
    QLabel *label3 = new QLabel(colorSpace);
    label3->setText(i18n("Color space: "));
    d->colorSpace = new KSqueezedTextLabel(QString::null, colorSpace);

    QHGroupBox *deviceClass = new QHGroupBox(metaData);
    deviceClass->setFrameStyle(QFrame::NoFrame);
    QLabel *label4 = new QLabel(deviceClass);
    label4->setText(i18n("Device class: "));
    d->deviceClass = new KSqueezedTextLabel(QString::null, deviceClass);

    QHGroupBox *renderingIntent = new QHGroupBox(metaData);
    renderingIntent->setFrameStyle(QFrame::NoFrame);
    QLabel *label5 = new QLabel(renderingIntent);
    label5->setText(i18n("Rendering intent: "));
    d->renderingIntent = new KSqueezedTextLabel(QString::null, renderingIntent);
    
    layout->addWidget(metaData);
}

ICCPreviewWidget::~ICCPreviewWidget()
{
    delete d;
}

void ICCPreviewWidget::showPreview( const KURL &url)
{
    if (url.isLocalFile())
    {
        kdDebug() << "Is Local file" << endl;
        d->currentURL = url;
        getICCData(d->currentURL);
    }
    else
    {
        kdDebug() << "Not Local file" << endl;
        clearPreview();
        return;
    }
}

void ICCPreviewWidget::clearPreview()
{
    d->name->clear();
    d->description->clear();
    d->colorSpace->clear();
    d->deviceClass->clear();
    d->renderingIntent->clear();
    d->currentURL = KURL();
}

void ICCPreviewWidget::getICCData( const KURL &url)
{
    cmsHPROFILE tmpProfile=0;
    QString space, device, intent;
    if (!url.hasPath())
        return;
    
    if (!QFileInfo::QFileInfo(url.path()).isFile())
        return;
    
    tmpProfile = cmsOpenProfileFromFile(QFile::encodeName(url.path()), "r");
    
    d->name->setText(QString(cmsTakeProductName(tmpProfile)));
    d->description->setText(QString(cmsTakeProductDesc(tmpProfile)));

    switch (cmsGetColorSpace(tmpProfile))
    {
        case icSigLabData:
            space = i18n("Lab");
            break;
        case icSigLuvData:
            space = i18n("Luv");
            break;
        case icSigRgbData:
            space = i18n("RGB");
            break;
        case icSigGrayData:
            space = i18n("GRAY");
            break;
        case icSigHsvData:
            space = i18n("HSV");
            break;
        case icSigHlsData:
            space = i18n("HLS");
            break;
        case icSigCmykData:
            space = i18n("CMYK");
            break;
        case icSigCmyData:
            space= i18n("CMY");
            break;
        default:
            space = i18n("Other");
            break;
    }

    d->colorSpace->setText(space);

    switch ((int)cmsGetDeviceClass(tmpProfile))
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
    }

    d->deviceClass->setText(device);

    //"Decode" profile rendering intent
    switch (cmsTakeRenderingIntent(tmpProfile))
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
    }
    
    d->renderingIntent->setText(intent);

    cmsCloseProfile(tmpProfile);
}

void ICCPreviewWidget::virtual_hook( int, void* )
{
}

} // namespace Digikam

#include "iccpreviewwidget.moc"
