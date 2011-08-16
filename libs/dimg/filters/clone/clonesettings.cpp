/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-07-08
 * Description : a digiKam image plugin to clone area .
 *
 * Copyright (C) 2011-07-08 by Zhang Jie <zhangjiehangyuan2005 dot at gmail dot com>
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

#include "clonesettings.moc"

// Qt includes

#include <QVariant>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QButtonGroup>
#include <QStringList>
#include <QDirIterator>
#include <QString>
#include <QPixmap>
#include <QMap>

// KDE includes

#include <kdebug.h>
#include <kurl.h>
#include <kdialog.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kseparator.h>
#include <kstandardguiitem.h>
#include <kpushbutton.h>

// LibKDcraw includes

#include <libkdcraw/rcombobox.h>
#include <libkdcraw/rnuminput.h>
#include <libkdcraw/rexpanderbox.h>

// Local includes

#include "refocusfilter.h"
#include "clonebrush.h"

using namespace KDcrawIface;

namespace Digikam
{

class CloneSettings::CloneSettingsPriv
{
public:

    CloneSettingsPriv()
      : diameterInput(0),
        opacityInput(0),
        brushID(1),
        selectMode(false),
        drawMode(false),
        drawEnable(false)
    {
    }

    static const QString  configBrushID;
    static const QString  configBrushShape;
    static const QString  configDiameter;
    static const QString  configMainDiameter;
    static const QString  configOpacity;
    static const QString  configSelectMode;
    static const QString  configDrawMode;

//  QButtonGroup*         brushGroup;
    RIntNumInput*         diameterInput;
    RIntNumInput*         opacityInput;

