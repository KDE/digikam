/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-11-30
 * Description : Save space slider widget
 *
 * Copyright (C) 2014-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C)      2010 by Justin Noel <justin at ics dot com>
 * Copyright (C)      2010 by Cyrille Berger <cberger at cberger dot net>
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

#include "dsliderspinbox.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QStyle>
#include <QPainter>
#include <QLineEdit>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QIntValidator>
#include <QDoubleSpinBox>

namespace Digikam
{

class DAbstractSliderSpinBoxPrivate
{
public:

    DAbstractSliderSpinBoxPrivate()
    {
        edit                    = 0;
        validator               = 0;
        dummySpinBox            = 0;
        upButtonDown            = false;
        downButtonDown          = false;
        factor                  = 1.0;
        fastSliderStep          = 5;
        slowFactor              = 0.1;
        shiftPercent            = 0.0;
        shiftMode               = false;
        exponentRatio           = 0.0;
        value                   = 0;
        maximum                 = 100;
        minimum                 = 0;
        singleStep              = 1;
        style                   = STYLE_NOQUIRK;
        blockUpdateSignalOnDrag = false;
        isDragging              = false;
    }

    enum Style
    {
        STYLE_NOQUIRK,
        STYLE_PLASTIQUE,
        STYLE_BREEZE,
        STYLE_FUSION,
    };

