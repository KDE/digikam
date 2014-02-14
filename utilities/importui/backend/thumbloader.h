#ifndef THUMBLOADER_H
#define THUMBLOADER_H

#include<QWidget>


namespace Digikam
{

class PixWorker : public QObject
{
    Q_OBJECT
public:
    PixWorker();

public slots:
    void doWork(const QString result);

signals:
    void resultReady(const QString result);
};

class ThumbLoader : public QObject
{
    Q_OBJECT
public:
    ThumbLoader(QObject* const parent, QString title, QString model, QString port, QString path);
    ~ThumbLoader();

    void addToWork(QString data);

private slots:
    void handleResult(QString result);

signals:
    void signalStartWork(QString todo);
private:
    class ThumbLoaderPriv;
    ThumbLoaderPriv* d;
};

} // namespace Digikam

#endif