    QMap<int, CloneBrush> brushMap;
    int                   brushID;
    bool                  selectMode;
    bool                  drawMode;
    bool                  drawEnable;
};

const QString CloneSettings::CloneSettingsPriv::configBrushID("BrushID");
const QString CloneSettings::CloneSettingsPriv::configBrushShape("BrushShape");
const QString CloneSettings::CloneSettingsPriv::configDiameter("BrushDiameter");
const QString CloneSettings::CloneSettingsPriv::configMainDiameter("MainDiameter");
const QString CloneSettings::CloneSettingsPriv::configOpacity("BrushOpacity");
const QString CloneSettings::CloneSettingsPriv::configSelectMode("SelectMode");
const QString CloneSettings::CloneSettingsPriv::configDrawMode("DrawMode");

// --------------------------------------------------------

CloneSettings::CloneSettings(QWidget* parent)
    : QWidget(parent),
      d(new CloneSettingsPriv)
{
    QGridLayout* globalLayout = new QGridLayout(parent);

    QLabel* label1            = new QLabel(i18n("BrushShape:"));
    QLineEdit* BrushShapeEdit = new QLineEdit(parent);
    BrushShapeEdit->setGeometry(QRect(100, 50, 151, 20));

    QLabel* label2            = new QLabel();
    label2->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/clone_brushshape.png")));

    //----------------Brushshape selection------------------------------

    QScrollArea* scrollAreaBrushShape;
    scrollAreaBrushShape = new QScrollArea(parent);
    scrollAreaBrushShape->setMouseTracking(true);
    scrollAreaBrushShape->setFocusPolicy(Qt::ClickFocus);
    scrollAreaBrushShape->setStyleSheet(QString::fromUtf8("background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(150, 203, 255, 255), stop:1 rgba(255, 255, 255, 255));"));
    scrollAreaBrushShape->setFrameShape(QFrame::WinPanel);
    scrollAreaBrushShape->setLineWidth(3);
    scrollAreaBrushShape->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollAreaBrushShape->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollAreaBrushShape->setWidgetResizable(true);
    scrollAreaBrushShape->setFixedSize(135,130);
    scrollAreaBrushShape->setFixedHeight(130);

    QButtonGroup* brushGroup = new QButtonGroup(scrollAreaBrushShape);

    QGridLayout *   gridLayout = new QGridLayout(scrollAreaBrushShape);
    gridLayout->setSpacing(0);
    gridLayout->setContentsMargins(11, 11, 11, 11);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    gridLayout->setSizeConstraint(QLayout::SetMinimumSize);
    gridLayout->setContentsMargins(0, 0, 0, 0);

    /////////////////////////read all png files in the img folder///////////////////////

    QStringList nameFilters;
    nameFilters << "*.png" ;
    QString path;
    path.append("../img");//need to be fixed, the path may cannot be found!!!!
    QDirIterator dirIterator(path, nameFilters, QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while(dirIterator.hasNext())
    {
        nameFilters << dirIterator.fileName();
        dirIterator.next();
    }

    KPushButton* buttons[4];
    if(nameFilters.size()>0)
    {
        for(int c = 0; c<4 ; c++)
        {
            buttons[c] = new KPushButton[(nameFilters.size()+3)/4];
        }
    }

    for (int i = 0; i < nameFilters.size(); ++i)
    {
        QString filename = nameFilters.at(i).toLocal8Bit().constData();
        QPixmap iconMap;
        iconMap.load(filename);
        CloneBrush brush;

        brush.setPixmap(iconMap);
        brush.setDia(iconMap.size().width());

        d->brushMap.insert(i+1,brush);
        buttons[i%4][i/4].setParent(parent);
        buttons[i%4][i/4].setFixedSize(30,23);
        if(!iconMap.isNull())
        {
            buttons[i%4][i/4].setIcon(QIcon(iconMap));
        }

        brushGroup->addButton(&buttons[i%4][i/4],i+1);
        gridLayout->addWidget(&buttons[i%4][i/4],i/4,i%4,1,1);
    }

    //=============================================================================

    QLabel* label3 = new QLabel(i18n("Select source/Draw stroke:"));

    // KHBox*  modeBox = new KHBox(parent);

    KPushButton* pushButton1 = new KPushButton(parent);
    pushButton1->setIcon(QIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/clone_selectButton.png"))));
    pushButton1->setToolTip(i18n("Select a source point of the source area."));
    pushButton1->setVisible(true);
    pushButton1->setIconSize(QSize(46, 48));
    pushButton1->setFixedSize(31, 31);

    KPushButton* pushButton2 = new KPushButton(parent);
    pushButton2->setIcon(QIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/clone_drawButton.png"))));
    pushButton2->setToolTip(i18n("Start to draw a stroke. To use this you should first click the left button to select a point"));
    pushButton2->setVisible(true);
    pushButton2->setIconSize(QSize(46, 48));
    pushButton2->setFixedSize(31, 31);

    QLabel* label4   = new QLabel(i18n("Main diameter(px):"));

    d->diameterInput = new RIntNumInput(parent);
    d->diameterInput->setRange(1, 200, 1);
    d->diameterInput->setSliderEnabled(true);
    d->diameterInput->setDefaultValue(1);
    d->diameterInput->setWhatsThis(i18n("Set the main diameter of the brush"));

    QLabel* label5   = new QLabel(i18n("Opacity(%):"));
    d->opacityInput  = new RIntNumInput(parent);
    d->opacityInput->setRange(1, 100, 1);
    d->opacityInput->setSliderEnabled(true);
    d->opacityInput->setDefaultValue(100);
    d->opacityInput->setWhatsThis(i18n("Set the opacity of the brush"));

    globalLayout-> addWidget(label1,              0,  0, 1, 2);
    globalLayout->addWidget(BrushShapeEdit,       0,  2, 1, 4);
    globalLayout->addWidget(label2,               1,  0, 2, 2);
    globalLayout->addWidget(scrollAreaBrushShape, 1,  2, 4, 4);
    globalLayout->addWidget(label3,               5,  0, 1, 5);
    globalLayout->addWidget(pushButton1,          6,  2, 1, 1);
    globalLayout->addWidget(pushButton2,          6,  4, 1, 1);
    globalLayout->addWidget(label4,               7,  0, 1, 4);
    globalLayout->addWidget(d->diameterInput,     8,  2, 1, 5);
    globalLayout->addWidget(label5,               9,  0, 1, 2);
    globalLayout->addWidget(d->opacityInput,      10, 2, 1, 5);

/*
    connect(d->horizontalSlider_MainDiameter, SIGNAL(valueChanged(int)),
            d->spinBox_MainDiameter, SLOT(setValue(int)));
*/
    connect(pushButton1, SIGNAL(pressed()),
            this,SLOT(slotSelectModeChanged()));

    connect(pushButton2, SIGNAL(pressed()),
            this,SLOT(slotDrawModeChanged()));

//for (int j = 0; j < nameFilters.size(); ++j)
    connect(brushGroup, SIGNAL(buttonClicked (int)),
            this, SLOT(slotBrushIdChanged(int)));


}

CloneSettings::~CloneSettings()
{
    delete d;
}

void CloneSettings::setSettings(const CloneContainer& settings)
{
    blockSignals(true);

    d->diameterInput->setValue(settings.mainDia);
    d->opacityInput->setValue(settings.opacity);
    
    blockSignals(false);
}

void CloneSettings::resetToDefault()
{
    setSettings(defaultSettings());
}

CloneContainer CloneSettings::defaultSettings() const
{
   CloneContainer prm;

   prm.brushID    = d->brushID;
//FIXME   prm.brush      = d->brushMap.find(prm.brushID);
   prm.brushDia   = prm.brush.getDia();
   prm.mainDia    = prm.brush.getDia();
   prm.selectMode = d->selectMode;
   prm.drawMode   = d->drawMode;

   return prm;
}

void CloneSettings::readSettings(KConfigGroup& group)
{
    CloneContainer prm;
    CloneContainer defaultPrm = defaultSettings();

    blockWidgetSignals(true);

    prm.brushID    = group.readEntry(d->configBrushID,      defaultPrm.brushID);
//FIXME    prm.brush      = group.readEntry(d->configBrushShape,   defaultPrm.brush);
    prm.brushDia   = group.readEntry(d->configDiameter,     defaultPrm.brushDia);
    prm.mainDia    = group.readEntry(d->configMainDiameter, defaultPrm.mainDia);
    prm.opacity    = group.readEntry(d->configOpacity,      defaultPrm.opacity);
    prm.selectMode = group.readEntry(d->configSelectMode,   defaultPrm.selectMode);
    prm.drawMode   = group.readEntry(d->configDrawMode,     defaultPrm.drawMode);

    blockWidgetSignals(false);

    setSettings(prm);
}

void CloneSettings::writeSettings(KConfigGroup& group)
{
   CloneContainer prm = settings();

   group.writeEntry(d->configBrushID,       prm.brushID);
//FIXME   group.writeEntry(d->configBrushShape,    prm.brush);
   group.writeEntry(d->configDiameter,      prm.brushDia);
   group.writeEntry(d->configMainDiameter,  prm.mainDia);
   group.writeEntry(d->configOpacity,       prm.opacity);
   group.writeEntry(d->configSelectMode,    prm.selectMode);
   group.writeEntry(d->configDrawMode,      prm.drawMode);
}

CloneContainer CloneSettings::settings()const
{
    CloneContainer prm;

    prm.brushID    = d->brushID;
//FIXME    prm.brush      = d->brushMap.find(prm.brushID);
    prm.brushDia   = prm.brush.getDia();
    prm.mainDia    = d->diameterInput->value();
    prm.opacity    = d->opacityInput->value();
    prm.selectMode = d->selectMode;
    prm.drawMode   = d->drawMode;

    return prm;
}

void CloneSettings::slotBrushIdChanged(int id)
{
    if(id >= 0 )
        d->brushID = id;
}

void CloneSettings::slotSelectModeChanged()
{
      d->selectMode = true;
      d->drawMode   = false;
      d->drawEnable = true;
}

void CloneSettings::slotDrawModeChanged()
{
    if(d->drawEnable)
    {
          d->drawMode   = true;
          d->selectMode = false;
    }
}

void CloneSettings::blockWidgetSignals(bool b)
{
    d->diameterInput->blockSignals(b);
    d->opacityInput->blockSignals(b);
}

} // namespace Digikam
