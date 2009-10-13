/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-10-12
 * Description : A button which appears on the side of a splitter handle 
 *               and allows easy collapsing of the widget on the opposite side.
 *
 * Copyright (C) 2009 by Gilles Caulier<caulier dot gilles at gmail dot com>
 * Copyright (c) 2009 Aurélien Gateau <agateau@kde.org>
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

#include "dsplitterbutton.h"
#include "dsplitterbutton.moc"

// Qt includes.

#include <QApplication>
#include <QEvent>
#include <QMouseEvent>
#include <QSplitter>
#include <QStyleOptionToolButton>
#include <QStylePainter>
#include <QTimeLine>

// KDE includes.

#include <kdebug.h>

namespace Digikam
{

enum Direction
{
   LTR      = 1 << 0,
   RTL      = 1 << 1,
   Vertical = 1 << 2,
   TTB      = Vertical + (1 << 0),
   BTT      = Vertical + (1 << 1)
};

const int TIMELINE_DURATION = 500;

const qreal MINIMUM_OPACITY = 0.3;

struct ArrowTypes
{
   ArrowTypes() {}

   ArrowTypes(Qt::ArrowType t1, Qt::ArrowType t2)
      : visible(t1), notVisible(t2) {}

   Qt::ArrowType visible, notVisible;

   Qt::ArrowType get(bool isVisible)
   {
      return isVisible ? visible : notVisible;
   }
};

struct DSplitterButtonPrivate
{
   DSplitterButton* q;
   QSplitter*         mSplitter;
   QWidget*           mWidget;
   Direction          mDirection;
   QTimeLine*         mOpacityTimeLine;
   int                mSizeAtCollaps;
   
   bool isVertical() const
   {
      return mDirection & Vertical;
   }
   
   bool isVisible() const
   {
      bool isVisible   = mWidget->isVisible();
      QRect widgetRect = mWidget->geometry();

      if (isVisible)
      {
         QPoint br = widgetRect.bottomRight();
         if ((br.x() <= 0) || (br.y() <=0))
         {
            isVisible = false;
         }
      }
      return isVisible;
   }
   
   void updatePosition()
   {
      int x, y;
      QRect widgetRect  = mWidget->geometry();
      int splitterWidth = mSplitter->width();
      int handleWidth   = mSplitter->handleWidth();
      int width         = q->width();
      
      if (!isVertical())
      {
         // FIXME: Make this configurable
         y = 30;
         if (mDirection == LTR)
         {
            if (isVisible())
            {
               x = widgetRect.right() + handleWidth;
            }
            else
            {
               x = 0;
            }
         }
         else
         { // RTL
            if (isVisible())
            {
               x = widgetRect.left() - handleWidth - width;
            }
            else
            {
               x = splitterWidth - handleWidth - width;
            }
         }
      }
      else
      {
         // FIXME
         x = 0;
         y = 0;
      }
      q->move(x, y);
   }

   void updateArrow()
   {
      static QMap<Direction, ArrowTypes> arrowForDirection;
      if (arrowForDirection.isEmpty())
      {
         arrowForDirection[LTR] = ArrowTypes(Qt::LeftArrow,  Qt::RightArrow);
         arrowForDirection[RTL] = ArrowTypes(Qt::RightArrow, Qt::LeftArrow);
         arrowForDirection[TTB] = ArrowTypes(Qt::UpArrow,    Qt::DownArrow);
         arrowForDirection[BTT] = ArrowTypes(Qt::DownArrow,  Qt::UpArrow);
      }
      q->setArrowType(arrowForDirection[mDirection].get(isVisible()));
   }

   void widgetEventFilter(QEvent* event)
   {
      switch (event->type()) 
      {
         case QEvent::Resize:
            updatePosition();
            updateOpacity();
            break;
            
         case QEvent::Move:
         case QEvent::Show:
         case QEvent::Hide:
            updatePosition();
            updateOpacity();
            updateArrow();
            break;
            
         default:
            break;
      }
   }

