/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-09-24
 * Description : a media server to export collections through DLNA.
 *               Implementation inspired on Platinum File Media Server.
 *
 * Copyright (C) 2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "dlnaserverdelegate.h"

// Platinum includes

#include "NptStreams.h"

#include "PltUPnP.h"
#include "PltMediaItem.h"
#include "PltService.h"
#include "PltTaskManager.h"
#include "PltHttpServer.h"
#include "PltDidl.h"
#include "PltVersion.h"
#include "PltMimeType.h"

// Qt includes

#include <QString>
#include <QUrl>
#include <QList>
#include <QMap>
#include <QImage>
#include <QByteArray>
#include <QBuffer>

// Local includes

#include "digikam_debug.h"
#include "previewloadthread.h"
#include "dimg.h"
#include "drawdecoder.h"

NPT_SET_LOCAL_LOGGER("digiKam.media.server.delegate")

namespace Digikam
{

class DLNAMediaServerDelegate::Private
{
public:

    Private() :
        filterUnknownOut(false),
        useCache(false)
    {
    }

    NPT_String                                                          urlRoot;
    NPT_String                                                          fileRoot;
    bool                                                                filterUnknownOut;
    bool                                                                useCache;

    MediaServerMap                                                      map;

    PLT_MediaCache<NPT_Reference<NPT_List<NPT_String> >, NPT_TimeStamp> dirCache;
};

DLNAMediaServerDelegate::DLNAMediaServerDelegate(const char* url_root,
                                                 bool        use_cache)
    : d(new Private)
{
      d->urlRoot          = url_root;
      d->filterUnknownOut = false;
      d->useCache         = use_cache;
}

DLNAMediaServerDelegate::~DLNAMediaServerDelegate()
{
    delete d;
}

void DLNAMediaServerDelegate::addAlbumsOnServer(const MediaServerMap& map)
{
    d->map = map;
}

NPT_Result DLNAMediaServerDelegate::ProcessFileRequest(NPT_HttpRequest&              request,
                                                       const NPT_HttpRequestContext& context,
                                                       NPT_HttpResponse&             response)
{
    NPT_HttpUrlQuery query(request.GetUrl().GetQuery());

    PLT_LOG_HTTP_MESSAGE(NPT_LOG_LEVEL_FINE, "DLNAMediaServerDelegate::ProcessFileRequest:", &request);

    if (request.GetMethod().Compare("GET") && request.GetMethod().Compare("HEAD"))
    {
        response.SetStatus(500, "Internal Server Error");
        return NPT_SUCCESS;
    }

    // Extract file path from url

    NPT_String file_path;
    NPT_CHECK_LABEL_WARNING(ExtractResourcePath(request.GetUrl(), file_path), failure);

    // Serve file

    NPT_CHECK_WARNING(ServeFile(request, context, response, NPT_FilePath::Create(d->fileRoot, file_path)));
    return NPT_SUCCESS;

failure:

    response.SetStatus(404, "File Not Found");
    return NPT_SUCCESS;
}

NPT_Result DLNAMediaServerDelegate::OnBrowseMetadata(PLT_ActionReference&          action,
                                                     const char*                   object_id,
                                                     const char*                   filter,
                                                     NPT_UInt32                    starting_index,
                                                     NPT_UInt32                    requested_count,
                                                     const char*                   sort_criteria,
                                                     const PLT_HttpRequestContext& context)
{
    NPT_COMPILER_UNUSED(sort_criteria);
    NPT_COMPILER_UNUSED(requested_count);
    NPT_COMPILER_UNUSED(starting_index);

    NPT_String didl;
    PLT_MediaObjectReference item;

    // Locate the file from the object ID

    NPT_String filepath;

    if (NPT_FAILED(GetFilePath(object_id, filepath)))
    {
        // error

        qCDebug(DIGIKAM_MEDIASRV_LOG) << "OnBrowseMetadata()" 
                                      << ":: ObjectID not found \""
                                      << object_id << "\"";
        action->SetError(701, "No Such Object.");
        return NPT_FAILURE;
    }

    // build the object didl

    item = BuildFromFilePath(filepath, context, true, false,
                             (NPT_String(filter).Find("ALLIP") != -1));

    if (item.IsNull())
        return NPT_FAILURE;

    NPT_String tmp;
    NPT_CHECK_SEVERE(PLT_Didl::ToDidl(*item.AsPointer(), filter, tmp));

    // add didl header and footer

    didl = didl_header + tmp + didl_footer;

    NPT_CHECK_SEVERE(action->SetArgumentValue("Result",         didl));
    NPT_CHECK_SEVERE(action->SetArgumentValue("NumberReturned", "1"));
    NPT_CHECK_SEVERE(action->SetArgumentValue("TotalMatches",   "1"));

    // update ID may be wrong here, it should be the one of the container?
    // TODO: We need to keep track of the overall updateID of the CDS

    NPT_CHECK_SEVERE(action->SetArgumentValue("UpdateId",       "1"));

    return NPT_SUCCESS;
}

NPT_Result DLNAMediaServerDelegate::OnBrowseDirectChildren(PLT_ActionReference&          action,
                                                           const char*                   object_id,
                                                           const char*                   filter,
                                                           NPT_UInt32                    starting_index,
                                                           NPT_UInt32                    requested_count,
                                                           const char*                   sort_criteria,
                                                           const PLT_HttpRequestContext& context)
{
    NPT_COMPILER_UNUSED(sort_criteria);

    // Locate the file from the object ID

    NPT_String   dir;
    NPT_FileInfo info;

    if (NPT_FAILED(GetFilePath(object_id, dir)))
    {
        // error

        NPT_LOG_WARNING_1("ObjectID \'%s\' not found or not allowed", object_id);
        action->SetError(701, "No such Object");
        NPT_CHECK_WARNING(NPT_FAILURE);
    }

    qCDebug(DIGIKAM_MEDIASRV_LOG) << "OnBrowseDirectChildren() :: Object id:"
                                  << object_id << "Dir:" << dir.GetChars();

    // get uuid from device via action reference

    NPT_String uuid = action->GetActionDesc().GetService()->GetDevice()->GetUUID();

    // Try to get list from cache if allowed

    NPT_Reference<NPT_List<NPT_String> > entries;
    NPT_TimeStamp                        cached_entries_time;

    if (!d->useCache                                                          ||
        NPT_FAILED(d->dirCache.Get(uuid, dir, entries, &cached_entries_time)) ||
        cached_entries_time < info.m_ModificationTime)
    {
        // if not found in cache or if current dir has newer modified time fetch fresh new list from source

        QStringList list;

        if (dir == "/")
        {
            foreach(QString s, d->map.keys())
            {
                list << s + QLatin1Char('/');
            }
        }
        else
        {
            QString container = QString::fromUtf8(dir.GetChars());
            QList<QUrl> urls  = d->map.value(container.remove(QLatin1Char('/')));

            foreach(QUrl u, urls)
            {
                // Internal URL separator between container path and local file path.
                // Ex: Linux => "/country/town/Paris/?file:/mnt/data/travel/Paris/eiffeltower.jpg
                //     Win32 => "/Friends/US/Brown/?file:C:/Users/Foo/My Images/Friends/US/Brown/homer.png
                list << QLatin1String("?file:") + u.toLocalFile();
            }
        }

        qCDebug(DIGIKAM_MEDIASRV_LOG) << "OnBrowseDirectChildren() ::"
                                      << "Populate cache with contents from Dir"
                                      << dir.GetChars();

        entries = new NPT_List<NPT_String>();

        foreach(QString path, list)
        {
            qCDebug(DIGIKAM_MEDIASRV_LOG) << "=>" << path;
            entries->Add(NPT_String(path.toUtf8().data(), path.toUtf8().size()));
        }

        // add new list to cache

        if (d->useCache)
        {
            d->dirCache.Put(uuid, dir, entries, &info.m_ModificationTime);
        }
    }

    unsigned long cur_index     = 0;
    unsigned long num_returned  = 0;
    unsigned long total_matches = 0;
    NPT_String    didl          = didl_header;
    bool          allip         = (NPT_String(filter).Find("ALLIP") != -1);

    PLT_MediaObjectReference item;

    for (NPT_List<NPT_String>::Iterator it = entries->GetFirstItem() ; it ; ++it)
    {
        NPT_String filepath = dir + (*it);

        // verify we want to process this file first

        if (!ProcessFile(filepath, filter))
            continue;

        qCDebug(DIGIKAM_MEDIASRV_LOG) << "OnBrowseDirectChildren()"
                                      << "::  Process item" << filepath.GetChars();

        // build item object from file path

        item = BuildFromFilePath(filepath,
                                 context,
                                 true,
                                 true,
                                 allip);

        // generate didl if within range requested

        if (!item.IsNull())
        {
            if ((cur_index >= starting_index) &&
                ((num_returned < requested_count) || (requested_count == 0)))
            {
                NPT_String tmp;
                NPT_CHECK_SEVERE(PLT_Didl::ToDidl(*item.AsPointer(), filter, tmp));

                didl += tmp;
                ++num_returned;
            }

            ++cur_index;
            ++total_matches;
        }
    };

    didl += didl_footer;

    NPT_LOG_FINE_6("BrowseDirectChildren from %s returning %d-%d/%d objects (%d out of %d requested)",
                   (const char*)context.GetLocalAddress().GetIpAddress().ToString(),
                   starting_index, starting_index + num_returned,
                   total_matches, num_returned, requested_count);

    NPT_CHECK_SEVERE(action->SetArgumentValue("Result",         didl));
    NPT_CHECK_SEVERE(action->SetArgumentValue("NumberReturned", NPT_String::FromInteger(num_returned)));

    // 0 means we don't know how many we have but most browsers don't like that!!
    NPT_CHECK_SEVERE(action->SetArgumentValue("TotalMatches",   NPT_String::FromInteger(total_matches))); 

    NPT_CHECK_SEVERE(action->SetArgumentValue("UpdateId",       "1"));

    return NPT_SUCCESS;
}

PLT_MediaObject* DLNAMediaServerDelegate::BuildFromFilePath(const NPT_String&             filepath,
                                                            const PLT_HttpRequestContext& context,
                                                            bool                          with_count,
                                                            bool                          keep_extension_in_title,
                                                            bool                          allip)
{
    PLT_MediaItemResource resource;
    PLT_MediaObject*      object = NULL;

    qCDebug(DIGIKAM_MEDIASRV_LOG) << "Building didl for file \"" << filepath.GetChars() << "\"";

    // retrieve the entry type (directory or file)

    if (!QString::fromUtf8(filepath.GetChars()).endsWith(QLatin1Char('/')))
    {
        qCDebug(DIGIKAM_MEDIASRV_LOG) << "BuildFromFilePath() :: regular file detected";

        object        = new PLT_MediaItem();

        // Set the title using the filename for now

        QString uri   = QString::fromUtf8(filepath.GetChars());
        int index     = uri.indexOf(QLatin1String("/?file:")) + 7;
        QString path  = uri.remove(0, index);
        QString title = path.section(QLatin1Char('/'), -1);

        if (!keep_extension_in_title)
        {
            title = title.section(QLatin1Char('.'), -2);
        }

        object->m_Title = NPT_String(title.toUtf8().data());

        if (DRawDecoder::isRawFile(QUrl::fromLocalFile(QString::fromUtf8(filepath.GetChars()))))
        {
            // Special case for RAW file where extension need to be patched as JPEG for client renderer,
            // as we provide a JPEG preview.
            object->m_Title += ".jpg";
        }

        if (object->m_Title.GetLength() == 0)
            goto failure;

        // make sure we return something with a valid mimetype

        if (d->filterUnknownOut &&
            NPT_StringsEqual(PLT_MimeType::GetMimeType(filepath, &context),
                             "application/octet-stream"))
        {
            goto failure;
        }

        qCDebug(DIGIKAM_MEDIASRV_LOG) << "BuildFromFilePath() :: Create item as MediaItem \""
                                      << object->m_Title.GetChars() << "\"";

        // Set the protocol Info from the extension

        resource.m_ProtocolInfo = PLT_ProtocolInfo::GetProtocolInfo(filepath, true, &context);

        if (!resource.m_ProtocolInfo.IsValid())
            goto failure;

        // format the resource URI

        NPT_String url  = filepath.SubString(filepath.Find("/?file:") + 7);

        qCDebug(DIGIKAM_MEDIASRV_LOG) << "BuildFromFilePath() :: Item URI:\""
                                      << url.GetChars() << "\"";

        // Set the resource file size

        NPT_FileInfo info;
        NPT_CHECK_LABEL_FATAL(NPT_File::GetInfo(url, &info), failure);
        resource.m_Size = info.m_Size;

        // get list of ip addresses

        NPT_List<NPT_IpAddress> ips;
        NPT_CHECK_LABEL_SEVERE(PLT_UPnPMessageHelper::GetIPAddresses(ips), failure);

        // if we're passed an interface where we received the request from
        // move the ip to the top so that it is used for the first resource

        if (context.GetLocalAddress().GetIpAddress().ToString() != "0.0.0.0")
        {
            ips.Remove(context.GetLocalAddress().GetIpAddress());
            ips.Insert(ips.GetFirstItem(), context.GetLocalAddress().GetIpAddress());
        }
        else if (!allip)
        {
            NPT_LOG_WARNING("Couldn't determine local interface IP so we might return an unreachable IP");
        }

        object->m_ObjectClass.type = PLT_MediaItem::GetUPnPClass(filepath, &context);

        // add as many resources as we have interfaces

        NPT_HttpUrl base_uri("127.0.0.1",
                             context.GetLocalAddress().GetPort(),
                             NPT_HttpUrl::PercentEncode(d->urlRoot, NPT_Uri::PathCharsToEncode));
        NPT_List<NPT_IpAddress>::Iterator ip = ips.GetFirstItem();

        while (ip)
        {
            resource.m_Uri = BuildResourceUri(base_uri, ip->ToString(), url);
            object->m_Resources.Add(resource);
            ++ip;

            // if we only want the one resource reachable by client

            if (!allip)
                break;
        }
    }
    else
    {
        qCDebug(DIGIKAM_MEDIASRV_LOG) << "BuildFromFilePath() :: directory detected";

        object = new PLT_MediaContainer;

        // Assign a title for this container

        if (filepath.Compare("/", true) == 0)
        {
            object->m_Title = "Root";
        }
        else
        {
            QString path    = QString::fromUtf8(filepath.GetChars());
            QString title   = path.section(QLatin1Char('/'), -2, -2); // We drop extra '/' too on the front of name
            object->m_Title = NPT_String(title.toUtf8().data());

            if (object->m_Title.GetLength() == 0)
            {
                qCDebug(DIGIKAM_MEDIASRV_LOG) << "BuildFromFilePath() :: MediaContainer item name is empty.";
                goto failure;
            }
        }

        // Get the number of children for this container

        NPT_LargeSize count = 0;

        if (with_count && NPT_SUCCEEDED(NPT_File::GetSize(filepath, count)))
        {
            ((PLT_MediaContainer*)object)->m_ChildrenCount = (NPT_Int32)count;
        }

        object->m_ObjectClass.type = "object.container.storageFolder";

        qCDebug(DIGIKAM_MEDIASRV_LOG) << "BuildFromFilePath() :: Create item as MediaContainer \""
                                      << object->m_Title.GetChars() << "\"";
    }

    // is it the root?

    if (filepath.Compare("/", true) == 0)
    {
        qCDebug(DIGIKAM_MEDIASRV_LOG) << "BuildFromFilePath() :: New item is root";
        object->m_ParentID = "-1";
        object->m_ObjectID = "0";
    }
    else
    {
        // is the parent path the root?

        if (filepath.EndsWith("/"))
        {
            object->m_ParentID = "0";
        }
        else
        {
            object->m_ParentID = "0" + filepath.Left(filepath.Find("/?file:") + 1);
        }

        object->m_ObjectID = "0" + filepath.SubString(0);
    }

    qCDebug(DIGIKAM_MEDIASRV_LOG) << "BuildFromFilePath() :: New item parent ID:"
                                  << object->m_ParentID.GetChars();
    qCDebug(DIGIKAM_MEDIASRV_LOG) << "BuildFromFilePath() :: New item object ID:"
                                  << object->m_ObjectID.GetChars();

    return object;

failure:

    qCDebug(DIGIKAM_MEDIASRV_LOG) << "Failed to build didl for file \""
                                  << filepath.GetChars() << "\"";

    delete object;
    return NULL;
}

NPT_Result DLNAMediaServerDelegate::GetFilePath(const char* object_id,
                                                NPT_String& filepath)
{
    if (!object_id)
        return NPT_ERROR_INVALID_PARAMETERS;

    filepath = "/";

    // object id is formatted as 0/<filepath>

    if (NPT_StringLength(object_id) >= 1)
    {
        int index = 0;

        if (object_id[0] == '0' && object_id[1] == '/')
            index = 2;
        else if (object_id[0] == '0')
            index = 1;

        filepath += (object_id + index);
    }

    qCDebug(DIGIKAM_MEDIASRV_LOG) << "GetFilePath() :: Object id:"
                                  << object_id << "filepath:" << filepath.GetChars();

    return NPT_SUCCESS;
}

NPT_Result DLNAMediaServerDelegate::OnSearchContainer(PLT_ActionReference&          action,
                                                      const char*                   object_id,
                                                      const char*                   search_criteria,
                                                      const char*                   /* filter */,
                                                      NPT_UInt32                    /* starting_index */,
                                                      NPT_UInt32                    /* requested_count */,
                                                      const char*                   /* sort_criteria */,
                                                      const PLT_HttpRequestContext& /* context */)
{
    // parse search criteria
    // TODO: HACK TO PASS DLNA

    qCDebug(DIGIKAM_MEDIASRV_LOG) << "Received Search request for object \""
                                  << object_id << "\" with search \""
                                  << search_criteria << "\"";

    if (search_criteria && NPT_StringsEqual(search_criteria, "Unknownfieldname"))
    {
        // error

        qCDebug(DIGIKAM_MEDIASRV_LOG) << "Unsupported or invalid search criteria" << search_criteria;
        action->SetError(708, "Unsupported or invalid search criteria");
        return NPT_FAILURE;
    }

    // locate the file from the object ID

    NPT_String dir;

    if (NPT_FAILED(GetFilePath(object_id, dir)))
    {
        // error

        qCDebug(DIGIKAM_MEDIASRV_LOG) << "ObjectID not found" << object_id;
        action->SetError(710, "No Such Container.");
        return NPT_FAILURE;
    }

    qCDebug(DIGIKAM_MEDIASRV_LOG) << "OnSearchContainer() :: dir =" << dir.GetChars();

    // retrieve the item type

    NPT_FileInfo info;
    NPT_Result   res = NPT_File::GetInfo(dir, &info);

    if (NPT_FAILED(res) || (info.m_Type != NPT_FileInfo::FILE_TYPE_DIRECTORY))
    {
        // error

        qCDebug(DIGIKAM_MEDIASRV_LOG) << "No such container" << dir.GetChars();
        action->SetError(710, "No such container");
        return NPT_FAILURE;
    }

    return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_String DLNAMediaServerDelegate::BuildSafeResourceUri(const NPT_HttpUrl& base_uri,
                                                         const char*        host,
                                                         const char*        file_path)
{
    NPT_String  result;
    NPT_HttpUrl uri = base_uri;

    if (host)
        uri.SetHost(host);

    NPT_String uri_path = uri.GetPath();

    if (!uri_path.EndsWith("/"))
        uri_path += "/";

    // some controllers (like WMP) will call us with an already urldecoded version.
    // We're intentionally prepending a known urlencoded string
    // to detect it when we receive the request urlencoded or already decoded to avoid double decoding

    uri_path += "%/";
    uri_path += file_path;

    // set path

    uri.SetPath(uri_path);

    // 360 hack: force inclusion of port in case it's 80

    return uri.ToStringWithDefaultPort(0);
}

NPT_Result DLNAMediaServerDelegate::ExtractResourcePath(const NPT_HttpUrl& url,
                                                        NPT_String&        file_path)
{
    // Extract non decoded path, we need to autodetect urlencoding

    NPT_String uri_path        = url.GetPath();
    NPT_String url_root_encode = NPT_Uri::PercentEncode(d->urlRoot, NPT_Uri::PathCharsToEncode);
    NPT_Ordinal skip           = 0;

    if (uri_path.StartsWith(d->urlRoot))
    {
        skip = d->urlRoot.GetLength();
    }
    else if (uri_path.StartsWith(url_root_encode))
    {
        skip = url_root_encode.GetLength();
    }
    else
    {
        return NPT_FAILURE;
    }

    // account for extra slash

    skip     += ((d->urlRoot == "/") ? 0 : 1);
    file_path = uri_path.SubString(skip);

    // detect if client such as WMP sent a non urlencoded url

    if (file_path.StartsWith("%/"))
    {
        NPT_LOG_FINE("Received a urldecoded version of our url!");
        file_path.Erase(0, 2);
    }
    else
    {
        // remove our prepended string we used to detect urldecoded version

        if (file_path.StartsWith("%25/"))
            file_path.Erase(0, 4);

        // ok to urldecode

        file_path = NPT_Uri::PercentDecode(file_path);
    }

    return NPT_SUCCESS;
}

NPT_Result DLNAMediaServerDelegate::ServeFile(const NPT_HttpRequest&        request,
                                              const NPT_HttpRequestContext& context,
                                              NPT_HttpResponse&             response,
                                              const NPT_String&             file_path)
{
    // Try to stream image file as transcoded preview.
    // This will serve image in reduced size, including all know image formats
    // supported by digiKam core, as JPEG, PNG, TIFF, and RAW files for ex.

    DImg dimg = PreviewLoadThread::loadFastSynchronously(QString::fromUtf8(file_path.GetChars()), 2048);

    if (dimg.isNull())
    {
        // Not a supported image format. Try to stream file as well, without transcoding.
        // TODO : support video file as transcoded video stream using QtAV (if possible).

        qCDebug(DIGIKAM_MEDIASRV_LOG) << file_path.GetChars() << "not recognized as an image to stream as preview.";

        NPT_CHECK_WARNING(PLT_HttpServer::ServeFile(request, context, response, file_path));
        return NPT_SUCCESS;
    }

    // This code is basically the same than PLT_HttpServer::ServeFile() excepted the
    // image trancoding pass served as byte stream.

    NPT_InputStreamReference stream;
    NPT_File                 file(file_path);
    NPT_FileInfo             file_info;

    // prevent hackers from accessing files outside of our root

    if ((file_path.Find("/..") >= 0)  ||
        (file_path.Find("\\..") >= 0) ||
        NPT_FAILED(NPT_File::GetInfo(file_path, &file_info)))
    {
        return NPT_ERROR_NO_SUCH_ITEM;
    }

    // check for range requests

    const NPT_String* range_spec = request.GetHeaders().GetHeaderValue(NPT_HTTP_HEADER_RANGE);

    // handle potential 304 only if range header not set

    NPT_DateTime  date;
    NPT_TimeStamp timestamp;

    if (NPT_SUCCEEDED(PLT_UPnPMessageHelper::GetIfModifiedSince((NPT_HttpMessage&)request, date)) &&
        !range_spec)
    {
        date.ToTimeStamp(timestamp);

        NPT_LOG_INFO_5("File %s timestamps: request=%d (%s) vs file=%d (%s)", 
                       (const char*)request.GetUrl().GetPath(),
                       (NPT_UInt32)timestamp.ToSeconds(),
                       (const char*)date.ToString(),
                       (NPT_UInt32)file_info.m_ModificationTime,
                       (const char*)NPT_DateTime(file_info.m_ModificationTime).ToString());

        if (timestamp >= file_info.m_ModificationTime)
        {
            // it's a match

            NPT_LOG_FINE_1("Returning 304 for %s", request.GetUrl().GetPath().GetChars());
            response.SetStatus(304, "Not Modified", NPT_HTTP_PROTOCOL_1_1);
            return NPT_SUCCESS;
        }
    }

    // Trancoding image as preview.

    QImage preview = dimg.copyQImage();
    QByteArray ba;
    QBuffer    buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    preview.save(&buffer, "JPG");
    buffer.close();

    stream         = new NPT_MemoryStream(ba.data(), (NPT_Size)ba.size());

    if (stream.IsNull())
    {
        return NPT_ERROR_NO_SUCH_ITEM;
    }

    // set Last-Modified and Cache-Control headers

    if (file_info.m_ModificationTime)
    {
        NPT_DateTime last_modified = NPT_DateTime(file_info.m_ModificationTime);
        response.GetHeaders().SetHeader("Last-Modified", last_modified.ToString(NPT_DateTime::FORMAT_RFC_1123), true);
        response.GetHeaders().SetHeader("Cache-Control", "max-age=0,must-revalidate", true);
    }

    PLT_HttpRequestContext tmp_context(request, context);

    NPT_CHECK_WARNING(PLT_HttpServer::ServeStream(request, context, response, stream,
                      "image/jpeg"));

    return NPT_SUCCESS;
}

NPT_String DLNAMediaServerDelegate::BuildResourceUri(const NPT_HttpUrl& base_uri,
                                                     const char*        host,
                                                     const char*        file_path)
{
    return BuildSafeResourceUri(base_uri, host, file_path);
}

NPT_Result DLNAMediaServerDelegate::OnUpdateObject(PLT_ActionReference&,
                                                   const char*,
                                                   NPT_Map<NPT_String,NPT_String>&,
                                                   NPT_Map<NPT_String,NPT_String>&,
                                                   const PLT_HttpRequestContext&)
{
    return NPT_SUCCESS;
}

bool DLNAMediaServerDelegate::ProcessFile(const NPT_String&, const char*)
{
    return true;
}

} // namespace Digikam
