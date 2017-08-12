/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-09-01
 * Description : a plugin to create photo layouts by fusion of several images.
 *
 * Copyright (C) 2011      by Lukasz Spas <lukasz dot spas at gmail dot com>
 * Copyright (C) 2009-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

/**
  *
  * In next version:
  *
  *     TODO : Gradient fill background:
  *               - create gradient editor widget
  *               - create MouseListener button
  *
  */

#include "CanvasEditTool.h"

#include <QApplication>
#include <QFormLayout>
#include <QGroupBox>
#include <QStackedLayout>
#include <QSpinBox>
#include <QImage>
#include <QImageReader>
#include <QCheckBox>
#include <QDebug>
#include <QLabel>
#include <QMessageBox>
#include <QStandardPaths>

#include <klocalizedstring.h>

#include "digikam_debug.h"
#include "dcolorselector.h"
#include "imagedialog.h"
#include "SceneBackground.h"
#include "MousePressListener.h"
#include "PatternsComboBox.h"
#include "SceneBorder.h"

using namespace Digikam;

namespace PhotoLayoutsEditor
{

class CanvasEditToolPrivate
{
    enum BackgroundType
    {
        ColorFill,
        PatternFill,
        GradientFill,
        ImageFill
    };

    enum ScallingType
    {
        Expanded = 1,
        Scaled = 2,
        Manual = 4
    };

    CanvasEditToolPrivate(CanvasEditTool * /*parent*/) :
//        m_parent(parent),
//        background_gradient_widget(0),
        background_type_widget(0),
        background_widgets(0),
        background_color_widget(0),
        background_color(0),
        background_image_widget(0),
        backgroundImageFormLayout(0),
//        background_image_file(0),
        background_image_label(0),
        background_image_scalling(0),
        background_image_tiled(0),
        background_alignBox(0),
        background_image_HAlign(0),
        background_image_VAlign(0),
        background_sizeBox(0),
        background_image_width(0),
        background_image_height(0),
        background_image_color(0),
        border_image_label(0),
        background_pattern_widget(0),
        background_pattern_color1(0),
        background_pattern_color2(0),
        background_pattern_type(0)
    {
        background_types.insert(i18n("Color"),    ColorFill);
        background_types.insert(i18n("Image"),    ImageFill);
        background_types.insert(i18n("Pattern"),  PatternFill);
//        background_types.insert(i18n("Gradient"), GradientFill);

        background_image_scalling_map.insert(Expanded, i18n("Expanded"));
        background_image_scalling_map.insert(Scaled, i18n("Scaled"));
        background_image_scalling_map.insert(Manual, i18n("Fixed size"));

        background_image_Halignment_map.insert(Qt::AlignHCenter, i18n("Center"));
        background_image_Halignment_map.insert(Qt::AlignLeft, i18n("Left"));
        background_image_Halignment_map.insert(Qt::AlignRight, i18n("Right"));

        background_image_Valignment_map.insert(Qt::AlignVCenter, i18n("Center"));
        background_image_Valignment_map.insert(Qt::AlignTop, i18n("Top"));
        background_image_Valignment_map.insert(Qt::AlignBottom, i18n("Bottom"));

        background_image_empty_pixmap = QPixmap(150, 100);
        background_image_empty_pixmap.fill(Qt::transparent);
        QPainter p(&background_image_empty_pixmap);
        p.drawText(background_image_empty_pixmap.rect(), Qt::AlignCenter, i18n("Click here to set an image"));

        border_image_empty_pixmap = QPixmap(150, 100);
        border_image_empty_pixmap.fill(Qt::transparent);
        QPainter p2(&border_image_empty_pixmap);
        p2.drawText(border_image_empty_pixmap.rect(), Qt::AlignCenter, i18n("Click here to set an image"));
    }

    void setImageWidgetsEnabled(bool enabled)
    {
        background_image_scalling->setEnabled(enabled);
        background_image_tiled->setEnabled(enabled);
        background_image_HAlign->setEnabled(enabled);
        background_image_VAlign->setEnabled(enabled);
        background_image_width->setEnabled(enabled);
        background_image_height->setEnabled(enabled);
    }

//    CanvasEditTool * m_parent;

    QMap<QString, BackgroundType> background_types;
    QComboBox * background_type_widget;
    QStackedLayout * background_widgets;

    QWidget * background_color_widget;
    DColorSelector * background_color;

//    QWidget * background_gradient_widget;

