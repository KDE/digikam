/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-07-13
 * Description : Model for image versions
 *
 * Copyright (C) 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
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

// KDE includes

#include <KLocale>
#include <KDebug>

// Local includes

#include "imageversionsmodel.h"
#include "workingwidget.h"

namespace Digikam
{

ImageVersionsModel::ImageVersionsModel(QObject* parent)
                  : QAbstractListModel(parent)
{
    m_data = new QList<QPair<QString, int> >;
    m_paintTree = false;
}

ImageVersionsModel::~ImageVersionsModel()
{
    //qDeleteAll(m_data);
}

Qt::ItemFlags ImageVersionsModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant ImageVersionsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if(role == Qt::DisplayRole && !m_data->isEmpty())
    {
        return m_data->at(index.row()).first;
    }
    else if(role == Qt::UserRole && !m_data->isEmpty())
    {
        return m_data->at(index.row()).second;
    }    
    else if(role == Qt::DisplayRole && m_data->isEmpty())
    {   //TODO: make this text Italic
        return QVariant(QString(i18n("No image selected")));
    }

    return QVariant();
}

int ImageVersionsModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return m_data->count();
}

void ImageVersionsModel::setupModelData(QList<QPair<QString, int> >& data)
{
    beginResetModel();

    m_data->clear();

    if(!data.isEmpty())
    {
        m_data->append(data);
    }
    else 
    {
        m_data->append(qMakePair(QString(i18n("This is original image")), 0));
    }

    endResetModel();
}

void ImageVersionsModel::clearModelData()
{
    beginResetModel();
    if(!m_data->isEmpty())
    {
        m_data->clear();
    }
    endResetModel();
}

void ImageVersionsModel::slotAnimationStep()
{
    emit dataChanged(createIndex(0, 0), createIndex(rowCount()-1, 1));
}

QString ImageVersionsModel::currentSelectedImage() const
{
    return m_currentSelectedImage;
}

void ImageVersionsModel::setCurrentSelectedImage(const QString& path)
{
    m_currentSelectedImage = path;
}

QModelIndex ImageVersionsModel::currentSelectedImageIndex() const
{
    return index(listIndexOf(m_currentSelectedImage), 0);
}

bool ImageVersionsModel::paintTree() const
{
    return m_paintTree;
}

void ImageVersionsModel::setPaintTree(bool paint)
{
    m_paintTree = paint;
}

int ImageVersionsModel::listIndexOf(const QString& item) const
{
    for(int i = 0; i < m_data->size(); i++)
    {
        if(m_data->at(i).first == item)
            return i;
    }
    return -1;
}

} // namespace Digikam