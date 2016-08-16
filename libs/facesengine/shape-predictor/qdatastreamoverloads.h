#ifndef QDATASTREAMOVERLOADS_H
#define QDATASTREAMOVERLOADS_H

#include <QDataStream>


QDataStream& operator>>( QDataStream& dataStream, unsigned long& in )
{
    qint64 x;
    dataStream>>x;
    in = x;
    return dataStream;
}

QDataStream& operator<<( QDataStream& dataStream, const unsigned long& in )
{
    qint64 x = in;
    dataStream<<x;
    return dataStream;
}


#endif // QDATASTREAMOVERLOADS_H