    QLineEdit*        edit;
    QDoubleValidator* validator;
    QSpinBox*         dummySpinBox;
    bool              upButtonDown;
    bool              downButtonDown;
    int               factor;
    int               fastSliderStep;
    double            slowFactor;
    double            shiftPercent;
    bool              shiftMode;
    QString           prefix;
    QString           suffix;
    double            exponentRatio;
    int               value;
    int               maximum;
    int               minimum;
    int               singleStep;
    Style             style;
    bool              blockUpdateSignalOnDrag;
    bool              isDragging;
};

DAbstractSliderSpinBox::DAbstractSliderSpinBox(QWidget* const parent, DAbstractSliderSpinBoxPrivate* const q)
    : QWidget(parent),
      d_ptr(q)
{
    Q_D(DAbstractSliderSpinBox);

    QEvent e(QEvent::StyleChange);
    changeEvent(&e);

    d->edit = new QLineEdit(this);
    d->edit->setContentsMargins(0, 0, 0, 0);
    d->edit->setAlignment(Qt::AlignCenter);
    d->edit->installEventFilter(this);
    d->edit->setFrame(false);
    d->edit->hide();

    // Make edit transparent
    d->edit->setAutoFillBackground(false);
    QPalette pal = d->edit->palette();
    pal.setColor(QPalette::Base, Qt::transparent);
    d->edit->setPalette(pal);

    connect(d->edit, SIGNAL(editingFinished()),
            this, SLOT(editLostFocus()));

    d->validator = new QDoubleValidator(d->edit);
    d->edit->setValidator(d->validator);

    setExponentRatio(1.0);

    // Set sane defaults
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    // dummy needed to fix a bug in the polyester theme
    d->dummySpinBox = new QSpinBox(this);
    d->dummySpinBox->hide();
}

DAbstractSliderSpinBox::~DAbstractSliderSpinBox()
{
    Q_D(DAbstractSliderSpinBox);
    delete d;
}

void DAbstractSliderSpinBox::showEdit()
{
    Q_D(DAbstractSliderSpinBox);

    if (d->edit->isVisible())
        return;

    if (d->style == DAbstractSliderSpinBoxPrivate::STYLE_PLASTIQUE)
    {
        d->edit->setGeometry(progressRect(spinBoxOptions()).adjusted(0, 0, -2, 0));
    }
    else
    {
        d->edit->setGeometry(progressRect(spinBoxOptions()));
    }

    d->edit->setText(valueString());
    d->edit->selectAll();
    d->edit->show();
    d->edit->setFocus(Qt::OtherFocusReason);
    update();
}

void DAbstractSliderSpinBox::hideEdit()
{
    Q_D(DAbstractSliderSpinBox);

    d->edit->hide();
    update();
}

void DAbstractSliderSpinBox::paintEvent(QPaintEvent* e)
{
    Q_D(DAbstractSliderSpinBox);
    Q_UNUSED(e)

    QPainter painter(this);

    switch (d->style)
    {
        case DAbstractSliderSpinBoxPrivate::STYLE_FUSION:
            paintFusion(painter);
            break;
        case DAbstractSliderSpinBoxPrivate::STYLE_PLASTIQUE:
            paintPlastique(painter);
            break;
        case DAbstractSliderSpinBoxPrivate::STYLE_BREEZE:
            paintBreeze(painter);
            break;
        default:
            paint(painter);
            break;
    }

    painter.end();
}

void DAbstractSliderSpinBox::paint(QPainter &painter)
{
    Q_D(DAbstractSliderSpinBox);

    // Create options to draw spin box parts
    QStyleOptionSpinBox spinOpts = spinBoxOptions();
    spinOpts.rect.adjust(0, 2, 0, -2);

    // Draw "SpinBox".Clip off the area of the lineEdit to avoid double
    // borders being drawn
    painter.save();
    painter.setClipping(true);

    QRect eraseRect(QPoint(rect().x(), rect().y()),
                    QPoint(progressRect(spinOpts).right(), rect().bottom()));

    painter.setClipRegion(QRegion(rect()).subtracted(eraseRect));
    style()->drawComplexControl(QStyle::CC_SpinBox, &spinOpts, &painter, d->dummySpinBox);
    painter.setClipping(false);
    painter.restore();


    QStyleOptionProgressBar progressOpts = progressBarOptions();
    progressOpts.rect.adjust(0, 2, 0, -2);
    style()->drawControl(QStyle::CE_ProgressBar, &progressOpts, &painter, 0);

    // Draw focus if necessary
    if (hasFocus() && d->edit->hasFocus())
    {
        QStyleOptionFocusRect focusOpts;
        focusOpts.initFrom(this);
        focusOpts.rect = progressOpts.rect;
        focusOpts.backgroundColor = palette().color(QPalette::Window);
        style()->drawPrimitive(QStyle::PE_FrameFocusRect, &focusOpts, &painter, this);
    }
}

void DAbstractSliderSpinBox::paintFusion(QPainter &painter)
{
    Q_D(DAbstractSliderSpinBox);

    QStyleOptionSpinBox spinOpts         = spinBoxOptions();
    QStyleOptionProgressBar progressOpts = progressBarOptions();
    spinOpts.frame                       = true;
    spinOpts.rect.adjust(0, -1, 0, 1);
    // spinOpts.palette().setBrush(QPalette::Base, palette().highlight());

    style()->drawComplexControl(QStyle::CC_SpinBox, &spinOpts, &painter, d->dummySpinBox);

    painter.save();

    QRect rect = progressOpts.rect.adjusted(1, 2, -4, -2);
    QRect leftRect;

    int progressIndicatorPos = (progressOpts.progress - double(progressOpts.minimum)) / qMax(double(1.0),
                                double(progressOpts.maximum) - progressOpts.minimum) * rect.width();

    if (progressIndicatorPos >= 0 && progressIndicatorPos <= rect.width())
    {
        leftRect = QRect(rect.left(), rect.top(), progressIndicatorPos, rect.height());
    }
    else if (progressIndicatorPos > rect.width())
    {
        painter.setPen(palette().highlightedText().color());
    }
    else
    {
        painter.setPen(palette().buttonText().color());
    }

    QRegion rightRect = rect;
    rightRect = rightRect.subtracted(leftRect);

    QTextOption textOption(Qt::AlignAbsolute | Qt::AlignHCenter | Qt::AlignVCenter);
    textOption.setWrapMode(QTextOption::NoWrap);

    if (!(d->edit && d->edit->isVisible()))
    {
        painter.setClipRegion(rightRect);
        painter.setClipping(true);
        painter.drawText(rect.adjusted(-2, 0, 2, 0), progressOpts.text, textOption);
        painter.setClipping(false);
    }

    if (!leftRect.isNull())
    {
        painter.setClipRect(leftRect.adjusted(0, -1, 1, 1));
        painter.setPen(palette().highlight().color());
        painter.setBrush(palette().highlight());

        spinOpts.palette.setBrush(QPalette::Base, palette().highlight());
        style()->drawComplexControl(QStyle::CC_SpinBox, &spinOpts, &painter, d->dummySpinBox);

        if (!(d->edit && d->edit->isVisible()))
        {
            painter.setPen(palette().highlightedText().color());
            painter.setClipping(true);
            painter.drawText(rect.adjusted(-2, 0, 2, 0), progressOpts.text, textOption);
        }

        painter.setClipping(false);
    }

    painter.restore();
}

void DAbstractSliderSpinBox::paintPlastique(QPainter &painter)
{
    Q_D(DAbstractSliderSpinBox);

    QStyleOptionSpinBox spinOpts         = spinBoxOptions();
    QStyleOptionProgressBar progressOpts = progressBarOptions();

    style()->drawComplexControl(QStyle::CC_SpinBox, &spinOpts, &painter, d->dummySpinBox);

    painter.save();

    QRect rect = progressOpts.rect.adjusted(2, 0, -2, 0);
    QRect leftRect;

    int progressIndicatorPos = (progressOpts.progress - double(progressOpts.minimum)) / qMax(double(1.0),
                                double(progressOpts.maximum) - progressOpts.minimum) * rect.width();

    if (progressIndicatorPos >= 0 && progressIndicatorPos <= rect.width())
    {
        leftRect = QRect(rect.left(), rect.top(), progressIndicatorPos, rect.height());
    }
    else if (progressIndicatorPos > rect.width())
    {
        painter.setPen(palette().highlightedText().color());
    }
    else
    {
        painter.setPen(palette().buttonText().color());
    }

    QRegion rightRect = rect;
    rightRect = rightRect.subtracted(leftRect);

    QTextOption textOption(Qt::AlignAbsolute | Qt::AlignHCenter | Qt::AlignVCenter);
    textOption.setWrapMode(QTextOption::NoWrap);

    if (!(d->edit && d->edit->isVisible()))
    {
        painter.setClipRegion(rightRect);
        painter.setClipping(true);
        painter.drawText(rect.adjusted(-2, 0, 2, 0), progressOpts.text, textOption);
        painter.setClipping(false);
    }

    if (!leftRect.isNull())
    {
        painter.setPen(palette().highlight().color());
        painter.setBrush(palette().highlight());
        painter.drawRect(leftRect.adjusted(0, 0, 0, -1));

        if (!(d->edit && d->edit->isVisible()))
        {
            painter.setPen(palette().highlightedText().color());
            painter.setClipRect(leftRect.adjusted(0, 0, 1, 0));
            painter.setClipping(true);
            painter.drawText(rect.adjusted(-2, 0, 2, 0), progressOpts.text, textOption);
            painter.setClipping(false);
        }
    }

    painter.restore();
}

void DAbstractSliderSpinBox::paintBreeze(QPainter &painter)
{
    Q_D(DAbstractSliderSpinBox);

    QStyleOptionSpinBox spinOpts         = spinBoxOptions();
    QStyleOptionProgressBar progressOpts = progressBarOptions();
    QString valueText                    = progressOpts.text;
    progressOpts.text                    = QLatin1String("");
    progressOpts.rect.adjust(0, 1, 0, -1);

    style()->drawComplexControl(QStyle::CC_SpinBox, &spinOpts, &painter, this);
    style()->drawControl(QStyle::CE_ProgressBarGroove, &progressOpts, &painter, this);

    painter.save();

    QRect leftRect;

    int progressIndicatorPos = (progressOpts.progress - double(progressOpts.minimum)) / qMax(double(1.0),
                                double(progressOpts.maximum) - progressOpts.minimum) * progressOpts.rect.width();

    if (progressIndicatorPos >= 0 && progressIndicatorPos <= progressOpts.rect.width())
    {
        leftRect = QRect(progressOpts.rect.left(), progressOpts.rect.top(), progressIndicatorPos, progressOpts.rect.height());
    }
    else if (progressIndicatorPos > progressOpts.rect.width())
    {
        painter.setPen(palette().highlightedText().color());
    }
    else
    {
        painter.setPen(palette().buttonText().color());
    }

    QRegion rightRect = progressOpts.rect;
    rightRect         = rightRect.subtracted(leftRect);
    painter.setClipRegion(rightRect);

    QTextOption textOption(Qt::AlignAbsolute | Qt::AlignHCenter | Qt::AlignVCenter);
    textOption.setWrapMode(QTextOption::NoWrap);

    if (!(d->edit && d->edit->isVisible()))
    {
        painter.drawText(progressOpts.rect, valueText, textOption);
    }

    if (!leftRect.isNull())
    {
        painter.setPen(palette().highlightedText().color());
        painter.setClipRect(leftRect);
        style()->drawControl(QStyle::CE_ProgressBarContents, &progressOpts, &painter, this);

        if (!(d->edit && d->edit->isVisible()))
        {
            painter.drawText(progressOpts.rect, valueText, textOption);
        }
    }

    painter.restore();
}

void DAbstractSliderSpinBox::mousePressEvent(QMouseEvent* e)
{
    Q_D(DAbstractSliderSpinBox);

    QStyleOptionSpinBox spinOpts = spinBoxOptions();

    // Depress buttons or highlight slider
    // Also used to emulate mouse grab...
    if (e->buttons() & Qt::LeftButton)
    {
        if (upButtonRect(spinOpts).contains(e->pos()))
        {
            d->upButtonDown = true;
        }
        else if (downButtonRect(spinOpts).contains(e->pos()))
        {
            d->downButtonDown = true;
        }
    }
    else if (e->buttons() & Qt::RightButton)
    {
        showEdit();
    }

    update();
}

void DAbstractSliderSpinBox::mouseReleaseEvent(QMouseEvent* e)
{
    Q_D(DAbstractSliderSpinBox);

    QStyleOptionSpinBox spinOpts = spinBoxOptions();

    d->isDragging = false;

    // Step up/down for buttons
    // Emualting mouse grab too
    if (upButtonRect(spinOpts).contains(e->pos()) && d->upButtonDown)
    {
        setInternalValue(d->value + d->singleStep);
    }
    else if (downButtonRect(spinOpts).contains(e->pos()) && d->downButtonDown)
    {
        setInternalValue(d->value - d->singleStep);
    }
    else if (progressRect(spinOpts).contains(e->pos()) &&
             !(d->edit->isVisible())                      &&
             !(d->upButtonDown || d->downButtonDown))
    {
        // Snap to percentage for progress area
        setInternalValue(valueForX(e->pos().x(),e->modifiers()));
    }
    else
    {
        // Confirm the last known value, since we might be ignoring move events
        setInternalValue(d->value);
    }

    d->upButtonDown   = false;
    d->downButtonDown = false;
    update();
}

void DAbstractSliderSpinBox::mouseMoveEvent(QMouseEvent* e)
{
    Q_D(DAbstractSliderSpinBox);

    if (e->modifiers() & Qt::ShiftModifier)
    {
        if (!d->shiftMode)
        {
            d->shiftPercent = pow(double(d->value - d->minimum) / double(d->maximum - d->minimum), 1 / double(d->exponentRatio));
            d->shiftMode = true;
        }
    }
    else
    {
        d->shiftMode = false;
    }

    // Respect emulated mouse grab.
    if (e->buttons() & Qt::LeftButton && !(d->downButtonDown || d->upButtonDown))
    {
        d->isDragging = true;
        setInternalValue(valueForX(e->pos().x(),e->modifiers()), d->blockUpdateSignalOnDrag);
        update();
    }
}

void DAbstractSliderSpinBox::keyPressEvent(QKeyEvent* e)
{
    Q_D(DAbstractSliderSpinBox);

    switch (e->key())
    {
        case Qt::Key_Up:
        case Qt::Key_Right:
            setInternalValue(d->value + d->singleStep);

            if (d->edit->isVisible())
            {
                d->edit->setText(valueString());
            }

            update();
            break;
        case Qt::Key_Down:
        case Qt::Key_Left:
            setInternalValue(d->value - d->singleStep);

            if (d->edit->isVisible())
            {
                d->edit->setText(valueString());
            }

            update();
            break;
        case Qt::Key_Shift:
            d->shiftPercent = pow(double(d->value - d->minimum) / double(d->maximum - d->minimum), 1 / double(d->exponentRatio));
            d->shiftMode = true;
            break;
        case Qt::Key_Enter: // Line edit isn't "accepting" key strokes...
        case Qt::Key_Return:
        case Qt::Key_Escape:
        case Qt::Key_Control:
        case Qt::Key_Alt:
        case Qt::Key_AltGr:
        case Qt::Key_Super_L:
        case Qt::Key_Super_R:
            break;
        default:
            showEdit();
            d->edit->event(e);
            break;
    }
}

void DAbstractSliderSpinBox::wheelEvent(QWheelEvent *e)
{
    Q_D(DAbstractSliderSpinBox);

    if (e->delta() > 0)
    {
        setInternalValue(d->value + d->singleStep);
    }
    else
    {
        setInternalValue(d->value - d->singleStep);
    }

    update();
    e->accept();
}

void DAbstractSliderSpinBox::focusInEvent(QFocusEvent* e)
{
    if (e->reason() == Qt::TabFocusReason)
    {
        showEdit();
    }

    e->accept();
}

bool DAbstractSliderSpinBox::eventFilter(QObject* recv, QEvent* e)
{
    Q_D(DAbstractSliderSpinBox);

    if (recv == static_cast<QObject*>(d->edit) && e->type() == QEvent::KeyRelease)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(e);

        switch (keyEvent->key())
        {
            case Qt::Key_Enter:
            case Qt::Key_Return:
                setInternalValue(QLocale::system().toDouble(d->edit->text()) * d->factor);
                hideEdit();
                return true;
            case Qt::Key_Escape:
                d->edit->setText(valueString());
                hideEdit();
                return true;
            default:
                break;
        }
    }
    else if (d->edit->isVisible() && e->type() == QEvent::ShortcutOverride)
    {
        QKeyEvent* const keyEvent = static_cast<QKeyEvent*>(e);

        switch (keyEvent->key())
        {
            case Qt::Key_Tab:
            case Qt::Key_Enter:
            case Qt::Key_Return:
            case Qt::Key_Escape:
                e->accept();
                return true;
            default:
                break;
        }
    }

