/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-03-14
 * Description : A widget to host settings as expander box
 *
 * Copyright (C) 2008-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "rexpanderbox.h"
#include "rexpanderbox.moc"

// Qt includes

#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QCursor>
#include <QStyle>
#include <QStyleOption>
#include <QGridLayout>

// KDE includes

#include <kvbox.h>
#include <kseparator.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <kdialog.h>
#include <klocale.h>

namespace Digikam
{

RClickLabel::RClickLabel(QWidget *parent)
           : QLabel(parent)
{
    setCursor(Qt::PointingHandCursor);
}

RClickLabel::RClickLabel(const QString& text, QWidget *parent)
           : QLabel(text, parent)
{
    setCursor(Qt::PointingHandCursor);
}

void RClickLabel::mouseReleaseEvent(QMouseEvent* event)
{
    QLabel::mouseReleaseEvent(event);

    if (event->button() == Qt::LeftButton)
    {
        emit leftClicked();
        emit activated();
        event->accept();
    }
}

void RClickLabel::keyPressEvent(QKeyEvent *e)
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

RSqueezedClickLabel::RSqueezedClickLabel(QWidget *parent)
                   : KSqueezedTextLabel(parent)
{
    setCursor(Qt::PointingHandCursor);
}

RSqueezedClickLabel::RSqueezedClickLabel(const QString& text, QWidget *parent)
                   : KSqueezedTextLabel(text, parent)
{
    setCursor(Qt::PointingHandCursor);
}

void RSqueezedClickLabel::mouseReleaseEvent(QMouseEvent* event)
{
    KSqueezedTextLabel::mouseReleaseEvent(event);

    if (event->button() == Qt::LeftButton)
    {
        emit leftClicked();
        emit activated();
        event->accept();
    }
}

void RSqueezedClickLabel::keyPressEvent(QKeyEvent *e)
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

RArrowClickLabel::RArrowClickLabel(QWidget *parent)
                : QWidget(parent), m_arrowType(Qt::DownArrow)
{
    setCursor(Qt::PointingHandCursor);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_size   = 8;
    m_margin = 2;
}

void RArrowClickLabel::setArrowType(Qt::ArrowType type)
{
    m_arrowType = type;
    update();
}

void RArrowClickLabel::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        emit leftClicked();
    }
}

void RArrowClickLabel::paintEvent(QPaintEvent*)
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

    if (width() < m_size + m_margin ||
        height() < m_size + m_margin)
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

QSize RArrowClickLabel::sizeHint() const
{
    return QSize(m_size + 2*m_margin, m_size + 2*m_margin);
}

// ------------------------------------------------------------------------

class RLabelExpanderPriv
{

public:

    RLabelExpanderPriv()
    {
        clickLabel      = 0;
        containerWidget = 0;
        pixmapLabel     = 0;
        grid            = 0;
        arrow           = 0;
        line            = 0;
        expandByDefault = true;
    }

    bool             expandByDefault;

    QLabel          *pixmapLabel;
    QWidget         *containerWidget;
    QGridLayout     *grid;

    KSeparator      *line;

    RArrowClickLabel *arrow;
    RClickLabel      *clickLabel;
};

RLabelExpander::RLabelExpander(QWidget *parent)
              : QWidget(parent), d(new RLabelExpanderPriv)
{
    d->grid        = new QGridLayout(this);
    d->line        = new KSeparator(Qt::Horizontal, this);
    d->arrow       = new RArrowClickLabel(this);
    d->pixmapLabel = new QLabel(this);
    d->clickLabel  = new RClickLabel(this);
    d->pixmapLabel->installEventFilter(this);
    d->pixmapLabel->setCursor(Qt::PointingHandCursor);

    d->grid->addWidget(d->line,        0, 0, 1, 3);
    d->grid->addWidget(d->arrow,       1, 0, 1, 1);
    d->grid->addWidget(d->pixmapLabel, 1, 1, 1, 1);
    d->grid->addWidget(d->clickLabel,  1, 2, 1, 1);
    d->grid->setColumnStretch(2, 10);
    d->grid->setMargin(KDialog::spacingHint());
    d->grid->setSpacing(KDialog::spacingHint());

    connect(d->arrow, SIGNAL(leftClicked()),
            this, SLOT(slotToggleContainer()));

    connect(d->clickLabel, SIGNAL(activated()),
            this, SLOT(slotToggleContainer()));
}

RLabelExpander::~RLabelExpander()
{
    delete d;
}

void RLabelExpander::setLineVisible(bool b)
{
    d->line->setVisible(b);
}

void RLabelExpander::setText(const QString& text)
{
    d->clickLabel->setText(QString("<qt><b>%1</b></qt>").arg(text));
}

void RLabelExpander::setPixmap(const QPixmap& pix)
{
    d->pixmapLabel->setPixmap(pix);
}

void RLabelExpander::setContainer(QWidget* widget)
{
    if (widget)
    {
        d->containerWidget = widget;
        d->containerWidget->setParent(this);
        d->grid->addWidget(d->containerWidget, 2, 0, 1, 3);
    }
}

