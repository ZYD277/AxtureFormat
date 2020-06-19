/*!
 *  @brief     单个转换任务
 *  @details   封装转换队列中的一条转换任务
 *  @author    wey
 *  @version   1.0
 *  @date      2020.03.15
 *  @warning
 *  @copyright NanJing RenGu.
 */
#ifndef SWITCHTASK_H
#define SWITCHTASK_H

#include <QObject>
#include "head.h"
#include "html/htmlparser.h"
#include "qui/qtoutput.h"

class SwitchTask : public QObject
{
    Q_OBJECT

public:
    SwitchTask();
    ~SwitchTask();

    void initTask(AxurePage page, QString outputDir, bool generateCode);

signals:
    void updateProgress(SwitchProgress prog);

private:
    enum Progress{
        P_CssParse = 1,
        P_HtmlParse,
        P_CopyFile,
        P_Finish
    };

private:
    void updataProcessBar(Progress curentProcess, bool hasError, QString description);

private:
    Html::HtmlParser m_htmlParser;
    RQt::QtOutput m_qtOutput;

    QString m_outputDir;        /*!< 生成ui文件的路径 */
    AxurePage m_page;
};

#endif // SWITCHTASK_H
