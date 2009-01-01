/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-06
 * Description : digiKam image editor Ratio Crop tool
 *
 * Copyright (C) 2007 by Jaromir Malenko <malenko at email dot cz>
 * Copyright (C) 2008 by Roberto Castagnola <roberto dot castagnola at gmail dot com>
 * Copyright (C) 2004-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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


#include "ratiocroptool.h"
#include "ratiocroptool.moc"

// Qt includes.

#include <QCheckBox>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QImage>
#include <QLabel>
#include <QPixmap>
#include <QRect>
#include <QSpinBox>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

// KDE includes.

#include <kapplication.h>
#include <kcolorbutton.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kcursor.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kstandardguiitem.h>
#include <kpushbutton.h>

// LibKDcraw includes.

#include <libkdcraw/rnuminput.h>
#include <libkdcraw/rcombobox.h>

// Local includes.

#include "editortoolsettings.h"
#include "imageiface.h"
#include "imageselectionwidget.h"


using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamImagesPluginCore
{

RatioCropTool::RatioCropTool(QObject* parent)
             : EditorTool(parent)
{
    setObjectName("aspectratiocrop");
    setToolName(i18n("Aspect Ratio Crop"));
    setToolIcon(SmallIcon("ratiocrop"));
    setToolHelp("ratiocroptool.anchor");

    // -------------------------------------------------------------

    m_imageSelectionWidget = new ImageSelectionWidget(480, 320);
    m_imageSelectionWidget->setWhatsThis(i18n("<p>Here you can see the aspect ratio selection preview "
                                              "used for cropping. You can use the mouse to move and "
                                              "resize the crop area.</p>"
                                              "<p>Press and hold the <b>CTRL</b> key to move the opposite corner too.</p>"
                                              "<p>Press and hold the <b>SHIFT</b> key to move the closest corner to the "
                                              "mouse pointer.</p>"));

    m_originalIsLandscape = ((m_imageSelectionWidget->getOriginalImageWidth()) >
    (m_imageSelectionWidget->getOriginalImageHeight()));

    setToolView(m_imageSelectionWidget);

    // -------------------------------------------------------------

    m_gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                            EditorToolSettings::Ok|
                                            EditorToolSettings::Try|
                                            EditorToolSettings::Cancel);

    // -------------------------------------------------------------

    // need to set the button to a KStdGuiItem that has no icon
    KPushButton *tryBtn = m_gboxSettings->button(EditorToolSettings::Try);
    tryBtn->setGuiItem(KStandardGuiItem::Test);
    tryBtn->setText(i18n("Max. Aspect"));
    tryBtn->setToolTip(i18n("Set selection area to the maximum size according "
                            "to the current ratio."));

    // -------------------------------------------------------------

    QGridLayout *gridBox2 = new QGridLayout(m_gboxSettings->plainPage());

    QFrame *cropSelection = new QFrame( m_gboxSettings->plainPage() );
    QGridLayout* grid     = new QGridLayout(cropSelection);
    cropSelection->setFrameStyle(QFrame::Panel|QFrame::Sunken);

    QLabel *label = new QLabel(i18n("Aspect ratio:"), cropSelection);
    m_ratioCB     = new RComboBox(cropSelection);
    m_ratioCB->addItem(i18nc("custom aspect ratio crop settings", "Custom"));
    m_ratioCB->addItem("1:1");
    m_ratioCB->addItem("2:3");
    m_ratioCB->addItem("3:4");
    m_ratioCB->addItem("4:5");
    m_ratioCB->addItem("5:7");
    m_ratioCB->addItem("7:10");
    m_ratioCB->addItem(i18n("Golden Ratio"));
    m_ratioCB->addItem(i18nc("no crop mode", "None"));
    m_ratioCB->setDefaultIndex(ImageSelectionWidget::RATIO03X04);
    setRatioCBText(ImageSelectionWidget::Landscape);
    m_ratioCB->setWhatsThis( i18n("<p>Select your constrained aspect ratio for cropping. "
                                  "Aspect Ratio Crop tool uses a relative ratio. That means it "
                                  "is the same if you use centimeters or inches and it does not "
                                  "specify the physical size.</p>"
                                  "<p>You can see below a correspondence list of traditional photographic "
                                  "paper sizes and aspect ratio crop:</p>"
                                  "<p><b>2:3</b>: 10x15cm, 20x30cm, 30x45cm, 4x6\", 8x12\", "
                                  "12x18\", 16x24\", 20x30\"</p>"
                                  "<p><b>3:4</b>: 6x8cm, 15x20cm, 18x24cm, 30x40cm, 3.75x5\", 4.5x6\", "
                                  "6x8\", 7.5x10\", 9x12\"</p>"
                                  "<p><b>4:5</b>: 20x25cm, 40x50cm, 8x10\", 16x20\"</p>"
                                  "<p><b>5:7</b>: 15x21cm, 30x42cm, 5x7\"</p>"
                                  "<p><b>7:10</b>: 21x30cm, 42x60cm, 3.5x5\"</p>"
                                  "<p>The <b>Golden Ratio</b> is 1:1.618. A composition following this rule "
                                  "is considered visually harmonious but can be unadapted to print on "
                                  "standard photographic paper.</p>"));

    m_preciseCrop = new QCheckBox(i18n("Exact"), cropSelection);
    m_preciseCrop->setWhatsThis( i18n("Enable this option to force exact aspect ratio crop."));

    m_orientLabel = new QLabel(i18n("Orientation:"), cropSelection);
    m_orientCB    = new RComboBox( cropSelection );
    m_orientCB->addItem( i18n("Landscape") );
    m_orientCB->addItem( i18n("Portrait") );
    m_orientCB->setWhatsThis( i18n("Select constrained aspect ratio orientation."));

    m_autoOrientation = new QCheckBox(i18n("Auto"), cropSelection);
    m_autoOrientation->setWhatsThis( i18n("Enable this option to automatically set the orientation."));

    // -------------------------------------------------------------

    m_customLabel1 = new QLabel(i18n("Custom ratio:"), cropSelection);
    m_customLabel1->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    m_customRatioNInput = new RIntNumInput(cropSelection);
    m_customRatioNInput->setRange(1, 10000, 1);
    m_customRatioNInput->setDefaultValue(1);
    m_customRatioNInput->setSliderEnabled(false);
    m_customRatioNInput->setWhatsThis( i18n("Set here the desired custom aspect numerator value."));

    m_customLabel2 = new QLabel(" : ", cropSelection);
    m_customLabel2->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
    m_customRatioDInput = new RIntNumInput(cropSelection);
    m_customRatioDInput->setRange(1, 10000, 1);
    m_customRatioDInput->setDefaultValue(1);
    m_customRatioDInput->setSliderEnabled(false);
    m_customRatioDInput->setWhatsThis( i18n("Set here the desired custom aspect denominator value."));

    // -------------------------------------------------------------

    m_xInput = new RIntNumInput(cropSelection);
    m_xInput->setWhatsThis( i18n("Set here the top left selection corner position for cropping."));
    m_xInput->input()->setLabel(i18nc("top left corner position for cropping", "X:"), Qt::AlignLeft|Qt::AlignVCenter);
    m_xInput->setRange(0, m_imageSelectionWidget->getOriginalImageWidth(), 1);
    m_xInput->setSliderEnabled(true);
    m_xInput->setDefaultValue(50);

    m_widthInput = new RIntNumInput(cropSelection);
    m_widthInput->input()->setLabel(i18n("Width:"), Qt::AlignLeft|Qt::AlignVCenter);
    m_widthInput->setWhatsThis( i18n("Set here the width selection for cropping."));
    m_widthInput->setRange(m_imageSelectionWidget->getMinWidthRange(),
                           m_imageSelectionWidget->getMaxWidthRange(),
                           m_imageSelectionWidget->getWidthStep());
    m_widthInput->setSliderEnabled(true);
    m_widthInput->setDefaultValue(800);

    m_centerWidth = new QToolButton(cropSelection);
    m_centerWidth->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/centerwidth.png")));
    m_centerWidth->setWhatsThis( i18n("Set width position to center."));

    // -------------------------------------------------------------

    m_yInput = new RIntNumInput(cropSelection);
    m_yInput->input()->setLabel(i18n("Y:"), Qt::AlignLeft|Qt::AlignVCenter);
    m_yInput->setWhatsThis( i18n("Set here the top left selection corner position for cropping."));
    m_yInput->setRange(0, m_imageSelectionWidget->getOriginalImageWidth(), 1);
    m_yInput->setSliderEnabled(true);
    m_yInput->setDefaultValue(50);

    m_heightInput = new RIntNumInput(cropSelection);
    m_heightInput->input()->setLabel(i18n("Height:"), Qt::AlignLeft|Qt::AlignVCenter);
    m_heightInput->setWhatsThis( i18n("Set here the height selection for cropping."));
    m_heightInput->setRange(m_imageSelectionWidget->getMinHeightRange(),
                            m_imageSelectionWidget->getMaxHeightRange(),
                            m_imageSelectionWidget->getHeightStep());
    m_heightInput->setSliderEnabled(true);
    m_heightInput->setDefaultValue(600);

    m_centerHeight = new QToolButton(cropSelection);
    m_centerHeight->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/centerheight.png")));
    m_centerHeight->setWhatsThis( i18n("Set height position to center."));

    // -------------------------------------------------------------

    QFrame* compositionGuide = new QFrame( m_gboxSettings->plainPage() );
    QGridLayout* grid2       = new QGridLayout(compositionGuide);
    compositionGuide->setFrameStyle(QFrame::Panel|QFrame::Sunken);

    QLabel *labelGuideLines = new QLabel(i18n("Composition guide:"), compositionGuide);
    m_guideLinesCB = new RComboBox(compositionGuide);
    m_guideLinesCB->addItem(i18n("Rules of Thirds"));
    m_guideLinesCB->addItem(i18n("Harmonious Triangles"));
    m_guideLinesCB->addItem(i18n("Golden Mean"));
    m_guideLinesCB->addItem(i18nc("no composition guide", "None"));
    m_guideLinesCB->setDefaultIndex(ImageSelectionWidget::GuideNone);
    m_guideLinesCB->setCurrentIndex(3);
    m_guideLinesCB->setWhatsThis( i18n("With this option, you can display guide lines "
                                       "which help you to compose your photograph."));

    m_goldenSectionBox = new QCheckBox(i18n("Golden sections"), compositionGuide);
    m_goldenSectionBox->setWhatsThis(i18n("Enable this option to show golden sections."));

    m_goldenSpiralSectionBox = new QCheckBox(i18n("Golden spiral sections"), compositionGuide);
    m_goldenSpiralSectionBox->setWhatsThis(i18n("Enable this option to show golden spiral sections."));

    m_goldenSpiralBox = new QCheckBox(i18n("Golden spiral"), compositionGuide);
    m_goldenSpiralBox->setWhatsThis(i18n("Enable this option to show golden spiral guide."));

    m_goldenTriangleBox = new QCheckBox(i18n("Golden triangles"), compositionGuide);
    m_goldenTriangleBox->setWhatsThis(i18n("Enable this option to show golden triangles."));

    m_flipHorBox = new QCheckBox(i18n("Flip horizontally"), compositionGuide);
    m_flipHorBox->setWhatsThis(i18n("Enable this option to flip horizontally guidelines."));

    m_flipVerBox = new QCheckBox(i18n("Flip vertically"), compositionGuide);
    m_flipVerBox->setWhatsThis(i18n("Enable this option to flip vertically guidelines."));

    m_colorGuideLabel = new QLabel(i18n("Color and width:"), compositionGuide);
    m_guideColorBt    = new KColorButton( QColor( 250, 250, 255 ), compositionGuide );
    m_guideSize       = new RIntNumInput(compositionGuide);
    m_guideSize->setRange(1, 5, 1);
    m_guideSize->setSliderEnabled(false);
    m_guideSize->setDefaultValue(1);
    m_guideColorBt->setWhatsThis(i18n("Set here the color used to draw composition guides."));
    m_guideSize->setWhatsThis(i18n("Set here the width in pixels used to draw composition guides."));

    // -------------------------------------------------------------

    grid->addWidget(label,                      0, 0, 1, 1);
    grid->addWidget(m_ratioCB,                  0, 1, 1, 3);
    grid->addWidget(m_preciseCrop,              0, 4, 1, 1);
    grid->addWidget(m_orientLabel,              2, 0, 1, 1);
    grid->addWidget(m_orientCB,                 2, 1, 1, 3);
    grid->addWidget(m_autoOrientation,          2, 4, 1, 1);
    grid->addWidget(m_customLabel1,             1, 0, 1, 1);
    grid->addWidget(m_customRatioNInput,        1, 1, 1, 1);
    grid->addWidget(m_customLabel2,             1, 2, 1, 1);
    grid->addWidget(m_customRatioDInput,        1, 3, 1, 1);
    grid->addWidget(m_xInput,                   3, 0, 1, 4);
    grid->addWidget(m_widthInput,               4, 0, 1, 4);
    grid->addWidget(m_centerWidth,              4, 4, 1, 1);
    grid->addWidget(m_yInput,                   5, 0, 1, 4);
    grid->addWidget(m_heightInput,              6, 0, 1, 4);
    grid->addWidget(m_centerHeight,             6, 4, 1, 1);
    grid->setMargin(m_gboxSettings->spacingHint());
    grid->setSpacing(m_gboxSettings->spacingHint());

    grid2->addWidget(labelGuideLines,           0, 0, 1, 1);
    grid2->addWidget(m_guideLinesCB,            0, 1, 1, 2);
    grid2->addWidget(m_goldenSectionBox,        1, 0, 1, 3);
    grid2->addWidget(m_goldenSpiralSectionBox,  2, 0, 1, 3);
    grid2->addWidget(m_goldenSpiralBox,         3, 0, 1, 3);
    grid2->addWidget(m_goldenTriangleBox,       4, 0, 1, 3);
    grid2->addWidget(m_flipHorBox,              5, 0, 1, 3);
    grid2->addWidget(m_flipVerBox,              6, 0, 1, 3);
    grid2->addWidget(m_colorGuideLabel,         7, 0, 1, 1);
    grid2->addWidget(m_guideColorBt,            7, 1, 1, 1);
    grid2->addWidget(m_guideSize,               7, 2, 1, 1);
    grid2->setMargin(m_gboxSettings->spacingHint());
    grid2->setSpacing(m_gboxSettings->spacingHint());

    gridBox2->addWidget(cropSelection,          0, 0, 1, 1);
    gridBox2->addWidget(compositionGuide,       1, 0, 1, 1);
    gridBox2->setRowStretch(2, 10);
    gridBox2->setMargin(m_gboxSettings->spacingHint());
    gridBox2->setSpacing(m_gboxSettings->spacingHint());

    setToolSettings(m_gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(m_ratioCB, SIGNAL(activated(int)),
            this, SLOT(slotRatioChanged(int)));

    connect(m_preciseCrop, SIGNAL(toggled(bool)),
            this, SLOT(slotPreciseCropChanged(bool)));

    connect(m_orientCB, SIGNAL(activated(int)),
            this, SLOT(slotOrientChanged(int)));

    connect(m_autoOrientation, SIGNAL(toggled(bool)),
            this, SLOT(slotAutoOrientChanged(bool)));

    connect(m_xInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotXChanged(int)));

    connect(m_yInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotYChanged(int)));

    connect(m_customRatioNInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotCustomRatioChanged()));

    connect(m_customRatioDInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotCustomRatioChanged()));

    connect(m_guideLinesCB, SIGNAL(activated(int)),
            this, SLOT(slotGuideTypeChanged(int)));

    connect(m_goldenSectionBox, SIGNAL(toggled(bool)),
            this, SLOT(slotGoldenGuideTypeChanged()));

    connect(m_goldenSpiralSectionBox, SIGNAL(toggled(bool)),
            this, SLOT(slotGoldenGuideTypeChanged()));

    connect(m_goldenSpiralBox, SIGNAL(toggled(bool)),
            this, SLOT(slotGoldenGuideTypeChanged()));

    connect(m_goldenTriangleBox, SIGNAL(toggled(bool)),
            this, SLOT(slotGoldenGuideTypeChanged()));

    connect(m_flipHorBox, SIGNAL(toggled(bool)),
            this, SLOT(slotGoldenGuideTypeChanged()));

    connect(m_flipVerBox, SIGNAL(toggled(bool)),
            this, SLOT(slotGoldenGuideTypeChanged()));

    connect(m_guideColorBt, SIGNAL(changed(const QColor &)),
            m_imageSelectionWidget, SLOT(slotChangeGuideColor(const QColor &)));

    connect(m_guideSize, SIGNAL(valueChanged(int)),
            m_imageSelectionWidget, SLOT(slotChangeGuideSize(int)));

    connect(m_widthInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotWidthChanged(int)));

    connect(m_heightInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotHeightChanged(int)));

    connect(m_imageSelectionWidget, SIGNAL(signalSelectionChanged(QRect)),
            this, SLOT(slotSelectionChanged(QRect)));

    connect(m_imageSelectionWidget, SIGNAL(signalSelectionMoved(QRect)),
            this, SLOT(slotSelectionChanged(QRect)));

    connect(m_imageSelectionWidget, SIGNAL(signalSelectionOrientationChanged(int)),
            this, SLOT(slotSelectionOrientationChanged(int)));

    connect(m_centerWidth, SIGNAL(clicked()),
            this, SLOT(slotCenterWidth()));

    connect(m_centerHeight, SIGNAL(clicked()),
            this, SLOT(slotCenterHeight()));

    // we need to disconnect the standard connection of the Try button first
    disconnect(m_gboxSettings, SIGNAL(signalTryClicked()),
               this, SLOT(slotEffect()));

    connect(m_gboxSettings, SIGNAL(signalTryClicked()),
            this, SLOT(slotMaxAspectRatio()));

    // -------------------------------------------------------------

    // Sets current region selection
    slotSelectionChanged(m_imageSelectionWidget->getRegionSelection());

    readSettings();
}

