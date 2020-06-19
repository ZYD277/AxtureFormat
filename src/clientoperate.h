/*!
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

#include <QMainWindow>
#include <QButtonGroup>
#include <QMutex>
#include <QSettings>

#include "head.h"
#include "global.h"
#include "logoutputer.h"

class ViewModel;
class ViewDelegate;

class ThreadPool;

namespace Ui {
class ClientOperate;
}

class ClientOperate : public QMainWindow
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
    void showTableContextMenu(QPoint point);
    void openAxureProject();
    void clearAxureTable();
    void chooseUserFilePath();

    void viewFile(QString filePath);
    void deletFileData(QString pageId);
    void batchDeleteRows(bool);
    void switchLineHtmlFile(QString pageId);

    void startSwitchFiles();
    void updateProgress(SwitchProgress proj);

    void chooseOutputPath(int);
    void controlWidget(bool state);

    void showLogWindow();

    void openHtml(bool flag);

    void clearRecentProjs(bool flag);
    void showRecentProject(bool flag);

    void updateItemClassName(int row,QString newValue);
    void updateItemError();

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

    void saveConfigFile();

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

    QButtonGroup* m_dirPathButtGroup;

    ViewModel* m_model;
    ViewDelegate * m_viewDelegate;

    QString m_lastAxureProjectPath;         /*!< 保存上次打开axure的工程目录，作为下次打开的默认位置 */

    const QString cssBaseFileName;
    const QString jsSinglePageFileName;
    const QString jsBaseFileName;

    AxturePages m_pageList;

    QPixmap m_waterMark;            /*!< 水印 */
    QPixmap m_normalPix;
    QPixmap m_activePix;
    bool m_b_mouseActive;           /*!< 鼠标是否激活监控区域 */

    LogOutputer * m_logger;

    QSettings * m_settings;
    int m_maxRecentSize;
    QMenu * m_recentMenu;
    QStringList m_recentProjects;
    QList<QAction *> m_recentActions;

    bool m_bTableMultiState;        /*!< 表格是否处于多选择状态下 */
};
#endif // CLIENTOPERATE_H


