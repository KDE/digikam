/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-07-14
 * Description : a digiKam image plugin to clone area .
 *
 * Copyright (C) 2011-07-14 by Zhang Jie <zhangjiehangyuan2005 dot at gmail dot com>
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

#include "clonetool.moc"

//Qt includes

#include <QGridLayout>
#include <QImage>
#include <QLabel>
#include <QPainter>
#include <QPen>
#include <QPixmap>

//KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <knuminput.h>
#include <kstandarddirs.h>

//LibDKcraw includes

#include <libkdcraw/rnuminput.h>

//Local includes

#include "clonesettings.h"
#include "clonefilter.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imageclonewidget.h"

namespace DigikamEnhanceImagePlugin
{

class CloneTool::CloneToolPriv
{

public:
    CloneToolPriv():
        configGroupName("Clone Tool"),
        origImage(0),
        previewRImage(0),
        resultImage(0),
        settingsView(0),
        previewWidget(0),
        gboxSettings(0)
    {
    }

    const QString configGroupName;

    DImg* origImage;
    DImg* previewRImage; //result of filter preview
    DImg* resultImage;   //result of filter originalImage

    CloneSettings* settingsView;
    CloneGuideWidget* previewWidget;
    EditorToolSettings* gboxSettings;

};

CloneTool::CloneTool(QObject* parent)
    :EditorToolThread(parent),
    d(new CloneToolPriv)
{
    setObjectName("Clone");
    setToolName(i18n("Clone Toll"));
    setToolIcon(SmallIcon("clone"));

   // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Cancel);

    // -------------------------------------------------------------

    d->settingsView = new CloneSettings(d->gboxSettings->plainPage());
    setToolSettings(d->gboxSettings);

    d->previewWidget = new CloneGuideWidget(this,d->settingsView->settings());
    d->previewWidget->imageIface()->getOriginalImg()->copy();
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::UnSplitPreviewModes);

    //-------------------save the original image, if cancel button is clicked, this will be used--------------
    uchar* data     = d->previewWidget->imageIface()->getOriginalImg();
    d->origImage->putImageData(data);
    d->origImage.setIccProfile( d->previewWidget->imageIface()->getOriginalImg()->getIccProfile());

    //==========================================================================================================

    init();

    // -------------------------------------------------------------
    connect(d->settingsView,SIGNAL(signalSettingsChanged()),this,SLOT(slotTimer()));
    connect(d->settingsView,SIGNAL(signalSettingsChanged()),this,SLOT(slotSettingsChanged()));
    connect(d->previewWidget,SIGNAL(drawingComplete()),this, SLOT(slotDrawingComplete()));
}

CloneTool::~CloneTool()
{
    if(d->origImage)
        delete origImage;
    if(d->previewRImage)
        delete previewRImage;
    if(d->resultImage)
        delete resultImage;

    delete d;
}

CloneTool::slotSettingsChanged()
{
    d->previewWidget->setContainer(settingsView->settings());
}

void CloneTool::slotDrawingComplete()
{
    QPoint dis = previewWidget->settings().getDis();
    CloneFilter*  previewFilter = newCloneFilter(previewWidget->getPreview(),previewWidget->getPreviewMask(),dis,this);
    setFilter(previewFilter);
    uchar* data = previewFilter->getResultImg()->bits();
    if(!data)
        return;
    d->previewRImage.detach();
    d->previewRImage.putImageData(data);

    d->previewWidget->imageIface()->putPreviewImage(d->previewRImage.stripImageData());
    d->previewWidget->setPreview();
    delete previewFilter;

    dis = previewWidget->settings().getOriDis();
    CloneFilter*  orignalFilter = newCloneFilter(previewWidget->getOrigImage(),previewWidget->getMaskImg(),dis,this);
    setFilter(previewFilter);
    uchar* data1 = previewFilter->getResultImg()->bits();
    if(!data)
        return;
    d->resultImage.detach();
    d->resultImage.putImageData(data);
    d->previewWidget->imageIface()->putOriginalImage(i18n("Clone Toll"), filter()->filterAction(),d->resultImage.stripImageData());
    d->previewWidget->updatePreview();
    delete orignalFilter;
}

void CloneTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    d->settingsView->readSettings(group);
    CloneContainer prm = d->settingsView->settings();

    d->previewWidget->setContainer(prm);  // get current container

    slotEffect();
}

void CloneTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    d->settingsView->writeSettings(group);
    group.sync();
}

void CloneTool::slotResetSettings()
{
    d->settingsView->resetToDefault();
    slotEffect();
}

void CloneTool::prepareEffect()
{
    d->previewWidget.m_settings = d->settingsView->settings();
    ImageIface* iface = d->previewWidget->imageIface();
    int previewWidth = iface->previewWidth();
    int previewHeight = iface->previewHeight();

    DImg* imTemp = iface->getOriginalImg()->smoothScale(previewWidth, previewHeight, Qt::KeepAspectRatio);
    DImg* maskTemp =  previewWidget->getPreviewMask();
    float ratio = maskTemp->width()/imTemp->width();// smooth processing keeps the height and width ratio, 
                                                    // so only compute width ratio is OK.
    if(ratio!=0)
    {
       QPoint dis = QPoint(previewWidget->getDis().x()/ratio, previewWidget->getDis().y()/ratio);
       maskTemp->smoothScale(previewWidth, previewHeight, Qt::KeepAspectRatio);   
       setFilter(new CloneFilter(imTemp,maskTemp,dis,this));
    }

    delete imTemp;
    delete maskTemp;
}

void CloneTool::prepareFinal()
{
    d->previewWidget.m_settings = d->settingsView->settings();
    ImageIface iface(0, 0);
    // DImg* imTemp = previewWidget->getPreview();
    // DImg* maskTemp =  previewWidget->getPreviewMask();
    DImg* maskTemp = previewWidget->getMaskImg();
    QPoint dis = previewWidget->getOriDis();
    setFilter(new CloneFilter(iface.getOriginalImg(), maskTemp, dis,this));
}

void CloneTool::putPreviewData()
{
  ImageIface* iface = d->previewWidget->imageIface();
  DImg previewImg = filter()->getTargetImage().smoothScale(iface->previewWidth(), ifaace->previewHeight());
  iface->putPreviewImage(previewImg.bits());
  d->previewWidget->setPreviewImage(preview);
  d->previewWIdget->updatePreview();
}

void CLoneTool::putFinalData()
{
    ImageIface iface(0, 0);
    iface.putOriginalImage(i18n("Clone Tool"), filter()->filterAction(), filter()->getTargetImage().bits());
}

} // namespace DigikamEnhanceImagePlugin
