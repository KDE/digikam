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

#include "CanvasSize.h"

using namespace PhotoLayoutsEditor;

QMap<CanvasSize::ResolutionUnits,qreal> CanvasSize::resolution_factors;
QMap<CanvasSize::ResolutionUnits,QString> CanvasSize::resolution_names;
QMap<CanvasSize::SizeUnits,qreal> CanvasSize::size_factors;
QMap<CanvasSize::SizeUnits,QString> CanvasSize::size_names;

void CanvasSize::prepare_maps()
{
    if (resolution_factors.isEmpty() || resolution_names.isEmpty())
    {
        resolution_factors.clear();
        resolution_names.clear();

        resolution_factors.insert(UnknownResolutionUnit, 0);
        resolution_factors.insert(PixelsPerMilimeter, 25.4);
        resolution_factors.insert(PixelsPerCentimeter, 2.54);
        resolution_factors.insert(PixelsPerInch, 1);
        resolution_factors.insert(PixelsPerPoint, 72.27);
        resolution_factors.insert(PixelsPerPicas, 6.0225);

        resolution_names.insert(PixelsPerMilimeter, QLatin1String("px/mm"));
        resolution_names.insert(PixelsPerCentimeter, QLatin1String("px/cm"));
        resolution_names.insert(PixelsPerInch, QLatin1String("px/in"));
        resolution_names.insert(PixelsPerPoint, QLatin1String("px/pt"));
        resolution_names.insert(PixelsPerPicas, QLatin1String("px/pc"));
    }

    if (size_factors.isEmpty() || size_names.isEmpty())
    {
        size_factors.clear();
        size_names.clear();

        size_factors.insert(UnknownSizeUnit, 0);
        size_factors.insert(Milimeters, 25.4);
        size_factors.insert(Centimeters, 2.54);
        size_factors.insert(Inches, 1);
        size_factors.insert(Points, 72.27);
        size_factors.insert(Picas, 6.0225);

        size_names.insert(Pixels, QLatin1String("px"));
        size_names.insert(Milimeters, QLatin1String("mm"));
        size_names.insert(Centimeters, QLatin1String("cm"));
        size_names.insert(Inches, QLatin1String("in"));
        size_names.insert(Points, QLatin1String("pt"));
        size_names.insert(Picas, QLatin1String("pc"));
    }
}

QList<qreal> CanvasSize::resolutionUnitsFactors()
{
    prepare_maps();
    return resolution_factors.values();
}

QList<QString> CanvasSize::resolutionUnitsNames()
{
    prepare_maps();
    return resolution_names.values();
}

QList<CanvasSize::ResolutionUnits> CanvasSize::resolutionUnits()
{
    prepare_maps();
    return resolution_factors.keys();
}

qreal CanvasSize::resolutionUnitFactor(CanvasSize::ResolutionUnits unit)
{
    prepare_maps();
    return resolution_factors.value(unit, 0);
}

qreal CanvasSize::resolutionUnitFactor(QString unitName)
{
    prepare_maps();
    return resolution_factors.value( resolution_names.key(unitName, UnknownResolutionUnit) );
}

QString CanvasSize::resolutionUnitName(CanvasSize::ResolutionUnits unit)
{
    prepare_maps();
    return resolution_names.value(unit);
}

QString CanvasSize::resolutionUnitName(qreal factor)
{
    prepare_maps();
    return resolution_names.value(resolution_factors.key(factor));
}

CanvasSize::ResolutionUnits CanvasSize::resolutionUnit(qreal factor)
{
    prepare_maps();
    return resolution_factors.key(factor, UnknownResolutionUnit);
}

CanvasSize::ResolutionUnits CanvasSize::resolutionUnit(QString name)
{
    prepare_maps();
    return resolution_names.key(name, UnknownResolutionUnit);
}

qreal CanvasSize::resolutionConvert(qreal value, ResolutionUnits from, ResolutionUnits to)
{
    qreal fromFactor = resolutionUnitFactor(from);
    qreal toFactor = resolutionUnitFactor(to);
    if (!fromFactor || !toFactor)
        return value;
    value /= fromFactor;
    value *= toFactor;
    return value;
}

QList<qreal> CanvasSize::sizeUnitsFactors()
{
    prepare_maps();
    return size_factors.values();
}

QList<QString> CanvasSize::sizeUnitsNames()
{
    prepare_maps();
    return size_names.values();
}

QList<CanvasSize::SizeUnits> CanvasSize::sizeUnits()
{
    prepare_maps();
    return size_factors.keys();
}

qreal CanvasSize::sizeUnitFactor(CanvasSize::SizeUnits unit)
{
    prepare_maps();
    return size_factors.value(unit);
}

qreal CanvasSize::sizeUnitFactor(QString unitName)
{
    prepare_maps();
    return size_factors.value(size_names.key(unitName, UnknownSizeUnit));
}

QString CanvasSize::sizeUnitName(CanvasSize::SizeUnits unit)
{
    prepare_maps();
    return size_names.value(unit);
}

