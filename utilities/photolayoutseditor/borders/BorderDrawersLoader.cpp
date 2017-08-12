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

#include "BorderDrawersLoader.h"

#include <QMap>
#include <QStringList>
#include <QMetaProperty>
#include <QUndoCommand>

#include "BorderDrawerInterface.h"
#include "BorderDrawerFactoryInterface.h"
#include "BorderChangeListener.h"
#include "KEditFactory.h"
#include "qttreepropertybrowser.h"

using namespace PhotoLayoutsEditor;

class PhotoLayoutsEditor::BorderDrawersLoaderPrivate
{
    QMap<QString,BorderDrawerFactoryInterface*> factories;

    friend class BorderDrawersLoader;
};

BorderDrawersLoader * BorderDrawersLoader::m_instance = 0;

BorderDrawersLoader::BorderDrawersLoader(QObject * parent) :
    QObject(parent),
    d(new BorderDrawersLoaderPrivate)
{
}

BorderDrawersLoader::~BorderDrawersLoader()
{
    m_instance = 0;
    delete d;
}

BorderDrawersLoader * BorderDrawersLoader::instance(QObject * parent)
{
    if (!m_instance)
        m_instance = new BorderDrawersLoader();
    if (parent)
        m_instance->setParent(parent);
    return m_instance;
}

void BorderDrawersLoader::registerDrawer(BorderDrawerFactoryInterface * factory)
{
    factory->setParent(instance());
    QStringList names = factory->drawersNames().split(QLatin1Char(';'), QString::SkipEmptyParts);
    foreach(QString name, names)
        instance()->d->factories.insert(name, factory);
}

QStringList BorderDrawersLoader::registeredDrawers()
{
    return instance()->d->factories.keys();
}

BorderDrawerFactoryInterface * BorderDrawersLoader::getFactoryByName(const QString & name)
{
    return instance()->d->factories.value(name, 0);
}

BorderDrawerInterface * BorderDrawersLoader::getDrawerByName(const QString & name)
{
    BorderDrawerFactoryInterface * factory = getFactoryByName(name);
    if (factory)
    {
        BorderDrawerInterface * drawer = factory->getDrawerInstance(name);
        if (!drawer)
            return 0;
        return drawer;
    }
    return 0;
}

BorderDrawerInterface * BorderDrawersLoader::getDrawerFromSvg(QDomElement & drawerElement)
{
    QMap<QString,QString> properties;
    QDomNamedNodeMap attributes = drawerElement.attributes();
    for (int j = attributes.count()-1; j >= 0; --j)
    {
        QDomAttr attr = attributes.item(j).toAttr();
        if (attr.isNull())
            continue;
        properties.insert(attr.name(), attr.value());
    }
    QString drawerName = properties.take(QLatin1String("name"));
    if (!instance()->registeredDrawers().contains(drawerName))
        return 0;
    BorderDrawerInterface * drawer = getDrawerByName(drawerName);
    const QMetaObject * meta = drawer->metaObject();
    int count = meta->propertyCount();
    for (int i = 0; i < count; ++i)
    {
        QMetaProperty p = meta->property(i);
        QString value = properties.take(QLatin1String(p.name()));
        if (value.isEmpty())
            continue;
        p.write(drawer, QVariant(QByteArray::fromBase64(value.toLatin1())));
    }
    return drawer;
}

QDomElement BorderDrawersLoader::drawerToSvg(BorderDrawerInterface * drawer, QDomDocument & document)
{
    if (!drawer)
        return QDomElement();
    QDomElement result = document.createElement(QLatin1String("g"));
    result.setAttribute(QLatin1String("name"), drawer->name());

    result.appendChild( drawer->toSvg(document) );

    const QMetaObject * meta = drawer->metaObject();
    int count = meta->propertyCount();
    for (int i = 0; i < count; ++i)
    {
        QMetaProperty p = meta->property(i);
        result.setAttribute(QLatin1String(p.name()), QLatin1String(p.read(drawer).toByteArray().toBase64()));
    }

    return result;
}