    return false;
}

QSize DAbstractSliderSpinBox::sizeHint() const
{
    const Q_D(DAbstractSliderSpinBox);

    QStyleOptionSpinBox spinOpts = spinBoxOptions();
    QFont ft(font());

    if (d->style == DAbstractSliderSpinBoxPrivate::STYLE_NOQUIRK)
    {
        // Some styles use bold font in progressbars
        // unfortunately there is no reliable way to check for that
        ft.setBold(true);
    }

    QFontMetrics fm(ft);
    QSize hint(fm.boundingRect(d->prefix + QString::number(d->maximum) + d->suffix).size());
    hint += QSize(0, 2);

    switch (d->style)
    {
        case DAbstractSliderSpinBoxPrivate::STYLE_FUSION:
            hint += QSize(8, 8);
            break;
        case DAbstractSliderSpinBoxPrivate::STYLE_PLASTIQUE:
            hint += QSize(8, 0);
            break;
        case DAbstractSliderSpinBoxPrivate::STYLE_BREEZE:
            hint += QSize(2, 0);
            break;
        case DAbstractSliderSpinBoxPrivate::STYLE_NOQUIRK:
            // almost all "modern" styles have a margin around controls
            hint += QSize(6, 6);
            break;
        default:
            break;
    }

    // Getting the size of the buttons is a pain as the calcs require a rect
    // that is "big enough". We run the calc twice to get the "smallest" buttons
    // This code was inspired by QAbstractSpinBox
    QSize extra(1000, 0);
    spinOpts.rect.setSize(hint + extra);
    extra += hint - style()->subControlRect(QStyle::CC_SpinBox, &spinOpts,
                                            QStyle::SC_SpinBoxEditField, this).size();
    spinOpts.rect.setSize(hint + extra);
    extra += hint - style()->subControlRect(QStyle::CC_SpinBox, &spinOpts,
                                            QStyle::SC_SpinBoxEditField, this).size();
    hint += extra;

    spinOpts.rect.setSize(hint);
    return style()->sizeFromContents(QStyle::CT_SpinBox, &spinOpts, hint)
                                     .expandedTo(QApplication::globalStrut());
}

