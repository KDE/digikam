/*
 *  Copyright (C) 2010, 2011 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of Herqq UPnP (HUPnP) library.
 *
 *  Herqq UPnP is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Herqq UPnP is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with Herqq UPnP. If not, see <http://www.gnu.org/licenses/>.
 */

#include "hdevicehost.h"
#include "hdevicehost_p.h"
#include "hevent_notifier_p.h"
#include "hpresence_announcer_p.h"
#include "hdevicehost_configuration.h"
#include "hserverdevicecontroller_p.h"
#include "hdevicehost_http_server_p.h"
#include "hdevicehost_ssdp_handler_p.h"
#include "hdevicehost_runtimestatus_p.h"
#include "hdevicehost_dataretriever_p.h"

#include "hservermodel_creator_p.h"

#include "../../general/hlogger_p.h"
#include "../../utils/hsysutils_p.h"

#include <ctime>

#include <QtCore/QTimer>

namespace Herqq
{

namespace Upnp
{

    /*******************************************************************************
     * HServerDeviceController
     ******************************************************************************/
    HServerDeviceController::HServerDeviceController(
        HServerDevice* device, qint32 deviceTimeoutInSecs, QObject* parent) :
            QObject(parent),
                m_statusNotifier(new QTimer(this)),
                m_deviceStatus(new HDeviceStatus()),
                    m_device(device)
    {
        Q_ASSERT(m_device);
        //m_device->setParent(this);

        m_statusNotifier->setInterval(deviceTimeoutInSecs * 1000);
        bool ok = connect(
            m_statusNotifier.data(), SIGNAL(timeout()), this, SLOT(timeout_()));

        Q_ASSERT(ok); Q_UNUSED(ok)
    }

    HServerDeviceController::~HServerDeviceController()
    {
    }

    qint32 HServerDeviceController::deviceTimeoutInSecs() const
    {
        return m_statusNotifier->interval() / 1000;
    }

    void HServerDeviceController::timeout_()
    {
        HLOG(H_AT, H_FUN);

        m_timedout = true;
        stopStatusNotifier();

        emit statusTimeout(this);
    }

    void HServerDeviceController::startStatusNotifier()
    {
        HLOG(H_AT, H_FUN);
        m_statusNotifier->start();
        m_timedout = false;
    }

    void HServerDeviceController::stopStatusNotifier()
    {
        HLOG(H_AT, H_FUN);
        m_statusNotifier->stop();
    }

    bool HServerDeviceController::isTimedout() const
    {
        return m_timedout;
    }
}
}
