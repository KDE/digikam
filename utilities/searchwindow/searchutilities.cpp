/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-03-14
 * Description : User interface for searches
 *
 * Copyright (C) 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "searchutilities.moc"

// C++ includes

#include <cmath>

// Qt includes

#include <QAbstractListModel>
#include <QItemDelegate>
#include <QLineEdit>
#include <QLinearGradient>
#include <QListView>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QPropertyAnimation>
#include <QStyle>
#include <QStyleOption>
#include <QTextEdit>
#include <QTimeLine>
#include <QVBoxLayout>

// KDE includes

#include <kdeversion.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kstandardguiitem.h>
#include <ktextedit.h>
#include <kdebug.h>

// Local includes

#include "albummodel.h"
#include "ratingwidget.h"
#include "thememanager.h"
#include "itemvisibilitycontroller.h"

namespace Digikam
{

// Initial revision copied from klineedit_p.h,
// Copyright (C) 2007 Aaron Seigo <aseigo@kde.org>
// Now substantially rewritten.

class AnimatedClearButton::Private : public AnimatedVisibility
{
public:

    explicit Private(QObject* const parent) : AnimatedVisibility(parent)
    {
        stayAlwaysVisible = false;
    }

    bool    stayAlwaysVisible;
    QPixmap pixmap;
};

AnimatedClearButton::AnimatedClearButton(QWidget* const parent)
    : QWidget(parent), d(new Private(this))
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    connect(d, SIGNAL(opacityChanged()),
            this, SLOT(update()));

    connect(d, SIGNAL(visibleChanged()),
            this, SLOT(visibleChanged()));

    connect(KGlobalSettings::self(), SIGNAL(settingsChanged(int)),
            this, SLOT(updateAnimationSettings()));
}

QSize AnimatedClearButton::sizeHint() const
{
    QFontMetrics fm(font());
    return QSize(d->pixmap.width(), fm.lineSpacing());
}

void AnimatedClearButton::stayVisibleWhenAnimatedOut(bool stayVisible)
{
    d->stayAlwaysVisible = stayVisible;
    visibleChanged();
}

void AnimatedClearButton::setShallBeShown(bool shown)
{
    d->controller()->setShallBeShownDirectly(shown);
    visibleChanged();
}

void AnimatedClearButton::animateVisible(bool visible)
{
    // skip animation if parent widget is not visible
    if (!parentWidget() || !parentWidget()->isVisible())
    {
        d->controller()->setDirectlyVisible(visible);
    }

    d->controller()->setAnimationDuration(visible ? 150 : 250);
    d->controller()->setVisible(visible);
}

void AnimatedClearButton::setDirectlyVisible(bool visible)
{
    d->controller()->setDirectlyVisible(visible);
}

void AnimatedClearButton::setPixmap(const QPixmap& p)
{
    d->pixmap = p;
}

QPixmap AnimatedClearButton::pixmap() const
{
    return d->pixmap;
}

void AnimatedClearButton::updateAnimationSettings()
{
}

void AnimatedClearButton::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    if (KGlobalSettings::graphicEffectsLevel() & KGlobalSettings::SimpleAnimationEffects)
    {
        if (d->opacity() == 0)
        {
            return;
        }

        QPainter p(this);
        p.setOpacity(d->opacity() * 255);
        p.drawPixmap((width()  - d->pixmap.width())  / 2,
                     (height() - d->pixmap.height()) / 2,
                     d->pixmap);
    }
    else
    {
        QPainter p(this);
        p.setOpacity(1); // make sure
        p.drawPixmap((width()  - d->pixmap.width())  / 2,
                     (height() - d->pixmap.height()) / 2,
                     d->pixmap);
    }
}

void AnimatedClearButton::visibleChanged()
{
    if (d->isVisible())
    {
        show();
    }
    else if (!d->controller()->shallBeShown() || !d->stayAlwaysVisible)
    {
        hide();
    }
}

void AnimatedClearButton::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        emit clicked();
    }
}

// ------------------------------------------------------------------------

class CustomStepsDoubleSpinBox::Private
{
public:

    Private() :
      beforeInitialValue(true),
      initialValue(0),
      smallerStep(0),
      largerStep(0),
      invertStepping(false)
    {
    }

    bool          beforeInitialValue;
    QList<double> values;
    double        initialValue;
    double        smallerStep;
    double        largerStep;
    bool          invertStepping;
};

CustomStepsDoubleSpinBox::CustomStepsDoubleSpinBox(QWidget* const parent)
    : QDoubleSpinBox(parent), d(new Private)
{
}

CustomStepsDoubleSpinBox::~CustomStepsDoubleSpinBox()
{
    delete d;
}