QSize DAbstractSliderSpinBox::minimumSizeHint() const
{
    return sizeHint();
}

QSize DAbstractSliderSpinBox::minimumSize() const
{
    return QWidget::minimumSize().expandedTo(minimumSizeHint());
}

QStyleOptionSpinBox DAbstractSliderSpinBox::spinBoxOptions() const
{
    const Q_D(DAbstractSliderSpinBox);

    QStyleOptionSpinBox opts;
    opts.initFrom(this);
    opts.frame         = false;
    opts.buttonSymbols = QAbstractSpinBox::UpDownArrows;
    opts.subControls   = QStyle::SC_SpinBoxUp | QStyle::SC_SpinBoxDown;

    // Disable non-logical buttons
    if (d->value == d->minimum)
    {
        opts.stepEnabled = QAbstractSpinBox::StepUpEnabled;
    }
    else if (d->value == d->maximum)
    {
        opts.stepEnabled = QAbstractSpinBox::StepDownEnabled;
    }
    else
    {
        opts.stepEnabled = QAbstractSpinBox::StepUpEnabled | QAbstractSpinBox::StepDownEnabled;
    }

    // Deal with depressed buttons
    if (d->upButtonDown)
    {
        opts.activeSubControls = QStyle::SC_SpinBoxUp;
    }
    else if (d->downButtonDown)
    {
        opts.activeSubControls = QStyle::SC_SpinBoxDown;
    }
    else
    {
        opts.activeSubControls = 0;
    }

    return opts;
}