RatioCropTool::~RatioCropTool()
{
}

void RatioCropTool::readSettings()
{
    QColor defaultGuideColor(250, 250, 255);
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("aspectratiocrop Tool");

    // No guide lines per default.
    m_guideLinesCB->setCurrentIndex(group.readEntry("Guide Lines Type",
                                    (int)ImageSelectionWidget::GuideNone));
    m_goldenSectionBox->setChecked(group.readEntry("Golden Section", true));
    m_goldenSpiralSectionBox->setChecked(group.readEntry("Golden Spiral Section", false));
    m_goldenSpiralBox->setChecked(group.readEntry("Golden Spiral", false));
    m_goldenTriangleBox->setChecked(group.readEntry("Golden Triangle", false));
    m_flipHorBox->setChecked(group.readEntry("Golden Flip Horizontal", false));
    m_flipVerBox->setChecked(group.readEntry("Golden Flip Vertical", false));
    m_guideColorBt->setColor(group.readEntry("Guide Color", defaultGuideColor));
    m_guideSize->setValue(group.readEntry("Guide Width", m_guideSize->defaultValue()));
    m_imageSelectionWidget->slotGuideLines(m_guideLinesCB->currentIndex());
    m_imageSelectionWidget->slotChangeGuideColor(m_guideColorBt->color());

    m_preciseCrop->setChecked( group.readEntry("Precise Aspect Ratio Crop", false) );
    m_imageSelectionWidget->setPreciseCrop( m_preciseCrop->isChecked() );

    // Empty selection so it can be moved w/out size constraint
    m_widthInput->setValue(0);
    m_heightInput->setValue(0);

    m_xInput->setValue(group.readEntry("Hor.Oriented Custom Aspect Ratio Xpos",
                       m_xInput->defaultValue()));
    m_yInput->setValue(group.readEntry("Hor.Oriented Custom Aspect Ratio Ypos",
                       m_yInput->defaultValue()));

    m_widthInput->setValue(group.readEntry("Hor.Oriented Custom Aspect Ratio Width",
                           m_widthInput->defaultValue()));
    m_heightInput->setValue(group.readEntry("Hor.Oriented Custom Aspect Ratio Height",
                            m_heightInput->defaultValue()));

    m_imageSelectionWidget->setSelectionOrientation(m_orientCB->currentIndex());

    m_customRatioNInput->setValue(group.readEntry("Hor.Oriented Custom Aspect Ratio Num",
                                  m_customRatioNInput->defaultValue()));
    m_customRatioDInput->setValue(group.readEntry("Hor.Oriented Custom Aspect Ratio Den",
                                  m_customRatioDInput->defaultValue()));
    m_ratioCB->setCurrentIndex(group.readEntry("Hor.Oriented Aspect Ratio",
                               m_ratioCB->defaultIndex()));

    if (m_originalIsLandscape)
    {
        m_orientCB->setCurrentIndex(group.readEntry("Hor.Oriented Aspect Ratio Orientation",
                                    (int)ImageSelectionWidget::Landscape));
        m_orientCB->setDefaultIndex(ImageSelectionWidget::Landscape);
    }
    else
    {
        m_orientCB->setCurrentIndex(group.readEntry("Ver.Oriented Aspect Ratio Orientation",
                                    (int)ImageSelectionWidget::Portrait));
        m_orientCB->setDefaultIndex(ImageSelectionWidget::Portrait);
    }
    applyRatioChanges(m_ratioCB->currentIndex());

    m_autoOrientation->setChecked(group.readEntry("Auto Orientation", false));
    slotAutoOrientChanged( m_autoOrientation->isChecked() );
}

void RatioCropTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("aspectratiocrop Tool");

    if (m_originalIsLandscape)
    {
        group.writeEntry("Hor.Oriented Aspect Ratio", m_ratioCB->currentIndex());
        group.writeEntry("Hor.Oriented Aspect Ratio Orientation", m_orientCB->currentIndex());
        group.writeEntry("Hor.Oriented Custom Aspect Ratio Num", m_customRatioNInput->value());
        group.writeEntry("Hor.Oriented Custom Aspect Ratio Den", m_customRatioDInput->value());

        group.writeEntry("Hor.Oriented Custom Aspect Ratio Xpos", m_xInput->value());
        group.writeEntry("Hor.Oriented Custom Aspect Ratio Ypos", m_yInput->value());
        group.writeEntry("Hor.Oriented Custom Aspect Ratio Width", m_widthInput->value());
        group.writeEntry("Hor.Oriented Custom Aspect Ratio Height", m_heightInput->value());
    }
    else
    {
        group.writeEntry("Ver.Oriented Aspect Ratio", m_ratioCB->currentIndex());
        group.writeEntry("Ver.Oriented Aspect Ratio Orientation", m_orientCB->currentIndex());
        group.writeEntry("Ver.Oriented Custom Aspect Ratio Num", m_customRatioNInput->value());
        group.writeEntry("Ver.Oriented Custom Aspect Ratio Den", m_customRatioDInput->value());

        group.writeEntry("Ver.Oriented Custom Aspect Ratio Xpos", m_xInput->value());
        group.writeEntry("Ver.Oriented Custom Aspect Ratio Ypos", m_yInput->value());
        group.writeEntry("Ver.Oriented Custom Aspect Ratio Width", m_widthInput->value());
        group.writeEntry("Ver.Oriented Custom Aspect Ratio Height", m_heightInput->value());
    }

    group.writeEntry("Precise Aspect Ratio Crop", m_preciseCrop->isChecked());
    group.writeEntry("Auto Orientation", m_autoOrientation->isChecked());
    group.writeEntry("Guide Lines Type", m_guideLinesCB->currentIndex());
    group.writeEntry("Golden Section", m_goldenSectionBox->isChecked());
    group.writeEntry("Golden Spiral Section", m_goldenSpiralSectionBox->isChecked());
    group.writeEntry("Golden Spiral", m_goldenSpiralBox->isChecked());
    group.writeEntry("Golden Triangle", m_goldenTriangleBox->isChecked());
    group.writeEntry("Golden Flip Horizontal", m_flipHorBox->isChecked());
    group.writeEntry("Golden Flip Vertical", m_flipVerBox->isChecked());
    group.writeEntry("Guide Color", m_guideColorBt->color());
    group.writeEntry("Guide Width", m_guideSize->value());
    group.sync();
}

