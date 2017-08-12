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

#include "AbstractPhoto_p.h"
#include "AbstractPhoto.h"

#include <QMenu>
#include <QStyle>
#include <QStyleOptionGraphicsItem>
#include <QPainterPath>
#include <QPolygonF>
#include <QPainter>
#include <QVariant>
#include <qmath.h>
#include <QDebug>

#include <klocalizedstring.h>

#include "digikam_debug.h"
#include "Scene.h"
#include "PhotoEffectsGroup.h"
#include "BordersGroup.h"
#include "global.h"

using namespace PhotoLayoutsEditor;

class PhotoLayoutsEditor::CropShapeChangeCommand : public QUndoCommand
{
    QPainterPath m_crop_shape;
    AbstractPhoto * m_item;
public:
    CropShapeChangeCommand(const QPainterPath & cropShape, AbstractPhoto * item, QUndoCommand * parent = 0) :
        QUndoCommand(i18n("Crop shape change"), parent),
        m_crop_shape(cropShape),
        m_item(item)
    {}
    virtual void redo()
    {
        this->run();
    }
    virtual void undo()
    {
        this->run();
    }
    void run()
    {
        QPainterPath temp = m_item->d->cropShape();
        m_item->d->setCropShape(m_crop_shape);
        m_crop_shape = temp;
    }
};
class PhotoLayoutsEditor::ItemNameChangeCommand : public QUndoCommand
{
    QString m_name;
    AbstractPhoto * m_item;
public:
    ItemNameChangeCommand(const QString & name, AbstractPhoto * item, QUndoCommand * parent = 0) :
        QUndoCommand(i18n("Name Change"), parent),
        m_name(name),
        m_item(item)
    {}
    virtual void redo()
    {
        this->run();
    }
    virtual void undo()
    {
        this->run();
    }
    void run()
    {
        QString temp = m_item->d->name();
        m_item->d->setName(m_name);
        m_name = temp;
    }
};

AbstractPhoto::AbstractPhoto(const QString & name, Scene * scene) :
    AbstractItemInterface(0, 0),
    d(new AbstractPhotoPrivate(this))
{
    if (scene)
        scene->addItem(this);

    setupItem();

    // Item's name
    d->setName(uniqueName( name.isEmpty() ? i18n("New layer") : name ) );

    // Effects group
    d->m_effects_group = new PhotoEffectsGroup(this);

    // Effects group
    d->m_borders_group = new BordersGroup(this);
}

AbstractPhoto::~AbstractPhoto()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Abstractphoto delete";
    d->m_effects_group->deleteLater();
    d->m_borders_group->deleteLater();
    delete d;
}

void AbstractPhoto::setupItem()
{
    this->setFlag(QGraphicsItem::ItemIsSelectable);
    this->setFlag(QGraphicsItem::ItemIsMovable);
    this->setFlag(QGraphicsItem::ItemSendsGeometryChanges);
    this->setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
}

QString AbstractPhoto::uniqueName(const QString & name)
{
    QString temp;
    QString result;
    if (name.isEmpty())
        return name;
    temp = name.simplified();
    if (temp.length() > 20)
    {
        temp = temp.left(20);
        temp.append(QLatin1String("..."));
    }
    result = temp;
    Scene * scene = qobject_cast<Scene*>(this->scene());
    if (!scene)
        return result;
    int nameNumber = 1;
    QList<QGraphicsItem*> items = scene->items();
    foreach(QGraphicsItem* item, items)
    {
        AbstractPhoto * myItem = dynamic_cast<AbstractPhoto*>(item);
        if (!myItem || myItem == this || myItem->name().isEmpty())
            continue;
        while (myItem->name() == result)
        {
            nameNumber += 1;
            result = temp + ((nameNumber > 1) ? (QString::fromLatin1(" ").append(QString::number(nameNumber))) : QLatin1String(" "));
        }
    }
    return result;
}

QRectF AbstractPhoto::boundingRect() const
{
    return shape().boundingRect();
}

QPainterPath AbstractPhoto::shape() const
{
    QPainterPath result = this->itemShape();
    if (d->m_borders_group)
        result = result.united(bordersGroup()->shape());
    return result;
}

QPainterPath AbstractPhoto::opaqueArea() const
{
    QPainterPath result = this->itemOpaqueArea();
    if (d->m_borders_group)
        result = result.united(bordersGroup()->shape());
    return result;
}