QWidget * BorderDrawersLoader::createEditor(BorderDrawerInterface * drawer, bool createCommands)
{
    if (!drawer)
        return 0;

    QtTreePropertyBrowser * browser = new QtTreePropertyBrowser();
    BorderChangeListener * listener = new BorderChangeListener(drawer, browser, createCommands);

    // QVariant type of property
    QtVariantPropertyManager * variantManager = 0;
    KVariantEditorFactory * variantFactory = 0;

    // Integer type of property
    QtIntPropertyManager * integerManager = 0;
    KSliderEditFactory * integerFactory = 0;

    // Double type of property
    QtDoublePropertyManager * doubleManager = 0;
    KDoubleSpinBoxFactory * doubleFactory = 0;

    // Enum type of property
    QtEnumPropertyManager * enumManager = 0;
    KEnumEditorFactory * enumFactory = 0;

    const QMetaObject * meta = drawer->metaObject();
    int propertiesCount = meta->propertyCount();
    for (int i = 0; i < propertiesCount; ++i)
    {
        QMetaProperty metaProperty = meta->property(i);
        QString propertyName = drawer->propertyName(metaProperty);
        if (propertyName.isEmpty())
            continue;
        QtProperty * property;
        switch (metaProperty.type())
        {
            case QVariant::Int:
                {
                    if (!integerManager || !integerFactory)
                    {
                        integerManager = new QtIntPropertyManager(browser);
                        integerFactory = new KSliderEditFactory(browser);
                        browser->setFactoryForManager(integerManager, integerFactory);
                    }
                    property = integerManager->addProperty(propertyName);
                    integerManager->setValue(property, metaProperty.read(drawer).toInt());
                    integerManager->setMinimum(property, drawer->minimumValue(metaProperty).toInt());
                    integerManager->setMaximum(property, drawer->maximumValue(metaProperty).toInt());
                }
                break;
            case QVariant::Double:
                {
                    if (!doubleManager || !doubleFactory)
                    {
                        doubleManager = new QtDoublePropertyManager(browser);
                        doubleFactory = new KDoubleSpinBoxFactory(browser);
                        browser->setFactoryForManager(doubleManager, doubleFactory);
                    }
                    property = doubleManager->addProperty(propertyName);
                    doubleManager->setValue(property, metaProperty.read(drawer).toDouble());
                    doubleManager->setMinimum(property, drawer->minimumValue(metaProperty).toDouble());
                    doubleManager->setMaximum(property, drawer->maximumValue(metaProperty).toDouble());
                }
                break;
            case QVariant::String:
                {
                    QStringList l = drawer->stringNames(metaProperty).toStringList();
                    if (l.count())
                    {
                        if (!enumManager || !enumFactory)
                        {
                            enumManager = new QtEnumPropertyManager(browser);
                            enumFactory = new KEnumEditorFactory(browser);
                            browser->setFactoryForManager(enumManager, enumFactory);
                        }
                        property = enumManager->addProperty(propertyName);
                        enumManager->setEnumNames(property, l);
                        enumManager->setValue(property, l.indexOf( metaProperty.read(drawer).toString() ));
                        break;
                    }
                }
            default:
                {
                    if (!variantManager || !variantFactory)
                    {
                        variantManager = new QtVariantPropertyManager(browser);
                        variantFactory = new KVariantEditorFactory(browser);
                        browser->setFactoryForManager(variantManager, variantFactory);
                    }
                    property = variantManager->addProperty(metaProperty.type(), propertyName);
                    variantManager->setValue(property, metaProperty.read(drawer));
                    foreach(QtProperty* p, property->subProperties())
                        p->setEnabled(false);
                }
        }
        browser->addProperty(property);
    }

    if (integerManager)
    {
        connect(integerFactory, SIGNAL(editingFinished()), listener, SLOT(editingFinished()));
        connect(integerManager, SIGNAL(propertyChanged(QtProperty*)), listener, SLOT(propertyChanged(QtProperty*)));
    }
    if (doubleManager)
    {
        connect(doubleFactory, SIGNAL(editingFinished()), listener, SLOT(editingFinished()));
        connect(doubleManager, SIGNAL(propertyChanged(QtProperty*)), listener, SLOT(propertyChanged(QtProperty*)));
    }
    if (enumManager)
    {
        connect(enumFactory, SIGNAL(editingFinished()), listener, SLOT(editingFinished()));
        connect(enumManager, SIGNAL(propertyChanged(QtProperty*)), listener, SLOT(propertyChanged(QtProperty*)));
    }
    if (variantManager)
    {
        connect(variantFactory, SIGNAL(editingFinished()), listener, SLOT(editingFinished()));
        connect(variantManager, SIGNAL(propertyChanged(QtProperty*)), listener, SLOT(propertyChanged(QtProperty*)));
    }

    return browser;
}