   void updateOpacity()
   {
      QPoint pos           = q->parentWidget()->mapFromGlobal(QCursor::pos());
      QRect opaqueRect     = q->geometry();
      bool opaqueCollapser = opaqueRect.contains(pos);
      int frame            = mOpacityTimeLine->currentFrame();
      
      if (opaqueCollapser && frame == mOpacityTimeLine->startFrame())
      {
         mOpacityTimeLine->setDirection(QTimeLine::Forward);
         startTimeLine();
      }
      else if (!opaqueCollapser && frame == mOpacityTimeLine->endFrame())
      {
         mOpacityTimeLine->setDirection(QTimeLine::Backward);
         startTimeLine();
      }
   }

   void startTimeLine()
   {
      if (mOpacityTimeLine->state() != QTimeLine::Running)
      {
         mOpacityTimeLine->start();
      }
   }
};

// --------------------------------------------------------------------------------------

DSplitterButton::DSplitterButton(QSplitter* splitter, QWidget* widget)
                 : QToolButton(), d(new DSplitterButtonPrivate)
{
   d->q = this;

   // We do not want our collapser to be added as a regular widget in the
   // splitter!
   setAttribute(Qt::WA_NoChildEventsForParent);

   d->mOpacityTimeLine = new QTimeLine(TIMELINE_DURATION, this);
   d->mOpacityTimeLine->setFrameRange(int(MINIMUM_OPACITY * 1000), 1000);
   
   connect(d->mOpacityTimeLine, SIGNAL(valueChanged(qreal)), 
           this, SLOT(update()));

   d->mWidget = widget;
   d->mWidget->installEventFilter(this);

   qApp->installEventFilter(this);

   d->mSplitter = splitter;
   setParent(d->mSplitter);

   int index = splitter->indexOf(widget);
   if (index < 1)
   {
      d->mDirection = LTR;
      kDebug() << "index: " << index << "count: " << splitter->count() << "LTR";
   }
   else
   {
      d->mDirection = RTL;
      kDebug() << "index: " << index << "count: " << splitter->count() << "RTL";
   }

   if (splitter->orientation() == Qt::Vertical)
   {
      // FIXME: Ugly!
      d->mDirection = static_cast<Direction>(int(d->mDirection) + int(TTB));
   }

   connect(this, SIGNAL(clicked()), 
           this, SLOT(slotClicked()));

   show();
}

DSplitterButton::~DSplitterButton()
{
   delete d;
}

bool DSplitterButton::eventFilter(QObject* object, QEvent* event)
{
   if (object == d->mWidget)
   {
      d->widgetEventFilter(event);
   }
   else
   {
      /* app */
      if (event->type() == QEvent::MouseMove)
      {
         d->updateOpacity();
      }
   }
   return false;
}

QSize DSplitterButton::sizeHint() const
{
   int extent = style()->pixelMetric(QStyle::PM_ScrollBarExtent);
   QSize sh(extent * 3 / 4, extent * 240 / 100);
   if (d->isVertical())
   {
      sh.transpose();
   }
   return sh;
}


void DSplitterButton::slotClicked()
{
   QList<int> sizes = d->mSplitter->sizes();
   int index        = d->mSplitter->indexOf(d->mWidget);
   if (d->isVisible())
   {
      d->mSizeAtCollaps = sizes[index];
      sizes[index]      = 0;
   }
   else
   {
      if (d->mSizeAtCollaps != 0)
      {
         sizes[index] = d->mSizeAtCollaps;
      }
      else
      {
         if (d->isVertical())
         {
            sizes[index] = d->mWidget->sizeHint().height();
         }
         else
         {
            sizes[index] = d->mWidget->sizeHint().width();
         }
      }
   }
   d->mSplitter->setSizes(sizes);
}

void DSplitterButton::paintEvent(QPaintEvent*)
{
   QStylePainter painter(this);
   qreal opacity = d->mOpacityTimeLine->currentFrame() / 1000.;
   painter.setOpacity(opacity);

   QStyleOptionToolButton opt;
   initStyleOption(&opt);
   if (d->mDirection == LTR)
   {
      opt.rect.setLeft(-width());
   }
   else
   {
      opt.rect.setWidth(width() * 2);
   }
   painter.drawPrimitive(QStyle::PE_PanelButtonTool, opt);

   QStyleOptionToolButton opt2;
   initStyleOption(&opt2);
   painter.drawControl(QStyle::CE_ToolButtonLabel, opt2);
}

} // namespace Digikam
