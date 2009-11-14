/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-10
 * Description : a plugin to apply texture over an image
 *
 * Copyright (C) 2005-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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


// #include "texturetool.h"
#include "texturetool.moc"

// Qt includes

#include <QGridLayout>
#include <QImage>
#include <QLabel>

// KDE includes

#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <knuminput.h>
#include <kstandarddirs.h>

// LibKDcraw includes

#include <libkdcraw/rcombobox.h>
#include <libkdcraw/rnuminput.h>

// Local includes

#include "daboutdata.h"
#include "dimg.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imagepanelwidget.h"
#include "texture.h"
#include "version.h"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamTextureImagesPlugin
{

class TextureToolPriv
{
public:

    TextureToolPriv():
        configGroupName("texture Tool"),
        configTextureTypeEntry("TextureType"),
        configBlendGainEntry("BlendGain"),

        textureType(0),
        blendGain(0),
        previewWidget(0),
        gboxSettings(0)
        {}

    const QString       configGroupName;
    const QString       configTextureTypeEntry;
    const QString       configBlendGainEntry;

    RComboBox*          textureType;
    RIntNumInput*       blendGain;
    ImagePanelWidget*   previewWidget;
    EditorToolSettings* gboxSettings;
};

TextureTool::TextureTool(QObject* parent)
           : EditorToolThreaded(parent),
             d(new TextureToolPriv)
{
    setObjectName("texture");
    setToolName(i18n("Texture"));
    setToolIcon(SmallIcon("texture"));

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setTools(EditorToolSettings::PanIcon);

    d->previewWidget = new ImagePanelWidget(470, 350, "texture Tool", d->gboxSettings->panIconView());

    // -------------------------------------------------------------

    QLabel *label1 = new QLabel(i18n("Type:"));
    d->textureType = new RComboBox;
    d->textureType->addItem(i18n("Paper"));
    d->textureType->addItem(i18n("Paper 2"));
    d->textureType->addItem(i18n("Fabric"));
    d->textureType->addItem(i18n("Burlap"));
    d->textureType->addItem(i18n("Bricks"));
    d->textureType->addItem(i18n("Bricks 2"));
    d->textureType->addItem(i18n("Canvas"));
    d->textureType->addItem(i18n("Marble"));
    d->textureType->addItem(i18n("Marble 2"));
    d->textureType->addItem(i18n("Blue Jean"));
    d->textureType->addItem(i18n("Cell Wood"));
    d->textureType->addItem(i18n("Metal Wire"));
    d->textureType->addItem(i18n("Modern"));
    d->textureType->addItem(i18n("Wall"));
    d->textureType->addItem(i18n("Moss"));
    d->textureType->addItem(i18n("Stone"));
    d->textureType->setDefaultIndex(PaperTexture);
    d->textureType->setWhatsThis( i18n("Set here the texture type to apply to image."));

    // -------------------------------------------------------------

    QLabel *label2 = new QLabel(i18n("Relief:"));
    d->blendGain   = new RIntNumInput;
    d->blendGain->setRange(1, 255, 1);
    d->blendGain->setSliderEnabled(true);
    d->blendGain->setDefaultValue(200);
    d->blendGain->setWhatsThis(i18n("Set here the relief gain used to merge texture and image."));

    // -------------------------------------------------------------

    QGridLayout* mainLayout = new QGridLayout;
    mainLayout->addWidget(label1,         0, 0, 1, 1);
    mainLayout->addWidget(d->textureType, 0, 1, 1, 1);
    mainLayout->addWidget(label2,         1, 0, 1, 2);
    mainLayout->addWidget(d->blendGain,   2, 0, 1, 2);
    mainLayout->setRowStretch(3, 10);
    mainLayout->setMargin(d->gboxSettings->spacingHint());
    mainLayout->setSpacing(d->gboxSettings->spacingHint());
    d->gboxSettings->plainPage()->setLayout(mainLayout);

    // -------------------------------------------------------------

    setToolSettings(d->gboxSettings);
    setToolView(d->previewWidget);
    init();

    // -------------------------------------------------------------

    connect(d->previewWidget, SIGNAL(signalOriginalClipFocusChanged()),
            this, SLOT(slotEffect()));

    connect(d->textureType, SIGNAL(activated(int)),
            this, SLOT(slotEffect()));

    connect(d->blendGain, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));
}

TextureTool::~TextureTool()
{
    delete d;
}

void TextureTool::renderingFinished()
{
    d->textureType->setEnabled(true);
    d->blendGain->setEnabled(true);
}

void TextureTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    d->textureType->blockSignals(true);
    d->blendGain->blockSignals(true);
    d->textureType->setCurrentIndex(group.readEntry(d->configTextureTypeEntry, d->textureType->defaultIndex()));
    d->blendGain->setValue(group.readEntry(d->configBlendGainEntry,            d->blendGain->defaultValue()));
    d->textureType->blockSignals(false);
    d->blendGain->blockSignals(false);
    slotEffect();
}

void TextureTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    group.writeEntry(d->configTextureTypeEntry, d->textureType->currentIndex());
    group.writeEntry(d->configBlendGainEntry,   d->blendGain->value());
    d->previewWidget->writeSettings();
    group.sync();
}

void TextureTool::slotResetSettings()
{
    d->textureType->blockSignals(true);
    d->blendGain->blockSignals(true);

    d->textureType->slotReset();
    d->blendGain->slotReset();

    d->textureType->blockSignals(false);
    d->blendGain->blockSignals(false);

    slotEffect();
}

void TextureTool::prepareEffect()
{
    d->textureType->setEnabled(false);
    d->blendGain->setEnabled(false);

    DImg image      = d->previewWidget->getOriginalRegionImage();
    QString texture = getTexturePath( d->textureType->currentIndex() );

    int b = 255 - d->blendGain->value();

    setFilter(dynamic_cast<DImgThreadedFilter*>(new Texture(&image, this, b, texture)));
}

void TextureTool::prepareFinal()
{
    d->textureType->setEnabled(false);
    d->blendGain->setEnabled(false);

    int b = 255 - d->blendGain->value();

    ImageIface iface(0, 0);
    QString texture = getTexturePath( d->textureType->currentIndex() );

    setFilter(dynamic_cast<DImgThreadedFilter*>(new Texture(iface.getOriginalImg(), this, b, texture)));
}

void TextureTool::putPreviewData()
{
    d->previewWidget->setPreviewImage(filter()->getTargetImage());
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