QStyleOptionProgressBar DAbstractSliderSpinBox::progressBarOptions() const
{
    const Q_D(DAbstractSliderSpinBox);

    QStyleOptionSpinBox spinOpts = spinBoxOptions();

    // Create opts for drawing the progress portion
    QStyleOptionProgressBar progressOpts;
    progressOpts.initFrom(this);
    progressOpts.maximum = d->maximum;
    progressOpts.minimum = d->minimum;

    double minDbl  = d->minimum;
    double dValues = (d->maximum - minDbl);

    progressOpts.progress = dValues * pow((d->value - minDbl) / dValues, 1.0 / d->exponentRatio) + minDbl;
    progressOpts.text = d->prefix + valueString() + d->suffix;
    progressOpts.textAlignment = Qt::AlignCenter;
    progressOpts.textVisible = !(d->edit->isVisible());

    // Change opts rect to be only the ComboBox's text area
    progressOpts.rect = progressRect(spinOpts);

    return progressOpts;
}

QRect DAbstractSliderSpinBox::progressRect(const QStyleOptionSpinBox& spinBoxOptions) const
{
    const Q_D(DAbstractSliderSpinBox);

    QRect ret = style()->subControlRect(QStyle::CC_SpinBox, &spinBoxOptions,
                                        QStyle::SC_SpinBoxEditField);

    switch (d->style)
    {
        case DAbstractSliderSpinBoxPrivate::STYLE_PLASTIQUE:
            ret.adjust(-2, 0, 1, 0);
            break;
        case DAbstractSliderSpinBoxPrivate::STYLE_BREEZE:
            ret.adjust(1, 0, 0, 0);
            break;
        default:
            break;
    }

    return ret;
}

