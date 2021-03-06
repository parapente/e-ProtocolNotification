#include <QDebug>
#include "filedownloader.h"
#include "logger.h"

extern Logger logger;

FileDownloader::FileDownloader(QObject *parent) :
 QObject(parent)
{
    connect(&netManager, SIGNAL (finished(QNetworkReply*)), this, SLOT (fileDownloaded(QNetworkReply*)));
    connect(this, SIGNAL(downloaded()), this, SLOT(checkHash()));
    remainingRequests=0;
}

FileDownloader::~FileDownloader() { }

void FileDownloader::fileDownloaded(QNetworkReply* reply) {
    int key;

    if (reply->error() == QNetworkReply::NoError) { // All OK
        key = urlList.indexOf(reply->request().url());
        downloadedDataMap[key] = reply->readAll();
    }
    else {
        qDebug() << "Error Downloading " << reply->request().url();
        qDebug() << "Download Error: " << reply->errorString();
        logger.write(QString("Error Downloading ") + reply->request().url().toString());
        logger.write(QString("Download Error: ") + reply->errorString());
        err = DownloadFailed;
    }
    reply->close();
    reply->deleteLater();
    remainingRequests--;
    if (!remainingRequests)
        emit downloaded();
}

QMap<int, QByteArray> FileDownloader::downloadedData() const {
    return downloadedDataMap;
}

void FileDownloader::getFiles(QVariantList filelist, QUrl baseUrl)
{
    if (!remainingRequests) { // Accept new list only when the previous is done
        // Init lists
        err = NoError;
        downloadedDataMap.clear();
        filenameList.clear();
        hashList.clear();
        hashOK.clear();
        hashTypeList.clear();
        urlList.clear();

        int size = filelist.size();
        if ((size % 3) == 0) {
            remainingRequests = size / 3;
            for (int i=0; i<size; i+=3) {
                filenameList << filelist[0].toString();
                hashList << filelist[1].toString();
                QString hash = filelist[2].toString();
                if (hash == "md5") {
                    hashTypeList << QCryptographicHash::Md5;
                }
                else if (hash == "sha1") {
                    hashTypeList << QCryptographicHash::Sha1;
                }
                urlList << QUrl(baseUrl.toString() + filelist[0].toString());
                netManager.get(QNetworkRequest(QUrl(baseUrl.toString() + filelist[0].toString())));
                qDebug() << "Downloading " << baseUrl.toString() << filelist[0].toString();
                logger.write(QString("Downloading ") + baseUrl.toString() + filelist[0].toString());
            }
        }
        else {
            qDebug() << "Download error: Invalid file list.";
            logger.write(QString("Download error: Invalid file list."));
        }
    }
    else {
        qDebug() << "Download error: Downloads still pending. Not queuing.";
        logger.write(QString("Download error: Downloads still pending. Not queuing."));
    }
}

void FileDownloader::checkHash()
{
    if (err == NoError) { // If all files were downloaded without problems
        int lsize = filenameList.size();

        for (int i=0; i<lsize; i++) {
            QByteArray tmpHash;
            tmpHash = QCryptographicHash::hash(downloadedDataMap[i],hashTypeList[i]).toHex();
            qDebug() << "New file's hash " << tmpHash << ". Should be " << hashList[i];
            logger.write(QString("New file's hash ") + tmpHash + ". Should be " + hashList[i]);
            if (tmpHash == hashList[i]) {
                hashOK << true;
            }
            else {
                hashOK << false;
                err = HashCheckFailed;
            }
        }
    }
    emit hashChecked();
}

QStringList FileDownloader::downloadedFiles() const
{
    return filenameList;
}

FileDownloader::Error FileDownloader::error()
{
    return err;
}
