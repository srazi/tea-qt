// "single_application.cpp"

/*
 * taken from http://berenger.eu/blog/c-qt-singleapplication-single-app-instance/
 * code by Berenger Bramas
 * modified by Peter Semiletov
 
 * 
 */


#include <QTimer>
#include <QByteArray>
#include <QMessageBox>
 
#include "single_application_shared.h"
 
CSingleApplicationShared::CSingleApplicationShared(int &argc, char *argv[], const QString uniqueKey) : QApplication(argc, argv)
{
#ifndef Q_OS_OS2

    qDebug() << "CSingleApplicationShared::CSingleApplicationShared";

    sharedMemory.setKey(uniqueKey);
 
    // when  can create it only if it doesn't exist
    if (sharedMemory.create(5000))
       {
        sharedMemory.lock();
        *(char*)sharedMemory.data() = '\0';
        sharedMemory.unlock();
 
        bAlreadyExists = false;
 
        // start checking for messages of other instances.
        QTimer *timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(checkForMessage()));
        timer->start(200);
       }
    // it exits, so we can attach it?!
    else if (sharedMemory.attach()){
        bAlreadyExists = true;
    }
    else{
        // error
    }
#endif 
}
 
 
// public slots.
 
void CSingleApplicationShared::checkForMessage()
{
#ifndef Q_OS_OS2
    
    QStringList arguments;
 
    sharedMemory.lock();
    char *from = (char*)sharedMemory.data();
 
    while(*from != '\0'){
        int sizeToRead = int(*from);
        ++from;
 
        QByteArray byteArray = QByteArray(from, sizeToRead);
        byteArray[sizeToRead] = '\0';
        from += sizeToRead;
 
        arguments << QString::fromUtf8(byteArray.constData());
    }
 
    *(char*)sharedMemory.data() = '\0';
    sharedMemory.unlock();
 
    if (arguments.size()) emit messageAvailable (arguments);
#endif    
}
 
// public functions.
 
bool CSingleApplicationShared::sendMessage(const QString &message)
{
#ifndef Q_OS_OS2

    //we cannot send mess if we are master process!
    if (isMasterApp()){
        return false;
    }
 
    QByteArray byteArray;
    byteArray.append(char(message.size()));
    byteArray.append(message.toUtf8());
    byteArray.append('\0');
 
    sharedMemory.lock();
    char *to = (char*)sharedMemory.data();
    while(*to != '\0'){
        int sizeToRead = int(*to);
        to += sizeToRead + 1;
    }
 
    const char *from = byteArray.data();
    memcpy(to, from, qMin(sharedMemory.size(), byteArray.size()));
    sharedMemory.unlock();
#endif 
    return true;
}
