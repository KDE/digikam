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


#include "texturetool.h"
#include "texturetool.moc"

// Qt includes.

#include <QGridLayout>
#include <QImage>
#include <QLabel>

// KDE includes.

#include <kaboutdata.h>
#include <kdebug.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <knuminput.h>
#include <kstandarddirs.h>

// LibKDcraw includes.

#include <libkdcraw/rnuminput.h>
#include <libkdcraw/rcombobox.h>

// Local includes.

#include "daboutdata.h"
#include "dimg.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imagepanelwidget.h"
#include "version.h"
#include "texture.h"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamTextureImagesPlugin
{

TextureTool::TextureTool(QObject* parent)
           : EditorToolThreaded(parent)
{
    setObjectName("texture");
    setToolName(i18n("Texture"));
    setToolIcon(SmallIcon("texture"));

    // -------------------------------------------------------------

    m_gboxSettings    = new EditorToolSettings(EditorToolSettings::Default|
                                               EditorToolSettings::Ok|
                                               EditorToolSettings::Cancel|
                                               EditorToolSettings::PanIcon);
    QGridLayout* grid = new QGridLayout(m_gboxSettings->plainPage());

    QLabel *label1    = new QLabel(i18n("Type:"), m_gboxSettings->plainPage());

    m_textureType     = new RComboBox(m_gboxSettings->plainPage());
    m_textureType->addItem(i18n("Paper"));
    m_textureType->addItem(i18n("Paper 2"));
    m_textureType->addItem(i18n("Fabric"));
    m_textureType->addItem(i18n("Burlap"));
    m_textureType->addItem(i18n("Bricks"));
    m_textureType->addItem(i18n("Bricks 2"));
    m_textureType->addItem(i18n("Canvas"));
    m_textureType->addItem(i18n("Marble"));
    m_textureType->addItem(i18n("Marble 2"));
    m_textureType->addItem(i18n("Blue Jean"));
    m_textureType->addItem(i18n("Cell Wood"));
    m_textureType->addItem(i18n("Metal Wire"));
    m_textureType->addItem(i18n("Modern"));
    m_textureType->addItem(i18n("Wall"));
    m_textureType->addItem(i18n("Moss"));
    m_textureType->addItem(i18n("Stone"));
    m_textureType->setDefaultIndex(PaperTexture);
    m_textureType->setWhatsThis( i18n("Set here the texture type to apply to image."));

    // -------------------------------------------------------------

    QLabel *label2 = new QLabel(i18n("Relief:"), m_gboxSettings->plainPage());

    m_blendGain    = new RIntNumInput(m_gboxSettings->plainPage());
    m_blendGain->setRange(1, 255, 1);
    m_blendGain->setSliderEnabled(true);
    m_blendGain->setDefaultValue(200);
    m_blendGain->setWhatsThis( i18n("Set here the relief gain used to merge "
                                    "texture and image."));

    // -------------------------------------------------------------

    grid->addWidget(label1,        0, 0, 1, 1);
    grid->addWidget(m_textureType, 0, 1, 1, 1);
    grid->addWidget(label2,        1, 0, 1, 2);
    grid->addWidget(m_blendGain,   2, 0, 1, 2);
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
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("texture Tool");
    m_textureType->blockSignals(true);
    m_blendGain->blockSignals(true);
    m_textureType->setCurrentIndex(group.readEntry("TextureType", m_textureType->defaultIndex()));
    m_blendGain->setValue(group.readEntry("BlendGain", m_blendGain->defaultValue()));
    m_textureType->blockSignals(false);
    m_blendGain->blockSignals(false);
}

void TextureTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("texture Tool");
    group.writeEntry("TextureType", m_textureType->currentIndex());
    group.writeEntry("BlendGain", m_blendGain->value());
    m_previewWidget->writeSettings();
    group.sync();
}

void TextureTool::slotResetSettings()
{
    m_textureType->blockSignals(true);
    m_blendGain->blockSignals(true);

    m_textureType->slotReset();
    m_blendGain->slotReset();

    m_textureType->blockSignals(false);
    m_blendGain->blockSignals(false);

    slotEffect();
}

void TextureTool::prepareEffect()
{
    m_textureType->setEnabled(false);
    m_blendGain->setEnabled(false);

    DImg image      = m_previewWidget->getOriginalRegionImage();
    QString texture = getTexturePath( m_textureType->currentIndex() );

    int b = 255 - m_blendGain->value();

    setFilter(dynamic_cast<DImgThreadedFilter*>(new Texture(&image, this, b, texture)));
}

void TextureTool::prepareFinal()
{
    m_textureType->setEnabled(false);
    m_blendGain->setEnabled(false);

    int b = 255 - m_blendGain->value();

    ImageIface iface(0, 0);
    QString texture = getTexturePath( m_textureType->currentIndex() );

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

    return (KStandardDirs::locate("data", QString("digikam/data/") + pattern + QString(".png")));
}

}  // namespace DigikamTextureImagesPlugin