void CustomStepsDoubleSpinBox::stepBy(int steps)
{
    if (d->invertStepping)
    {
        steps = -steps;
    }

    if (d->values.isEmpty())
    {
        QDoubleSpinBox::stepBy(steps);
        return;
    }

    if (d->beforeInitialValue && d->initialValue > minimum())
    {
        setValue(d->initialValue);
        return;
    }

    double v = value();

    if (v >= d->values.first() && v <= d->values.last())
    {
        int nextStep = 0;

        if (steps > 0)
        {
            // find the next value in d->values after current value
            for (nextStep = 0; nextStep < d->values.count(); ++nextStep)
            {
                if (v <= d->values.at(nextStep))
                {
                    ++nextStep;
                    break;
                }
            }

            // go as many steps in d->values as we need
            int stepsToGo = steps;

            for (; stepsToGo > 0 && nextStep < d->values.count(); --stepsToGo)
            {
                v = d->values.at(nextStep++);
            }

            // set the new value
            setValue(v);

            // if anything is left, use Qt code
            if (stepsToGo)
            {
                QDoubleSpinBox::stepBy(stepsToGo);
            }
        }
        else
        {
            for (nextStep = d->values.count() - 1; nextStep >= 0; --nextStep)
            {
                if (v >= d->values.at(nextStep))
                {
                    --nextStep;
                    break;
                }
            }

            int stepsToGo = -steps;

            for (; stepsToGo > 0 && nextStep >= 0; --stepsToGo)
            {
                v = d->values.at(nextStep--);
            }

            setValue(v);

            if (stepsToGo)
            {
                QDoubleSpinBox::stepBy(-stepsToGo);
            }
        }
    }
    else
    {
        QDoubleSpinBox::stepBy(steps);
    }
}

void CustomStepsDoubleSpinBox::setSuggestedValues(const QList<double>& values)
{
    connect(this, SIGNAL(valueChanged(double)),
            this, SLOT(slotValueChanged(double)));

    d->values = values;
    qSort(d->values);
}

void CustomStepsDoubleSpinBox::setSuggestedInitialValue(double initialValue)
{
    d->initialValue = initialValue;
}

void CustomStepsDoubleSpinBox::setSingleSteps(double smaller, double larger)
{
    d->smallerStep = smaller;
    d->largerStep  = larger;
}

void CustomStepsDoubleSpinBox::setInvertStepping(bool invert)
{
    d->invertStepping = invert;
}

void CustomStepsDoubleSpinBox::reset()
{
    setValue(minimum());
    d->beforeInitialValue = true;
}

void CustomStepsDoubleSpinBox::slotValueChanged(double val)
{
    if (val != minimum())
    {
        d->beforeInitialValue = false;
    }

    if (!d->values.isEmpty())
    {
        if (d->largerStep && val >= d->values.last())
        {
            setSingleStep(d->largerStep);
        }
        else if (d->smallerStep)
        {
            setSingleStep(d->smallerStep);
        }
    }
}

// ------------------------------------------------------------------------

CustomStepsIntSpinBox::CustomStepsIntSpinBox(QWidget* const parent)
    : QSpinBox(parent),
      m_beforeInitialValue(true),
      m_initialValue(0),
      m_smallerStep(0),
      m_largerStep(0),
      m_invertStepping(false)
{
}

void CustomStepsIntSpinBox::stepBy(int steps)
{
    if (m_invertStepping)
    {
        steps = -steps;
    }

    if (m_values.isEmpty())
    {
        QSpinBox::stepBy(steps);
        return;
    }

    if (m_beforeInitialValue && m_initialValue > minimum())
    {
        setValue(m_initialValue);
        return;
    }

    int v = value();

    if (v >= m_values.first() && v <= m_values.last())
    {
        int nextStep = 0;

        if (steps > 0)
        {
            // find the next value in m_values after current value
            for (nextStep = 0; nextStep < m_values.count(); ++nextStep)
            {
                if (v <= m_values.at(nextStep))
                {
                    ++nextStep;
                    break;
                }
            }

            // go as many steps in m_values as we need
            int stepsToGo = steps;

            for (; stepsToGo > 0 && nextStep < m_values.count(); --stepsToGo)
            {
                v = m_values.at(nextStep++);
            }

            // set the new value
            setValue(v);

            // if anything is left, use Qt code
            if (stepsToGo)
            {
                QSpinBox::stepBy(stepsToGo);
            }
        }
        else
        {
            for (nextStep = m_values.count() - 1; nextStep >= 0; --nextStep)
            {
                if (v >= m_values.at(nextStep))
                {
                    --nextStep;
                    break;
                }
            }

            int stepsToGo = -steps;

            for (; stepsToGo > 0 && nextStep >= 0; --stepsToGo)
            {
                v = m_values.at(nextStep--);
            }

            setValue(v);

            if (stepsToGo)
            {
                QSpinBox::stepBy(-stepsToGo);
            }
        }
    }
    else
    {
        QSpinBox::stepBy(steps);
    }
}

