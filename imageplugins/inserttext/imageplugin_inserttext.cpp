/* ============================================================
 * File  : imageplugin_inserttext.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-01-20
 * Description : 
 * 
 * Copyright 2005 by Gilles Caulier
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

// KDE includes.
  
#include <klocale.h>
#include <kgenericfactory.h>
#include <klibloader.h>
#include <kaction.h>
#include <kcursor.h>
#include <kdebug.h>

// Local includes.

#include "bannerwidget.h"
#include "imageeffect_inserttext.h"
#include "imageplugin_inserttext.h"

K_EXPORT_COMPONENT_FACTORY( digikamimageplugin_inserttext,
                            KGenericFactory<ImagePlugin_InsertText>("digikamimageplugin_inserttext"));

ImagePlugin_InsertText::ImagePlugin_InsertText(QObject *parent, const char*, const QStringList &)
                      : Digikam::ImagePlugin(parent, "ImagePlugin_InsertText")
{
    m_insertTextAction = new KAction(i18n("Insert Text..."), "inserttext", 0, 
                         this, SLOT(slotInsertText()),
                         actionCollection(), "imageplugin_inserttext");

    setXMLFile("digikamimageplugin_inserttext_ui.rc");
    
    kdDebug() << "ImagePlugin_InsertText plugin loaded" << endl;
}

ImagePlugin_InsertText::~ImagePlugin_InsertText()
{
}

void ImagePlugin_InsertText::setEnabledActions(bool enable)
{
    m_insertTextAction->setEnabled(enable);
}

void ImagePlugin_InsertText::slotInsertText()
{
    QString title = i18n("Insert Text on Photograph");
    QFrame *headerFrame = new DigikamImagePlugins::BannerWidget(0, title);
    DigikamInsertTextImagesPlugin::ImageEffect_InsertText dlg(parentWidget(),
                                    title, headerFrame);
    dlg.exec();
    delete headerFrame;
}

#include "imageplugin_inserttext.moc"
