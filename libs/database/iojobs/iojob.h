#ifndef IOJOB_H
#define IOJOB_H

// Qt includes

#include "QUrl"

// KDCraw includes

#include "KDCRAW/RActionJob"

// Local includes

#include "album.h"
#include "item/imageinfo.h"
#include "framework/databaseaccess.h"

using namespace KDcrawIface;

namespace Digikam
{

class ImageInfo;

class IOJob : public RActionJob
{
    Q_OBJECT

public:

    IOJob();

Q_SIGNALS:

    void error(const QString &errMsg);
};

// ---------------------------------------

class CopyJob : public IOJob
{
    Q_OBJECT

public:

    enum OperationType
    {
        Copy = 0,
        Move
    };

    CopyJob(const PAlbum *dest, OperationType type);

protected:
    const PAlbum  *m_dest;
    OperationType  m_type;
};

// -----------

class CopyFileJob : public CopyJob
{
    Q_OBJECT

public:

    CopyFileJob(const QUrl &src, const PAlbum *dest, OperationType type);

protected:

    void run();

private:

    QUrl m_srcFile;
};

// -----------

class CopyAlbumJob : public CopyJob
{
    Q_OBJECT

public:

    CopyAlbumJob(const PAlbum *src, const PAlbum *dest, OperationType type);

protected:

    void run();

private:

    const PAlbum *m_srcAlbum;
};

// ---------------------------------------

class DeleteJob : public IOJob
{
    Q_OBJECT

public:

    DeleteJob(bool isPermanentDeletion);

protected:

    bool m_useTrash;
};

// -----------

class DeleteFileJob : public DeleteJob
{
    Q_OBJECT

public:

    DeleteFileJob(const ImageInfo &srcToDelete, bool useTrash);

protected:

    void run();

private:

    ImageInfo m_srcToDelete;
};

// -----------

class DeleteAlbumJob : public DeleteJob
{
    Q_OBJECT

public:

    DeleteAlbumJob(const PAlbum *album, bool useTrash);

protected:

    void run();

private:

    const PAlbum *m_albumToDelete;
};

// ---------------------------------------

//class RenameJob : public FileSystemJob
//{

//};

// ---------------------------------------

//class StatJob : public FileSystemJob
//{
//    Q_OBJECT

//public:

//    StatJob();

//protected:

//    void run();
//};

// ---------------------------------------

//class MkdirJob : public FileSystemJob
//{
//    Q_OBJECT

//public:

//    MkdirJob();
//};


} // namespace Digikam

#endif // IOJOB_H
