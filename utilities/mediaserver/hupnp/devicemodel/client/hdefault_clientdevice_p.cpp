

#include "hclientdevice.h"
#include "hclientdevice_p.h"
#include "hdefault_clientdevice_p.h"
#include "hdefault_clientservice_p.h"

#include "hlogger_p.h"
#include "hupnp_global_p.h"

#include "hserviceid.h"
#include "hdeviceinfo.h"
#include "hserviceinfo.h"

#include <QtCore/QTimer>
#include <QtCore/QString>


namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HDefaultClientDevice
 ******************************************************************************/
HDefaultClientDevice::HDefaultClientDevice(
    const QString& description,
    const QList<QUrl>& locations,
    const HDeviceInfo& info,
    qint32 deviceTimeoutInSecs,
    HDefaultClientDevice* parentDev) :
        HClientDevice(info, parentDev),
            m_timedout(false),
            m_statusNotifier(new QTimer(this)),
            m_deviceStatus(new HDeviceStatus()),
            m_configId(0)
{
    h_ptr->m_deviceDescription = description;
    h_ptr->m_locations = locations;

    m_statusNotifier->setInterval(deviceTimeoutInSecs * 1000);
    bool ok = connect(
        m_statusNotifier.data(), SIGNAL(timeout()), this, SLOT(timeout_()));

    Q_ASSERT(ok); Q_UNUSED(ok)
}

void HDefaultClientDevice::setServices(
    const QList<HDefaultClientService*>& services)
{
    h_ptr->m_services.clear();
    foreach(HDefaultClientService* srv, services)
    {
        h_ptr->m_services.append(srv);
    }
}

void HDefaultClientDevice::setEmbeddedDevices(
    const QList<HDefaultClientDevice*>& devices)
{
    h_ptr->m_embeddedDevices.clear();
    foreach(HDefaultClientDevice* dev, devices)
    {
        h_ptr->m_embeddedDevices.append(dev);
    }
}

quint32 HDefaultClientDevice::deviceTimeoutInSecs() const
{
    return m_statusNotifier->interval() / 1000;
}

void HDefaultClientDevice::timeout_()
{
    HLOG(H_AT, H_FUN);

    m_timedout = true;
    stopStatusNotifier(HDefaultClientDevice::ThisOnly);

    emit statusTimeout(this);
}

void HDefaultClientDevice::startStatusNotifier(SearchCriteria searchCriteria)
{
    HLOG(H_AT, H_FUN);

    m_statusNotifier->start();
    if (searchCriteria & Services)
    {
        // TODO
    }
    if (searchCriteria & EmbeddedDevices)
    {
        foreach(HClientDevice* dc, h_ptr->m_embeddedDevices)
        {
            static_cast<HDefaultClientDevice*>(dc)->startStatusNotifier(searchCriteria);
        }
    }

    m_timedout = false;
}

void HDefaultClientDevice::stopStatusNotifier(SearchCriteria searchCriteria)
{
    HLOG(H_AT, H_FUN);

    m_statusNotifier->stop();
    if (searchCriteria & Services)
    {
        // TODO
    }
    if (searchCriteria & EmbeddedDevices)
    {
        foreach(HClientDevice* dc, h_ptr->m_embeddedDevices)
        {
            static_cast<HDefaultClientDevice*>(dc)->stopStatusNotifier(searchCriteria);
        }
    }
}

bool HDefaultClientDevice::isTimedout(SearchCriteria searchCriteria) const
{
    if (m_timedout)
    {
        return true;
    }

    if (searchCriteria & Services)
    {
        // TODO
    }

    if (searchCriteria & EmbeddedDevices)
    {
        foreach(HClientDevice* dc, h_ptr->m_embeddedDevices)
        {
            if (static_cast<HDefaultClientDevice*>(dc)->isTimedout(searchCriteria))
            {
                return true;
            }
        }
    }

    return false;
}

namespace
{
bool shouldAdd(const HClientDevice* device, const QUrl& location)
{
    Q_ASSERT(!device->parentDevice());
    // embedded devices always query the parent device for locations

    QList<QUrl> locations = device->locations();
    QList<QUrl>::const_iterator ci = locations.constBegin();

    for(; ci != locations.constEnd(); ++ci)
    {
        if ((*ci).host() == location.host())
        {
            return false;
        }
    }

    return true;
}
}

bool HDefaultClientDevice::addLocation(const QUrl& location)
{
    if (shouldAdd(this, location))
    {
        h_ptr->m_locations.push_back(location);
        emit locationsChanged();
        return true;
    }

    return false;
}

void HDefaultClientDevice::addLocations(const QList<QUrl>& locations)
{
    QList<QUrl>::const_iterator ci = locations.constBegin();
    for(; ci != locations.constEnd(); ++ci)
    {
        addLocation(*ci);
    }
}

void HDefaultClientDevice::clearLocations()
{
    Q_ASSERT(!parentDevice());
    h_ptr->m_locations.clear();
    emit locationsChanged();
}

HDefaultClientDevice* HDefaultClientDevice::rootDevice() const
{
    return static_cast<HDefaultClientDevice*>(HClientDevice::rootDevice());
}

}}