QRect DAbstractSliderSpinBox::upButtonRect(const QStyleOptionSpinBox& spinBoxOptions) const
{
    return style()->subControlRect(QStyle::CC_SpinBox, &spinBoxOptions,
                                   QStyle::SC_SpinBoxUp);
}

QRect DAbstractSliderSpinBox::downButtonRect(const QStyleOptionSpinBox& spinBoxOptions) const
{
    return style()->subControlRect(QStyle::CC_SpinBox, &spinBoxOptions,
                                   QStyle::SC_SpinBoxDown);
}

int DAbstractSliderSpinBox::valueForX(int x, Qt::KeyboardModifiers modifiers) const
{
    const Q_D(DAbstractSliderSpinBox);

    QStyleOptionSpinBox spinOpts = spinBoxOptions();

    QRect correctedProgRect;

    if (d->style == DAbstractSliderSpinBoxPrivate::STYLE_FUSION)
    {
        correctedProgRect = progressRect(spinOpts).adjusted(2, 0, -2, 0);
    }
    else if (d->style == DAbstractSliderSpinBoxPrivate::STYLE_BREEZE)
    {
        correctedProgRect = progressRect(spinOpts);
    }
    else
    {
        // Adjust for magic number in style code (margins)
        correctedProgRect = progressRect(spinOpts).adjusted(2, 2, -2, -2);
    }

    // Compute the distance of the progress bar, in pixel
    double leftDbl  = correctedProgRect.left();
    double xDbl     = x - leftDbl;

    // Compute the ration of the progress bar used, linearly (ignoring the exponent)
    double rightDbl = correctedProgRect.right();
    double minDbl   = d->minimum;
    double maxDbl   = d->maximum;

    double dValues  = (maxDbl - minDbl);
    double percent  = (xDbl / (rightDbl - leftDbl));

    // If SHIFT is pressed, movement should be slowed.
    if (modifiers & Qt::ShiftModifier)
    {
        percent = d->shiftPercent + (percent - d->shiftPercent) * d->slowFactor;
    }

    // Final value
    double realvalue = ((dValues * pow(percent, d->exponentRatio)) + minDbl);
    // If key CTRL is pressed, round to the closest step.
    if (modifiers & Qt::ControlModifier)
    {
        double fstep = d->fastSliderStep;

        if (modifiers & Qt::ShiftModifier)
        {
            fstep *= d->slowFactor;
        }

        realvalue = floor((realvalue+fstep / 2) / fstep) * fstep;
    }

    return int(realvalue);
}

