#ifndef _digikamthumbnail_H_
#define _digikamthumbnail_H_

#include <kio/slavebase.h>

class KURL;
class QCString;
class QString;
class QImage;

class kio_digikamthumbnailProtocol : public KIO::SlaveBase
{
public:

    kio_digikamthumbnailProtocol(const QCString &pool_socket,
                                 const QCString &app_socket);
    virtual ~kio_digikamthumbnailProtocol();
    virtual void get(const KURL& url);

private:

    bool loadJPEG(QImage& image, const QString& path);
    
    int size_;
};

#endif