QString CanvasSize::sizeUnitName(qreal factor)
{
    prepare_maps();
    return size_names.value(size_factors.key(factor));
}

CanvasSize::SizeUnits CanvasSize::sizeUnit(qreal factor)
{
    prepare_maps();
    return size_factors.key(factor, UnknownSizeUnit);
}

CanvasSize::SizeUnits CanvasSize::sizeUnit(QString name)
{
    prepare_maps();
    return size_names.key(name, UnknownSizeUnit);
}

qreal CanvasSize::sizeConvert(qreal value, SizeUnits from, SizeUnits to)
{
    qreal fromFactor = sizeUnitFactor(from);
    qreal toFactor = sizeUnitFactor(to);
    if (!fromFactor || !toFactor)
        return value;
    value /= fromFactor;
    value *= toFactor;
    return value;
}

int CanvasSize::toPixels(qreal value, qreal resolution, SizeUnits sUnit, ResolutionUnits rUnit)
{
    if (sUnit == Pixels)
        return value;
    qreal result = (resolutionUnitFactor(rUnit) * resolution * value)
                   / sizeUnitFactor(sUnit);
    return qRound(result);
}

qreal CanvasSize::fromPixels(int pixels, qreal resolution, SizeUnits sUnit, ResolutionUnits rUnit)
{
    qreal sizeFactor = sizeUnitFactor(sUnit);
    qreal resolutionFactor = resolutionUnitFactor(rUnit);
    return (pixels * sizeFactor) / (resolution * resolutionFactor);
}

CanvasSize::CanvasSize()
{
    prepare_maps();
    m_size = QSizeF();
    m_size_unit = UnknownSizeUnit;
    m_resolution = QSizeF();
    m_resolution_unit = UnknownResolutionUnit;
}

CanvasSize::CanvasSize(const QSizeF & size, SizeUnits sUnit, const QSizeF & resolution, ResolutionUnits rUnit)
{
    prepare_maps();
    m_size = size;
    m_size_unit = sUnit;
    m_resolution = resolution;
    m_resolution_unit = rUnit;
}

QSizeF CanvasSize::size() const
{
    return m_size;
}

void CanvasSize::setSize(const QSizeF & size)
{
    if (!size.isValid())
        return;
    m_size = size;
}

QSizeF CanvasSize::size(SizeUnits unit) const
{
    QSizeF result;
    result.setWidth( toPixels(m_size.width(), m_resolution.width(), m_size_unit, m_resolution_unit) );
    result.setHeight( toPixels(m_size.height(), m_resolution.height(), m_size_unit, m_resolution_unit) );
    if (unit != Pixels)
    {
        result.setWidth( fromPixels(result.width(), m_resolution.width(), unit, m_resolution_unit) );
        result.setHeight( fromPixels(result.height(), m_resolution.height(), unit, m_resolution_unit) );
    }
    return result;
}

CanvasSize::SizeUnits CanvasSize::sizeUnit() const
{
    return m_size_unit;
}

void CanvasSize::setSizeUnit(SizeUnits unit)
{
    if (unit < Pixels || unit > Picas)
        return;
    m_size_unit = unit;
}

QSizeF CanvasSize::resolution() const
{
    return m_resolution;
}

QSizeF CanvasSize::resolution(ResolutionUnits unit) const
{
    if (!this->isValid())
        return QSizeF();

    QSizeF result = m_resolution;
    if (m_resolution_unit != CanvasSize::PixelsPerInch)
    {
        qreal factor = CanvasSize::resolutionUnitFactor(m_resolution_unit);
        result.setWidth(result.width() * factor);
        result.setHeight(result.height() * factor);
    }

    if (unit != m_resolution_unit &&
            unit != CanvasSize::UnknownResolutionUnit)
    {
        qreal factor = CanvasSize::resolutionUnitFactor(unit);
        result.setWidth(result.width() / factor);
        result.setHeight(result.height() / factor);
    }
    return result;
}

void CanvasSize::setResolution(const QSizeF & resolution)
{
    if (!resolution.isValid())
        return;
    m_resolution = resolution;
}

CanvasSize::ResolutionUnits CanvasSize::resolutionUnit() const
{
    return m_resolution_unit;
}

void CanvasSize::setResolutionUnit(ResolutionUnits unit)
{
    if (unit < PixelsPerMilimeter || unit > PixelsPerPicas)
        return;
    m_resolution_unit = unit;
}

bool CanvasSize::isValid() const
{
    return m_size.isValid() &&
            m_resolution.isValid() &&
            (m_size_unit != UnknownSizeUnit) &&
            (m_resolution_unit != UnknownResolutionUnit);
}

bool CanvasSize::operator ==(const CanvasSize & size) const
{
    return this->m_size == size.m_size &&
            this->m_size_unit == size.m_size_unit &&
            this->m_resolution == size.m_resolution &&
            this->m_resolution_unit == size.m_resolution_unit;
}

bool CanvasSize::operator !=(const CanvasSize & size) const
{
    return !(*this == size);
}