    QWidget * background_image_widget;
    QFormLayout * backgroundImageFormLayout;
//    QUrlRequester * background_image_file;
    QPixmap background_image_empty_pixmap;
    QPushButton * background_image_label;
    QComboBox * background_image_scalling;
    QMap<ScallingType, QString> background_image_scalling_map;
    QCheckBox * background_image_tiled;
    QGroupBox * background_alignBox;
    QComboBox * background_image_HAlign;
    QMap<Qt::Alignment, QString> background_image_Halignment_map;
    QComboBox * background_image_VAlign;
    QMap<Qt::Alignment, QString> background_image_Valignment_map;
    QGroupBox * background_sizeBox;
    QSpinBox * background_image_width;
    QSpinBox * background_image_height;
    DColorSelector * background_image_color;
    QImage m_image;

    QPixmap border_image_empty_pixmap;
    QPushButton * border_image_label;
    QImage m_border_image;

    QWidget * background_pattern_widget;
    DColorSelector * background_pattern_color1;
    DColorSelector * background_pattern_color2;
    PatternsComboBox * background_pattern_type;

    MousePressListener mouse_listener;

    friend class CanvasEditTool;
};

CanvasEditTool::CanvasEditTool(Scene * scene, QWidget * parent)
    : AbstractTool(scene, Canvas::Viewing, parent),
      d(new CanvasEditToolPrivate(this)),
      hold_update(false)
{
    setupGUI();
}

CanvasEditTool::~CanvasEditTool()
{
    delete d;
}

void CanvasEditTool::backgroundTypeChanged(const QString & typeName)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << typeName;
    CanvasEditToolPrivate::BackgroundType bt = d->background_types.value(typeName);
    switch (bt)
    {
        case CanvasEditToolPrivate::ColorFill:
            colorBackgroundSelected();
            break;
        case CanvasEditToolPrivate::GradientFill:
            gradientBackgroundSelected();
            break;
        case CanvasEditToolPrivate::ImageFill:
            imageBackgroundSelected();
            break;
        case CanvasEditToolPrivate::PatternFill:
            patternBackgroundSelected();
            break;
    }
}

void CanvasEditTool::sceneChange()
{
    Scene * scene = this->scene();
    if (!scene)
        return;
}

void CanvasEditTool::sceneChanged()
{
    Scene * scene = this->scene();
    if (!scene)
        return;

    SceneBackground * background = scene->background();
    this->connect(background, SIGNAL(changed()), this, SLOT(updateWidgets()));

    this->updateWidgets();
}

void CanvasEditTool::colorBackgroundSelected()
{
    d->background_widgets->setCurrentWidget(d->background_color_widget);

    if (this->hold_update)
        return;

    SceneBackground * background = scene()->background();
    background->setSolidColor(d->background_color->color());
}

void CanvasEditTool::gradientBackgroundSelected()
{}

void CanvasEditTool::imageBackgroundSelected()
{
    d->background_widgets->setCurrentWidget(d->background_image_widget);
    if (d->m_image.isNull())
        return;
    Scene * scene = this->scene();
    if (!scene)
        return;

    if (this->hold_update)
        return;

    this->setImageBackground();
}

void CanvasEditTool::patternBackgroundSelected()
{
    d->background_widgets->setCurrentWidget(d->background_pattern_widget);

    if (this->hold_update)
        return;

    this->setPatternBackground();
}

void CanvasEditTool::solidColorChanged(const QColor & color)
{
    Scene * scene = this->scene();
    if (!scene)
        return;
    scene->background()->setSolidColor(color);
}

void CanvasEditTool::imageBackgroundColorChanged(const QColor & color)
{
    if (this->hold_update)
        return;
    Scene * scene = this->scene();
    if (!scene)
        return;
    scene->background()->setSecondColor(color);
}

void CanvasEditTool::patternFirstColorChanged(const QColor & /*color*/)
{
    if (this->hold_update)
        return;
    Scene * scene = this->scene();
    if (!scene)
        return;
    this->setPatternBackground();
}

void CanvasEditTool::patternSecondColorChanged(const QColor & /*color*/)
{
    if (this->hold_update)
        return;
    Scene * scene = this->scene();
    if (!scene)
        return;
    this->setPatternBackground();
}

void CanvasEditTool::patternStyleChanged(Qt::BrushStyle /*patternStyle*/)
{
    if (this->hold_update)
        return;
    Scene * scene = this->scene();
    if (!scene)
        return;
    this->setPatternBackground();
}

