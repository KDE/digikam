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
 * Copyright (C) 2009-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "PhotoEffectsGroup.h"
#include "PhotoEffectsLoader.h"
#include "AbstractPhoto.h"
#include "AbstractPhotoEffectFactory.h"
#include "AbstractPhotoEffectInterface.h"
#include "UndoCommandEvent.h"
#include "global.h"

#include <QPainter>

#include <klocalizedstring.h>
#include <QApplication>

using namespace PhotoLayoutsEditor;

PhotoEffectsGroup::PhotoEffectsGroup(AbstractPhoto * photo, QObject * parent) :
    AbstractMovableModel(parent),
    m_photo(photo)
{
    connect(this, SIGNAL(effectsChanged()), photo, SLOT(refresh()));
}

PhotoEffectsGroup::~PhotoEffectsGroup()
{
}

QDomElement PhotoEffectsGroup::toSvg(QDomDocument & document) const
{
    QDomElement effectsGroup = document.createElement(QLatin1String("effects"));
    for (int i = m_effects_list.count()-1; i >= 0; --i)
    {
        QDomElement e = PhotoEffectsLoader::effectToSvg(m_effects_list[i], document);
        if (e.isNull())
            continue;
        effectsGroup.appendChild(e);
    }
    return effectsGroup;
}

PhotoEffectsGroup * PhotoEffectsGroup::fromSvg(const QDomElement & element, AbstractPhoto * graphicsItem)
{
    QDomElement temp = element;
    if (temp.tagName() != QLatin1String("effects"))
        temp = temp.firstChildElement(QLatin1String("effects"));
    if (temp.isNull())
        return 0;
    PhotoEffectsGroup * group = new PhotoEffectsGroup(0);
    QDomNodeList effectsList = temp.childNodes();
    for (int i = effectsList.count()-1; i >= 0; --i)
    {
        QDomElement effect = effectsList.at(i).toElement();
        if (effect.isNull())
            continue;
        AbstractPhotoEffectInterface * interface = PhotoEffectsLoader::getEffectFromSvg(effect);
        if (interface)
            group->push_back(interface);
    }
    group->m_photo = graphicsItem;
    return group;
}

void PhotoEffectsGroup::push_back(AbstractPhotoEffectInterface * effect)
{
    m_effects_list.push_back(effect);
    connect(effect, SIGNAL(changed()), this, SLOT(emitEffectsChanged()));
    effect->setParent(this);
    effect->setGroup(this);
    emit layoutChanged();
}

void PhotoEffectsGroup::push_front(AbstractPhotoEffectInterface * effect)
{
    m_effects_list.push_front(effect);
    connect(effect, SIGNAL(changed()), this, SLOT(emitEffectsChanged()));
    effect->setParent(this);
    effect->setGroup(this);
    emit layoutChanged();
}

QImage PhotoEffectsGroup::apply(const QImage & image)
{
    QImage temp = image;
    for (int i = m_effects_list.count()-1; i >= 0; --i)
    {
        AbstractPhotoEffectInterface * effect = m_effects_list[i];
        if (effect)
            temp = effect->apply(temp);
    }
    return temp;
}

AbstractPhoto * PhotoEffectsGroup::photo() const
{
    return m_photo;
}

QObject * PhotoEffectsGroup::item(const QModelIndex & index) const
{
    if (index.isValid() && index.row() < rowCount())
        return m_effects_list.at(index.row());
    return 0;
}

void PhotoEffectsGroup::setItem(QObject * item, const QModelIndex & index)
{
    AbstractPhotoEffectInterface * effect = dynamic_cast<AbstractPhotoEffectInterface*>(item);
    if (!effect || !index.isValid())
        return;
    int row = index.row();
    if (row < 0 || row >= rowCount())
        return;
    AbstractPhotoEffectInterface * temp = m_effects_list.takeAt(row);
    if (temp)
        temp->disconnect(this);
    m_effects_list.removeAt(row);
    m_effects_list.insert(row, effect);
    effect->setParent(this);
    effect->setGroup(this);
    connect(effect, SIGNAL(changed()), this, SLOT(emitEffectsChanged()));
    emitEffectsChanged(effect);
}