void RatioCropTool::slotResetSettings()
{
    m_imageSelectionWidget->resetSelection();
}

void RatioCropTool::slotMaxAspectRatio()
{
    m_imageSelectionWidget->maxAspectSelection();
}

void RatioCropTool::slotCenterWidth()
{
    m_imageSelectionWidget->setCenterSelection(ImageSelectionWidget::CenterWidth);
}

void RatioCropTool::slotCenterHeight()
{
    m_imageSelectionWidget->setCenterSelection(ImageSelectionWidget::CenterHeight);
}

void RatioCropTool::slotSelectionChanged(QRect rect)
{
    m_xInput->blockSignals(true);
    m_yInput->blockSignals(true);
    m_widthInput->blockSignals(true);
    m_heightInput->blockSignals(true);

    m_xInput->setRange(0, m_imageSelectionWidget->getOriginalImageWidth() - rect.width(), 1);
    m_yInput->setRange(0, m_imageSelectionWidget->getOriginalImageHeight() - rect.height(), 1);
    m_widthInput->setRange(m_imageSelectionWidget->getMinWidthRange(),
                           m_imageSelectionWidget->getMaxWidthRange(),
                           m_imageSelectionWidget->getWidthStep());
    m_heightInput->setRange(m_imageSelectionWidget->getMinHeightRange(),
                            m_imageSelectionWidget->getMaxHeightRange(),
                            m_imageSelectionWidget->getHeightStep());

    m_xInput->setValue(rect.x());
    m_yInput->setValue(rect.y());
    m_widthInput->setValue(rect.width());
    m_heightInput->setValue(rect.height());

    m_gboxSettings->enableButton(EditorToolSettings::Ok,
                                 rect.isValid());

    m_preciseCrop->setEnabled(m_imageSelectionWidget->preciseCropAvailable());

    m_xInput->blockSignals(false);
    m_yInput->blockSignals(false);
    m_widthInput->blockSignals(false);
    m_heightInput->blockSignals(false);
}

