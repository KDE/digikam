/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright 2002-2004 by Renchi Raju
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
 * ============================================================ */

#include <qpixmap.h>
#include <qfont.h>
#include <qfontmetrics.h>
#include <qpalette.h>
#include <qpainter.h>
#include <qevent.h>
#include <qtextedit.h>

#include "thumbview.h"
#include "thumbitem.h"

// -- Renaming -----------------------------------------------

class ThumbItemLineEdit : public QTextEdit
{

public:

    ThumbItemLineEdit(const QString& text, QWidget* parent,
                      ThumbItem* item);

private:

    void keyPressEvent(QKeyEvent *e);
    void focusOutEvent(QFocusEvent *e);

    ThumbItem *thumbItem;
    QString    startText;
};

ThumbItemLineEdit::ThumbItemLineEdit(const QString& text,
                                     QWidget* parent,
                                     ThumbItem* item)
    : QTextEdit(parent), thumbItem(item), startText(text)
{
    setFrameStyle(QFrame::Plain | QFrame::Box);
    setLineWidth(1);

    setHScrollBarMode( AlwaysOff );
    setVScrollBarMode( AlwaysOff );

    setWordWrap( WidgetWidth );
    setWrapColumnOrWidth(item->pixmapRect().width());
    resize(200, 200);
    setText(text);
    setAlignment(Qt::AlignCenter);
    resize(wrapColumnOrWidth() + 2,
           heightForWidth( wrapColumnOrWidth()) + 2);

}

void ThumbItemLineEdit::keyPressEvent( QKeyEvent *e )
{
    if ( e->key()  == Key_Escape ) {
	thumbItem->setText(startText );
	thumbItem->cancelRenameItem();
    } else if ( e->key() == Key_Enter ||
		e->key() == Key_Return ) {
	thumbItem->renameItem();
    } else {
	QTextEdit::keyPressEvent( e );
	sync();
	//resize( width(), document()->height() + 2 );
    }
}

void ThumbItemLineEdit::focusOutEvent( QFocusEvent *e )
{
    if ( e->reason() != QFocusEvent::Popup )
	thumbItem->cancelRenameItem();
}

// -----------------------------------------------------------

class ThumbItemPrivate {

public:

    QString text;
    QPixmap* pixmap;

    QRect rect;
    QRect textRect;
    QRect pixmapRect;

    bool selected;
    QString    key;
};

ThumbItem::ThumbItem(ThumbView* parent,
                     const QString& text,
                     const QPixmap& pixmap)
{
    
    view = parent;
    next = 0;
    prev = 0;
    renameBox = 0;

    d = new ThumbItemPrivate;
    d->text = text;
    d->pixmap = new QPixmap(pixmap);
    d->selected = false;
    d->key = d->text;

    // make sure to calcrect before inserting.
    // this will calculate the size of the rect.
    // inserting would move it to the required location
    d->rect       = QRect(0, 0, 0, 0);
    d->textRect   = QRect(0, 0, 0, 0);
    d->pixmapRect = QRect(0, 0, 0, 0);
    calcRect();

    view->insertItem(this);

}

ThumbItem::~ThumbItem()
{
    view->takeItem(this);
    delete d->pixmap;
    delete d;
}

void ThumbItem::calcRect()
{
    QRect rect(d->rect);
    QRect textRect(d->textRect);
    QRect pixmapRect(d->pixmapRect);

    // set initial pixrect
    int pw = d->pixmap->width();
    int ph = d->pixmap->height();

    pixmapRect.setWidth(pw);
    pixmapRect.setHeight(ph);

    // word wrap
    QFontMetrics fm(view->font());
    QRect r = QRect(fm.boundingRect(0, 0, pixmapRect.width(),
                                    0xFFFFFFFF, Qt::AlignHCenter |
                                    Qt::WordBreak | Qt::BreakAnywhere,
                                    d->text));
    r.setWidth(r.width() + 4);

    textRect.setWidth(r.width());
    textRect.setHeight(r.height());


    // Now start updating the rects
    int w = QMAX(textRect.width(), pixmapRect.width());
    int h = textRect.height() + pixmapRect.height() + 1;

    rect.setWidth(w);
    rect.setHeight(h);

    // Center the pix and text rect
    pixmapRect = QRect((rect.width() - pixmapRect.width())/2,
                       0,
                       pixmapRect.width(), pixmapRect.height());
    textRect   = QRect((rect.width() - textRect.width())/2,
                       rect.height() - textRect.height(),
                       textRect.width(), textRect.height());

    // Finally save the settings
    setRect(rect);
    setPixmapRect(pixmapRect);
    setTextRect(textRect);
}

void ThumbItem::paintItem(QPainter *,
                          const QColorGroup& cg)
{

    QRect pRect=pixmapRect(true);
    QRect tRect=textRect(true);

    QPixmap pix(rect().width(), rect().height());
    pix.fill(cg.base());
    QPainter painter(&pix);
    painter.drawPixmap(pRect.x(), pRect.y(), *pixmap() );

    if (isSelected()) {
        QPen pen;
        pen.setColor(cg.highlight());
        painter.setPen(pen);
        painter.drawRect(0, 0, pix.width(), pix.height());
        painter.fillRect(0, tRect.y(), pix.width(),
                     tRect.height(), cg.highlight() );
        painter.setPen( QPen( cg.highlightedText() ) );
    }
    else
        painter.setPen( cg.text() );

    painter.drawText(tRect, Qt::WordBreak|Qt::BreakAnywhere|
                     Qt::AlignHCenter|Qt::AlignTop,text());


    painter.end();

    QRect r(rect());
    r = QRect(view->contentsToViewport(QPoint(r.x(), r.y())),
              QSize(r.width(), r.height()));

    bitBlt(view->viewport(), r.x(), r.y(), &pix,
           0, 0, r.width(), r.height());
    
}

