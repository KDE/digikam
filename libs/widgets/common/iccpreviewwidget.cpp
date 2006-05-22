/* ============================================================
 * Author: Francisco J. Cruz <fj.cruz@supercable.es>
 * Date  : 2006-01-12
 * Description : a widget to display ICC profiles descriptions
 *               in file dialog preview.
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

#include "cietonguewidget.h"
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
        cieTongue       = 0;
    }

    KSqueezedTextLabel *name;
    KSqueezedTextLabel *description;
    KSqueezedTextLabel *colorSpace;
    KSqueezedTextLabel *deviceClass;
    KSqueezedTextLabel *renderingIntent;

    KURL                currentURL;
    
    CIETongueWidget    *cieTongue;
};

ICCPreviewWidget::ICCPreviewWidget(QWidget *parent)
                : KPreviewWidgetBase( parent )
{
    d = new ICCPreviewWidgetPriv;

    QBoxLayout* vlay = new QVBoxLayout(this);
        
    QLabel *label1 = new QLabel(i18n("Name:"), this);
    d->name        = new KSqueezedTextLabel(QString::null, this);
    vlay->addWidget(label1);
    vlay->addWidget(d->name);
      
    QLabel *label2 = new QLabel(i18n("Description:"), this);
    d->description = new KSqueezedTextLabel(QString::null, this);
    vlay->addWidget(label2);
    vlay->addWidget(d->description);
        
    QLabel *label3 = new QLabel(i18n("Color space:"), this);
    d->colorSpace  = new KSqueezedTextLabel(QString::null, this);
    vlay->addWidget(label3);
    vlay->addWidget(d->colorSpace);

    QLabel *label4 = new QLabel(i18n("Device class:"), this);
    d->deviceClass = new KSqueezedTextLabel(QString::null, this);
    vlay->addWidget(label4);
    vlay->addWidget(d->deviceClass);
    
    QLabel *label5     = new QLabel(i18n("Rendering intent:"), this);
    d->renderingIntent = new KSqueezedTextLabel(QString::null, this);
    vlay->addWidget(label5);
    vlay->addWidget(d->renderingIntent);

    QLabel *label6 = new QLabel(i18n("CIE diagram:"), this);
    d->cieTongue   = new CIETongueWidget(256, 256, this);
    vlay->addWidget(label6);
    vlay->addWidget(d->cieTongue);
    vlay->addStretch();
}

ICCPreviewWidget::~ICCPreviewWidget()
{
    delete d;
}

void ICCPreviewWidget::showPreview( const KURL &url)
{
    clearPreview();
    
    if (url.isLocalFile())
    {
        kdDebug() << url << "Is a local file" << endl;
        d->currentURL = url;
        getICCData(d->currentURL);
    }
    else
    {
        kdDebug() << url << "Not a local file" << endl;
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
    d->cieTongue->setProfileData();
}

void ICCPreviewWidget::getICCData( const KURL &url)
{
    cmsHPROFILE tmpProfile=0;
    QString     space, device, intent;
    
    if (!url.hasPath())
        return;
    
    if (!QFileInfo::QFileInfo(url.path()).isFile())
        return;
    
    cmsErrorAction(LCMS_ERROR_SHOW);    
    tmpProfile = cmsOpenProfileFromFile(QFile::encodeName(url.path()), "r");
    if (!tmpProfile)
        return;

    d->name->setText(QString("<b>%1</b>").arg(QString(cmsTakeProductName(tmpProfile))));
    d->description->setText(QString("<b>%1</b>").arg(QString(cmsTakeProductDesc(tmpProfile))));

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

    d->colorSpace->setText(QString("<b>%1</b>").arg(space));

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
        default:
            device = i18n("Unknown");
            break;
    }

    d->deviceClass->setText(QString("<b>%1</b>").arg(device));

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
        default:
            intent = i18n("Unknown");
            break;
    }
    
    d->renderingIntent->setText(QString("<b>%1</b>").arg(intent));

    d->cieTongue->setProfileHandler(tmpProfile);
    
    cmsCloseProfile(tmpProfile);
}

} // namespace Digikam

#include "iccpreviewwidget.moc"