void DAbstractSliderSpinBox::setPrefix(const QString& prefix)
{
    Q_D(DAbstractSliderSpinBox);
    d->prefix = prefix;
}

void DAbstractSliderSpinBox::setSuffix(const QString& suffix)
{
    Q_D(DAbstractSliderSpinBox);
    d->suffix = suffix;
}

void DAbstractSliderSpinBox::setExponentRatio(double dbl)
{
    Q_D(DAbstractSliderSpinBox);
    Q_ASSERT(dbl > 0);
    d->exponentRatio = dbl;
}

void DAbstractSliderSpinBox::setBlockUpdateSignalOnDrag(bool blockUpdateSignal)
{
    Q_D(DAbstractSliderSpinBox);
    d->blockUpdateSignalOnDrag = blockUpdateSignal;
}

void DAbstractSliderSpinBox::contextMenuEvent(QContextMenuEvent* event)
{
    event->accept();
}

void DAbstractSliderSpinBox::editLostFocus()
{
    Q_D(DAbstractSliderSpinBox);

    if (!d->edit->hasFocus())
    {
        if (d->edit->isModified())
        {
            setInternalValue(QLocale::system().toDouble(d->edit->text()) * d->factor);
        }

        hideEdit();
    }
}

void DAbstractSliderSpinBox::setInternalValue(int value)
{
    setInternalValue(value, false);
}

bool DAbstractSliderSpinBox::isDragging() const
{
    Q_D(const DAbstractSliderSpinBox);
    return d->isDragging;
}

void DAbstractSliderSpinBox::changeEvent(QEvent* e)
{
    Q_D(DAbstractSliderSpinBox);

    QWidget::changeEvent(e);

    if (e->type() == QEvent::StyleChange)
    {
        if (style()->objectName() == QLatin1String("fusion"))
        {
            d->style = DAbstractSliderSpinBoxPrivate::STYLE_FUSION;
        }
        else if (style()->objectName() == QLatin1String("plastique"))
        {
            d->style = DAbstractSliderSpinBoxPrivate::STYLE_PLASTIQUE;
        }
        else if (style()->objectName() == QLatin1String("breeze"))
        {
            d->style = DAbstractSliderSpinBoxPrivate::STYLE_BREEZE;
        }
        else
        {
            d->style = DAbstractSliderSpinBoxPrivate::STYLE_NOQUIRK;
        }
    }
}

// ---------------------------------------------------------------------------------------------

class DSliderSpinBoxPrivate : public DAbstractSliderSpinBoxPrivate
{
};

DSliderSpinBox::DSliderSpinBox(QWidget* parent) : DAbstractSliderSpinBox(parent, new DSliderSpinBoxPrivate)
{
    setRange(0, 99);
}

DSliderSpinBox::~DSliderSpinBox()
{
}

void DSliderSpinBox::setRange(int minimum, int maximum)
{
    Q_D(DSliderSpinBox);

    d->minimum = minimum;
    d->maximum = maximum;
    d->fastSliderStep = (maximum-minimum + 1) / 20;
    d->validator->setRange(minimum, maximum, 0);
    update();
}

int DSliderSpinBox::minimum() const
{
    const Q_D(DSliderSpinBox);
    return d->minimum;
}

void DSliderSpinBox::setMinimum(int minimum)
{
    Q_D(DSliderSpinBox);
    setRange(minimum, d->maximum);
}

int DSliderSpinBox::maximum() const
{
    const Q_D(DSliderSpinBox);
    return d->maximum;
}

void DSliderSpinBox::setMaximum(int maximum)
{
    Q_D(DSliderSpinBox);
    setRange(d->minimum, maximum);
}

int DSliderSpinBox::fastSliderStep() const
{
    const Q_D(DSliderSpinBox);
    return d->fastSliderStep;
}

void DSliderSpinBox::setFastSliderStep(int step)
{
    Q_D(DSliderSpinBox);
    d->fastSliderStep = step;
}