void ThumbItem::repaint()
{
    QRect r(view->contentsRectToViewport(d->rect));
    view->viewport()->repaint(r);
}

QRect ThumbItem::rect() const
{
    return d->rect;
}

QRect ThumbItem::textRect(bool relative) const
{
    if (relative) {
        return d->textRect;
    }
    else {
        QRect r(x() + d->textRect.x(), y() + d->textRect.y(),
                d->textRect.width(), d->textRect.height());
        //QRect r(d->textRect);
        //r.moveBy(d->rect.left(), d->rect.top());
        return r;
    }
}

QRect ThumbItem::pixmapRect(bool relative) const
{
    if (relative) {
        return d->pixmapRect;
    }
    else {
        QRect r(x() + d->pixmapRect.x(), y() + d->pixmapRect.y(),
                d->pixmapRect.width(), d->pixmapRect.height());
        //QRect r(d->pixmapRect);
        //r.moveBy(d->rect.left(), d->rect.top());
        return r;
    }

}

void ThumbItem::setRect(const QRect& rect)
{
   if (rect.isValid()) {
        d->rect = rect;
    }
}

void ThumbItem::setTextRect(const QRect& rect)
{
   if (rect.isValid()) {
        d->textRect = rect;
    }
}

void ThumbItem::setPixmapRect(const QRect& rect)
{
   if (rect.isValid()) {
        d->pixmapRect = rect;
    }
}

QPixmap * ThumbItem::pixmap() const
{
    return d->pixmap;
}

QString ThumbItem::text() const
{
    return d->text;
}

int ThumbItem::x() const
{
    return d->rect.x();
}

int ThumbItem::y() const
{
    return d->rect.y();
}

int ThumbItem::width() const
{
    return d->rect.width();
}

int ThumbItem::height() const
{
    return d->rect.height();
}

bool ThumbItem::move(int _x, int _y)
{
    if (_x == x() && _y == y())
	return false;

    d->rect.setRect(_x, _y, d->rect.width(),
                    d->rect.height());
    return true;
}

void ThumbItem::setSelected(bool val, bool cb)
{
    if (cb) {
        view->blockSignals(true);
        view->clearSelection();
        view->blockSignals(false);
    }
    
    d->selected = val;
    view->selectItem(this, val);
    QRect r(d->rect);
    r = QRect(view->contentsToViewport(QPoint(r.x(), r.y())),
               QSize(r.width(), r.height()));
    view->viewport()->update(r);
}

bool ThumbItem::isSelected()
{
    return d->selected;
}

void ThumbItem::setPixmap(const QPixmap& pixmap)
{
    if (d->pixmap) {
        delete d->pixmap;
        d->pixmap = 0;
    }
    d->pixmap = new QPixmap(pixmap);

    QRect oR(d->rect);
    calcRect();
    oR = oR.unite(d->rect);
    oR = QRect(view->contentsToViewport(QPoint(oR.x(), oR.y())),
               QSize(oR.width(), oR.height()));

    view->updateItemContainer(this);

    if (oR.intersects(QRect(view->contentsX(), view->contentsY(),
                            view->visibleWidth(), view->visibleHeight())))
        view->viewport()->repaint(oR);
}

void ThumbItem::setText(const QString& text)
{
    d->text = text;
    d->key  = text;

    QRect oR(d->rect);
    calcRect();
    oR = oR.unite(d->rect);
    oR = QRect(view->contentsToViewport(QPoint(oR.x(), oR.y())),
               QSize(oR.width(), oR.height()));

    view->updateItemContainer(this);
    
    if (oR.intersects(QRect(view->contentsX(), view->contentsY(),
                            view->visibleWidth(), view->visibleHeight())))
        view->viewport()->repaint(oR);
}

ThumbItem * ThumbItem::nextItem()
{
    return next;
}

ThumbItem * ThumbItem::prevItem()
{
    return prev;
}

ThumbView* ThumbItem::iconView()
{
    return view;    
}

void ThumbItem::rename()
{
    if (renameBox) {
        delete renameBox;
        renameBox = 0;
    }

    renameBox = new ThumbItemLineEdit(d->text, view->viewport(),
                                      this);
    QRect tr(textRect(false));
    view->addChild(renameBox,
                   tr.x() + (tr.width()/2 -
                             renameBox->width()/2 ),
                   tr.y() - 3);
    renameBox->selectAll();
    view->viewport()->setFocusProxy(renameBox);
    renameBox->setFocus();
    renameBox->show();

    view->renamingItem = this;
}

void ThumbItem::renameItem()
{
    if (!renameBox)
	return;
    setText(renameBox->text());

    bool resetFocus = view->viewport()->focusProxy() == renameBox;
    delete renameBox;
    renameBox = 0;
    if (resetFocus) {
	view->viewport()->setFocusProxy(view);
	view->setFocus();
    }

    view->renamingItem = 0;
    view->emitRenamed(this);
}

void ThumbItem::cancelRenameItem()
{
    repaint();

    bool resetFocus = view->viewport()->focusProxy() == renameBox;
    delete renameBox;
    renameBox = 0;
    if (resetFocus) {
	view->viewport()->setFocusProxy(view);
	view->setFocus();
    }

    view->renamingItem = 0;
}


int ThumbItem::compare(ThumbItem *item)
{
    return key().localeAwareCompare(item->key());
}

QString ThumbItem::key() const
{
    return d->key;    
}