void RatioCropTool::setRatioCBText(int orientation)
{
    int item = m_ratioCB->currentIndex();
    m_ratioCB->blockSignals(true);
    m_ratioCB->combo()->clear();
    m_ratioCB->addItem(i18nc("custom ratio crop settings", "Custom"));
    m_ratioCB->addItem("1:1");
    if (orientation == ImageSelectionWidget::Landscape)
    {
        m_ratioCB->addItem("3:2");
        m_ratioCB->addItem("4:3");
        m_ratioCB->addItem("5:4");
        m_ratioCB->addItem("7:5");
        m_ratioCB->addItem("10:7");
    }
    else
    {
        m_ratioCB->addItem("2:3");
        m_ratioCB->addItem("3:4");
        m_ratioCB->addItem("4:5");
        m_ratioCB->addItem("5:7");
        m_ratioCB->addItem("7:10");
    }
    m_ratioCB->addItem(i18n("Golden Ratio"));
    m_ratioCB->addItem(i18nc("no aspect ratio", "None"));
    m_ratioCB->setCurrentIndex(item);
    m_ratioCB->blockSignals(false);
}

void RatioCropTool::slotSelectionOrientationChanged(int newOrientation)
{
    // Change text for Aspect ratio ComboBox

    setRatioCBText(newOrientation);

    // Change Orientation ComboBox

    m_orientCB->setCurrentIndex(newOrientation);

    // Reverse custom values

    if ( ( m_customRatioNInput->value() < m_customRatioDInput->value() &&
           newOrientation == ImageSelectionWidget::Landscape ) ||
         ( m_customRatioNInput->value() > m_customRatioDInput->value() &&
           newOrientation == ImageSelectionWidget::Portrait ) )
    {
        m_customRatioNInput->blockSignals(true);
        m_customRatioDInput->blockSignals(true);

        int tmp = m_customRatioNInput->value();
        m_customRatioNInput->setValue(m_customRatioDInput->value());
        m_customRatioDInput->setValue(tmp);

        m_customRatioNInput->blockSignals(false);
        m_customRatioDInput->blockSignals(false);
    }
}

