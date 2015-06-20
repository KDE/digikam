#ifndef IOJOBSTHREAD_H
#define IOJOBSTHREAD_H

#include "KDCRAW/RActionThreadBase"
#include "album.h"
#include "iojob.h"
#include "imageinfo.h"

using namespace KDcrawIface;

namespace Digikam
{

class IOJobsThread : public RActionThreadBase
{
    Q_OBJECT

public:
    IOJobsThread(QObject *const parent);

    void copyPAlbum(const PAlbum *srcAlbum, const PAlbum *destAlbum, const CopyJob::OperationType opType);
    void copyFiles(const QList<QUrl> &srcFiles, const PAlbum *destAlbum, const CopyJob::OperationType opType);

    void deletePAlbum(const PAlbum *albumToDelete, bool useTrash);
    void deleteFiles(const QList<ImageInfo> &srcsToDelete, bool isPermanentDeletion);
};

} // namespace Digikam

#endif // IOJOBSTHREAD_H
