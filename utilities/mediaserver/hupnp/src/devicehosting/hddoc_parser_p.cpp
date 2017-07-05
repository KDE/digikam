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

#include "hddoc_parser_p.h"

#include "../dataelements/hudn.h"
#include "../dataelements/hdeviceinfo.h"
#include "../dataelements/hresourcetype.h"

#include "../general/hupnp_global_p.h"
#include "../general/hupnp_datatypes_p.h"

#include "../general/hlogger_p.h"

namespace Herqq
{

namespace Upnp
{

/*******************************************************************************
 * HDocParser
 ******************************************************************************/
HDocParser::HDocParser(
    const QByteArray& loggingIdentifier, HValidityCheckLevel clevel) :
        m_loggingIdentifier(loggingIdentifier), m_cLevel(clevel),
        m_lastErrorDescription(), m_lastError(NoError)
{
}

HStateVariableInfo HDocParser::parseStateVariableInfo_str(
    const QString& name, const QVariant& defValue, const QDomElement& svElement,
    HStateVariableInfo::EventingType evType, HInclusionRequirement incReq)
{
    //HLOG2(H_AT, H_FUN, QLatin1String(m_loggingIdentifier));

    QStringList allowedValues;

    QDomElement allowedValueListElement =
        svElement.firstChildElement(QLatin1String("allowedValueList"));

    if (!allowedValueListElement.isNull())
    {
        QDomElement allowedValueElement =
            allowedValueListElement.firstChildElement(QLatin1String("allowedValue"));

        while(!allowedValueElement.isNull())
        {
            allowedValues.push_back(allowedValueElement.text());

            allowedValueElement =
                allowedValueElement.nextSiblingElement(QLatin1String("allowedValue"));
        }
    }

    return HStateVariableInfo(
        name, defValue, allowedValues, evType, incReq, &m_lastErrorDescription);
}

HStateVariableInfo HDocParser::parseStateVariableInfo_numeric(
    const QString& name, const QVariant& defValue, const QDomElement& svElement,
    HStateVariableInfo::EventingType evType, HInclusionRequirement incReq,
    HUpnpDataTypes::DataType dataTypeEnumValue)
{
   // HLOG2(H_AT, H_FUN, QLatin1String(m_loggingIdentifier));

    QDomElement allowedValueRangeElement =
        svElement.firstChildElement(QLatin1String("allowedValueRange"));

    if (allowedValueRangeElement.isNull())
    {
        return HStateVariableInfo(
            name, dataTypeEnumValue, defValue, evType, incReq, &m_lastErrorDescription);
    }

    QString minimumStr = readElementValue(QLatin1String("minimum"), allowedValueRangeElement);

    if (minimumStr.isEmpty())
    {
        QString descr = QString(QLatin1String(
            "State variable [%1] is missing a mandatory <minimum> element "
            "within <allowedValueRange>.")).arg(name);

        if (m_cLevel == StrictChecks)
        {
            m_lastError = InvalidServiceDescriptionError;
            m_lastErrorDescription = descr;
            return HStateVariableInfo();
        }
        else
        {
            HLOG_WARN_NONSTD(descr);
            minimumStr = QString::number(INT_MIN);
        }
    }

    QString maximumStr = readElementValue(QLatin1String("maximum"), allowedValueRangeElement);

    if (maximumStr.isEmpty())
    {
        QString descr = QString(QLatin1String(
            "State variable [%1] is missing a mandatory <maximum> element "
            "within <allowedValueRange>.")).arg(name);

        if (m_cLevel == StrictChecks)
        {
            m_lastError = InvalidServiceDescriptionError;
            m_lastErrorDescription = descr;
            return HStateVariableInfo();
        }
        else
        {
            HLOG_WARN_NONSTD(descr);
            maximumStr = QString::number(INT_MAX);
        }
    }

    QString stepStr = readElementValue(QLatin1String("step"), allowedValueRangeElement);

    if (stepStr.isEmpty())
    {
        if (HUpnpDataTypes::isRational(dataTypeEnumValue))
        {
            bool ok = false;
            double maxTmp = maximumStr.toDouble(&ok);
            if (ok && maxTmp < 1)
            {
                stepStr = QString::number(maxTmp / 10);
            }
            else
            {
                stepStr = QLatin1String("1.0");
            }
        }
        else
        {
            stepStr =QLatin1String("1");
        }
    }

    return HStateVariableInfo(
        name, dataTypeEnumValue, defValue, minimumStr, maximumStr, stepStr,
        evType, incReq, &m_lastErrorDescription);
}

bool HDocParser::parseActionArguments(
    const QDomElement& argListElement,
    const QHash<QString, HStateVariableInfo>& stateVars,
    QVector<HActionArgument>* inArgs,
    QVector<HActionArgument>* outArgs,
    bool* hasRetVal)
{
    //HLOG2(H_AT, H_FUN, QLatin1String(m_loggingIdentifier));

    bool firstOutArgFound  = false;

    QDomElement argumentElement = argListElement.firstChildElement(QLatin1String("argument"));
    while(!argumentElement.isNull())
    {
        QString name = readElementValue(QLatin1String("name"), argumentElement);
        QString dirStr = readElementValue(QLatin1String("direction"), argumentElement);

        bool retValWasDefined = false;
        readElementValue(QLatin1String("retval"), argumentElement, &retValWasDefined);

        QString relatedSvStr =
            readElementValue(QLatin1String("relatedStateVariable"), argumentElement);

        if (!stateVars.contains(relatedSvStr))
        {
            m_lastError = InvalidServiceDescriptionError;
            m_lastErrorDescription = QString(QLatin1String(
                "Invalid action argument: the specified <relatedStateVariable> "
                "[%1] is undefined.")).arg(relatedSvStr);

            return false;
        }

        HActionArgument createdArg;
        if (dirStr.compare(QLatin1String("out"), Qt::CaseInsensitive) == 0)
        {
            if (retValWasDefined)
            {
                if (firstOutArgFound)
                {
                    m_lastError = InvalidServiceDescriptionError;
                    m_lastErrorDescription = QString(QLatin1String(
                        "Invalid action argument ordering: "
                        "[retval] MUST be the first [out] argument."));

                    return false;
                }

                *hasRetVal = true;
            }

            firstOutArgFound = true;

            createdArg = HActionArgument(
                name, stateVars.value(relatedSvStr), &m_lastErrorDescription);

            if (!createdArg.isValid())
            {
                m_lastError = InvalidServiceDescriptionError;
                m_lastErrorDescription = QString(QLatin1String(
                    "Invalid action argument: %1")).arg(m_lastErrorDescription);

                return false;
            }

            outArgs->push_back(createdArg);
        }
        else if (dirStr.compare(QLatin1String("in"), Qt::CaseInsensitive) == 0)
        {
            if (firstOutArgFound)
            {
                m_lastError = InvalidServiceDescriptionError;
                m_lastErrorDescription =QLatin1String(
                    "Invalid action argument order. Input arguments MUST all come "
                    "before output arguments.");

                return false;
            }

            createdArg = HActionArgument(name, stateVars.value(relatedSvStr));

            if (!createdArg.isValid())
            {
                m_lastError = InvalidServiceDescriptionError;
                m_lastErrorDescription = QString(QLatin1String(
                    "Invalid action argument: %1")).arg(m_lastErrorDescription);

                return false;
            }

            inArgs->push_back(createdArg);
        }
        else
        {
            m_lastError = InvalidServiceDescriptionError;
            m_lastErrorDescription = QString(QLatin1String(
                "Invalid action argument: "
                "invalid [direction] value: [%1].")).arg(dirStr);

            return false;
        }

        argumentElement = argumentElement.nextSiblingElement(QLatin1String("argument"));
    }

    return true;
}

QList<QUrl> HDocParser::parseIconList(const QDomElement& iconListElement)
{
    //HLOG2(H_AT, H_FUN, QLatin1String(m_loggingIdentifier));

    QList<QUrl> retVal;

    QDomElement iconElement = iconListElement.firstChildElement(QLatin1String("icon"));
    while(!iconElement.isNull())
    {
        QUrl iconUrl = QUrl(readElementValue(QLatin1String("url"), iconElement));

        QString iconUrlAsStr = iconUrl.toString();
        retVal.append(QUrl(iconUrlAsStr));

        iconElement = iconElement.nextSiblingElement(QLatin1String("icon"));
    }

    return retVal;
}

bool HDocParser::parseRoot(
    const QString& docStr, QDomDocument* doc, QDomElement* rootEl)
{
    //HLOG2(H_AT, H_FUN, QLatin1String(m_loggingIdentifier));

    Q_ASSERT(doc);
    Q_ASSERT(rootEl);

    QString errMsg; qint32 errLine = 0;
    if (!doc->setContent(docStr, false, &errMsg, &errLine))
    {
        m_lastError = InvalidDeviceDescriptionError;
        m_lastErrorDescription = QString(QLatin1String(
            "Failed to parse the device description: [%1] @ line [%2].")).arg(
                errMsg, QString::number(errLine));

        return false;
    }

    QDomElement rootElement = doc->firstChildElement(QLatin1String("root"));

    // "urn:schemas-upnp-org:device-1-0",

    if (rootElement.isNull())
    {
        m_lastError = InvalidDeviceDescriptionError;
        m_lastErrorDescription =QLatin1String(
            "Invalid device description: missing <root> element.");

        return false;
    }

    if (!verifySpecVersion(rootElement, &m_lastErrorDescription))
    {
        if (m_cLevel == StrictChecks)
        {
            m_lastError = InvalidDeviceDescriptionError;
            return false;
        }
        else
        {
            HLOG_WARN_NONSTD(QString(QLatin1String(
                "Error in device description: %1")).arg(m_lastErrorDescription));
        }
    }

    QDomElement rootDeviceElement = rootElement.firstChildElement(QLatin1String("device"));
    if (rootDeviceElement.isNull())
    {
        m_lastError = InvalidDeviceDescriptionError;
        m_lastErrorDescription =QLatin1String(
            "Invalid device description: no valid root device definition "
            "was found.");

        return false;
    }

    *rootEl = rootDeviceElement;
    return true;
}

qint32 HDocParser::readConfigId(const QDomElement& rootElement)
{
   // HLOG2(H_AT, H_FUN, QLatin1String(m_loggingIdentifier));

    bool ok = false;

    QString cid = readElementValue(QLatin1String("configId"), rootElement);
    qint32 retVal = cid.toInt(&ok);
    if (!ok || retVal < 0 || retVal > ((1 << 24)-1))
    {
        return 0;
    }

    return retVal;
}

bool HDocParser::parseDeviceInfo(
    const QDomElement& deviceElement, HDeviceInfo* info)
{
   // HLOG2(H_AT, H_FUN,QLatin1String( m_loggingIdentifier));
    Q_ASSERT(info);

    QString deviceType       =
        readElementValue(QLatin1String("deviceType")      , deviceElement);

    QString friendlyName     =
        readElementValue(QLatin1String("friendlyName")    , deviceElement);

    QString manufacturer     =
        readElementValue(QLatin1String("manufacturer")    , deviceElement);

    QString manufacturerURL  =
        readElementValue(QLatin1String("manufacturerURL") , deviceElement);

    QString modelDescription =
        readElementValue(QLatin1String("modelDescription"), deviceElement);

    QString modelName        =
        readElementValue(QLatin1String("modelName")       , deviceElement);

    QString modelNumber      =
        readElementValue(QLatin1String("modelNumber")     , deviceElement);

    QUrl    modelUrl         =QUrl(
        readElementValue(QLatin1String("modelURL")        , deviceElement));

    QString serialNumber     =
        readElementValue(QLatin1String("serialNumber")    , deviceElement);

    HUdn udn(readElementValue(QLatin1String("UDN")        , deviceElement));

    QString upc              =
        readElementValue(QLatin1String("UPC")             , deviceElement);

    QDomElement iconListElement = deviceElement.firstChildElement(QLatin1String("iconList"));
    QList<QUrl> icons;
    if (!iconListElement.isNull())
    {
        icons = parseIconList(iconListElement);
    }

    bool wasDefined = false;

    QString tmp = readElementValue(QLatin1String("presentationURL"), deviceElement, &wasDefined);

    if (wasDefined && tmp.isEmpty())
    {
        QString err = QLatin1String("Presentation URL should be defined if the "
                      "corresponding element is used.");

        if (m_cLevel == StrictChecks)
        {
            m_lastError = InvalidDeviceDescriptionError;
            m_lastErrorDescription = err;
            return false;
        }
        else
        {
            HLOG_WARN(QString(QLatin1String("Non-critical error in a device description: %1")).arg(err));
        }
    }

    QUrl presentationUrl(tmp);

    *info = HDeviceInfo(
        HResourceType(deviceType),
        friendlyName,
        manufacturer,
        QUrl(manufacturerURL),
        modelDescription,
        modelName,
        modelNumber,
        modelUrl,
        serialNumber,
        udn,
        upc,
        icons,
        presentationUrl,
        m_cLevel,
        &m_lastErrorDescription);

    if (!info->isValid(m_cLevel))
    {
        m_lastError = InvalidDeviceDescriptionError;
        m_lastErrorDescription = QString(QLatin1String(
            "Invalid device description: %1")).arg(m_lastErrorDescription);

        return false;
    }

    return true;
}

bool HDocParser::parseServiceInfo(
    const QDomElement& serviceDefinition, HServiceInfo* serviceInfo)
{
    //HLOG2(H_AT, H_FUN, QLatin1String(m_loggingIdentifier));

    Q_ASSERT(serviceInfo);
    Q_ASSERT(!serviceDefinition.isNull());

    bool wasDefined = false;

    HServiceId serviceId =
        readElementValue(QLatin1String("serviceId"), serviceDefinition, &wasDefined);

    if (!wasDefined)
    {
        m_lastError = InvalidDeviceDescriptionError;
        m_lastErrorDescription = QString(QLatin1String(
            "Invalid <service> definition. "
            "Missing mandatory <serviceId> element:\n%1")).arg(
                toString(serviceDefinition));

        return false;
    }

    HResourceType resourceType =
        readElementValue(QLatin1String("serviceType"), serviceDefinition, &wasDefined);

    if (!wasDefined)
    {
        m_lastError = InvalidDeviceDescriptionError;
        m_lastErrorDescription = QString(QLatin1String(
            "Invalid <service> definition. "
            "Missing mandatory <serviceType> element:\n%1")).arg(
                toString(serviceDefinition));

        return false;
    }

    QUrl scpdUrl = QUrl(readElementValue(QLatin1String("SCPDURL"), serviceDefinition, &wasDefined));
    if (!wasDefined)
    {
        m_lastError = InvalidDeviceDescriptionError;
        m_lastErrorDescription = QString(QLatin1String(
            "Invalid <service> definition. "
            "Missing mandatory <SCPDURL> element:\n%1")).arg(
                toString(serviceDefinition));

        return false;
    }

    QUrl controlUrl =
        QUrl(readElementValue(QLatin1String("controlURL") , serviceDefinition, &wasDefined));

    if (!wasDefined)
    {
        m_lastError = InvalidDeviceDescriptionError;
        m_lastErrorDescription = QString(QLatin1String(
            "Invalid <service> definition. "
            "Missing mandatory <controlURL> element:\n%1")).arg(
                toString(serviceDefinition));

        return false;
    }

    QUrl eventSubUrl =
        QUrl(readElementValue(QLatin1String("eventSubURL"), serviceDefinition, &wasDefined));

    if (!wasDefined)
    {
        m_lastError = InvalidDeviceDescriptionError;
        m_lastErrorDescription = QString(QLatin1String(
            "Invalid <service> definition. "
            "Missing mandatory <eventSubURL> element:\n%1")).arg(
                toString(serviceDefinition));

        return false;
    }

    HServiceInfo tmpServiceInfo(
        serviceId, resourceType, controlUrl, eventSubUrl, scpdUrl,
        InclusionMandatory, m_cLevel, &m_lastErrorDescription);

    if (!tmpServiceInfo.isValid(m_cLevel))
    {
        m_lastError = InvalidDeviceDescriptionError;
        m_lastErrorDescription =
            QString(QLatin1String("%1:\n%2")).arg(
                m_lastErrorDescription, toString(serviceDefinition));

        return false;
    }

    *serviceInfo = tmpServiceInfo;
    return true;
}

bool HDocParser::parseServiceDescription(
    const QString& docStr, QDomDocument* doc,
    QDomElement* stateVarElement, QDomElement* retVal)
{
    //HLOG2(H_AT, H_FUN, QLatin1String(m_loggingIdentifier));

    Q_ASSERT(stateVarElement);
    Q_ASSERT(retVal);

    qint32 errLine;
    QString errMsg;
    if (!doc->setContent(docStr, false, &errMsg, &errLine))
    {
        m_lastError = InvalidServiceDescriptionError;
        m_lastErrorDescription = QString(QLatin1String(
            "Failed to parse the service description: [%1] @ line [%2].")).arg(
                errMsg, QString::number(errLine));

        return false;
    }
    //QDomNodeList scpdElementNodeList =
        //tmp.elementsByTagNameNS("urn:schemas-upnp-org:service-1-0","scpd");

    QDomElement scpdElement = doc->firstChildElement(QLatin1String("scpd"));
    if (scpdElement.isNull())
    {
        m_lastError = InvalidServiceDescriptionError;
        m_lastErrorDescription =QLatin1String(
            "Invalid service description: missing <scpd> element.");

        return false;
    }

    if (!verifySpecVersion(scpdElement, &m_lastErrorDescription))
    {
        if (m_cLevel == StrictChecks)
        {
            m_lastError = InvalidServiceDescriptionError;
            return false;
        }
        else
        {
            HLOG_WARN_NONSTD(QString(QLatin1String("Error in service description: %1")).arg(
                m_lastErrorDescription));
        }
    }

    QDomElement serviceStateTableElement =
        scpdElement.firstChildElement(QLatin1String("serviceStateTable"));

    if (serviceStateTableElement.isNull())
    {
        m_lastError = InvalidServiceDescriptionError;
        m_lastErrorDescription =QLatin1String(
            "Service description is missing a mandatory <serviceStateTable> element.");

        return false;
    }

    QDomElement stateVariableElement =
        serviceStateTableElement.firstChildElement(QLatin1String("stateVariable"));

    if (stateVariableElement.isNull())
    {
        QString err = QLatin1String("Service description document does not have a "
                      "single <stateVariable> element. "
                      "Each service MUST have at least one state variable.");

        if (m_cLevel == StrictChecks)
        {
            m_lastError = InvalidServiceDescriptionError;
            m_lastErrorDescription = err;
            return false;
        }
        else
        {
            HLOG_WARN_NONSTD(err);
        }
    }

    QDomElement actionListElement = scpdElement.firstChildElement(QLatin1String("actionList"));
    if (actionListElement.isNull())
    {
        return true;
    }

    QDomElement actionElement = actionListElement.firstChildElement(QLatin1String("action"));
    if (actionElement.isNull())
    {
        QString err = QLatin1String("Service description document has <actionList> "
                      "element that has no <action> elements.");

        if (m_cLevel == StrictChecks)
        {
            m_lastError = InvalidServiceDescriptionError;
            m_lastErrorDescription = err;
            return false;
        }
        else
        {
            HLOG_WARN(err);
        }
    }

    *stateVarElement = stateVariableElement;
    *retVal = actionElement;
    return true;
}

bool HDocParser::parseStateVariable(
    const QDomElement& stateVariableElement, HStateVariableInfo* svInfo)
{
    //HLOG2(H_AT, H_FUN, QLatin1String(m_loggingIdentifier));

    Q_ASSERT(svInfo);

    QString strSendEvents = stateVariableElement.attribute(QLatin1String("sendEvents"), QLatin1String("no"));
    bool bSendEvents      = false;
    if (strSendEvents.compare(QLatin1String("yes"), Qt::CaseInsensitive) == 0)
    {
        bSendEvents = true;
    }
    else if (strSendEvents.compare(QLatin1String("no"), Qt::CaseInsensitive) != 0)
    {
        m_lastError = InvalidServiceDescriptionError;
        m_lastErrorDescription = QString(QLatin1String(
            "Invalid <stateVariable> definition: "
            "invalid value for [sendEvents] attribute:\n%1.")).arg(
                toString(stateVariableElement));

        return false;
    }

    QString strMulticast  = stateVariableElement.attribute(QLatin1String("multicast"), QLatin1String("no"));
    bool bMulticast       = false;
    if (strMulticast.compare(QLatin1String("yes"), Qt::CaseInsensitive) == 0)
    {
        bMulticast = true;
    }
    else if (strMulticast.compare(QLatin1String("no"), Qt::CaseInsensitive) != 0)
    {
        m_lastError = InvalidServiceDescriptionError;
        m_lastErrorDescription = QString(QLatin1String(
            "Invalid <stateVariable> definition: "
            "invalid value for [multicast]: %1.")).arg(
                toString(stateVariableElement));

        return false;
    }

    HStateVariableInfo::EventingType evType = HStateVariableInfo::NoEvents;
    if (bSendEvents)
    {
        evType = bMulticast ?
            HStateVariableInfo::UnicastAndMulticast : HStateVariableInfo::UnicastOnly;
    }

    QString name = readElementValue(QLatin1String("name"), stateVariableElement);
    QString dataType = readElementValue(QLatin1String("dataType"), stateVariableElement);

    HUpnpDataTypes::DataType dtEnumValue = HUpnpDataTypes::dataType(dataType);

    bool defValueWasDefined = false;
    QString defaultValueStr = readElementValue(
        QLatin1String("defaultValue"), stateVariableElement, &defValueWasDefined);

    QVariant defaultValue =
        defValueWasDefined ?
            HUpnpDataTypes::convertToRightVariantType(
                defaultValueStr, dtEnumValue) : QVariant();

    HStateVariableInfo parsedInfo;

    if (dtEnumValue == HUpnpDataTypes::string)
    {
        parsedInfo = parseStateVariableInfo_str(
            name,
            defValueWasDefined ? defaultValueStr : QVariant(),
            stateVariableElement,
            evType,
            InclusionMandatory);
    }
    else if (HUpnpDataTypes::isNumeric(dtEnumValue))
    {
        parsedInfo = parseStateVariableInfo_numeric(
            name,
            defaultValue,
            stateVariableElement,
            evType,
            InclusionMandatory,
            dtEnumValue);
    }
    else
    {
        parsedInfo = HStateVariableInfo(
            name,
            dtEnumValue,
            defaultValue,
            evType,
            InclusionMandatory,
            &m_lastErrorDescription);
    }

    if (!parsedInfo.isValid())
    {
        m_lastError = InvalidServiceDescriptionError;
        m_lastErrorDescription =
            QString(QLatin1String("Invalid <stateVariable> [%1] definition: %2")).arg(
                name, m_lastErrorDescription);

        return false;
    }

    *svInfo = parsedInfo;
    return true;
}

bool HDocParser::parseActionInfo(
    const QDomElement& actionElement,
    const QHash<QString, HStateVariableInfo>& stateVars,
    HActionInfo* ai)
{
    //HLOG2(H_AT, H_FUN, QLatin1String(m_loggingIdentifier));

    QString name = readElementValue(QLatin1String("name"), actionElement);

    bool hasRetVal = false;
    QVector<HActionArgument> inputArguments;
    QVector<HActionArgument> outputArguments;

    QDomElement argumentListElement =
        actionElement.firstChildElement(QLatin1String("argumentList"));

    if (!argumentListElement.isNull())
    {
        if (!parseActionArguments(
                argumentListElement,
                stateVars,
                &inputArguments,
                &outputArguments,
                &hasRetVal))
        {
            m_lastErrorDescription = QString(QLatin1String(
                "Invalid action [%1] definition: %2")).arg(
                    name, m_lastErrorDescription);

            return false;
        }
    }

    HActionArguments inArgs(inputArguments);
    HActionArguments outArgs(outputArguments);

    HActionInfo actionInfo(
        name, inArgs, outArgs, hasRetVal, InclusionMandatory, &m_lastErrorDescription);

    if (!actionInfo.isValid())
    {
        m_lastError = InvalidServiceDescriptionError;
        m_lastErrorDescription = QString(QLatin1String(
            "Invalid <action> [%1] definition: %2")).arg(
                name, m_lastErrorDescription);

        return false;
    }

    *ai = actionInfo;
    return true;
}

bool HDocParser::verifySpecVersion(const QDomElement& rootElement, QString* err)
{
    //HLOG2(H_AT, H_FUN, QLatin1String(m_loggingIdentifier));

    QDomElement specVersionElement = rootElement.firstChildElement(QLatin1String("specVersion"));
    if (specVersionElement.isNull())
    {
        if (err) { *err = QLatin1String("Missing mandatory <specVersion> element."); }
        return false;
    }

    QString minorVersion = readElementValue(QLatin1String("minor"), specVersionElement);
    QString majorVersion = readElementValue(QLatin1String("major"), specVersionElement);

    bool ok;
    qint32 major = majorVersion.toInt(&ok);
    if (!ok || major != 1)
    {
        if (err) { *err = QLatin1String("Major element of <specVersion> is not 1."); }
        return false;
    }

    qint32 minor = minorVersion.toInt(&ok);
    if (!ok || (minor != 1 && minor != 0))
    {
        if (err) { *err = QLatin1String("Minor element of <specVersion> is not 0 or 1."); }
        return false;
    }

    return true;
}

}
}
