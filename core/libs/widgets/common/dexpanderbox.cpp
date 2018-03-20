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

#include "dexpanderbox.h"

// Qt includes

#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QDesktopWidget>

// KDE includes

#include <kconfiggroup.h>

// Local includes

#include "dlayoutbox.h"

namespace Digikam
{

DLineWidget::DLineWidget(Qt::Orientation orientation, QWidget* const parent)
    : QFrame(parent)
{
    setLineWidth(1);
    setMidLineWidth(0);
    setFrameShadow(QFrame::Sunken);

    if (orientation == Qt::Vertical)
    {
        setFrameShape(QFrame::VLine);
        setMinimumSize(2, 0);
    }
    else
    {
        setFrameShape(QFrame::HLine);
        setMinimumSize(0, 2);
    }

    updateGeometry();
}

DLineWidget::~DLineWidget()
{
}

// ------------------------------------------------------------------------------------

class Q_DECL_HIDDEN DAdjustableLabel::Private
{
public:

    Private()
    {
        emode = Qt::ElideMiddle;
    }

    QString           ajdText;
    Qt::TextElideMode emode;
};

DAdjustableLabel::DAdjustableLabel(QWidget* const parent)
    : QLabel(parent),
      d(new Private)
{
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
}

DAdjustableLabel::~DAdjustableLabel()
{
    delete d;
}

void DAdjustableLabel::resizeEvent(QResizeEvent*)
{
    adjustTextToLabel();
}

QSize DAdjustableLabel::minimumSizeHint() const
{
    QSize sh = QLabel::minimumSizeHint();
    sh.setWidth(-1);
    return sh;
}

QSize DAdjustableLabel::sizeHint() const
{
    QFontMetrics fm(fontMetrics());
    int maxW     = QApplication::desktop()->screenGeometry(this).width() * 3 / 4;
    int currentW = fm.width(d->ajdText);

    return (QSize(currentW > maxW ? maxW : currentW, QLabel::sizeHint().height()));
}

void DAdjustableLabel::setAdjustedText(const QString& text)
{
    d->ajdText = text;

    if (d->ajdText.isNull())
        QLabel::clear();

    adjustTextToLabel();
}

QString DAdjustableLabel::adjustedText() const
{
    return d->ajdText;
}

void DAdjustableLabel::setAlignment(Qt::Alignment alignment)
{
    QString tmp(d->ajdText);
    QLabel::setAlignment(alignment);
    d->ajdText = tmp;
}

void DAdjustableLabel::setElideMode(Qt::TextElideMode mode)
{
    d->emode = mode;
    adjustTextToLabel();
}

void DAdjustableLabel::adjustTextToLabel()
{
    QFontMetrics fm(fontMetrics());
    QStringList adjustedLines;
    int lblW      = size().width();
    bool adjusted = false;

    Q_FOREACH(const QString& line, d->ajdText.split(QLatin1Char('\n')))
    {
        int lineW = fm.width(line);

        if (lineW > lblW)
        {
            adjusted = true;
            adjustedLines << fm.elidedText(line, d->emode, lblW);
        }
        else
        {
            adjustedLines << line;
        }
    }

    if (adjusted)
    {
        QLabel::setText(adjustedLines.join(QLatin1String("\n")));
        setToolTip(d->ajdText);
    }
    else
    {
        QLabel::setText(d->ajdText);
        setToolTip(QString());
    }
}

// ------------------------------------------------------------------------------------

DClickLabel::DClickLabel(QWidget* const parent)
    : QLabel(parent)
{
    setCursor(Qt::PointingHandCursor);
}

DClickLabel::DClickLabel(const QString& text, QWidget* const parent)
    : QLabel(text, parent)
{
    setCursor(Qt::PointingHandCursor);
}

DClickLabel::~DClickLabel()
{
}

void DClickLabel::mousePressEvent(QMouseEvent* event)
{
    QLabel::mousePressEvent(event);

    /*
     * In some contexts, like QGraphicsView, there will be no
     * release event if the press event was not accepted.
     */
    if (event->button() == Qt::LeftButton)
    {
        event->accept();
    }
}

void DClickLabel::mouseReleaseEvent(QMouseEvent* event)
{
    QLabel::mouseReleaseEvent(event);

    if (event->button() == Qt::LeftButton)
    {
        emit leftClicked();
        emit activated();
        event->accept();
    }
}

void DClickLabel::keyPressEvent(QKeyEvent* e)
{
    switch (e->key())
    {
        case Qt::Key_Down:
        case Qt::Key_Right:
        case Qt::Key_Space:
            emit activated();
            return;
        default:
            break;
    }

    QLabel::keyPressEvent(e);
}

// ------------------------------------------------------------------------

DSqueezedClickLabel::DSqueezedClickLabel(QWidget* const parent)
    : DAdjustableLabel(parent)
{
    setCursor(Qt::PointingHandCursor);
}

DSqueezedClickLabel::DSqueezedClickLabel(const QString& text, QWidget* const parent)
    : DAdjustableLabel(parent)
{
    setAdjustedText(text);
    setCursor(Qt::PointingHandCursor);
}

DSqueezedClickLabel::~DSqueezedClickLabel()
{
}

void DSqueezedClickLabel::mouseReleaseEvent(QMouseEvent* event)
{
    QLabel::mouseReleaseEvent(event);

    if (event->button() == Qt::LeftButton)
    {
        emit leftClicked();
        emit activated();
        event->accept();
    }
}

void DSqueezedClickLabel::mousePressEvent(QMouseEvent* event)
{
    QLabel::mousePressEvent(event);

    /*
     * In some contexts, like QGraphicsView, there will be no
     * release event if the press event was not accepted.
     */
    if (event->button() == Qt::LeftButton)
    {
        event->accept();
    }
}

void DSqueezedClickLabel::keyPressEvent(QKeyEvent* e)
{
    switch (e->key())
    {
        case Qt::Key_Down:
        case Qt::Key_Right:
        case Qt::Key_Space:
            emit activated();
            return;
        default:
            break;
    }

    QLabel::keyPressEvent(e);
}

// ------------------------------------------------------------------------

DArrowClickLabel::DArrowClickLabel(QWidget* const parent)
    : QWidget(parent),
      m_arrowType(Qt::DownArrow)
{
    setCursor(Qt::PointingHandCursor);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_size   = 8;
    m_margin = 2;
}

void DArrowClickLabel::setArrowType(Qt::ArrowType type)
{
    m_arrowType = type;
    update();
}

DArrowClickLabel::~DArrowClickLabel()
{
}

Qt::ArrowType DArrowClickLabel::arrowType() const
{
    return m_arrowType;
}

void DArrowClickLabel::mousePressEvent(QMouseEvent* event)
{
    /*
     * In some contexts, like QGraphicsView, there will be no
     * release event if the press event was not accepted.
     */
    if (event->button() == Qt::LeftButton)
    {
        event->accept();
    }
}

void DArrowClickLabel::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        emit leftClicked();
    }
}

