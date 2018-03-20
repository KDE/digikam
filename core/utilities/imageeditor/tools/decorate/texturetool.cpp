/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-10
 * Description : a tool to apply texture over an image
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Qt includes

#include <QGridLayout>
#include <QImage>
#include <QLabel>
#include <QIcon>
#include <QStandardPaths>

// KDE includes

#include <ksharedconfig.h>
#include <klocalizedstring.h>

// Local includes

#include "dnuminput.h"
#include "dimg.h"
#include "dcombobox.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imageregionwidget.h"
#include "texturefilter.h"

namespace Digikam
{

class TextureTool::Private
{
public:

    enum TextureTypes
    {
        PaperTexture=0,
        Paper2Texture,
        FabricTexture,
        BurlapTexture,
        BricksTexture,
        Bricks2Texture,
        CanvasTexture,
        MarbleTexture,
        Marble2Texture,
        BlueJeanTexture,
        CellWoodTexture,
        MetalWireTexture,
        ModernTexture,
        WallTexture,
        MossTexture,
        StoneTexture
    };

public:

    Private():
        textureType(0),
        blendGain(0),
        previewWidget(0),
        gboxSettings(0)
    {
    }

    static const QString configGroupName;
    static const QString configTextureTypeEntry;
    static const QString configBlendGainEntry;

    DComboBox*           textureType;
    DIntNumInput*        blendGain;
    ImageRegionWidget*   previewWidget;
    EditorToolSettings*  gboxSettings;
};

const QString TextureTool::Private::configGroupName(QLatin1String("texture Tool"));
const QString TextureTool::Private::configTextureTypeEntry(QLatin1String("TextureType"));
const QString TextureTool::Private::configBlendGainEntry(QLatin1String("BlendGain"));

// --------------------------------------------------------

TextureTool::TextureTool(QObject* const parent)
    : EditorToolThreaded(parent),
      d(new Private)
{
    setObjectName(QLatin1String("texture"));
    setToolName(i18n("Texture"));
    setToolIcon(QIcon::fromTheme(QLatin1String("texture")));

    // -------------------------------------------------------------

    d->gboxSettings  = new EditorToolSettings;
    d->previewWidget = new ImageRegionWidget;

    // -------------------------------------------------------------

    QLabel* label1 = new QLabel(i18n("Type:"));
    d->textureType = new DComboBox;
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
    d->textureType->setDefaultIndex(Private::PaperTexture);
    d->textureType->setWhatsThis( i18n("Set here the texture type to apply to image."));

    // -------------------------------------------------------------

    QLabel* label2 = new QLabel(i18n("Relief:"));
    d->blendGain   = new DIntNumInput;
    d->blendGain->setRange(1, 255, 1);
    d->blendGain->setDefaultValue(200);
    d->blendGain->setWhatsThis(i18n("Set here the relief gain used to merge texture and image."));

    // -------------------------------------------------------------

    const int spacing = d->gboxSettings->spacingHint();

    QGridLayout* mainLayout = new QGridLayout;
    mainLayout->addWidget(label1,         0, 0, 1, 1);
    mainLayout->addWidget(d->textureType, 0, 1, 1, 1);
    mainLayout->addWidget(label2,         1, 0, 1, 2);
    mainLayout->addWidget(d->blendGain,   2, 0, 1, 2);
    mainLayout->setRowStretch(3, 10);
    mainLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    mainLayout->setSpacing(spacing);
    d->gboxSettings->plainPage()->setLayout(mainLayout);

    // -------------------------------------------------------------

    setToolSettings(d->gboxSettings);
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::AllPreviewModes);

    // -------------------------------------------------------------

    connect(d->textureType, SIGNAL(activated(int)),
            this, SLOT(slotPreview()));

    connect(d->blendGain, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));
}

TextureTool::~TextureTool()
{
    delete d;
}

void TextureTool::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);
    d->textureType->blockSignals(true);
    d->blendGain->blockSignals(true);
    d->textureType->setCurrentIndex(group.readEntry(d->configTextureTypeEntry, d->textureType->defaultIndex()));
    d->blendGain->setValue(group.readEntry(d->configBlendGainEntry,            d->blendGain->defaultValue()));
    d->textureType->blockSignals(false);
    d->blendGain->blockSignals(false);
    slotPreview();
}

void TextureTool::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);
    group.writeEntry(d->configTextureTypeEntry, d->textureType->currentIndex());
    group.writeEntry(d->configBlendGainEntry,   d->blendGain->value());
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

    slotPreview();
}

void TextureTool::preparePreview()
{
    DImg image      = d->previewWidget->getOriginalRegionImage();
    QString texture = getTexturePath( d->textureType->currentIndex() );
    int b           = 255 - d->blendGain->value();

    setFilter(new TextureFilter(&image, this, b, texture));
}

void TextureTool::prepareFinal()
{
    ImageIface iface;
    QString texture = getTexturePath( d->textureType->currentIndex() );
    int b           = 255 - d->blendGain->value();

    setFilter(new TextureFilter(iface.original(), this, b, texture));
}

void TextureTool::setPreviewImage()
{
    d->previewWidget->setPreviewImage(filter()->getTargetImage());
}

void TextureTool::setFinalImage()
{
    ImageIface iface;
    iface.setOriginal(i18n("Texture"), filter()->filterAction(), filter()->getTargetImage());
}

QString TextureTool::getTexturePath(int texture)
{
    QString pattern;

    switch (texture)
    {
        case Private::PaperTexture:
            pattern = QLatin1String("paper-texture");
            break;

        case Private::Paper2Texture:
            pattern = QLatin1String("paper2-texture");
            break;

        case Private::FabricTexture:
            pattern = QLatin1String("fabric-texture");
            break;

        case Private::BurlapTexture:
            pattern = QLatin1String("burlap-texture");
            break;

        case Private::BricksTexture:
            pattern = QLatin1String("bricks-texture");
            break;

        case Private::Bricks2Texture:
            pattern = QLatin1String("bricks2-texture");
            break;

        case Private::CanvasTexture:
            pattern = QLatin1String("canvas-texture");
            break;

        case Private::MarbleTexture:
            pattern = QLatin1String("marble-texture");
            break;

        case Private::Marble2Texture:
            pattern = QLatin1String("marble2-texture");
            break;

        case Private::BlueJeanTexture:
            pattern = QLatin1String("bluejean-texture");
            break;

        case Private::CellWoodTexture:
            pattern = QLatin1String("cellwood-texture");
            break;

        case Private::MetalWireTexture:
            pattern = QLatin1String("metalwire-texture");
            break;

        case Private::ModernTexture:
            pattern = QLatin1String("modern-texture");
            break;

        case Private::WallTexture:
            pattern = QLatin1String("wall-texture");
            break;

        case Private::MossTexture:
            pattern = QLatin1String("moss-texture");
            break;

        case Private::StoneTexture:
            pattern = QLatin1String("stone-texture");
            break;
    }

    return (QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/") + pattern + QLatin1String(".png")));
}

}  // namespace Digikam
