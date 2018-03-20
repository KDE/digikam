/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-03-14
 * Description : A widget to host settings as expander box
 *
 * Copyright (C) 2008-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C)      2010 by Manuel Viet <contact at 13zenrv dot fr>
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

#ifndef DEXPANDER_BOX_H
#define DEXPANDER_BOX_H

// Qt includes

#include <QObject>
#include <QPixmap>
#include <QLabel>
#include <QWidget>
#include <QScrollArea>
#include <QFrame>

// Local includes

#include "digikam_export.h"

class KConfigGroup;

namespace Digikam
{

/**
 * A widget to show an horizontal or vertical line separator
 **/
class DIGIKAM_EXPORT DLineWidget : public QFrame
{
    Q_OBJECT

public:

    explicit DLineWidget(Qt::Orientation orientation, QWidget* const parent=0);
    virtual ~DLineWidget();
};

// -------------------------------------------------------------------------

/** A label to show text adjusted to widget size
 */
class DIGIKAM_EXPORT DAdjustableLabel : public QLabel
{
    Q_OBJECT

public:

    explicit DAdjustableLabel(QWidget* const parent=0);
    virtual ~DAdjustableLabel();

    QSize minimumSizeHint() const;
    QSize sizeHint()        const;

    void setAlignment(Qt::Alignment align);
    void setElideMode(Qt::TextElideMode mode);

    QString adjustedText() const;

public Q_SLOTS:

    void setAdjustedText(const QString& text=QString());

private:

    void resizeEvent(QResizeEvent*);
    void adjustTextToLabel();

    // Disabled methods from QLabel
    QString text() const { return QString(); }; // Use adjustedText() instead.
    void setText(const QString&) {};            // Use setAdjustedText(text) instead.
    void clear() {};                            // Use setdjustedText(QString()) instead.

private:

    class Private;
    Private* const d;
};

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT DClickLabel : public QLabel
{
    Q_OBJECT

public:

    DClickLabel(QWidget* const parent = 0);
    explicit DClickLabel(const QString& text, QWidget* const parent = 0);
    ~DClickLabel();

Q_SIGNALS:

    /// Emitted when activated by left mouse click
    void leftClicked();
    /// Emitted when activated, by mouse or key press
    void activated();

protected:

    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void keyPressEvent(QKeyEvent* event);
};

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT DSqueezedClickLabel : public DAdjustableLabel
{
    Q_OBJECT

public:

    DSqueezedClickLabel(QWidget* const parent = 0);
    explicit DSqueezedClickLabel(const QString& text, QWidget* const parent = 0);
    ~DSqueezedClickLabel();

Q_SIGNALS:

    void leftClicked();
    void activated();

protected:

    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void keyPressEvent(QKeyEvent* event);
};

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT DArrowClickLabel : public QWidget
{
    Q_OBJECT

public:

    DArrowClickLabel(QWidget* const parent = 0);
    ~DArrowClickLabel();

    void setArrowType(Qt::ArrowType arrowType);
    Qt::ArrowType arrowType() const;

    virtual QSize sizeHint () const;

Q_SIGNALS:

    void leftClicked();

protected:

    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void paintEvent(QPaintEvent* event);

protected:

    Qt::ArrowType m_arrowType;
    int           m_size;
    int           m_margin;
};

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT DLabelExpander : public QWidget
{
    Q_OBJECT

public:

    DLabelExpander(QWidget* const parent = 0);
    ~DLabelExpander();

    void setCheckBoxVisible(bool b);
    bool checkBoxIsVisible() const;

    void setChecked(bool b);
    bool isChecked() const;

    void setLineVisible(bool b);
    bool lineIsVisible() const;

    void setText(const QString& txt);
    QString text() const;

    void setIcon(const QIcon &icon);
    QIcon icon() const;

    void setWidget(QWidget* const widget);
    QWidget* widget() const;

    void setExpanded(bool b);
    bool isExpanded() const;

    void setExpandByDefault(bool b);
    bool isExpandByDefault() const;

Q_SIGNALS:

    void signalExpanded(bool);
    void signalToggled(bool);

private Q_SLOTS:

    void slotToggleContainer();

private:

    bool eventFilter(QObject* obj, QEvent* ev);

private:

    class Private;
    Private* const d;
};

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT DExpanderBox : public QScrollArea
{
    Q_OBJECT

public:

    DExpanderBox(QWidget* const parent = 0);
    ~DExpanderBox();

    /** Add DLabelExpander item at end of box layout with these settings :
        'w'               : the widget hosted by DLabelExpander.
        'pix'             : pixmap used as icon to item title.
        'txt'             : text used as item title.
        'objName'         : item object name used to read/save expanded settings to rc file.
        'expandBydefault' : item state by default (expanded or not).
     */
    void addItem(QWidget* const w, const QIcon &icon, const QString& txt,
                 const QString& objName, bool expandBydefault);
    void addItem(QWidget* const w, const QString& txt,
                 const QString& objName, bool expandBydefault);

    /** Insert DLabelExpander item at box layout index with these settings :
        'w'               : the widget hosted by DLabelExpander.
        'pix'             : pixmap used as icon to item title.
        'txt'             : text used as item title.
        'objName'         : item object name used to read/save expanded settings to rc file.
        'expandBydefault' : item state by default (expanded or not).
     */
    void insertItem(int index, QWidget* const w, const QIcon &icon, const QString& txt,
                    const QString& objName, bool expandBydefault);
    void insertItem(int index, QWidget* const w, const QString& txt,
                    const QString& objName, bool expandBydefault);

    void removeItem(int index);

    void setCheckBoxVisible(int index, bool b);
    bool checkBoxIsVisible(int index) const;

    void setChecked(int index, bool b);
    bool isChecked(int index) const;

    void setItemText(int index, const QString& txt);
    QString itemText (int index) const;

    void setItemIcon(int index, const QIcon &icon);
    QIcon itemIcon(int index) const;

    void setItemToolTip(int index, const QString& tip);
    QString itemToolTip(int index) const;

    void setItemEnabled(int index, bool enabled);
    bool isItemEnabled(int index) const;

    void addStretch();
    void insertStretch(int index);

    void setItemExpanded(int index, bool b);
    bool isItemExpanded(int index) const;

    int  count() const;

    DLabelExpander* widget(int index) const;
    int indexOf(DLabelExpander* const widget) const;

    virtual void readSettings(KConfigGroup& group);
    virtual void writeSettings(KConfigGroup& group);

Q_SIGNALS:

    void signalItemExpanded(int index, bool b);
    void signalItemToggled(int index, bool b);

private Q_SLOTS:

    void slotItemExpanded(bool b);
    void slotItemToggled(bool b);

private:

    class Private;
    Private* const d;
};

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT DExpanderBoxExclusive : public DExpanderBox
{
    Q_OBJECT

public:

    DExpanderBoxExclusive(QWidget* const parent = 0);
    ~DExpanderBoxExclusive();

    /** Show one expander open at most */
    void setIsToolBox(bool b);
    bool isToolBox() const;

private Q_SLOTS:

    void slotItemExpanded(bool b);

private:

    bool m_toolbox;
};

} // namespace Digikam

#endif // DEXPANDER_BOX_H