void DArrowClickLabel::paintEvent(QPaintEvent*)
{
    // Inspired by karrowbutton.cpp,
    //  Copyright (C) 2001 Frerich Raabe <raabe@kde.org>

    QPainter p(this);

    QStyleOptionFrame opt;
    opt.init(this);
    opt.lineWidth    = 2;
    opt.midLineWidth = 0;

    /*
    p.fillRect( rect(), palette().brush( QPalette::Background ) );
    style()->drawPrimitive( QStyle::PE_Frame, &opt, &p, this);
    */

    if (m_arrowType == Qt::NoArrow)
        return;

    if (width() < m_size + m_margin || height() < m_size + m_margin)
        return; // don't draw arrows if we are too small

    unsigned int x = 0, y = 0;
    if (m_arrowType == Qt::DownArrow)
    {
        x = (width() - m_size) / 2;
        y = height() - (m_size + m_margin);
    }
    else if (m_arrowType == Qt::UpArrow)
    {
        x = (width() - m_size) / 2;
        y = m_margin;
    }
    else if (m_arrowType == Qt::RightArrow)
    {
        x = width() - (m_size + m_margin);
        y = (height() - m_size) / 2;
    }
    else // arrowType == LeftArrow
    {
        x = m_margin;
        y = (height() - m_size) / 2;
    }

/*
    if (isDown())
    {
        ++x;
        ++y;
    }
*/

    QStyle::PrimitiveElement e = QStyle::PE_IndicatorArrowLeft;

    switch (m_arrowType)
    {
        case Qt::LeftArrow:
            e = QStyle::PE_IndicatorArrowLeft;
            break;
        case Qt::RightArrow:
            e = QStyle::PE_IndicatorArrowRight;
            break;
        case Qt::UpArrow:
            e = QStyle::PE_IndicatorArrowUp;
            break;
        case Qt::DownArrow:
            e = QStyle::PE_IndicatorArrowDown;
            break;
        case Qt::NoArrow:
            break;
    }

    opt.state |= QStyle::State_Enabled;
    opt.rect   = QRect( x, y, m_size, m_size);

    style()->drawPrimitive( e, &opt, &p, this );
}