QDomDocument AbstractPhoto::toSvg() const
{
    QDomDocument document;
    QTransform transform = this->transform();
    QString translate = QLatin1String("translate(")+
                        QString::number(this->pos().x())+
                        QLatin1Char(QLatin1Char(','))+
                        QString::number(this->pos().y())+
                        QLatin1Char(')');
    QString matrix = QLatin1String("matrix(")+
                     QString::number(transform.m11())+
                     QLatin1Char(',')+
                     QString::number(transform.m12())+
                     QLatin1Char(',')+
                     QString::number(transform.m21())+
                     QLatin1Char(',')+
                     QString::number(transform.m22())+
                     QLatin1Char(',')+
                     QString::number(transform.m31())+
                     QLatin1Char(',')+
                     QString::number(transform.m32())+
                     QLatin1Char(')');
    QDomElement itemSVG = document.createElement(QLatin1String("g"));
    document.appendChild(itemSVG);
    itemSVG.setAttribute(QLatin1String("transform"), translate + QLatin1Char(' ') + matrix);
    itemSVG.setAttribute(QLatin1String("id"), this->id());
    itemSVG.setAttribute(QLatin1String("name"), QString::fromUtf8(this->name().toUtf8()));
    if (!this->isVisible())
        itemSVG.setAttribute(QLatin1String("visibility"), QLatin1String("hide"));

    // 'defs' tag
    QDomElement defs = document.createElement(QLatin1String("defs"));
    defs.setAttribute(QLatin1String("id"), QLatin1String("data_")+this->id());
    itemSVG.appendChild(defs);

    // 'defs'->'clipPath'
    QDomElement clipPath = document.createElement(QLatin1String("clipPath"));
    clipPath.setAttribute(QLatin1String("id"), QLatin1String("clipPath_")+this->id());
    defs.appendChild(clipPath);

    // 'defs'->'ple:data'
    QDomElement appNSData = document.createElementNS(PhotoLayoutsEditor::uri(), QLatin1String("data"));
    appNSData.setPrefix(PhotoLayoutsEditor::name());
    defs.appendChild(appNSData);
    appNSData.appendChild(d->m_effects_group->toSvg(document));

    // 'defs'->'ple:data'->'crop_path'
    QDomElement cropPath = document.createElement(QLatin1String("crop_path"));
    cropPath.appendChild( PhotoLayoutsEditor::pathToSvg(this->cropShape()).documentElement() );
    appNSData.appendChild(cropPath);

    // Convert visible area to SVG path's 'd' attribute
    QPainterPath visibleArea = this->shape();
    if (!visibleArea.isEmpty())
    {
        // 'defs'->'clipPath'->'path'
        clipPath.appendChild( PhotoLayoutsEditor::pathToSvg(visibleArea).documentElement() );
    }

    QDomElement visibleData = document.createElement(QLatin1String("g"));
    visibleData.setAttribute(QLatin1String("id"), QLatin1String("vis_data_") + this->id());
    defs.appendChild(visibleData);
    visibleData.appendChild(this->svgVisibleArea());
    visibleData.appendChild(d->m_borders_group->toSvg(document));

    // 'use'
    QDomElement use = document.createElement(QLatin1String("use"));
    use.setAttribute(QLatin1String("xlink:href"),QLatin1Char('#')+visibleData.attribute(QLatin1String("id")));
    use.setAttribute(QLatin1String("style"), QLatin1String("clip-path: url(#") + clipPath.attribute(QLatin1String("id")) + QLatin1String(");"));
    itemSVG.appendChild(use);

    // 'g'
    QDomElement g2 = document.createElement(QLatin1String("g"));
    itemSVG.appendChild(g2);

    // 'g'->'use'
    QDomElement use3 = document.createElement(QLatin1String("use"));
    use3.setAttribute(QLatin1String("xlink:href"),QLatin1Char('#') + clipPath.attribute(QLatin1String("id")));
    g2.appendChild(use3);

     /*
      * <g>
      *     <defs>
      *         <clipPath>      // clippingPath == m_image_path
      *             <path />
      *         </clipPath>
      *         <g>
      *             .........     // Children data
      *             .........     // Borders applied to the item
      *         </g>
      *         <ple:data>
      *             .........     // Effects applied to the item
      *             .........     // Crop path
      *             .........     // Other data from the AbstractPhoto class.
      *         </ple:data>
      *     </defs>
      *     <use />             // Cuts image
      *     <g>
      *         <use />         // Print cut image
      *     </g>
      * </g>
      */

    return document;
}

