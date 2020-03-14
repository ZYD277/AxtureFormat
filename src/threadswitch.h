#ifndef THREADSWITCH_H
#define THREADSWITCH_H

#include <QObject>
#include <QThread>
#include <head.h>

#include <QQueue>
#include <QWaitCondition>
#include <QMutex>

#include "html/htmlparser.h"
#include "css/cssparser.h"
#include "qui/qtoutput.h"

class ThreadSwitch : public QThread
{
    Q_OBJECT
public:
    explicit ThreadSwitch();
    ~ThreadSwitch();

signals:
    void updateProgress(QString);
    void updateErrorProgress(QString);
    void updateWaringProgress(QString);
    void updataProgressBar(AxturePage);
    void updataWidgetState(bool);
public:
    void run();
    void threadStart();
    void threadStop();
    void updataProcessBar(AxturePage page, int curentProcess, bool symbol);
public slots:

    void getFileInfo(AxtureProject, QString targetProPath);

private:

    Html::HtmlParser m_htmlParser;
    RQt::QtOutput m_qtOutput;

    QString outputDir;
    QString projectPath;

    bool threadSymbol;

    QQueue<AxturePage> myQueue;
    QWaitCondition condition;

     QMutex t_lock;
};
#endif // THREADSWITCH_H
