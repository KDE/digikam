/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2014-11-30
 * @brief  Save space slider widget
 *
 * @author Copyright (C) 2014-2016 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 * @author Copyright (C) 2010 by Justin Noel
 *         <a href="mailto:justin at ics dot com">justin at ics dot com</a>
 * @author Copyright (C) 2010 by Cyrille Berger
 *         <a href="mailto:cberger at cberger dot net">cberger at cberger dot net</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "dsliderspinbox.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QPainter>
#include <QStyle>
#include <QLineEdit>
#include <QApplication>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QIntValidator>
#include <QTimer>
#include <QtDebug>
#include <QDoubleSpinBox>

namespace Digikam
{

class DAbstractSliderSpinBoxPrivate
{
public:

    DAbstractSliderSpinBoxPrivate()
    {
        edit            = 0;
        validator       = 0;
        dummySpinBox    = 0;
        upButtonDown    = false;
        downButtonDown  = false;
        shiftMode       = false;
        factor          = 1.0;
        fastSliderStep  = 5;
        slowFactor      = 0.1;
        shiftPercent    = 0.0;
        exponentRatio   = 0.0;
        value           = 0;
        maximum         = 100;
        minimum         = 0;
        singleStep      = 1;
    }

    QLineEdit*        edit;
    QDoubleValidator* validator;
    bool              upButtonDown;
    bool              downButtonDown;
    int               factor;
    int               fastSliderStep;
    double            slowFactor;
    double            shiftPercent;
    bool              shiftMode;
    QString           suffix;
    double            exponentRatio;
    int               value;
    int               maximum;
    int               minimum;
    int               singleStep;
    QSpinBox*         dummySpinBox;
};

DAbstractSliderSpinBox::DAbstractSliderSpinBox(QWidget* const parent, DAbstractSliderSpinBoxPrivate* const q)
    : QWidget(parent),
      d_ptr(q)
{
    Q_D(DAbstractSliderSpinBox);

    d->edit = new QLineEdit(this);
    d->edit->setFrame(false);
    d->edit->setAlignment(Qt::AlignCenter);
    d->edit->hide();
    d->edit->installEventFilter(this);

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

    if (d->edit->isVisible()) return;

    d->edit->setGeometry(editRect(spinBoxOptions()));
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

    // Create options to draw spin box parts
    QStyleOptionSpinBox spinOpts = spinBoxOptions();

    // Draw "SpinBox".Clip off the area of the lineEdit to avoid double borders being drawn

    painter.save();
    painter.setClipping(true);
    QRect eraseRect(QPoint(rect().x(), rect().y()),
                    QPoint(editRect(spinOpts).right(), rect().bottom()));
    painter.setClipRegion(QRegion(rect()).subtracted(eraseRect));
    style()->drawComplexControl(QStyle::CC_SpinBox, &spinOpts, &painter, d->dummySpinBox);
    painter.setClipping(false);
    painter.restore();


    // Create options to draw progress bar parts
    QStyleOptionProgressBar progressOpts = progressBarOptions();

    // Draw "ProgressBar" in SpinBox
    style()->drawControl(QStyle::CE_ProgressBar, &progressOpts, &painter, 0);

    // Draw focus if necessary
    if (hasFocus() && d->edit->hasFocus())
    {
        QStyleOptionFocusRect focusOpts;
        focusOpts.initFrom(this);
        focusOpts.rect            = progressOpts.rect;
        focusOpts.backgroundColor = palette().color(QPalette::Window);
        style()->drawPrimitive(QStyle::PE_FrameFocusRect, &focusOpts, &painter, this);
    }
}

void DAbstractSliderSpinBox::mousePressEvent(QMouseEvent* e)
{
    Q_D(DAbstractSliderSpinBox);
    QStyleOptionSpinBox spinOpts         = spinBoxOptions();
    QStyleOptionProgressBar progressOpts = progressBarOptions();

    // Depress buttons or highlight slider. Also used to emulate mouse grab.

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
        else if (labelRect(progressOpts).contains(e->pos()))
        {
            showEdit();
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

    // Step up/down for buttons. Emulating mouse grab too.

    if (upButtonRect(spinOpts).contains(e->pos()) && d->upButtonDown)
    {
        setInternalValue(d->value + d->singleStep);
    }
    else if (downButtonRect(spinOpts).contains(e->pos()) && d->downButtonDown)
    {
        setInternalValue(d->value - d->singleStep);
    }
    else if (editRect(spinOpts).contains(e->pos()) &&
             !(d->edit->isVisible())                   &&
             !(d->upButtonDown || d->downButtonDown))
    {
        // Snap to percentage for progress area
        setInternalValue(valueForX(e->pos().x(),e->modifiers()));
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
            d->shiftPercent = pow(double(d->value - d->minimum)/double(d->maximum - d->minimum),
                                  1 / double(d->exponentRatio));
            d->shiftMode    = true;
        }
    }
    else
    {
        d->shiftMode = false;
    }

    // Respect emulated mouse grab.
    if (e->buttons() & Qt::LeftButton && !(d->downButtonDown || d->upButtonDown))
    {
        setInternalValue(valueForX(e->pos().x(),e->modifiers()));
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
            break;
        case Qt::Key_Down:
        case Qt::Key_Left:
            setInternalValue(d->value - d->singleStep);
            break;
        case Qt::Key_Shift:
            d->shiftPercent = pow( double(d->value - d->minimum)/double(d->maximum - d->minimum), 1/double(d->exponentRatio) );
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

    int step = d->fastSliderStep;

    if (e->modifiers() & Qt::ShiftModifier)
    {
        step = d->singleStep;
    }

    if (e->delta() > 0)
    {
        setInternalValue(d->value + step);
    }
    else
    {
        setInternalValue(d->value - step);
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
        QKeyEvent* const keyEvent = static_cast<QKeyEvent*>(e);

        switch (keyEvent->key())
        {
            case Qt::Key_Enter:
            case Qt::Key_Return:
                setInternalValue(QLocale::system().toDouble(d->edit->text()) * d->factor);
                hideEdit();
                setFocus();
                return true;
            case Qt::Key_Escape:
                hideEdit();
                setFocus();
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
                if (d->edit->isModified())
                {
                    setInternalValue(QLocale::system().toDouble(d->edit->text()) * d->factor);
                    hideEdit();
                    setFocus();
                }

                e->accept();
                return true;
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
    QFontMetrics fm(font());

    // We need at least 50 pixels or things start to look bad
    int w = qMax(fm.width(QString::number(d->maximum)), 50);
    QSize hint(w, d->edit->sizeHint().height() + 3);

    // Getting the size of the buttons is a pain as the calcs require a rect
    // that is "big enough". We run the calc twice to get the "smallest" buttons
    // This code was inspired by QAbstractSpinBox.

    QSize extra(35, 6);
    spinOpts.rect.setSize(hint + extra);
    extra += hint - style()->subControlRect(QStyle::CC_SpinBox, &spinOpts,
                                            QStyle::SC_SpinBoxEditField, this).size();

    spinOpts.rect.setSize(hint + extra);
    extra += hint - style()->subControlRect(QStyle::CC_SpinBox, &spinOpts,
                                            QStyle::SC_SpinBoxEditField, this).size();
    hint += extra;

    spinOpts.rect = rect();

    return style()->sizeFromContents(QStyle::CT_SpinBox, &spinOpts, hint, 0)
                   .expandedTo(QApplication::globalStrut());
}

QSize DAbstractSliderSpinBox::minimumSizeHint() const
{
    return sizeHint();
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
    progressOpts.maximum       = d->maximum;
    progressOpts.minimum       = d->minimum;

    double minDbl              = d->minimum;
    double dValues             = (d->maximum - minDbl);

    progressOpts.progress      = dValues * pow((d->value - minDbl) / dValues, 1.0 / d->exponentRatio) + minDbl;
    progressOpts.text          = valueString() + d->suffix;
    progressOpts.textAlignment = Qt::AlignCenter;
    progressOpts.textVisible   = !(d->edit->isVisible());

    // Change opts rect to be only the ComboBox's text area
    progressOpts.rect          = editRect(spinOpts);

    return progressOpts;
}

QRect DAbstractSliderSpinBox::editRect(const QStyleOptionSpinBox& spinBoxOptions) const
{
    return style()->subControlRect(QStyle::CC_SpinBox, &spinBoxOptions,
                                   QStyle::SC_SpinBoxEditField);
}

QRect DAbstractSliderSpinBox::labelRect(const QStyleOptionProgressBar& progressBarOptions) const
{
    return style()->subElementRect(QStyle::SE_ProgressBarLabel, &progressBarOptions);
}

QRect DAbstractSliderSpinBox::progressRect(const QStyleOptionProgressBar& progressBarOptions) const
{
    return style()->subElementRect(QStyle::SE_ProgressBarGroove, &progressBarOptions);
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
    QStyleOptionProgressBar progressOpts = progressBarOptions();

    // Adjust for magic number in style code (margins)
    QRect correctedProgRect = progressRect(progressOpts).adjusted(2, 2, -2, -2);

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
    if ( modifiers & Qt::ShiftModifier )
    {
        percent = d->shiftPercent + (percent - d->shiftPercent) * d->slowFactor;
    }

    // Final value
    double realvalue = ((dValues * pow(percent, d->exponentRatio)) + minDbl);

    // If key CTRL is pressed, round to the closest step.

    if ( modifiers & Qt::ControlModifier )
    {
        double fstep = d->fastSliderStep;

        if( modifiers & Qt::ShiftModifier )
        {
            fstep *= d->slowFactor;
        }

        realvalue = floor((realvalue + fstep / 2) / fstep) * fstep;
    }

    // Return the value
    return int(realvalue);
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

void DAbstractSliderSpinBox::contextMenuEvent(QContextMenuEvent* event)
{
    event->accept();
}

void DAbstractSliderSpinBox::editLostFocus()
{
    // only hide on focus lost, if editing is finished that will be handled in eventFilter
    Q_D(DAbstractSliderSpinBox);

    if (!d->edit->hasFocus())
    {
        hideEdit();
    }
}

// ---------------------------------------------------------------------------------------------

class DSliderSpinBoxPrivate : public DAbstractSliderSpinBoxPrivate
{
};

DSliderSpinBox::DSliderSpinBox(QWidget* const parent)
    : DAbstractSliderSpinBox(parent, new DSliderSpinBoxPrivate)
{
    setRange(0,99);
}

DSliderSpinBox::~DSliderSpinBox()
{
}

void DSliderSpinBox::setRange(int minimum, int maximum)
{
    Q_D(DSliderSpinBox);
    d->minimum        = minimum;
    d->maximum        = maximum;
    d->fastSliderStep = (maximum-minimum+1)/20;
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

int DSliderSpinBox::value() const
{
    const Q_D(DSliderSpinBox);
    return d->value;
}

void DSliderSpinBox::setValue(int value)
{
    setInternalValue(value);
    update();
}

QString DSliderSpinBox::valueString() const
{
    const Q_D(DSliderSpinBox);
    return QLocale::system().toString(d->value);
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

void DSliderSpinBox::setInternalValue(int _value)
{
    Q_D(DAbstractSliderSpinBox);
    d->value = qBound(d->minimum, _value, d->maximum);
    emit(valueChanged(value()));
}

// ---------------------------------------------------------------------------------------------

class DDoubleSliderSpinBoxPrivate : public DAbstractSliderSpinBoxPrivate
{
};

DDoubleSliderSpinBox::DDoubleSliderSpinBox(QWidget* const parent)
    : DAbstractSliderSpinBox(parent, new DDoubleSliderSpinBoxPrivate)
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
        //Quick step on integers
        d->fastSliderStep = int(pow(10.0, decimals));
    }
    else if(decimals == 1)
    {
        d->fastSliderStep = (maximum-minimum)*d->factor/10;
    }
    else
    {
        d->fastSliderStep = (maximum-minimum)*d->factor/20;
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
    d->fastSliderStep = step * d->factor;
}

double DDoubleSliderSpinBox::value() const
{
    const Q_D(DAbstractSliderSpinBox);
    return (double)d->value / d->factor;
}

void DDoubleSliderSpinBox::setValue(double value)
{
    Q_D(DAbstractSliderSpinBox);
    setInternalValue(qRound(value * d->factor));
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
    return QLocale::system().toString((double)d->value / d->factor, 'f', d->validator->decimals());
}

void DDoubleSliderSpinBox::setInternalValue(int val)
{
    Q_D(DAbstractSliderSpinBox);
    d->value = qBound(d->minimum, val, d->maximum);
    emit(valueChanged(value()));
}

}  // namespace Digikam