void CustomStepsIntSpinBox::setSuggestedValues(const QList<int>& values)
{
    connect(this, SIGNAL(valueChanged(int)),
            this, SLOT(slotValueChanged(int)));

    m_values = values;
    qSort(m_values);
}

void CustomStepsIntSpinBox::setSuggestedInitialValue(int initialValue)
{
    m_initialValue = initialValue;
}

void CustomStepsIntSpinBox::setSingleSteps(int smaller, int larger)
{
    m_smallerStep = smaller;
    m_largerStep  = larger;
}

void CustomStepsIntSpinBox::setInvertStepping(bool invert)
{
    m_invertStepping = invert;
}

void CustomStepsIntSpinBox::enableFractionMagic(const QString& prefix)
{
    m_fractionPrefix = prefix;
    qSort(m_values.begin(), m_values.end(), qGreater<int>());
}

void CustomStepsIntSpinBox::reset()
{
    setValue(minimum());
    m_beforeInitialValue = true;
}

QString CustomStepsIntSpinBox::textFromValue(int value) const
{
    // reimplemented for fraction magic handling
    if (m_fractionPrefix.isNull())
    {
        return QSpinBox::textFromValue(value);
    }

    if (value < 0)
    {
        return m_fractionPrefix + QSpinBox::textFromValue(- value);
    }
    else
    {
        return QSpinBox::textFromValue(value);
    }
}

int CustomStepsIntSpinBox::valueFromText(const QString& text) const
{
    // reimplemented for fraction magic handling
    if (m_fractionPrefix.isNull())
    {
        return QSpinBox::valueFromText(text);
    }

    if (text.startsWith(m_fractionPrefix))
    {
        return - QSpinBox::valueFromText(text.mid(m_fractionPrefix.length()));
    }
    else
    {
        return QSpinBox::valueFromText(text);
    }
}

QAbstractSpinBox::StepEnabled CustomStepsIntSpinBox::stepEnabled() const
{
    if (m_fractionPrefix.isNull())
    {
        return QSpinBox::stepEnabled();
    }

    QAbstractSpinBox::StepEnabled s;

    if (value() > minimum() || value() == minimum())
    {
        s |= QAbstractSpinBox::StepUpEnabled;
    }

    if (value() < maximum())
    {
        s |= QAbstractSpinBox::StepDownEnabled;
    }

    return s;
}

double CustomStepsIntSpinBox::fractionMagicValue() const
{
    if (m_fractionPrefix.isNull())
    {
        return value();
    }

    int v = QSpinBox::value();

    if (v < 0)
    {
        return - 1.0 / v;
    }
    else
    {
        return v;
    }
}

void CustomStepsIntSpinBox::setFractionMagicValue(double value)
{
    if (m_fractionPrefix.isNull())
    {
        setValue((int)value);
        return;
    }

    if (value < 1.0)
    {
        setValue(- lround(1.0 / value));
    }
    else
    {
        setValue((int)value);
    }
}

void CustomStepsIntSpinBox::slotValueChanged(int d)
{
    if (d != minimum())
    {
        m_beforeInitialValue = false;
    }

    if (!m_values.isEmpty())
    {
        if (m_largerStep && d >= m_values.last())
        {
            setSingleStep(m_largerStep);
        }
        else if (m_smallerStep)
        {
            setSingleStep(m_smallerStep);
        }
    }
}

// ------------------------------------------------------------------------

StyleSheetDebugger::StyleSheetDebugger(QWidget* const object)
    : QWidget(0), m_widget(object)
{
    setAttribute(Qt::WA_DeleteOnClose);

    QVBoxLayout* vbox = new QVBoxLayout;

    m_edit            = new KTextEdit;
    vbox->addWidget(m_edit, 1);
    m_okButton        = new KPushButton(KStandardGuiItem::ok());
    vbox->addWidget(m_okButton, 0, Qt::AlignRight);

    setLayout(vbox);

    connect(m_okButton, SIGNAL(clicked()),
            this, SLOT(buttonClicked()));

    m_edit->setPlainText(m_widget->styleSheet());

    resize(400, 300);
    show();
}

void StyleSheetDebugger::buttonClicked()
{
    m_widget->setStyleSheet(m_edit->toPlainText());
}

} // namespace Digikam