QSize DArrowClickLabel::sizeHint() const
{
    return QSize(m_size + 2*m_margin, m_size + 2*m_margin);
}

// ------------------------------------------------------------------------

class Q_DECL_HIDDEN DLabelExpander::Private
{

public:

    Private()
    {
        clickLabel      = 0;
        containerWidget = 0;
        pixmapLabel     = 0;
        grid            = 0;
        arrow           = 0;
        line            = 0;
        hbox            = 0;
        checkBox        = 0;
        expandByDefault = true;
    }

    bool              expandByDefault;

    QCheckBox*        checkBox;
    QLabel*           pixmapLabel;
    QWidget*          containerWidget;
    QGridLayout*      grid;

    DLineWidget*      line;
    QWidget*          hbox;

    DArrowClickLabel* arrow;
    DClickLabel*      clickLabel;
};

DLabelExpander::DLabelExpander(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    d->grid        = new QGridLayout(this);
    d->line        = new DLineWidget(Qt::Horizontal, this);
    d->hbox        = new QWidget(this);
    d->arrow       = new DArrowClickLabel(d->hbox);
    d->checkBox    = new QCheckBox(d->hbox);
    d->pixmapLabel = new QLabel(d->hbox);
    d->clickLabel  = new DClickLabel(d->hbox);

    QHBoxLayout* const hlay = new QHBoxLayout(d->hbox);
    hlay->addWidget(d->arrow);
    hlay->addWidget(d->checkBox);
    hlay->addWidget(d->pixmapLabel);
    hlay->addWidget(d->clickLabel, 10);
    hlay->setContentsMargins(QMargins());
    hlay->setSpacing(spacing);

    d->pixmapLabel->installEventFilter(this);
    d->pixmapLabel->setCursor(Qt::PointingHandCursor);

    d->hbox->setCursor(Qt::PointingHandCursor);
    setCheckBoxVisible(false);

    d->grid->addWidget(d->line, 0, 0, 1, 3);
    d->grid->addWidget(d->hbox, 1, 0, 1, 3);
    d->grid->setColumnStretch(2, 10);
    d->grid->setContentsMargins(spacing, spacing, spacing, spacing);
    d->grid->setSpacing(spacing);

    connect(d->arrow, &DArrowClickLabel::leftClicked,
            this, &DLabelExpander::slotToggleContainer);

    connect(d->clickLabel, &DClickLabel::activated,
            this, &DLabelExpander::slotToggleContainer);

    connect(d->checkBox, &QCheckBox::toggled,
            this, &DLabelExpander::signalToggled);
}

DLabelExpander::~DLabelExpander()
{
    delete d;
}

void DLabelExpander::setCheckBoxVisible(bool b)
{
    d->checkBox->setVisible(b);
}

bool DLabelExpander::checkBoxIsVisible() const
{
    return d->checkBox->isVisible();
}

void DLabelExpander::setChecked(bool b)
{
    d->checkBox->setChecked(b);
}

bool DLabelExpander::isChecked() const
{
    return d->checkBox->isChecked();
}

void DLabelExpander::setLineVisible(bool b)
{
    d->line->setVisible(b);
}

bool DLabelExpander::lineIsVisible() const
{
    return d->line->isVisible();
}

void DLabelExpander::setText(const QString& txt)
{
    d->clickLabel->setText(QString::fromUtf8("<qt><b>%1</b></qt>").arg(txt));
}

QString DLabelExpander::text() const
{
    return d->clickLabel->text();
}

void DLabelExpander::setIcon(const QIcon& icon)
{
    d->pixmapLabel->setPixmap(icon.pixmap(style()->pixelMetric(QStyle::PM_SmallIconSize)));
}

QIcon DLabelExpander::icon() const
{
    return QIcon(*d->pixmapLabel->pixmap());
}

void DLabelExpander::setWidget(QWidget* const widget)
{
    if (widget)
    {
        d->containerWidget = widget;
        d->containerWidget->setParent(this);
        d->grid->addWidget(d->containerWidget, 2, 0, 1, 3);
    }
}

QWidget* DLabelExpander::widget() const
{
    return d->containerWidget;
}

void DLabelExpander::setExpandByDefault(bool b)
{
    d->expandByDefault = b;
}

bool DLabelExpander::isExpandByDefault() const
{
    return d->expandByDefault;
}

void DLabelExpander::setExpanded(bool b)
{
    if (d->containerWidget)
    {
        d->containerWidget->setVisible(b);

        if (b)
           d->arrow->setArrowType(Qt::DownArrow);
        else
           d->arrow->setArrowType(Qt::RightArrow);
    }

    emit signalExpanded(b);
}

