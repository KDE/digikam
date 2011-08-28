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
#include <kdebug.h>
#include <kconfiggroup.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <knuminput.h>
#include <kstandarddirs.h>
#include <kstandarddirs.h>

//LibDKcraw includes

#include <libkdcraw/rnuminput.h>

//Local includes

#include "clonesettings.h"
#include "clonecontainer.h"
#include "clonefilter.h"
#include "imageclonewidget.h"
#include "editortoolsettings.h"
#include "imageiface.h"
//#include "editortooliface.h"



namespace DigikamEnhanceImagePlugin
{

class CloneTool::CloneToolPriv
{

public:
    CloneToolPriv():
        configGroupName("Clone Tool"),
        settingsView(0),
        previewWidget(0),
        gboxSettings(0)
    {
    }

    const QString       configGroupName;
    bool                strokeOver;    //be set true whenever a stroke is over
    DImg                origImage;
    DImg                previewRImage; //result of filter preview
    DImg                resultImage;   //result of filter originalImage

    CloneSettings*      settingsView;
    ImageCloneWidget*   previewWidget;
    EditorToolSettings* gboxSettings;
};

CloneTool::CloneTool(QObject* parent)
    :EditorToolThreaded(parent),
    d(new CloneToolPriv)
{
    setObjectName("clonetool");
    setToolName(i18n("Clone Tool"));
    setToolIcon(SmallIcon("clone"));
    //setToolHelp("clonetool.anchor");    

    d->previewWidget = new ImageCloneWidget;//(0,d->settingsView->settings())
    setToolView(d->previewWidget);
    d->previewWidget->setWhatsThis(i18n("The image preview with clone applied "
                                        "is shown here."));
    setPreviewModeMask(PreviewToolBar::UnSplitPreviewModes);
    init();
   // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;
    //d->gboxSettings->setTools(EditorToolSettings::Histogram);
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Cancel);

    // -------------------------------------------------------------

    d->settingsView = new CloneSettings(d->gboxSettings->plainPage());
    setToolSettings(d->gboxSettings);

    d->strokeOver    = false;

    //-------------------save the original image, if cancel button is clicked, this will be used--------------
    d->resultImage   = DImg();
    d->previewRImage = DImg();
    d->origImage     = DImg();
    //uchar* data      = d->previewWidget->imageIface()->getOriginalImg()->bits();
    //if(!data)
    //{
    d->origImage = d->previewWidget->imageIface()->getOriginalImg()->copyImageData();
    d->origImage.setIccProfile( d->previewWidget->imageIface()->getOriginalImg()->getIccProfile());
  
    //==========================================================================================================
    // -------------------------------------------------------------
    //connect(d->settingsView,SIGNAL(signalSettingsChanged()),this,SLOT(slotTimer()));
    connect(d->settingsView,SIGNAL(signalSettingsChanged()),this,SLOT(slotSettingsChanged()));
    //connect(d->previewWidget,SIGNAL(signalResized()),this,SLOT(slotEffect()));
    connect(d->previewWidget,SIGNAL(signalStrokeOver()),this,SLOT(slotStrokeOver()));//FIXME
 }

CloneTool::~CloneTool()
{
    delete d;
}

void CloneTool::slotSettingsChanged()
{
    d->previewWidget->setContainer(d->settingsView->settings());
    kDebug()<<"slotSettingsChanged is called";
}

void CloneTool::slotStrokeOver()  //only a stroke operation will be stored, use once operation mode
{
    //d->strokeOver = true;
        if(d->settingsView->getDrawEnable())
        {
            kDebug()<<"slotStrokeOver is called";
            //d->previewWidget->setContainer(d->settingsView->settings());
            QPoint dis  = d->previewWidget->getDis();
            DImg orimg  = d->previewWidget->imageIface()->getPreviewImg();            
            //DImg desimg = DImg(orimg.width(), orimg.height(), orimg.sixteenBit(), orimg.hasAlpha());
            CloneFilter*  previewFilter = new CloneFilter(&orimg, d->previewWidget->getPreviewMask(), dis, this);
            slotEffect();//FIXME            
            setFilter(previewFilter); 
            //ImageIface* iface = d->previewWidget->imageIface();            

    }
}