QDomDocument AbstractPhoto::toTemplateSvg() const
{
    QDomDocument document;
    QTransform transform = this->transform();
    QString translate = QLatin1String("translate(")+
                        QString::number(this->pos().x())+
                        QLatin1Char(',')+
                        QString::number(this->pos().y())+
                        QLatin1Char(')');
    QString matrix = QLatin1String("matrix(")+
                     QString::number(transform.m11())+
                     QLatin1Char(',')+
                     QString::number(transform.m12())+
                     QLatin1Char(',')+
                     QString::number(transform.m21())+
                     QLatin1Char(',')+
                     QString::number(transform.m22())+
                     QLatin1Char(',')+
                     QString::number(transform.m31())+
                     QLatin1Char(',')+
                     QString::number(transform.m32())+
                     QLatin1Char(')');
    QDomElement itemSVG = document.createElement(QLatin1String("g"));
    document.appendChild(itemSVG);
    itemSVG.setAttribute(QLatin1String("transform"), translate + QLatin1Char(' ') + matrix);
    itemSVG.setAttribute(QLatin1String("id"), this->id());
    itemSVG.setAttribute(QLatin1String("name"), QString::fromUtf8(this->name().toUtf8()));
    if (!this->isVisible())
        itemSVG.setAttribute(QLatin1String("visibility"), QLatin1String("hide"));

    // 'defs' tag
    QDomElement defs = document.createElement(QLatin1String("defs"));
    defs.setAttribute(QLatin1String("id"), QLatin1String("data_")+this->id());
    itemSVG.appendChild(defs);

    // 'defs'->'clipPath'
    QDomElement clipPath = document.createElement(QLatin1String("clipPath"));
    clipPath.setAttribute(QLatin1String("id"), QLatin1String("clipPath_")+this->id());
    defs.appendChild(clipPath);

    // 'defs'->'ple:data'
    QDomElement appNSData = document.createElementNS(PhotoLayoutsEditor::uri(), QLatin1String("data"));
    appNSData.setPrefix(PhotoLayoutsEditor::name());
    defs.appendChild(appNSData);
    appNSData.appendChild(d->m_effects_group->toSvg(document));

    // 'defs'->'ple:data'->'crop_path'
    QDomElement cropPath = document.createElement(QLatin1String("crop_path"));
    cropPath.appendChild( PhotoLayoutsEditor::pathToSvg(this->cropShape()).documentElement() );
    appNSData.appendChild(cropPath);

    // Convert visible area to SVG path's 'd' attribute
    QPainterPath visibleArea = this->shape();
    if (!visibleArea.isEmpty())
    {
        // 'defs'->'clipPath'->'path'
        clipPath.appendChild( PhotoLayoutsEditor::pathToSvg(visibleArea).documentElement() );
    }

    QDomElement visibleData = document.createElement(QLatin1String("g"));
    visibleData.setAttribute(QLatin1String("id"), QLatin1String("vis_data_") + this->id());
    defs.appendChild(visibleData);
    visibleData.appendChild(this->svgTemplateArea());
    visibleData.appendChild(d->m_borders_group->toSvg(document));

    // 'use'
    QDomElement use = document.createElement(QLatin1String("use"));
    use.setAttribute(QLatin1String("xlink:href"),QLatin1Char('#')+visibleData.attribute(QLatin1String("id")));
    use.setAttribute(QLatin1String("style"),QLatin1String("clip-path: url(#") + clipPath.attribute(QLatin1String("id")) + QLatin1String(");"));
    itemSVG.appendChild(use);

    // 'g'
    QDomElement g2 = document.createElement(QLatin1String("g"));
    itemSVG.appendChild(g2);

    // 'g'->'use'
    QDomElement use3 = document.createElement(QLatin1String("use"));
    use3.setAttribute(QLatin1String("xlink:href"),QLatin1Char('#') + clipPath.attribute(QLatin1String("id")));
    g2.appendChild(use3);

     /*
      * <g>
      *     <defs>
      *         <clipPath>      // clippingPath == m_image_path
      *             <path />
      *         </clipPath>
      *         <g>
      *             .........     // Children data
      *             .........     // Borders applied to the item
      *         </g>
      *         <ple:data>
      *             .........     // Effects applied to the item
      *             .........     // Crop path
      *             .........     // Other data from the AbstractPhoto class.
      *         </ple:data>
      *     </defs>
      *     <use />             // Cuts image
      *     <g>
      *         <use />         // Print cut image
      *     </g>
      * </g>
      */

    return document;
}