void RatioCropTool::slotXChanged(int x)
{
    m_imageSelectionWidget->setSelectionX(x);
}

void RatioCropTool::slotYChanged(int y)
{
    m_imageSelectionWidget->setSelectionY(y);
}

void RatioCropTool::slotWidthChanged(int w)
{
    m_imageSelectionWidget->setSelectionWidth(w);
}

void RatioCropTool::slotHeightChanged(int h)
{
    m_imageSelectionWidget->setSelectionHeight(h);
}

void RatioCropTool::slotPreciseCropChanged(bool a)
{
    m_imageSelectionWidget->setPreciseCrop(a);
}

void RatioCropTool::slotOrientChanged(int o)
{
    m_imageSelectionWidget->setSelectionOrientation(o);

    // Reset selection area.
    slotResetSettings();
}

void RatioCropTool::slotAutoOrientChanged(bool a)
{
    m_orientCB->setEnabled(!a /*|| m_ratioCB->currentIndex() == ImageSelectionWidget::RATIONONE*/);
    m_imageSelectionWidget->setAutoOrientation(a);
}

void RatioCropTool::slotRatioChanged(int a)
{
    applyRatioChanges(a);

    // Reset selection area.
    slotResetSettings();
}

void RatioCropTool::applyRatioChanges(int a)
{
    m_imageSelectionWidget->setSelectionAspectRatioType(a);

    if (a == ImageSelectionWidget::RATIOCUSTOM)
    {
        m_customLabel1->setEnabled(true);
        m_customLabel2->setEnabled(true);
        m_customRatioNInput->setEnabled(true);
        m_customRatioDInput->setEnabled(true);
        m_orientLabel->setEnabled(true);
        m_orientCB->setEnabled(!m_autoOrientation->isChecked());
        m_autoOrientation->setEnabled(true);
        slotCustomRatioChanged();
    }
    else if (a == ImageSelectionWidget::RATIONONE)
    {
        m_orientLabel->setEnabled(false);
        m_orientCB->setEnabled(false);
        m_autoOrientation->setEnabled(false);
        m_customLabel1->setEnabled(false);
        m_customLabel2->setEnabled(false);
        m_customRatioNInput->setEnabled(false);
        m_customRatioDInput->setEnabled(false);
    }
    else // Pre-config ratio selected.
    {
        m_orientLabel->setEnabled(true);
        m_orientCB->setEnabled(!m_autoOrientation->isChecked());
        m_autoOrientation->setEnabled(true);
        m_customLabel1->setEnabled(false);
        m_customLabel2->setEnabled(false);
        m_customRatioNInput->setEnabled(false);
        m_customRatioDInput->setEnabled(false);
    }
}