bool DLabelExpander::isExpanded() const
{
    return (d->arrow->arrowType() == Qt::DownArrow);
}

void DLabelExpander::slotToggleContainer()
{
    if (d->containerWidget)
        setExpanded(!d->containerWidget->isVisible());
}

bool DLabelExpander::eventFilter(QObject* obj, QEvent* ev)
{
    if ( obj == d->pixmapLabel)
    {
        if ( ev->type() == QEvent::MouseButtonRelease)
        {
            slotToggleContainer();
            return false;
        }
        else
        {
            return false;
        }
    }
    else
    {
        // pass the event on to the parent class
        return QWidget::eventFilter(obj, ev);
    }
}

// ------------------------------------------------------------------------

class Q_DECL_HIDDEN DExpanderBox::Private
{
public:

    Private(DExpanderBox* const box)
    {
        parent = box;
        vbox   = 0;
    }

    void createItem(int index, QWidget* const w, const QIcon& icon, const QString& txt,
                    const QString& objName, bool expandBydefault)
    {
        DLabelExpander* const exp = new DLabelExpander(parent->viewport());
        exp->setText(txt);
        exp->setIcon(icon.pixmap(QApplication::style()->pixelMetric(QStyle::PM_SmallIconSize)));
        exp->setWidget(w);
        exp->setLineVisible(!wList.isEmpty());
        exp->setObjectName(objName);
        exp->setExpandByDefault(expandBydefault);

        if (index >= 0)
        {
            vbox->insertWidget(index, exp);
            wList.insert(index, exp);
        }
        else
        {
            vbox->addWidget(exp);
            wList.append(exp);
        }

        parent->connect(exp, SIGNAL(signalExpanded(bool)),
                        parent, SLOT(slotItemExpanded(bool)));

        parent->connect(exp, SIGNAL(signalToggled(bool)),
                        parent, SLOT(slotItemToggled(bool)));
    }

public:

    QList<DLabelExpander*> wList;

    QVBoxLayout*           vbox;

    DExpanderBox*          parent;
};

DExpanderBox::DExpanderBox(QWidget* const parent)
    : QScrollArea(parent),
      d(new Private(this))
{
    setFrameStyle(QFrame::NoFrame);
    setWidgetResizable(true);
    QWidget* const main = new QWidget(viewport());
    d->vbox             = new QVBoxLayout(main);
    d->vbox->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    d->vbox->setContentsMargins(QMargins());
    setWidget(main);

    setAutoFillBackground(false);
    viewport()->setAutoFillBackground(false);
    main->setAutoFillBackground(false);
}

DExpanderBox::~DExpanderBox()
{
    d->wList.clear();
    delete d;
}

void DExpanderBox::setCheckBoxVisible(int index, bool b)
{
    if (index > d->wList.count() || index < 0) return;
    d->wList[index]->setCheckBoxVisible(b);
}

bool DExpanderBox::checkBoxIsVisible(int index) const
{
    if (index > d->wList.count() || index < 0) return false;
    return d->wList[index]->checkBoxIsVisible();
}

void DExpanderBox::setChecked(int index, bool b)
{
    if (index > d->wList.count() || index < 0) return;
    d->wList[index]->setChecked(b);
}

bool DExpanderBox::isChecked(int index) const
{
    if (index > d->wList.count() || index < 0) return false;
    return d->wList[index]->isChecked();
}

void DExpanderBox::addItem(QWidget* const w, const QIcon& icon, const QString& txt,
                           const QString& objName, bool expandBydefault)
{
    d->createItem(-1, w, icon, txt, objName, expandBydefault);
}

void DExpanderBox::addItem(QWidget* const w, const QString& txt,
                           const QString& objName, bool expandBydefault)
{
    addItem(w, QIcon(), txt, objName, expandBydefault);
}

void DExpanderBox::addStretch()
{
    d->vbox->addStretch(10);
}

void DExpanderBox::insertItem(int index, QWidget* const w, const QIcon& icon, const QString& txt,
                              const QString& objName, bool expandBydefault)
{
    d->createItem(index, w, icon, txt, objName, expandBydefault);
}

void DExpanderBox::slotItemExpanded(bool b)
{
    DLabelExpander* const exp = dynamic_cast<DLabelExpander*>(sender());

    if (exp)
    {
        int index = indexOf(exp);
        emit signalItemExpanded(index, b);
    }
}

void DExpanderBox::slotItemToggled(bool b)
{
    DLabelExpander* const exp = dynamic_cast<DLabelExpander*>(sender());

    if (exp)
    {
        int index = indexOf(exp);
        emit signalItemToggled(index, b);
    }
}