bool AbstractPhoto::fromSvg(QDomElement & element)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "1";

    if (element.tagName() != QLatin1String("g"))
        return false;

    qCDebug(DIGIKAM_GENERAL_LOG) << "2";

    if (element.attribute(QLatin1String("visibility")) == QLatin1String("hide"))
        this->setVisible(false);

    qCDebug(DIGIKAM_GENERAL_LOG) << "3";

    // Position & transformation
    this->setPos(0,0);
    QString transform = element.attribute(QLatin1String("transform"));
    if (!transform.isEmpty())
    {
        QRegExp tr(QLatin1String("translate\\([-0-9.]+,[-0-9.]+\\)"));
        if (tr.indexIn(transform) >= 0)
        {
            QStringList list = tr.capturedTexts();
            QString translate = list.at(0);
            list = translate.split(QLatin1Char(','));
            QString x = list.at(0);
            QString y = list.at(1);
            this->setPos( x.right(x.length()-10).toDouble(),
                          y.left(y.length()-1).toDouble());
        }

        QRegExp rot(QLatin1String("matrix\\([-0-9.]+,[-0-9.]+,[-0-9.]+,[-0-9.]+,[-0-9.]+,[-0-9.]+\\)"));
        if (rot.indexIn(transform) >= 0)
        {
            QStringList list = rot.capturedTexts();
            QString matrix = list.at(0);
            matrix.remove(matrix.length()-1,1).remove(0,7);
            list = matrix.split(QLatin1Char(','));
            QString m11 = list.at(0);
            QString m12 = list.at(1);
            QString m21 = list.at(2);
            QString m22 = list.at(3);
            QString m31 = list.at(4);
            QString m32 = list.at(5);
            this->setTransform(QTransform(m11.toDouble(), m12.toDouble(), 0,
                                          m21.toDouble(), m22.toDouble(), 0,
                                          m31.toDouble(), m32.toDouble(), 1));
        }
    }


    qCDebug(DIGIKAM_GENERAL_LOG) << "4";

    if (element.firstChildElement().tagName() == QLatin1String("g"))
    {
        element = element.firstChildElement();
        QString transform = element.attribute(QLatin1String("transform"));
        if (!transform.isEmpty())
        {
            QRegExp rot(QLatin1String("matrix\\([-0-9.]+,[-0-9.]+,[-0-9.]+,[-0-9.]+,[-0-9.]+,[-0-9.]+\\)"));
            if (rot.indexIn(transform) >= 0)
            {
                QStringList list = rot.capturedTexts();
                QString matrix = list.at(0);
                matrix.remove(matrix.length()-1,1).remove(0,7);
                list = matrix.split(QLatin1Char(','));
                QString m11 = list.at(0);
                QString m12 = list.at(1);
                QString m21 = list.at(2);
                QString m22 = list.at(3);
                QString m31 = list.at(4);
                QString m32 = list.at(5);
                this->setTransform(QTransform(m11.toDouble(), m12.toDouble(), 0,
                                              m21.toDouble(), m22.toDouble(), 0,
                                              m31.toDouble(), m32.toDouble(), 1));
            }
        }
    }


    qCDebug(DIGIKAM_GENERAL_LOG) << "5";

    // ID & name
    d->m_id = element.attribute(QLatin1String("id"));
    d->setName(element.attribute(QLatin1String("name")));


    qCDebug(DIGIKAM_GENERAL_LOG) << "6";

    // Validation purpose
    QDomElement defs = element.firstChildElement(QLatin1String("defs"));
    while (!defs.isNull() && defs.attribute(QLatin1String("id")) != QLatin1String("data_")+d->m_id)
        defs = defs.nextSiblingElement(QLatin1String("defs"));
    if (defs.isNull())
        return false;


    qCDebug(DIGIKAM_GENERAL_LOG) << "7";

    QDomElement itemDataElement = defs.firstChildElement(QLatin1String("g"));
    while (!itemDataElement.isNull() && itemDataElement.attribute(QLatin1String("id")) != QLatin1String("vis_data_")+this->id())
        itemDataElement = itemDataElement.nextSiblingElement(QLatin1String("g"));
    if (itemDataElement.isNull())
        return false;


    qCDebug(DIGIKAM_GENERAL_LOG) << "8";

    // Borders
    if (d->m_borders_group)
    {
        d->m_borders_group->deleteLater();
        d->m_borders_group = 0;
    }
    d->m_borders_group = BordersGroup::fromSvg(itemDataElement, this);
    if (!d->m_borders_group)
        return false;


    qCDebug(DIGIKAM_GENERAL_LOG) << "9";

    QDomElement clipPath = defs.firstChildElement(QLatin1String("clipPath"));
    if (clipPath.isNull() || clipPath.attribute(QLatin1String("id")) != QLatin1String("clipPath_")+this->id())
        return false;


    qCDebug(DIGIKAM_GENERAL_LOG) << "10";

    // Other application specific data
    QDomElement appNS = defs.firstChildElement(QLatin1String("data"));
    if (appNS.isNull() || appNS.prefix() != PhotoLayoutsEditor::name())
        return false;


    qCDebug(DIGIKAM_GENERAL_LOG) << "11";

    // Effects
    if (d->m_effects_group)
        delete d->m_effects_group;
    d->m_effects_group = PhotoEffectsGroup::fromSvg(appNS, this);
    if (!d->m_effects_group)
        return false;


    qCDebug(DIGIKAM_GENERAL_LOG) << "12";

    // Crop path
    QDomElement cropPath = appNS.firstChildElement(QLatin1String("crop_path"));
    if (!cropPath.isNull())
        this->d->setCropShape( PhotoLayoutsEditor::pathFromSvg( cropPath.firstChildElement(QLatin1String("path")) ) );
    else
        return false;


    qCDebug(DIGIKAM_GENERAL_LOG) << "13";

    return true;
}