AbstractPhotoEffectInterface * PhotoEffectsGroup::graphicsItem(const QModelIndex & index) const
{
    return static_cast<AbstractPhotoEffectInterface*>(index.internalPointer());
}

bool PhotoEffectsGroup::moveRowsData(int sourcePosition, int sourceCount, int destPosition)
{
    if (  (sourcePosition <= destPosition && sourcePosition+sourceCount >= destPosition) ||
            sourceCount <= 0 ||
            m_effects_list.count() <= sourcePosition+sourceCount-1 ||
            sourcePosition < 0 ||
            destPosition < 0 ||
            m_effects_list.count() < destPosition)
        return false;

    beginMoveRows(QModelIndex(), sourcePosition, sourcePosition+sourceCount-1, QModelIndex(), destPosition);
    QList<AbstractPhotoEffectInterface*> movingItems;
    if (destPosition > sourcePosition)
        destPosition -= sourceCount;
    while(sourceCount--)
        movingItems.push_back(m_effects_list.takeAt(sourcePosition));
    for ( ; movingItems.count() ; movingItems.pop_back())
        m_effects_list.insert(destPosition, movingItems.last());
    endMoveRows();
    this->emitEffectsChanged();
    emit layoutChanged();
    return true;
}

int PhotoEffectsGroup::columnCount(const QModelIndex & /*parent*/) const
{
    return 1;
}

QVariant PhotoEffectsGroup::data(const QModelIndex & index, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (!index.isValid())
        return i18n("Effect name");
    else
    {
        AbstractPhotoEffectInterface * effect = graphicsItem(index);
        if (effect)
            return effect->toString();
        else
            return QVariant();
    }
}

Qt::ItemFlags PhotoEffectsGroup::flags(const QModelIndex & index) const
{
    Qt::ItemFlags result = QAbstractItemModel::flags(index);
    if (index.isValid() && !index.internalPointer())
        result |= Qt::ItemIsEditable;
    return result;
}

QModelIndex PhotoEffectsGroup::index(int row, int column, const QModelIndex & parent) const
{
    if (column != 0)
        return QModelIndex();
    if (row >= m_effects_list.count())
        return QModelIndex();
    if (parent.isValid())
        return QModelIndex();
    return createIndex(row,column,m_effects_list.at(row));
}

bool PhotoEffectsGroup::insertRows(int row, int count, const QModelIndex & parent)
{
    if (row < 0 || row > rowCount() || count < 1 || parent.isValid())
        return false;
    beginInsertRows(parent, row, row+count-1);
    while(count--)
        m_effects_list.insert(row,0);
    endInsertRows();
    emit layoutChanged();
    return true;
}

QModelIndex PhotoEffectsGroup::parent(const QModelIndex & /*index*/) const
{
    return QModelIndex();
}

int PhotoEffectsGroup::rowCount(const QModelIndex & parent) const
{
    if (!parent.isValid())
        return this->m_effects_list.count();
    else
        return 0;
}

bool PhotoEffectsGroup::removeRows(int row, int count, const QModelIndex & parent)
{
    if (count <= 0 || parent.isValid() || row < 0 || row >= rowCount(parent) || row+count > rowCount(parent))
        return false;
    beginRemoveRows(QModelIndex(), row, row+count-1);
    while (count--)
        m_effects_list.removeAt(row);
    endRemoveRows();
    this->emitEffectsChanged();
    emit layoutChanged();
    return true;
}

void PhotoEffectsGroup::emitEffectsChanged(AbstractPhotoEffectInterface * effect)
{
    if (!m_photo)
        return;
    m_photo->refresh();
    if (effect)
    {
        int row = m_effects_list.indexOf(effect);
        QModelIndex indexChanged = index(row,0);
        emit dataChanged(indexChanged,indexChanged);
    }
    else if (rowCount())
        emit dataChanged(index(0,0),index(rowCount()-1,0));
    emit effectsChanged();
}
