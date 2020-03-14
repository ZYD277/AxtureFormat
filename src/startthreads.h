#ifndef STARTTHREADS_H
#define STARTTHREADS_H

#include <QWidget>
#include <QList>

#include "head.h"
#include "global.h"

#include "html/htmlparser.h"
#include "css/cssparser.h"
#include "qui/qtoutput.h"

#define THREAD_NUM 4
class StartThreads : public QWidget
{
    Q_OBJECT
public:
    explicit StartThreads(QWidget *parent = 0);
signals:
    void updateProgress(QString record);
    void updateErrorProgress(QString record);
    void updateWaringProgress(QString record);

    void updataProgressBar(AxturePage newFuleInfo);

private slots:
//    void chooseAxtureProject(AxtureProject project, QString targetProPath);
    void threadProcessingData(AxturePage page, QString outputDir, QString projectPath);

private:
    void updataProcessBar(AxturePage page, int curentProcess, bool success);
private:

    Html::HtmlParser m_htmlParser;
    RQt::QtOutput m_qtOutput;

};

#endif // STARTTHREADS_H