void DExpanderBox::insertItem(int index, QWidget* const w, const QString& txt,
                              const QString& objName, bool expandBydefault)
{
    insertItem(index, w, QIcon(), txt, objName, expandBydefault);
}

void DExpanderBox::insertStretch(int index)
{
    d->vbox->insertStretch(index, 10);
}

void DExpanderBox::removeItem(int index)
{
    if (index > d->wList.count() || index < 0) return;
    d->wList[index]->hide();
    d->wList.removeAt(index);
}

void DExpanderBox::setItemText(int index, const QString& txt)
{
    if (index > d->wList.count() || index < 0) return;
    d->wList[index]->setText(txt);
}

QString DExpanderBox::itemText(int index) const
{
    if (index > d->wList.count() || index < 0) return QString();
    return d->wList[index]->text();
}

void DExpanderBox::setItemIcon(int index, const QIcon& icon)
{
    if (index > d->wList.count() || index < 0) return;
    d->wList[index]->setIcon(icon.pixmap(style()->pixelMetric(QStyle::PM_SmallIconSize)));
}

QIcon DExpanderBox::itemIcon(int index) const
{
    if (index > d->wList.count() || index < 0) return QIcon();
    return d->wList[index]->icon();
}

int DExpanderBox::count() const
{
    return d->wList.count();
}

void DExpanderBox::setItemToolTip(int index, const QString& tip)
{
    if (index > d->wList.count() || index < 0) return;
    d->wList[index]->setToolTip(tip);
}

QString DExpanderBox::itemToolTip(int index) const
{
    if (index > d->wList.count() || index < 0) return QString();
    return d->wList[index]->toolTip();
}

void DExpanderBox::setItemEnabled(int index, bool enabled)
{
    if (index > d->wList.count() || index < 0) return;
    d->wList[index]->setEnabled(enabled);
}

bool DExpanderBox::isItemEnabled(int index) const
{
    if (index > d->wList.count() || index < 0) return false;
    return d->wList[index]->isEnabled();
}

DLabelExpander* DExpanderBox::widget(int index) const
{
    if (index > d->wList.count() || index < 0) return 0;
    return d->wList[index];
}

int DExpanderBox::indexOf(DLabelExpander* const widget) const
{
    for (int i = 0 ; i < count(); ++i)
    {
        DLabelExpander* const exp = d->wList[i];

        if (widget == exp)
            return i;
    }
    return -1;
}

void DExpanderBox::setItemExpanded(int index, bool b)
{
    if (index > d->wList.count() || index < 0) return;

    DLabelExpander* const exp = d->wList[index];

    if (!exp) return;

    exp->setExpanded(b);
}

bool DExpanderBox::isItemExpanded(int index) const
{
    if (index > d->wList.count() || index < 0) return false;

    DLabelExpander* const exp = d->wList[index];

    if (!exp)
        return false;

    return (exp->isExpanded());
}

void DExpanderBox::readSettings(KConfigGroup& group)
{
    for (int i = 0 ; i < count(); ++i)
    {
        DLabelExpander* const exp = d->wList[i];

        if (exp)
        {
            exp->setExpanded(group.readEntry(QString::fromUtf8("%1 Expanded").arg(exp->objectName()),
                                             exp->isExpandByDefault()));
        }
    }
}

void DExpanderBox::writeSettings(KConfigGroup& group)
{
    for (int i = 0 ; i < count(); ++i)
    {
        DLabelExpander* const exp = d->wList[i];

        if (exp)
        {
            group.writeEntry(QString::fromUtf8("%1 Expanded").arg(exp->objectName()),
                             exp->isExpanded());
        }
    }
}

// ------------------------------------------------------------------------

DExpanderBoxExclusive::DExpanderBoxExclusive(QWidget* const parent)
    : DExpanderBox(parent)
{
    setIsToolBox(true);
}

DExpanderBoxExclusive::~DExpanderBoxExclusive()
{
}

void DExpanderBoxExclusive::slotItemExpanded(bool b)
{
    DLabelExpander* const exp = dynamic_cast<DLabelExpander*>(sender());

    if (!exp)
        return;

    if (isToolBox() && b)
    {
        int item = 0;

        while (item < count())
        {
            if (isItemExpanded(item) && item != indexOf(exp))
            {
                setItemExpanded(item, false);
            }

            item++;
        }
    }
    emit signalItemExpanded(indexOf(exp), b);
}

void DExpanderBoxExclusive::setIsToolBox(bool b)
{
    m_toolbox = b;
}

bool DExpanderBoxExclusive::isToolBox() const
{
    return (m_toolbox);
}

} // namespace Digikam
