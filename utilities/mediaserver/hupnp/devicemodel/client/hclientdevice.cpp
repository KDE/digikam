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

#include "hclientdevice.h"
#include "hclientdevice_p.h"
#include "hdefault_clientdevice_p.h"
#include "hdefault_clientservice_p.h"

#include "../../general/hlogger_p.h"
#include "../../general/hupnp_global_p.h"

#include "../../dataelements/hserviceid.h"
#include "../../dataelements/hdeviceinfo.h"
#include "../../dataelements/hserviceinfo.h"

#include <QtCore/QTimer>
#include <QtCore/QString>

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HClientDevicePrivate
 ******************************************************************************/

/*******************************************************************************
 * HClientDevice
 ******************************************************************************/
HClientDevice::HClientDevice(
    const HDeviceInfo& info, HClientDevice* parentDev) :
        QObject(parentDev),
            h_ptr(new HClientDevicePrivate())
{
    h_ptr->m_parentDevice = parentDev;
    h_ptr->m_deviceInfo.reset(new HDeviceInfo(info));
    h_ptr->q_ptr = this;
}

HClientDevice::~HClientDevice()
{
    delete h_ptr;
}

HClientDevice* HClientDevice::parentDevice() const
{
    return h_ptr->m_parentDevice;
}

HClientDevice* HClientDevice::rootDevice() const
{
    return h_ptr->rootDevice();
}

HClientService* HClientDevice::serviceById(const HServiceId& serviceId) const
{
    return h_ptr->serviceById(serviceId);
}

HClientServices HClientDevice::services() const
{
    return h_ptr->m_services;
}

HClientServices HClientDevice::servicesByType(
    const HResourceType& type, HResourceType::VersionMatch versionMatch) const
{
    return h_ptr->servicesByType(type, versionMatch);
}

HClientDevices HClientDevice::embeddedDevices() const
{
    return h_ptr->m_embeddedDevices;
}

HClientDevices HClientDevice::embeddedDevicesByType(
    const HResourceType& type, HResourceType::VersionMatch versionMatch) const
{
    return h_ptr->embeddedDevicesByType(type, versionMatch);
}

const HDeviceInfo& HClientDevice::info() const
{
    return *h_ptr->m_deviceInfo;
}

QString HClientDevice::description() const
{
    return h_ptr->m_deviceDescription;
}

QList<QUrl> HClientDevice::locations(LocationUrlType urlType) const
{
    if (h_ptr->m_parentDevice)
    {
        // the root device "defines" the locations and they are the same for each
        // embedded device.
        return h_ptr->m_parentDevice->locations(urlType);
    }

    QList<QUrl> retVal;
    QList<QUrl>::iterator ci;
    for(ci = h_ptr->m_locations.begin(); ci != h_ptr->m_locations.end(); ++ci)
    {
        retVal.push_back(urlType == AbsoluteUrl ? *ci : extractBaseUrl(*ci));
    }

    return retVal;
}

}
}