int DSliderSpinBox::value()
{
    Q_D(DSliderSpinBox);
    return d->value;
}

void DSliderSpinBox::setValue(int value)
{
    setInternalValue(value, false);
    update();
}

QString DSliderSpinBox::valueString() const
{
    const Q_D(DSliderSpinBox);

    QLocale locale;
    return locale.toString((double)d->value, 'f', d->validator->decimals());
}

void DSliderSpinBox::setSingleStep(int value)
{
    Q_D(DSliderSpinBox);
    d->singleStep = value;
}

void DSliderSpinBox::setPageStep(int value)
{
    Q_UNUSED(value);
}

void DSliderSpinBox::setInternalValue(int _value, bool blockUpdateSignal)
{
    Q_D(DAbstractSliderSpinBox);

    d->value = qBound(d->minimum, _value, d->maximum);

    if (!blockUpdateSignal)
    {
        emit(valueChanged(value()));
    }
}

// ---------------------------------------------------------------------------------------------

class DDoubleSliderSpinBoxPrivate : public DAbstractSliderSpinBoxPrivate
{
};

DDoubleSliderSpinBox::DDoubleSliderSpinBox(QWidget* parent) : DAbstractSliderSpinBox(parent, new DDoubleSliderSpinBoxPrivate)
{
}

DDoubleSliderSpinBox::~DDoubleSliderSpinBox()
{
}

void DDoubleSliderSpinBox::setRange(double minimum, double maximum, int decimals)
{
    Q_D(DDoubleSliderSpinBox);

    d->factor = pow(10.0, decimals);

    d->minimum = minimum * d->factor;
    d->maximum = maximum * d->factor;
    // This code auto-compute a new step when pressing control.
    // A flag defaulting to "do not change the fast step" should be added, but it implies changing every call
    if (maximum - minimum >= 2.0 || decimals <= 0)
    {
        // Quick step on integers
        d->fastSliderStep = int(pow(10.0, decimals));
    }
    else if (decimals == 1)
    {
        d->fastSliderStep = (maximum - minimum) * d->factor / 10;
    }
    else
    {
        d->fastSliderStep = (maximum - minimum) * d->factor / 20;
    }

    d->validator->setRange(minimum, maximum, decimals);
    update();
    setValue(value());
}

double DDoubleSliderSpinBox::minimum() const
{
    const Q_D(DAbstractSliderSpinBox);
    return d->minimum / d->factor;
}

void DDoubleSliderSpinBox::setMinimum(double minimum)
{
    Q_D(DAbstractSliderSpinBox);
    setRange(minimum, d->maximum);
}

double DDoubleSliderSpinBox::maximum() const
{
    const Q_D(DAbstractSliderSpinBox);
    return d->maximum / d->factor;
}

void DDoubleSliderSpinBox::setMaximum(double maximum)
{
    Q_D(DAbstractSliderSpinBox);
    setRange(d->minimum, maximum);
}

double DDoubleSliderSpinBox::fastSliderStep() const
{
    const Q_D(DAbstractSliderSpinBox);
    return d->fastSliderStep;
}
void DDoubleSliderSpinBox::setFastSliderStep(double step)
{
    Q_D(DAbstractSliderSpinBox);
    d->fastSliderStep = step;
}

double DDoubleSliderSpinBox::value()
{
    Q_D(DAbstractSliderSpinBox);
    return (double)d->value / d->factor;
}

void DDoubleSliderSpinBox::setValue(double value)
{
    Q_D(DAbstractSliderSpinBox);

    setInternalValue(d->value = qRound(value * d->factor), false);
    update();
}

void DDoubleSliderSpinBox::setSingleStep(double value)
{
    Q_D(DAbstractSliderSpinBox);
    d->singleStep = value * d->factor;
}

QString DDoubleSliderSpinBox::valueString() const
{
    const Q_D(DAbstractSliderSpinBox);

    QLocale locale;
    return locale.toString((double)d->value / d->factor, 'f', d->validator->decimals());
}

void DDoubleSliderSpinBox::setInternalValue(int _value, bool blockUpdateSignal)
{
    Q_D(DAbstractSliderSpinBox);

    d->value = qBound(d->minimum, _value, d->maximum);

    if (!blockUpdateSignal)
    {
        emit(valueChanged(value()));
    }
}

}  // namespace Digikam
