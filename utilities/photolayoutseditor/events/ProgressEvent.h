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

#ifndef PROGRESSEVENT_H
#define PROGRESSEVENT_H

#include <QEvent>
#include <QVariant>

namespace PhotoLayoutsEditor
{
    class ProgressEvent : public QEvent
    {
        public:

            enum Type
            {
                Unknown,
                Init,
                ProgressUpdate,
                ActionUpdate,
                Finish
            };

            explicit ProgressEvent(QObject * sender) :
                QEvent(ProgressEvent::registeredEventType()),
                m_type(Unknown),
                m_sender(sender)
            {
            }

            void setData(ProgressEvent::Type type, QVariant data)
            {
                this->m_type = type;
                this->m_data = data;
            }

            ProgressEvent::Type type() const
            {
                return this->m_type;
            }

            QVariant data() const
            {
                return this->m_data;
            }

            QObject * sender() const
            {
                return m_sender;
            }

            static QEvent::Type registeredEventType()
            {
                static QEvent::Type myType = static_cast<QEvent::Type>(QEvent::registerEventType());
                return myType;
            }

        private:

            ProgressEvent::Type m_type;
            QVariant m_data;
            QObject * m_sender;

            Q_DISABLE_COPY(ProgressEvent)
    };
}

#endif // PROGRESSEVENT_H