void RatioCropTool::slotGuideTypeChanged(int t)
{
    if (t == ImageSelectionWidget::GuideNone)
    {
        m_goldenSectionBox->setEnabled(false);
        m_goldenSpiralSectionBox->setEnabled(false);
        m_goldenSpiralBox->setEnabled(false);
        m_goldenTriangleBox->setEnabled(false);
        m_flipHorBox->setEnabled(false);
        m_flipVerBox->setEnabled(false);
        m_colorGuideLabel->setEnabled(false);
        m_guideColorBt->setEnabled(false);
        m_guideSize->setEnabled(false);
    }
    else if (t == ImageSelectionWidget::RulesOfThirds)
    {
        m_goldenSectionBox->setEnabled(false);
        m_goldenSpiralSectionBox->setEnabled(false);
        m_goldenSpiralBox->setEnabled(false);
        m_goldenTriangleBox->setEnabled(false);
        m_flipHorBox->setEnabled(false);
        m_flipVerBox->setEnabled(false);
        m_colorGuideLabel->setEnabled(true);
        m_guideColorBt->setEnabled(true);
        m_guideSize->setEnabled(true);
    }
    else if (t == ImageSelectionWidget::HarmoniousTriangles)
    {
        m_goldenSectionBox->setEnabled(false);
        m_goldenSpiralSectionBox->setEnabled(false);
        m_goldenSpiralBox->setEnabled(false);
        m_goldenTriangleBox->setEnabled(false);
        m_flipHorBox->setEnabled(true);
        m_flipVerBox->setEnabled(true);
        m_colorGuideLabel->setEnabled(true);
        m_guideColorBt->setEnabled(true);
        m_guideSize->setEnabled(true);
    }
    else
    {
        m_goldenSectionBox->setEnabled(true);
        m_goldenSpiralSectionBox->setEnabled(true);
        m_goldenSpiralBox->setEnabled(true);
        m_goldenTriangleBox->setEnabled(true);
        m_flipHorBox->setEnabled(true);
        m_flipVerBox->setEnabled(true);
        m_colorGuideLabel->setEnabled(true);
        m_guideColorBt->setEnabled(true);
        m_guideSize->setEnabled(true);
    }

    m_imageSelectionWidget->setGoldenGuideTypes(m_goldenSectionBox->isChecked(),
                                                m_goldenSpiralSectionBox->isChecked(),
                                                m_goldenSpiralBox->isChecked(),
                                                m_goldenTriangleBox->isChecked(),
                                                m_flipHorBox->isChecked(),
                                                m_flipVerBox->isChecked());
    m_imageSelectionWidget->slotGuideLines(t);
}

