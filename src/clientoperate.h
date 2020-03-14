#ifndef CLIENTOPERATE_H
#define CLIENTOPERATE_H

#include "global.h"
#include "viewdelegate.h"
#include "viewmoudel.h"
#include "startthreads.h"

#include <QWidget>
#include <QButtonGroup>
#include <QAbstractTableModel>
#include <QStyledItemDelegate>
#include <QItemDelegate>
#include <QProcess>

#include "threadswitch.h"

#define MAXTHREAD 4

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
    void switchObjFile(AxturePage,QString,QString);

private slots:
    void on_openAxPrjBtn_clicked();
    void switchLineHtmlFile(QString htmlFilePath);
    void startSwitchFiles();
    void slot_QtVisonGroupClicked(int);
    void slot_DirPathGroupClicked(int);
    void on_deletListBtn_clicked();
    void on_selectDirBtn_clicked();
    void deletFileData(QString htmlFilePath);
    void appendErrorRecord(QString record);
    void appendWarningRecord(QString record);
    void appendRecord(QString record);
    void updateProcess(AxturePage newProcessInfo);
    void controlWidget(bool state);

private:
    void innitSwitchNeedMsgStru();
    void findFixedFiles();
    bool findHadHtmlFile(QString newFilePath);
    void updateListInformation();
    bool updateBranch(QString projectName, AxtureProject newProject);

    bool checkJsCssExisted(QString path,bool isSinglePage = true);
    QPair<QString, QString> getJsCssFile(QString path,bool isSinglePage = true);
    void showWarnings(QString content);

    void createThreads(AxtureProject project, QString outputDir);

private:
    Ui::ClientOperate *ui;

    QButtonGroup* QtVsionButtonGroup;
    QButtonGroup* DirPathButtonGroup;

    const QString cssBaseFileName;
    const QString jsSinglePageFileName;
    const QString jsBaseFileName;


    QList<AxtureProject> rootPathList;

    bool radioBtnSign;

    ViewMoudel* m_model;
    ViewDelegate *m_viewDelegate;
    ThreadSwitch *sonThread;
    StartThreads *m_signalFile;
//    ThreadSwitch *sonThread[MAXTHREAD];
};
#endif // CLIENTOPERATE_H


