#ifndef IOJOBSMANAGER_H
#define IOJOBSMANAGER_H

#include <QObject>
#include <QUrl>
#include "album.h"
#include "iojobsthread.h"
#include "imageinfo.h"

namespace Digikam
{

class IOJobsManager : public QObject
{

public:
    IOJobsManager();

    static IOJobsManager *instance();

    IOJobsThread *startCopyJob(const PAlbum *srcAlbum, const PAlbum *destAlbum, const CopyJob::OperationType opType);
    IOJobsThread *startCopyJob(const QList<QUrl> &srcsList, const PAlbum *destAlbum, const CopyJob::OperationType opType);

    IOJobsThread *startDeleteJob(const PAlbum *albumToDelete, bool useTrash = true);
    IOJobsThread *startDeleteJob(const QList<ImageInfo> &filesToDelete, bool useTrash = true);

// TODO
//    IOJobsThread *startRenameFileJob();
//    IOJobsThread *startRenameAlbumJob();

private:

    friend class FileSystemJobsManagerCreator;
};

} // namespace Digikam

#endif // FILESYSTEMJOBSMANAGER_H