void RatioCropTool::slotGoldenGuideTypeChanged()
{
    slotGuideTypeChanged(m_guideLinesCB->currentIndex());
}

void RatioCropTool::slotCustomNRatioChanged(int a)
{
    if ( ! m_autoOrientation->isChecked() )
    {
        if ( ( m_orientCB->currentIndex() == ImageSelectionWidget::Portrait &&
               m_customRatioDInput->value() < a ) ||
             ( m_orientCB->currentIndex() == ImageSelectionWidget::Landscape &&
               m_customRatioDInput->value() > a ) )
        {
            m_customRatioDInput->blockSignals(true);
            m_customRatioDInput->setValue(a);
            m_customRatioDInput->blockSignals(false);
        }
    }

    slotCustomRatioChanged();
}

void RatioCropTool::slotCustomDRatioChanged(int a)
{
    if ( ! m_autoOrientation->isChecked() )
    {
        if ( ( m_orientCB->currentIndex() == ImageSelectionWidget::Landscape &&
               m_customRatioNInput->value() < a ) ||
             ( m_orientCB->currentIndex() == ImageSelectionWidget::Portrait &&
               m_customRatioNInput->value() > a ) )
        {
            m_customRatioNInput->blockSignals(true);
            m_customRatioNInput->setValue(a);
            m_customRatioNInput->blockSignals(false);
        }
    }

    slotCustomRatioChanged();
}

void RatioCropTool::slotCustomRatioChanged()
{
    m_imageSelectionWidget->setSelectionAspectRatioValue(
            m_customRatioNInput->value(), m_customRatioDInput->value() );

    // Reset selection area.
    slotResetSettings();
}

void RatioCropTool::finalRendering()
{
    kapp->setOverrideCursor( Qt::WaitCursor );

    QRect currentRegion        = m_imageSelectionWidget->getRegionSelection();
    ImageIface* iface = m_imageSelectionWidget->imageIface();
    uchar *data                = iface->getOriginalImage();
    int w                      = iface->originalWidth();
    int h                      = iface->originalHeight();
    bool a                     = iface->originalHasAlpha();
    bool sb                    = iface->originalSixteenBit();

    QRect normalizedRegion = currentRegion.normalized();
    if (normalizedRegion.right() > w) normalizedRegion.setRight(w);
    if (normalizedRegion.bottom() > h) normalizedRegion.setBottom(h);

    DImg imOrg(w, h, sb, a, data);
    delete [] data;
    imOrg.crop(normalizedRegion);

    iface->putOriginalImage(i18n("Aspect Ratio Crop"), imOrg.bits(), imOrg.width(), imOrg.height());

    kapp->restoreOverrideCursor();
    writeSettings();
}

}  // namespace DigikamImagesPluginCore