void RLabelExpander::setExpandByDefault(bool b)
{
    d->expandByDefault = b;
}

bool RLabelExpander::expandByDefault()
{
    return d->expandByDefault;
}

void RLabelExpander::setExpanded(bool b)
{
    if (d->containerWidget)
    {
        d->containerWidget->setVisible(b);
        if (b)
           d->arrow->setArrowType(Qt::DownArrow);
        else
           d->arrow->setArrowType(Qt::RightArrow);
    }
}

bool RLabelExpander::isExpanded()
{
    if (d->containerWidget)
        return (d->containerWidget->isVisible());

    return false;
}

void RLabelExpander::slotToggleContainer()
{
    if (d->containerWidget)
        setExpanded(!d->containerWidget->isVisible());
}

bool RLabelExpander::eventFilter(QObject *obj, QEvent *ev)
{
    if ( obj == d->pixmapLabel )
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

class RExpanderBoxPriv
{

public:

    RExpanderBoxPriv()
    {
        vbox = 0;
    }

    QList<RLabelExpander*>  wList;

    QVBoxLayout            *vbox;
};

RExpanderBox::RExpanderBox(QWidget *parent)
            : QScrollArea(parent), d(new RExpanderBoxPriv)
{
    setFrameStyle(QFrame::NoFrame);
    setWidgetResizable(true);
    viewport()->setAutoFillBackground(false);
    QWidget *main = new QWidget(viewport());
    d->vbox       = new QVBoxLayout(main);
    d->vbox->setMargin(0);
    d->vbox->setSpacing(KDialog::spacingHint());
    setWidget(main);
}

RExpanderBox::~RExpanderBox()
{
    delete d;
}

void RExpanderBox::addItem(QWidget *w, const QPixmap& pix, const QString& txt,
                           const QString& objName, bool expandBydefault)
{
    RLabelExpander *exp = new RLabelExpander(viewport());
    exp->setText(txt);
    exp->setPixmap(pix);
    exp->setContainer(w);
    exp->setLineVisible(!d->wList.isEmpty());
    exp->setObjectName(objName);
    exp->setExpandByDefault(expandBydefault);
    d->vbox->addWidget(exp);
    d->wList.append(exp);
}

void RExpanderBox::addItem(QWidget *w, const QString& txt,
                           const QString& objName, bool expandBydefault)
{
    addItem(w, QPixmap(), txt, objName, expandBydefault);
}

void RExpanderBox::addStretch()
{
    d->vbox->addStretch(10);
}

void RExpanderBox::insertItem(int index, QWidget *w, const QPixmap& pix, const QString& txt,
                              const QString& objName, bool expandBydefault)
{
    RLabelExpander *exp = new RLabelExpander(viewport());
    exp->setText(txt);
    exp->setPixmap(pix);
    exp->setContainer(w);
    exp->setLineVisible(!d->wList.isEmpty());
    exp->setObjectName(objName);
    exp->setExpandByDefault(expandBydefault);
    d->vbox->insertWidget(index, exp);
    d->wList.insert(index, exp);
}

void RExpanderBox::insertItem(int index, QWidget *w, const QString& txt,
                              const QString& objName, bool expandBydefault)
{
    insertItem(index, w, QPixmap(), txt, objName, expandBydefault);
}

void RExpanderBox::insertStretch(int index)
{
    d->vbox->insertStretch(index, 10);
}

void RExpanderBox::removeItem(int index)
{
    if (index > d->wList.count() || index < 0) return;
    d->wList[index]->hide();
    d->wList.removeAt(index);
}

void RExpanderBox::setItemIcon(int index, const QPixmap& pix)
{
    if (index > d->wList.count() || index < 0) return;
    d->wList[index]->setPixmap(pix);
}

int RExpanderBox::count()
{
    return d->wList.count();
}

RLabelExpander* RExpanderBox::widget(int index) const
{
    if (index > d->wList.count() || index < 0) return 0;

    return d->wList[index];
}

void RExpanderBox::setItemExpanded(int index, bool b)
{
    if (index > d->wList.count() || index < 0) return;

    RLabelExpander *exp = d->wList[index];
    if (!exp) return;

    exp->setExpanded(b);
}

bool RExpanderBox::itemIsExpanded(int index)
{
    if (index > d->wList.count() || index < 0) return false;

    RLabelExpander *exp = d->wList[index];
    if (!exp) return false;

    return (exp->isExpanded());
}

void RExpanderBox::readSettings(KConfigGroup& group)
{
    for (int i = 0 ; i < count(); ++i)
    {
        RLabelExpander *exp = d->wList[i];
        if (exp)
        {
            exp->setExpanded(group.readEntry(QString("%1 Expanded").arg(exp->objectName()),
                                             exp->expandByDefault()));
        }
    }
}

void RExpanderBox::writeSettings(KConfigGroup& group)
{
    for (int i = 0 ; i < count(); ++i)
    {
        RLabelExpander *exp = d->wList[i];
        if (exp)
        {
            group.writeEntry(QString("%1 Expanded").arg(exp->objectName()),
                             exp->isExpanded());
        }
    }
}

} // namespace Digikam
