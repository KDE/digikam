/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-05-19
 * Description : a widget to draw sketch.
 *
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SKETCHWIDGET_H
#define SKETCHWIDGET_H

// Qt includes.

#include <QWidget>
#include <QDomDocument>
#include <QDomElement>

namespace Digikam
{

class SketchWidgetPriv;

class SketchWidget : public QWidget
{
    Q_OBJECT

public:

    SketchWidget(QWidget *parent=0);
    ~SketchWidget();

    QColor  penColor() const;
    int     penWidth() const;
    bool    isClear()  const;

    QImage  sketchImage() const;
    void    setSketchImage(const QImage& image);

    /** This method return the drawing line history
     *  as XML, to be stored in database as SAlbum data.
     */
    QString sketchImageToXML();

    /** This method set sketch image using XML data based 
     *  on drawing line history.
     *  Retrun true if data are imported sucessfully.
     */
    bool setSketchImageFromXML(const QString& xml);

signals:

    void signalSketchChanged(const QImage&);

public slots:

    void setPenColor(const QColor& newColor);
    void setPenWidth(int newWidth);
    void slotClear();
    void slotUndo();
    void slotRedo();

protected:

    void mousePressEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void paintEvent(QPaintEvent*);

private:

    void replayEvents(int index);
    void drawLineTo(const QPoint& endPoint);
    void drawLineTo(int width, const QColor& color, const QPoint& start, const QPoint& end);
    QDomElement addXmlTextElement(QDomDocument &document, QDomElement &target,
                                  const QString& tag, const QString& text);

private:

    SketchWidgetPriv *d;
};

}  // namespace Digikam

#endif // SKETCHWIDGET_H
