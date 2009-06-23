/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-20
 * Description : metadata template manager.
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef TEMPLATEMANAGER_H
#define TEMPLATEMANAGER_H

// Qt includes

#include <QList>
#include <QObject>

class QString;
class QDateTime;

class KAction;

namespace Digikam
{

class Photographer;
class TemplateManagerPrivate;

class TemplateManager : public QObject
{
    Q_OBJECT

public:

    TemplateManager(QObject *parent, const QString& file);
    ~TemplateManager();

    bool load();
    bool save();
    void clear();

    void insert(Photographer* photographer);
    void remove(Photographer* photographer);

    Photographer* find(const QString& author) const;

    QList<Photographer*>* templateList();

    static TemplateManager* defaultManager();

Q_SIGNALS:

    void signalPhotographerAdded(Photographer*);
    void signalPhotographerRemoved(Photographer*);

private:

    void insertPrivate(Photographer* photographer);
    void removePrivate(Photographer* photographer);

private:

    static TemplateManager* m_defaultManager;
    TemplateManagerPrivate* const d;
};

}  // namespace Digikam

#endif /* TEMPLATEMANAGER_H */
