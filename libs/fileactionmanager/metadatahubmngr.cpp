#include "metadatahubmngr.h"


#include <QMutex>

#include "imageinfo.h"
#include "metadatahub.h"
#include "imageinfolist.h"
#include "metadatasynchronizer.h"

namespace Digikam
{

QPointer<MetadataHubMngr> MetadataHubMngr::internalPtr = QPointer<MetadataHubMngr>();


class MetadataHubMngr::Private
{
public:
    Private()
    {

    }

    ImageInfoList pendingItems;
    QMutex mutex;
};
MetadataHubMngr* MetadataHubMngr::instance()
{
    if(internalPtr.isNull())
        internalPtr = new MetadataHubMngr();
    return internalPtr;
}

MetadataHubMngr::~MetadataHubMngr()
{

}

void MetadataHubMngr::addPending(ImageInfo &info)
{
    QMutexLocker locker(&d->mutex);
    if(!d->pendingItems.contains(info))
        d->pendingItems.append(info);
}

void MetadataHubMngr::slotApplyPending()
{
    QMutexLocker lock(&d->mutex);
    ImageInfoList infos(d->pendingItems);
    d->pendingItems.clear();

    MetadataSynchronizer* const tool = new MetadataSynchronizer(infos,
                                                                MetadataSynchronizer::WriteFromDatabaseToFile);
    tool->start();
}

MetadataHubMngr::MetadataHubMngr()
{

}

}
