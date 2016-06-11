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

#ifndef SEARCHUTILITIES_H
#define SEARCHUTILITIES_H

// Qt includes

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QString>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSpinBox>

// Local includes

#include "comboboxutilities.h"

class QTextEdit;
class QPushButton;

namespace Digikam
{

class AnimatedClearButton : public QWidget
{
    Q_OBJECT

public:

    explicit AnimatedClearButton(QWidget* const parent = 0);

    QSize sizeHint() const;

    void setPixmap(const QPixmap& p);
    QPixmap pixmap() const;

    /**
     * Sets a primary condition for the button to be shown.
     * If false, animateVisible() will have no effect.
     */
    void setShallBeShown(bool show);

    /** This parameter determines the behavior when the animation
     *  to hide the widget has finished:
     *  If stayVisible is true, the widget remains visible,
     *  but paints nothing.
     *  If stayVisible is false, setVisible(false) is called,
     *  which removes the widget for layouting etc.
     *  Default: false
     */
    void stayVisibleWhenAnimatedOut(bool stayVisible);

public Q_SLOTS:

    /// Set visible, possibly with animation
    void animateVisible(bool visible);

    /// Set visible without animation
    void setDirectlyVisible(bool visible);

Q_SIGNALS:

    void clicked();

protected:

    virtual void paintEvent(QPaintEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);

protected Q_SLOTS:

    void visibleChanged();
    void updateAnimationSettings();

private:

    class Private;
    Private* const d;
};

// -------------------------------------------------------------------------

class CustomStepsDoubleSpinBox : public QDoubleSpinBox
{
    Q_OBJECT

public:

    /** This is a normal QDoubleSpinBox which allows to
     *  customize the stepping behavior, for cases where
     *  linear steps are not applicable
     */
    explicit CustomStepsDoubleSpinBox(QWidget* const parent = 0);
    ~CustomStepsDoubleSpinBox();

    /** Set a list of values that are usually applicable for the
     *  type of data of the combo box. The user can still type in
     *  any other value. Boundaries are not touched.
     *  Up or below the min and max values of the list given,
     *  default stepping is used.
     */
    void setSuggestedValues(const QList<double>& values);

    /** Sets the value that should be set as first value
     *  when first moving away from the minimum value.
     */
    void setSuggestedInitialValue(double initialValue);

    /** Allows to set to different default single steps,
     *  for the range below m_values, the other for above.
     */
    void setSingleSteps(double smaller, double larger);

    void setInvertStepping(bool invert);

    /** Resets to minimum value.
     */
    void reset();

    virtual void stepBy(int steps);

private Q_SLOTS:

    void slotValueChanged(double val);

private:

    class Private;
    Private* const d;
};

// -------------------------------------------------------------------------

class CustomStepsIntSpinBox : public QSpinBox
{
    Q_OBJECT

public:

    /** This is a normal QIntSpinBox which allows to
     *  customize the stepping behavior, for cases where
     *  linear steps are not applicable
     */
    explicit CustomStepsIntSpinBox(QWidget* const parent = 0);
    ~CustomStepsIntSpinBox();

    /** Set a list of values that are usually applicable for the
     *  type of data of the combo box. The user can still type in
     *  any other value. Boundaries are not touched.
     *  Up or below the min and max values of the list given,
     *  default stepping is used.
     */
    void setSuggestedValues(const QList<int>& values);

    /** Sets the value that should be set as first value
     *  when first moving away from the minimum value.
     */
    void setSuggestedInitialValue(int initialValue);

    /** Allows to set to different default single steps,
     *  for the range below m_values, the other for above.
     */
    void setSingleSteps(int smaller, int larger);

    void setInvertStepping(bool invert);

    /** Call this with a fraction prefix (like "1/") to enable
     *  magic handling of the value as fraction denominator.
     */
    void enableFractionMagic(const QString& prefix);

    /** Resets to minimum value
     */
    void reset();

    /** value() and setValue() for fraction magic value.
     */
    double fractionMagicValue() const;
    void   setFractionMagicValue(double value);

    virtual void stepBy(int steps);

protected:

    virtual QString textFromValue(int value) const;
    virtual int valueFromText(const QString& text) const;
    virtual StepEnabled stepEnabled() const;

private Q_SLOTS:

    void slotValueChanged(int val);

private:

    class Private;
    Private* const d;
};

// -------------------------------------------------------------------------

class StyleSheetDebugger : public QWidget
{
    Q_OBJECT

public:

    /** This widget is for development purpose only:
     *  It allows the developer to change the style sheet
     *  on a widget dynamically.
     *  If you want to develop or debug the stylesheet on your widget,
     *  add temporary code:
     *  new StyleSheetDebugger(myWidget);
     *  That's all. Change the style sheet by editing it and pressing Ok.
     */
    explicit StyleSheetDebugger(QWidget* const object);

protected Q_SLOTS:

    void buttonClicked();

protected:

    QTextEdit*   m_edit;
    QPushButton* m_okButton;
    QWidget*     m_widget;
};

} // namespace Digikam

#endif // SEARCHUTILITIES_H
