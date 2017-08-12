/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-09-01
 * Description : a plugin to create photo layouts by fusion of several images.
 * 
 *
 * Copyright (C) 2011      by Lukasz Spas <lukasz dot spas at gmail dot com>
 * Copyright (C) 2009-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "ZoomTool.h"
#include "MousePressListener.h"

#include <QButtonGroup>
#include <QRadioButton>

using namespace PhotoLayoutsEditor;

class ZoomTool::ZoomToolPrivate
{
    ZoomToolPrivate() :
        out(0),
        in(0),
        listener(0)
    {}

    QRadioButton * out;
    QRadioButton * in;
    MousePressListener * listener;
    friend class ZoomTool;
};

ZoomTool::ZoomTool(Scene * scene, QWidget * parent) :
    AbstractTool(scene, Canvas::Viewing, parent),
    d(new ZoomToolPrivate)
{
    QVBoxLayout * layout = new QVBoxLayout();
    this->setLayout(layout);

    QButtonGroup * group = new QButtonGroup(this);
    d->out = new QRadioButton(i18n("Zoom out"), this);
    group->addButton(d->out);
    layout->addWidget(d->out);
    d->in  = new QRadioButton(i18n("Zoom in"), this);
    group->addButton(d->in);
    layout->addWidget(d->in);

    layout->addSpacerItem(new QSpacerItem(10,10));
    layout->setStretch(2,1);

    d->listener = new MousePressListener(this);
    connect(d->listener, SIGNAL(mouseReleased(QPointF)), this, SLOT(zoom(QPointF)));

    d->in->setChecked(true);
}

ZoomTool::~ZoomTool()
{
    Scene * scene = this->scene();
    if (!scene)
        return;
    scene->readSceneMousePress( 0 );
    delete d;
}

void ZoomTool::sceneChange()
{
    Scene * scene = this->scene();
    if (!scene)
        return;
    scene->readSceneMousePress( 0 );
}

void ZoomTool::sceneChanged()
{
    Scene * scene = this->scene();
    if (!scene)
        return;
    scene->readSceneMousePress( d->listener );
}

void ZoomTool::zoom(const QPointF & point)
{
    Scene * scene = this->scene();
    if (!scene)
        return;
    QList<QGraphicsView*> views = scene->views();
    qreal factor = 1 + (d->out->isChecked() ? -0.1 : 0.1);
    foreach(QGraphicsView* view, views)
    {
        Canvas * canvas = qobject_cast<Canvas*>(view);
        if (!canvas)
            continue;
        if (d->listener->wasDragged())
        {
            QRect r(canvas->mapFromScene(d->listener->mousePressPosition()),
                                              canvas->mapFromScene(point));
            if (r.width() > 20 && r.height() > 20)
                canvas->scale(r);
            else
                canvas->scale(factor, canvas->mapFromScene(point));
        }
        else
            canvas->scale(factor, canvas->mapFromScene(point));
    }
}