//FIXME
/*
void CloneTool::slotDrawingComplete()
{
    kDebug()<<"slotDrawingComplete is called";
    QPoint dis  = d->previewWidget->getDis();
    DImg orimg  = d->previewWidget->imageIface()->getPreviewImg();
    DImg desimg = DImg(orimg.width(), orimg.height(), orimg.sixteenBit(), orimg.hasAlpha());
    CloneFilter*  previewFilter = new CloneFilter(orimg, desimg, d->previewWidget->getPreviewMask(), dis);
    setFilter(previewFilter);    
    d->previewRImage.detach();
    //d->previewRImage = previewFilter->getTargetImage(); //FIXME
    d->previewRImage = filter()->getTargetImage();
    //if(!data)
    //    return;    
    //d->previewRImage->putImageData(data);
    d->previewWidget->imageIface()->putPreviewImage(d->previewRImage.bits());
    d->previewWidget->setPreview();
    delete previewFilter;

    dis = d->previewWidget->getOriDis();
    DImg orimg1  = d->previewWidget->imageIface()->getOriginalImg()->copyImageData();
    DImg desimg1 = DImg(orimg1.width(), orimg1.height(), orimg1.sixteenBit(), orimg1.hasAlpha());
    CloneFilter*  orignalFilter = new CloneFilter(orimg1, desimg1, d->previewWidget->getMaskImg(), dis);
    //CloneFilter*  orignalFilter = new CloneFilter(d->previewWidget->imageIface()->getOriginalImg()->copyImageData(),d->previewWidget->getMaskImg(),dis);
    setFilter(orignalFilter);
    // uchar* data1 = previewFilter->getResultImg()->bits();
    d->resultImage.detach();
    //d->resultImage = previewFilter->getTargetImage (); //FIXME
    d->resultImage = filter()->getTargetImage();
    // if(!data)
    //    return;    
    // d->resultImage->putImageData(data);
    d->previewWidget->imageIface()->putOriginalImage(i18n("Clone Toll"), filter()->filterAction(),d->resultImage.bits());
    d->previewWidget->updatePreview();
    delete orignalFilter;
}
*/

//FIXME
void CloneTool::readSettings()
{
    kDebug()<<"readSettings is called";
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    d->settingsView->readSettings(group);
    CloneContainer prm = d->settingsView->settings();

    d->previewWidget->setContainer(prm);  // get current container

    slotEffect();
}

void CloneTool::writeSettings()
{
    kDebug()<<"writeSettings is called";
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    d->settingsView->writeSettings(group);
    group.sync();
}

void CloneTool::slotResetSettings()
{
    kDebug()<<"slotResetSettings is called";
    d->settingsView->resetToDefault();
    slotEffect();
}


void CloneTool::prepareEffect()
{
}
//FIXME
/*
void CloneTool::prepareEffect()
{
    if(d->strokeOver)
    {
        if(d->settingsView->getDrawEnable())
        {
            kDebug()<<"prepareEffect is called";
            d->previewWidget->setContainer(d->settingsView->settings());
            QPoint dis  = d->previewWidget->getDis();
            DImg orimg  = d->previewWidget->imageIface()->getPreviewImg();
            DImg desimg = DImg(orimg.width(), orimg.height(), orimg.sixteenBit(), orimg.hasAlpha());
            CloneFilter*  previewFilter = new CloneFilter(orimg, desimg, d->previewWidget->getPreviewMask(), dis);
            setFilter(previewFilter);    
            d->previewRImage.detach();
            //d->previewRImage = previewFilter->getTargetImage(); //FIXME
            d->previewRImage = filter()->getTargetImage();
            //if(!data)
            //    return;    
            //d->previewRImage->putImageData(data);
            d->previewWidget->imageIface()->putPreviewImage(d->previewRImage.bits());
            d->previewWidget->setPreview();
            delete previewFilter;

            dis = d->previewWidget->getOriDis();
            DImg orimg1  = d->previewWidget->imageIface()->getOriginalImg()->copyImageData();
            DImg desimg1 = DImg(orimg1.width(), orimg1.height(), orimg1.sixteenBit(), orimg1.hasAlpha());
            CloneFilter*  orignalFilter = new CloneFilter(orimg1, desimg1, d->previewWidget->getMaskImg(), dis);
            //CloneFilter*  orignalFilter = new CloneFilter(d->previewWidget->imageIface()->getOriginalImg()->copyImageData(),d->previewWidget->getMaskImg(),dis);
            setFilter(orignalFilter);
            // uchar* data1 = previewFilter->getResultImg()->bits();
            d->resultImage.detach();
            //d->resultImage = previewFilter->getTargetImage (); //FIXME
            d->resultImage = filter()->getTargetImage();
            // if(!data)
            //    return;    
            // d->resultImage->putImageData(data);
            d->previewWidget->imageIface()->putOriginalImage(i18n("Clone Toll"), filter()->filterAction(),d->resultImage.bits());
            d->previewWidget->updatePreview();
            delete orignalFilter;
            d->strokeOver = false;
*/