void AbstractPhoto::setName(const QString & name)
{
    QString newName = this->uniqueName(name);
    QUndoCommand * command = new ItemNameChangeCommand(newName, this);
    PLE_PostUndoCommand(command);
}

QString AbstractPhoto::name() const
{
    return d->name();
}

void AbstractPhoto::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * /*widget*/)
{
    if (d->m_borders_group)
        d->m_borders_group->paint(painter, option);
}

QVariant AbstractPhoto::itemChange(GraphicsItemChange change, const QVariant & value)
{
    switch (change)
    {
        case ItemVisibleHasChanged:
            d->m_visible = value.toBool();
            break;
        case ItemScaleHasChanged:
        case ItemRotationHasChanged:
        case ItemTransformHasChanged:
            d->m_transform = this->transform();
            emit changed();
            break;
        case ItemPositionHasChanged:
        case ItemScenePositionHasChanged:
            d->m_pos = this->pos();
            emit changed();
            break;
        default:
            break;
    }
    return AbstractItemInterface::itemChange(change, value);
}

void AbstractPhoto::dragEnterEvent(QGraphicsSceneDragDropEvent * event)
{
    event->accept();
}

void AbstractPhoto::dragLeaveEvent(QGraphicsSceneDragDropEvent * event)
{
    event->accept();
}

void AbstractPhoto::dragMoveEvent(QGraphicsSceneDragDropEvent * event)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "dragMoveEvent";
    event->accept();
}

void AbstractPhoto::dropEvent(QGraphicsSceneDragDropEvent * event)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "dropEvent";
    event->accept();
}

void AbstractPhoto::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * /*event*/)
{
}

void AbstractPhoto::mouseMoveEvent(QGraphicsSceneMouseEvent * /*event*/)
{
}

void AbstractPhoto::mousePressEvent(QGraphicsSceneMouseEvent * /*event*/)
{
}

void AbstractPhoto::mouseReleaseEvent(QGraphicsSceneMouseEvent * /*event*/)
{
}

void AbstractPhoto::hoverEnterEvent(QGraphicsSceneHoverEvent * event)
{
    QGraphicsItem::hoverEnterEvent(event);
}

void AbstractPhoto::hoverLeaveEvent(QGraphicsSceneHoverEvent * event)
{
    QGraphicsItem::hoverLeaveEvent(event);
    this->unsetCursor();
}

QIcon & AbstractPhoto::icon()
{
    return d->m_icon;
}

const QIcon & AbstractPhoto::icon() const
{
    return d->m_icon;
}

void AbstractPhoto::setIcon(const QIcon & icon)
{
    if (icon.isNull())
        return;
    d->m_icon = icon;
    emit changed();
}

PhotoEffectsGroup * AbstractPhoto::effectsGroup() const
{
    return d->m_effects_group;
}

BordersGroup * AbstractPhoto::bordersGroup() const
{
    return d->m_borders_group;
}

QString AbstractPhoto::id() const
{
    if (d->m_id.isEmpty())
        d->m_id = QString::number((long long)this, 16);
    return d->m_id;
}

void AbstractPhoto::refresh()
{
    this->setVisible(d->m_visible);
    this->setPos(d->m_pos);
    this->setTransform(d->m_transform);
    this->refreshItem();
    if (d->m_borders_group)
        d->m_borders_group->refresh();
    emit changed();
}

void AbstractPhoto::setCropShape(const QPainterPath & cropShape)
{
    if (cropShape != this->d->cropShape())
    {
        QUndoCommand * command = new CropShapeChangeCommand(cropShape, this);
        PLE_PostUndoCommand(command);
    }
}

QPainterPath AbstractPhoto::cropShape() const
{
    return d->cropShape();
}
