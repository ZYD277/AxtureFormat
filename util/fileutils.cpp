#include "fileutils.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextCodec>
#include <QDebug>

RFile::RFile(const QString &fileName):QFile(fileName)
{

}

bool RFile::startParse(OpenMode openMode)
{
    QFileInfo info(fileName());
    if(!info.exists()|| info.isDir()){
        return false;
    }

    if(!open(openMode)){
        return false;
    }
    return true;
}

bool RFile::startSave(OpenMode openMode)
{
    QFileInfo info(fileName());
    if(info.isDir()){
        return false;
    }

    if(!open(openMode)){
        return false;
    }

    return true;
}


RXmlFile::RXmlFile(const QString &fileName):RFile(fileName),parseMethod(NULL),m_bAutoAddXmlSuffix(true)
{

}

RXmlFile::~RXmlFile()
{
    if(parseMethod && isAutoReleaseParseMethod)
        delete parseMethod;
}

void RXmlFile::disableAutoAddXmlSuffix()
{
    m_bAutoAddXmlSuffix = false;
}

bool RXmlFile::startParse(OpenMode  openMode)
{
    if(parseMethod  && RFile::startParse(openMode)){
        QDomDocument doc;
        QString errorMsg;
        int errorRow = 0,errorCol = 0;
        if(!doc.setContent(this, false, &errorMsg, &errorRow, &errorCol)){
            qDebug()<<"parse error:"<<errorMsg<<errorRow<<errorCol;
            close();
            return false;
        }
        close();
        QDomElement root = doc.documentElement();
        if(!root.isNull())
        {
             return parseMethod->startParse(root.toElement());
        }
    }
    qDebug()<<("Not set xml parseMethod!");
    return false;
}

bool RXmlFile::startSave(OpenMode  openMode, bool autoAddProcessingNode)
{
    if(m_bAutoAddXmlSuffix && fileName().lastIndexOf(".xml") < 0)
        setFileName(fileName()+".xml");

    if(parseMethod && RFile::startSave(openMode))
    {
        QTextStream stream(this);
        stream.setCodec(QTextCodec::codecForLocale());

        QDomDocument doc("");

        if(autoAddProcessingNode){
            QDomProcessingInstruction instruction = doc.createProcessingInstruction("xml","version='1.0' encoding='UTF-8'");
            doc.appendChild(instruction);
        }

        bool result = parseMethod->startSave(doc);
        if(result)
            doc.save(stream,4);
        return result;
    }
    qDebug()<<("Not set xml parseMethod!");
    return false;
}

RTextFile::RTextFile(const QString &fileName):RFile(fileName),parseMethod(NULL)
{

}

RTextFile::~RTextFile()
{
    if(parseMethod && isAutoReleaseParseMethod)
        delete parseMethod;
}

bool RTextFile::startParse(OpenMode  openMode, bool isRecordLogflie)
{
    if(parseMethod  && RFile::startParse(openMode)){
        return parseMethod->startParse(this);
    }

    if(isRecordLogflie){
        qDebug()<<("Not set text file parseMethod!");
    }
    return false;
}

bool RTextFile::startSave(OpenMode  openMode, bool isRecordLogfile)
{
    if(parseMethod && RFile::startSave(openMode))
    {
        return parseMethod->startSave(this);
    }
    if(isRecordLogfile){
        qDebug()<<("Not set text file parseMethod!");
    }
    return false;
}
