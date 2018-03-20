/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-05-19
 * Description : a widget to draw sketch.
 *
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef SKETCH_WIDGET_H
#define SKETCH_WIDGET_H

// Qt includes

#include <QWidget>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

namespace Digikam
{

class SketchWidget : public QWidget
{
    Q_OBJECT

public:

    explicit SketchWidget(QWidget* const parent = 0);
    virtual ~SketchWidget();

    QColor  penColor() const;
    int     penWidth() const;
    bool    isClear()  const;

    QImage  sketchImage() const;
    void    setSketchImage(const QImage& image);

    /** This method return the drawing line history
     *  as XML, to be stored in database as SAlbum data.
     */
    void sketchImageToXML(QXmlStreamWriter& writer);
    QString sketchImageToXML();

    /** This method set sketch image using XML data based
     *  on drawing line history.
     *  Return true if data are imported successfully.
     */
    bool setSketchImageFromXML(QXmlStreamReader& reader);
    bool setSketchImageFromXML(const QString& xml);

Q_SIGNALS:

    void signalSketchChanged(const QImage&);
    void signalPenSizeChanged(int);
    void signalPenColorChanged(const QColor&);
    void signalUndoRedoStateChanged(bool hasUndo, bool hasRedo);

public Q_SLOTS:

    void setPenColor(const QColor& newColor);
    void setPenWidth(int newWidth);
    void slotClear();
    void slotUndo();
    void slotRedo();

protected:

    void mousePressEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void wheelEvent(QWheelEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void keyPressEvent(QKeyEvent*);
    void keyReleaseEvent(QKeyEvent*);
    void paintEvent(QPaintEvent*);

private:

    void updateDrawCursor();
    void replayEvents(int index);
    void drawLineTo(const QPoint& endPoint);
    void drawLineTo(int width, const QColor& color, const QPoint& start, const QPoint& end);
    void drawPath(int width, const QColor& color, const QPainterPath& path);
    void addPath(QXmlStreamReader& reader);
    //QDomElement addXmlTextElement(QDomDocument& document, QDomElement& target,
    //                              const QString& tag, const QString& text);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // SKETCH_WIDGET_H
