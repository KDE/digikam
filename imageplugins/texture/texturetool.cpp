/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-10
 * Description : a plugin to apply texture over an image
 *
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qwhatsthis.h>

// KDE includes.

#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>

// LibKDcraw includes.

#include <libkdcraw/rnuminput.h>
#include <libkdcraw/rcombobox.h>

// Local includes.

#include "daboutdata.h"
#include "ddebug.h"
#include "dimg.h"
#include "imageiface.h"
#include "imagepanelwidget.h"
#include "editortoolsettings.h"
#include "texture.h"
#include "texturetool.h"
#include "texturetool.moc"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamTextureImagesPlugin
{

TextureTool::TextureTool(QObject* parent)
           : EditorToolThreaded(parent)
{
    setName("texture");
    setToolName(i18n("Texture"));
    setToolIcon(SmallIcon("texture"));

    // -------------------------------------------------------------

    m_gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                            EditorToolSettings::Ok|
                                            EditorToolSettings::Cancel|
                                            EditorToolSettings::PanIcon);
    QGridLayout* grid = new QGridLayout( m_gboxSettings->plainPage(), 3, 1);
    QLabel *label1    = new QLabel(i18n("Type:"), m_gboxSettings->plainPage());

    m_textureType = new RComboBox(m_gboxSettings->plainPage());
    m_textureType->insertItem(i18n("Paper"));
    m_textureType->insertItem(i18n("Paper 2"));
    m_textureType->insertItem(i18n("Fabric"));
    m_textureType->insertItem(i18n("Burlap"));
    m_textureType->insertItem(i18n("Bricks"));
    m_textureType->insertItem(i18n("Bricks 2"));
    m_textureType->insertItem(i18n("Canvas"));
    m_textureType->insertItem(i18n("Marble"));
    m_textureType->insertItem(i18n("Marble 2"));
    m_textureType->insertItem(i18n("Blue Jean"));
    m_textureType->insertItem(i18n("Cell Wood"));
    m_textureType->insertItem(i18n("Metal Wire"));
    m_textureType->insertItem(i18n("Modern"));
    m_textureType->insertItem(i18n("Wall"));
    m_textureType->insertItem(i18n("Moss"));
    m_textureType->insertItem(i18n("Stone"));
    m_textureType->setDefaultItem(PaperTexture);
    QWhatsThis::add( m_textureType, i18n("<p>Set here the texture type to apply to the image."));

    // -------------------------------------------------------------

    QLabel *label2 = new QLabel(i18n("Relief:"), m_gboxSettings->plainPage());

    m_blendGain = new RIntNumInput(m_gboxSettings->plainPage());
    m_blendGain->setRange(1, 255, 1);
    m_blendGain->setDefaultValue(200);
    QWhatsThis::add( m_blendGain, i18n("<p>Set here the relief gain used to merge texture and image."));

    grid->addMultiCellWidget(label1,        0, 0, 0, 0);
    grid->addMultiCellWidget(m_textureType, 0, 0, 1, 1);
    grid->addMultiCellWidget(label2,        1, 1, 0, 1);
    grid->addMultiCellWidget(m_blendGain,   2, 2, 0, 1);
    grid->setRowStretch(3, 10);
    grid->setMargin(m_gboxSettings->spacingHint());
    grid->setSpacing(m_gboxSettings->spacingHint());

    setToolSettings(m_gboxSettings);

    // -------------------------------------------------------------

    m_previewWidget = new ImagePanelWidget(470, 350, "texture Tool", m_gboxSettings->panIconView());

    setToolView(m_previewWidget);
    init();

    // -------------------------------------------------------------

    connect(m_textureType, SIGNAL(activated(int)),
            this, SLOT(slotEffect()));

    connect(m_blendGain, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));
}

TextureTool::~TextureTool()
{
}

void TextureTool::renderingFinished()
{
    m_textureType->setEnabled(true);
    m_blendGain->setEnabled(true);
}

void TextureTool::readSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("texture Tool");

    m_textureType->blockSignals(true);
    m_blendGain->blockSignals(true);

    m_textureType->setCurrentItem(config->readNumEntry("TextureType", m_textureType->defaultItem()));
    m_blendGain->setValue(config->readNumEntry("BlendGain", m_blendGain->defaultValue()));

    m_textureType->blockSignals(false);
    m_blendGain->blockSignals(false);
}

void TextureTool::writeSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("texture Tool");
    config->writeEntry("TextureType", m_textureType->currentItem());
    config->writeEntry("BlendGain", m_blendGain->value());
    m_previewWidget->writeSettings();
    config->sync();
}

void TextureTool::slotResetSettings()
{
    m_textureType->blockSignals(true);
    m_blendGain->blockSignals(true);

    m_textureType->slotReset();
    m_blendGain->slotReset();

    m_textureType->blockSignals(false);
    m_blendGain->blockSignals(false);
}

void TextureTool::prepareEffect()
{
    m_textureType->setEnabled(false);
    m_blendGain->setEnabled(false);

    DImg image = m_previewWidget->getOriginalRegionImage();
    QString texture = getTexturePath( m_textureType->currentItem() );

    int b = 255 - m_blendGain->value();

    setFilter(dynamic_cast<DImgThreadedFilter*>(new Texture(&image, this, b, texture)));
}

void TextureTool::prepareFinal()
{
    m_textureType->setEnabled(false);
    m_blendGain->setEnabled(false);

    int b = 255 - m_blendGain->value();

    ImageIface iface(0, 0);
    QString texture = getTexturePath( m_textureType->currentItem() );

    setFilter(dynamic_cast<DImgThreadedFilter*>(new Texture(iface.getOriginalImg(), this, b, texture)));
}

void TextureTool::putPreviewData()
{
    m_previewWidget->setPreviewImage(filter()->getTargetImage());
}

void TextureTool::putFinalData()
{
    ImageIface iface(0, 0);
    iface.putOriginalImage(i18n("Texture"), filter()->getTargetImage().bits());
}

QString TextureTool::getTexturePath(int texture)
{
    QString pattern;

    switch (texture)
       {
       case PaperTexture:
          pattern = "paper-texture";
          break;

       case Paper2Texture:
          pattern = "paper2-texture";
          break;

       case FabricTexture:
          pattern = "fabric-texture";
          break;

       case BurlapTexture:
          pattern = "burlap-texture";
          break;

       case BricksTexture:
          pattern = "bricks-texture";
          break;

       case Bricks2Texture:
          pattern = "bricks2-texture";
          break;

       case CanvasTexture:
          pattern = "canvas-texture";
          break;

       case MarbleTexture:
          pattern = "marble-texture";
          break;

       case Marble2Texture:
          pattern = "marble2-texture";
          break;

       case BlueJeanTexture:
          pattern = "bluejean-texture";
          break;

       case CellWoodTexture:
          pattern = "cellwood-texture";
          break;

       case MetalWireTexture:
          pattern = "metalwire-texture";
          break;

       case ModernTexture:
          pattern = "modern-texture";
          break;

       case WallTexture:
          pattern = "wall-texture";
          break;

       case MossTexture:
          pattern = "moss-texture";
          break;

       case StoneTexture:
          pattern = "stone-texture";
          break;
       }

    KGlobal::dirs()->addResourceType(pattern.ascii(), KGlobal::dirs()->kde_default("data") + "digikam/data");
    return (KGlobal::dirs()->findResourceDir(pattern.ascii(), pattern + ".png") + pattern + ".png" );
}

}  // NameSpace DigikamTextureImagesPlugin