void CanvasEditTool::imageUrlRequest()
{
    static QString startUrl(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
    QUrl url = ImageDialog::getImageURL(this, QUrl::fromLocalFile(startUrl), QString());
    if (url.isEmpty())
        return;

    bool valid = false;
    QImageReader ir(url.toLocalFile());

    if (ir.canRead())
    {
        if (ir.read(&d->m_image))
        {
            QPixmap tempPX = QPixmap::fromImage(d->m_image.scaled(QSize(150,150), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            d->background_image_label->setIcon(QIcon(tempPX));
            d->background_image_label->setIconSize(tempPX.size());

            this->hold_update = true;
            QSizeF sceneSize = scene()->sceneRect().size();
            QSize  imageSize = d->m_image.size();
            bool widthSmaller = sceneSize.width() > imageSize.width();
            bool heightSmaller = sceneSize.height()> imageSize.height();
            d->background_image_width->setValue(imageSize.width());
            d->background_image_height->setValue(imageSize.height());
            if (widthSmaller && heightSmaller)
                d->background_image_scalling->setCurrentText( d->background_image_scalling_map.value(CanvasEditToolPrivate::Manual) );
            else if (widthSmaller || heightSmaller)
                d->background_image_scalling->setCurrentText( d->background_image_scalling_map.value(CanvasEditToolPrivate::Expanded) );
            else
                d->background_image_scalling->setCurrentText( d->background_image_scalling_map.value(CanvasEditToolPrivate::Scaled) );

            this->hold_update = false;
            this->setImageBackground();

            valid = true;
        }
    }

    if (!valid)
    {
        QMessageBox::critical(qApp->activeWindow(), i18n("Error"), i18n("Invalid or unsupported image file."));
        d->background_image_label->setIcon(QIcon(d->background_image_empty_pixmap));
        d->background_image_label->setIconSize(d->background_image_empty_pixmap.size());
    }
    d->setImageWidgetsEnabled(valid);

    startUrl = QFileInfo(url.toLocalFile()).path();
}

void CanvasEditTool::borderImageUrlRequest()
{
    static QString startUrl(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
    QUrl url = ImageDialog::getImageURL(this, QUrl::fromLocalFile(startUrl), QString());
    if (url.isEmpty())
        return;

    bool valid = false;
    QImageReader ir(url.toLocalFile());

    if (ir.canRead())
    {
        if (ir.read(&d->m_border_image))
        {
            QPixmap tempPX = QPixmap::fromImage(d->m_border_image.scaled(QSize(150,150), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            d->border_image_label->setIcon(QIcon(tempPX));
            d->border_image_label->setIconSize(tempPX.size());

            this->setImageBorder();
            valid = true;
        }
    }

    if (!valid)
    {
        QMessageBox::critical(qApp->activeWindow(), i18n("Error"), i18n("Invalid or unsupported image file."));
        d->border_image_label->setIcon(QIcon(d->border_image_empty_pixmap));
        d->border_image_label->setIconSize(d->border_image_empty_pixmap.size());
    }

    startUrl = QFileInfo(url.toLocalFile()).path();
}

void CanvasEditTool::imageScallingChanged(const QString & scallingName)
{
    CanvasEditToolPrivate::ScallingType st = d->background_image_scalling_map.key(scallingName);
    d->background_sizeBox->setVisible(st == CanvasEditToolPrivate::Manual);
    this->setImageBackground();
}

void CanvasEditTool::imageTiledChanged(int /*state*/)
{
    this->setImageBackground();
}

void CanvasEditTool::imageHorizontalAlignmentChanged(int /*index*/)
{
    this->setImageBackground();
}

void CanvasEditTool::imageVerticalAlignmentChanged(int /*index*/)
{
    this->setImageBackground();
}

void CanvasEditTool::imageWidthChanged()
{
    static int width = -1;
    if (width != d->background_image_width->value())
        this->setImageBackground();
    width = d->background_image_width->value();
}

void CanvasEditTool::imageHeightChanged()
{
    static int height = -1;
    if (height != d->background_image_height->value())
        this->setImageBackground();
    height = d->background_image_height->value();
}

void CanvasEditTool::setImageBackground()
{
    if (d->m_image.isNull() || this->hold_update)
        return;

    SceneBackground * background = scene()->background();
    bool tiled = d->background_image_tiled->isChecked();
    Qt::Alignment alignment = d->background_image_Halignment_map.key( d->background_image_HAlign->currentText() ) |
                              d->background_image_Valignment_map.key( d->background_image_VAlign->currentText() );
    CanvasEditToolPrivate::ScallingType scalingMode = d->background_image_scalling_map.key(d->background_image_scalling->currentText());
    if (scalingMode == CanvasEditToolPrivate::Manual)
    {
        QSize size(d->background_image_width->value(),
                   d->background_image_height->value());
        background->setImage(d->m_image, d->background_image_color->color(), alignment, size, tiled);
    }
    else
    {
        Qt::AspectRatioMode aspectRatio;
        switch (scalingMode)
        {
            case CanvasEditToolPrivate::Expanded:
                aspectRatio = Qt::KeepAspectRatioByExpanding;
                break;
            default:
                aspectRatio = Qt::KeepAspectRatio;
        }
        background->setImage(d->m_image, d->background_image_color->color(), alignment, aspectRatio, tiled);
    }
}

void CanvasEditTool::setPatternBackground()
{
    if (this->hold_update)
        return;

    Scene * scene = this->scene();
    scene->background()->setPattern(d->background_pattern_color1->color(),
                                    d->background_pattern_color2->color(),
                                    d->background_pattern_type->pattern());
}

void CanvasEditTool::setImageBorder()
{
    if (d->m_border_image.isNull() || this->hold_update)
        return;

    SceneBorder * border = scene()->border();
    if (border)
        border->setImage(d->m_border_image);
}

void CanvasEditTool::setupGUI()
{
    QVBoxLayout * layout = new QVBoxLayout();
    this->setLayout(layout);

    // Canvas border group
    QGroupBox * borderGroup = new QGroupBox(i18n("Border"), this);
    layout->addWidget(borderGroup);
    QFormLayout * borderLayout = new QFormLayout();
    borderGroup->setLayout(borderLayout);
    {
        d->border_image_label = new QPushButton(borderGroup);
        borderLayout->addRow(d->border_image_label);
        d->border_image_label->setFlat(true);
        d->border_image_label->setFocusPolicy(Qt::NoFocus);
        d->border_image_label->setIcon(QIcon(d->border_image_empty_pixmap));
        d->border_image_label->setIconSize(d->border_image_empty_pixmap.size());
    }

    // Canvas background group
    QGroupBox * backgroundGroup = new QGroupBox(i18n("Background"), this);
    layout->addWidget(backgroundGroup);
    QFormLayout * backgroundLayout = new QFormLayout();
    backgroundGroup->setLayout(backgroundLayout);
    {
        // Background type widget
        d->background_type_widget = new QComboBox(backgroundGroup);
        d->background_type_widget->addItems( d->background_types.keys() );
        backgroundLayout->addRow(i18n("Type"), d->background_type_widget);

        d->background_widgets = new QStackedLayout();
        backgroundLayout->addRow(d->background_widgets);

        // Color type widget
        d->background_color_widget = new QWidget(backgroundGroup);
        QFormLayout * colorFormLayout = new QFormLayout();
        d->background_color_widget->setLayout(colorFormLayout);
        d->background_widgets->addWidget(d->background_color_widget);
        d->background_color = new DColorSelector(d->background_color_widget);
        //d->background_color->setAlphaChannelEnabled(true);
        d->background_color->setColor(Qt::transparent);
        colorFormLayout->addRow(i18n("Color"), d->background_color);

        // Image type widget
        d->background_image_widget = new QWidget(backgroundGroup);
        d->backgroundImageFormLayout = new QFormLayout();
        d->background_image_widget->setLayout(d->backgroundImageFormLayout);
        d->background_widgets->addWidget(d->background_image_widget);

        d->background_image_label = new QPushButton(d->background_image_widget);
        d->backgroundImageFormLayout->addRow(d->background_image_label);
        d->background_image_label->setFlat(true);
        d->background_image_label->setFocusPolicy(Qt::NoFocus);
        d->background_image_label->setIcon(QIcon(d->background_image_empty_pixmap));
        d->background_image_label->setIconSize(d->background_image_empty_pixmap.size());

        d->background_image_scalling = new QComboBox(d->background_image_widget);
        d->background_image_scalling->addItems(d->background_image_scalling_map.values());
        d->background_image_scalling->setEnabled(false);
        d->backgroundImageFormLayout->addRow(i18n("Scaling"), d->background_image_scalling);
        d->background_image_scalling->setCurrentIndex(-1);

        d->background_image_tiled = new QCheckBox(d->background_image_widget);
        d->background_image_tiled->setEnabled(false);
        d->backgroundImageFormLayout->addRow(i18n("Tiled"), d->background_image_tiled);

        d->background_alignBox = new QGroupBox(i18n("Alignment"));
        QFormLayout * alignForm = new QFormLayout();
        d->background_alignBox->setLayout(alignForm);
            d->background_image_HAlign = new QComboBox(d->background_image_widget);
            d->background_image_HAlign->addItems(d->background_image_Halignment_map.values());
            d->background_image_HAlign->setEnabled(false);
            alignForm->addRow(i18n("Horizontal"), d->background_image_HAlign);
            d->background_image_VAlign = new QComboBox(d->background_image_widget);
            d->background_image_VAlign->addItems(d->background_image_Valignment_map.values());
            d->background_image_VAlign->setEnabled(false);
            alignForm->addRow(i18n("Vertical"), d->background_image_VAlign);
        d->backgroundImageFormLayout->addRow(d->background_alignBox);

        d->background_sizeBox = new QGroupBox(i18n("Size"));
        QFormLayout * sizeForm = new QFormLayout();
        d->background_sizeBox->setLayout(sizeForm);
            d->background_image_width = new QSpinBox(d->background_image_widget);
            d->background_image_width->setEnabled(false);
            d->background_image_width->setMinimum(1);
            d->background_image_width->setMaximum(99999);
            sizeForm->addRow(i18n("Width"), d->background_image_width);
            d->background_image_height = new QSpinBox(d->background_image_widget);
            d->background_image_height->setEnabled(false);
            d->background_image_height->setMinimum(1);
            d->background_image_height->setMaximum(99999);
            sizeForm->addRow(i18n("Height"), d->background_image_height);
        d->backgroundImageFormLayout->addRow(d->background_sizeBox);

        d->background_image_color = new DColorSelector(d->background_image_widget);
        d->background_image_color->setColor(Qt::transparent);
        //d->background_image_color->setAlphaChannelEnabled(true);
        d->backgroundImageFormLayout->addRow(i18n("Color"), d->background_image_color);

        // Pattern type widget
        d->background_pattern_widget = new QWidget(backgroundGroup);
        QFormLayout * patternFormLayout = new QFormLayout();
        d->background_pattern_widget->setLayout(patternFormLayout);
        d->background_widgets->addWidget(d->background_pattern_widget);
        d->background_pattern_color1 = new DColorSelector(d->background_pattern_widget);
        d->background_pattern_color1->setColor(Qt::transparent);
        //d->background_pattern_color1->setAlphaChannelEnabled(true);
        patternFormLayout->addRow(i18n("Color 1"), d->background_pattern_color1);
        d->background_pattern_color2 = new DColorSelector(d->background_pattern_widget);
        d->background_pattern_color2->setColor(Qt::transparent);
        //d->background_pattern_color2->setAlphaChannelEnabled(true);
        patternFormLayout->addRow(i18n("Color 2"), d->background_pattern_color2);
        d->background_pattern_type = new PatternsComboBox(d->background_pattern_widget);
        patternFormLayout->addRow(i18n("Pattern"), d->background_pattern_type);
    }

    connect(d->background_type_widget, SIGNAL(currentIndexChanged(QString)), this, SLOT(backgroundTypeChanged(QString)));
    connect(d->background_color, SIGNAL(signalColorSelected(QColor)), this, SLOT(solidColorChanged(QColor)));
    connect(d->background_image_label, SIGNAL(clicked()), this, SLOT(imageUrlRequest()));
    connect(d->background_image_scalling, SIGNAL(currentIndexChanged(QString)), this, SLOT(imageScallingChanged(QString)));
    connect(d->background_image_tiled, SIGNAL(stateChanged(int)), this, SLOT(imageTiledChanged(int)));
    connect(d->background_image_HAlign, SIGNAL(currentIndexChanged(int)), this, SLOT(imageHorizontalAlignmentChanged(int)));
    connect(d->background_image_VAlign, SIGNAL(currentIndexChanged(int)), this, SLOT(imageVerticalAlignmentChanged(int)));
    connect(d->background_image_width, SIGNAL(editingFinished()), this, SLOT(imageWidthChanged()));
    connect(d->background_image_height, SIGNAL(editingFinished()), this, SLOT(imageHeightChanged()));
    connect(d->background_image_color, SIGNAL(signalColorSelected(QColor)), this, SLOT(imageBackgroundColorChanged(QColor)));
    connect(d->background_pattern_color1, SIGNAL(signalColorSelected(QColor)), this, SLOT(patternFirstColorChanged(QColor)));
    connect(d->background_pattern_color2, SIGNAL(signalColorSelected(QColor)), this, SLOT(patternSecondColorChanged(QColor)));
    connect(d->background_pattern_type, SIGNAL(currentPatternChanged(Qt::BrushStyle)), this, SLOT(patternStyleChanged(Qt::BrushStyle)));
    connect(&(d->mouse_listener), SIGNAL(mousePressed(QPointF)), this, SLOT(readMousePosition(QPointF)));
    connect(d->border_image_label, SIGNAL(clicked()), this, SLOT(borderImageUrlRequest()));

    if (scene())
    {
        Scene * scene = this->scene();
        SceneBackground * background = scene->background();
        if (background->isColor())
            this->colorBackgroundSelected();
        else if (background->isGradient())
            this->gradientBackgroundSelected();
        else if (background->isImage())
            this->imageBackgroundSelected();
        else if (background->isPattern())
            this->patternBackgroundSelected();

        SceneBorder * border = scene->border();
        d->m_border_image = border->image();
        if (!d->m_border_image.isNull())
        {
            QPixmap tempPX = QPixmap::fromImage(d->m_border_image.scaled(QSize(150,150), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            d->border_image_label->setIcon(QIcon(tempPX));
            d->border_image_label->setIconSize(tempPX.size());
        }
    }
}

void CanvasEditTool::readMousePosition(const QPointF & scenePos)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << scenePos;
}

void CanvasEditTool::updateWidgets()
{
    Scene * scene = this->scene();
    if (!scene)
        return;

    SceneBackground * background = scene->background();
    if (!background)
        return;

    SceneBorder * border = scene->border();
    if (!border)
        return;

    d->m_border_image = border->image();
    if (!d->m_border_image.isNull())
    {
        QPixmap tempPX = QPixmap::fromImage(d->m_border_image.scaled(QSize(150,150), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        d->border_image_label->setIcon(QIcon(tempPX));
        d->border_image_label->setIconSize(tempPX.size());
    }

    this->hold_update = true;

    if (background->isPattern())
    {
        d->background_widgets->setCurrentWidget(d->background_pattern_widget);
        d->background_pattern_type->setPattern(background->pattern());
        d->background_pattern_color1->setColor(background->firstColor());
        d->background_pattern_color2->setColor(background->secondColor());
        d->background_type_widget->setCurrentText(d->background_types.key(CanvasEditToolPrivate::PatternFill));
    }
    else if (background->isImage())
    {
        d->background_widgets->setCurrentWidget(d->background_image_widget);
        d->m_image = background->image();
        QPixmap tempPX = QPixmap::fromImage(d->m_image.scaled(QSize(150,150), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        d->background_image_label->setIcon(QIcon(tempPX));
        d->background_image_label->setIconSize(tempPX.size());
        d->background_image_HAlign->setCurrentText( d->background_image_Halignment_map.value(background->imageAlignment() & (Qt::AlignHorizontal_Mask)) );
        d->background_image_VAlign->setCurrentText( d->background_image_Valignment_map.value(background->imageAlignment() & (Qt::AlignVertical_Mask)) );
        d->background_image_width->setValue( background->imageSize().width() );
        d->background_image_height->setValue( background->imageSize().height() );
        CanvasEditToolPrivate::ScallingType scallingType;
        switch (background->imageAspectRatio())
        {
            case Qt::KeepAspectRatioByExpanding:
                scallingType = CanvasEditToolPrivate::Expanded;
                break;
            case Qt::KeepAspectRatio:
                scallingType = CanvasEditToolPrivate::Scaled;
                break;
            default:
                scallingType = CanvasEditToolPrivate::Manual;
        }
        d->background_image_scalling->setCurrentText( d->background_image_scalling_map.value(scallingType) );
        d->background_image_tiled->setChecked( background->imageRepeated() );
        d->background_image_color->setColor( background->secondColor() );
        d->background_type_widget->setCurrentText(d->background_types.key(CanvasEditToolPrivate::ImageFill));
        d->setImageWidgetsEnabled(!d->m_image.isNull());
    }
    else
    {
        d->background_widgets->setCurrentWidget(d->background_color_widget);
        d->background_color->setColor(background->firstColor());
        d->background_type_widget->setCurrentText(d->background_types.key(CanvasEditToolPrivate::ColorFill));
    }
    this->hold_update = false;
}

} // namespace PhotoLayoutsEditor
