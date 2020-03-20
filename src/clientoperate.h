﻿/*!
 *  @brief     转换控制页面
 *  @details
 *  @author    zyd
 *  @version   1.0
 *  @date      2020.03.09
 *  @warning
 *  @copyright NanJing RenGu.
 */
#ifndef CLIENTOPERATE_H
#define CLIENTOPERATE_H

#include <QWidget>
#include <QButtonGroup>
#include <QMutex>

#include "head.h"

#include "global.h"

#define MAXTHREAD 4

class ViewModel;
class ViewDelegate;

class ThreadPool;

namespace Ui {
class ClientOperate;
}

class ClientOperate : public QWidget
{
    Q_OBJECT

public:
    explicit ClientOperate(QWidget *parent = 0);
    ~ClientOperate();

signals:
    void switchObjFile(AxurePage,QString,QString);

protected:
    bool eventFilter(QObject *watched, QEvent *event);

private slots:
    void openAxureProject();
    void clearAxureTable();
    void chooseUserFilePath();

    void viewFile(QString htmlFilePath);
    void deletFileData(QString pageId);
    void switchLineHtmlFile(QString pageId);

    void startSwitchFiles();
    void updateProgress(SwitchProgress proj);

    void chooseQtVersion(int);
    void chooseOutputPath(int);
    void controlWidget(bool state);

private:
    enum LogLevel{
        LogNormal,
        LogWarn,
        LogError
    };

private:
    void initView();
    void findFixedFiles();
    bool isRepeatedFile(QString filePath);
    void updateTableModel();

    void appendLog(LogLevel level,QString record);

    bool checkJsCssExisted(QString path,bool isSinglePage = true);
    QPair<QString, QString> getJsCssFile(QString path,bool isSinglePage = true);
    void showWarnings(QString content);

    bool checkBeforeSwitch();
    void generateTask(AxurePage & page);

    QString chooseAxureProject();
    void parseAxureProject(QString projectPath);
    void switchMainWidget();

private:
    enum QtVersion{
        Qt4,
        Qt5
    };

    enum ExportPath{
        Path_Current,
        Path_SelfDefine
    };

private:
    Ui::ClientOperate *ui;

    QMutex m_progressMutex;
    ThreadPool * m_pool;

    QButtonGroup* m_versionButtGroup;
    QButtonGroup* m_dirPathButtGroup;

    ViewModel* m_model;
    ViewDelegate * m_viewDelegate;

    QString m_lastAxureProjectPath;         /*!< 保存上次打开axure的工程目录，作为下次打开的默认位置 */

    const QString cssBaseFileName;
    const QString jsSinglePageFileName;
    const QString jsBaseFileName;

    AxturePages m_pageList;

    QPixmap m_normalPix;
    QPixmap m_activePix;
    bool m_b_mouseActive;           /*!< 鼠标是否激活监控区域 */
};
#endif // CLIENTOPERATE_H