/*
            ImageIface* iface = d->previewWidget->imageIface();
            int previewWidth = iface->previewWidth();
            int previewHeight = iface->previewHeight();

            DImg  imTemp   = iface->getOriginalImg()->smoothScale(previewWidth, previewHeight, Qt::KeepAspectRatio);
            DImg desTemg   = DImg(imTemp.width(), imTemp.height(), imTemp.sixteenBit(), imTemp.hasAlpha());
            DImg* maskTemp =  d->previewWidget->getPreviewMask();
            float ratio    = maskTemp->width()/imTemp.width();// smooth processing keeps the height and width ratio, 
                                                        // so only compute width ratio is OK.
            if(ratio!=0)
            {
               QPoint dis = QPoint(d->previewWidget->getDis().x()/ratio, d->previewWidget->getDis().y()/ratio);
               maskTemp->smoothScale(previewWidth, previewHeight, Qt::KeepAspectRatio);   
               setFilter(new CloneFilter(imTemp, desTemg, maskTemp, dis, this));
            }

          delete maskTemp;
*/
//        }
    
//    }
//}

/*

void CloneTool::slotOk()
{
    writeSettings();

    setRenderingMode(EditorToolThreaded::FinalRendering);
    kDebug() << "Final " << toolName() << " started...";

    toolSettings()->enableButton(EditorToolSettings::Ok,      false);
    toolSettings()->enableButton(EditorToolSettings::SaveAs,  false);
    toolSettings()->enableButton(EditorToolSettings::Load,    false);
    toolSettings()->enableButton(EditorToolSettings::Default, false);
    toolSettings()->enableButton(EditorToolSettings::Try,     false);
    toolView()->setEnabled(false);

    //EditorToolIface::editorToolIface()->setToolStartProgress(d->progressMess.isEmpty() ? toolName() : d->progressMess);
    kapp->setOverrideCursor( Qt::WaitCursor );

    //if (d->delFilter && d->threadedFilter)
    //{
   //     delete d->threadedFilter;
   //     d->threadedFilter = 0;
   // }

    prepareFinal();
}
*/
void CloneTool::prepareFinal()
{
    kDebug()<<"prepareFinal is called";
    d->previewWidget->setContainer(d->settingsView->settings());
    QPoint dis = d->previewWidget->getOriDis();
    ImageIface iface(0, 0);
    setFilter(new CloneFilter(iface.getOriginalImg(), d->previewWidget->getMaskImg(), dis, this));

}

void CloneTool::putPreviewData()
{
    kDebug()<<"putPreviewData is called";
    ImageIface* iface = d->previewWidget->imageIface();
    if(!filter()->getTargetImage().isNull())
    {
        DImg imDest   = filter()->getTargetImage().smoothScale(iface->previewWidth(), iface->previewHeight());
        d->previewWidget->setPreviewImage(imDest);
/*
        imDest.save("../imDest", DImg::PNG);
        iface->putPreviewImage(imDest.bits());
        DImg previewImage = iface->getPreviewImg();
        previewImage.save("../previewImage", DImg::PNG);
        //d->previewWidget->updateResult();  
        d->previewWidget->updatePreview();
*/
    }                  

}

void CloneTool::putFinalData()
{    
    kDebug()<<"putFinalData is called";
    ImageIface iface(0, 0);
    iface.putOriginalImage(i18n("Clone Tool"), filter()->filterAction(), filter()->getTargetImage().bits());
}

/*
void CloneTool::slotOk()
{
    writeSettings();
    toolSettings()->enableButton(EditorToolSettings::Ok,      false);
    toolSettings()->enableButton(EditorToolSettings::Default, false);
    toolView()->setEnabled(false);
    prepareFinal();
}

void CloneTool::slotCancel()
{
    writeSettings();
    d->previewWidget->imageIface()->putOriginalImage(i18n("Clone Tool"), filter()->filterAction(),d->origImage.bits());
    slotAbort();
    emit cancelClicked();
}
*/

} // namespace DigikamEnhanceImagePlugin  
            //d->previewRImage.detach();
