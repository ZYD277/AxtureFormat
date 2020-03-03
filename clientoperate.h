#ifndef CLIENTOPERATE_H
#define CLIENTOPERATE_H

#include "global.h"
#include "viewdelegate.h"
#include "viewmoudel.h"
#include "widget.h"

#include <QWidget>
#include <QButtonGroup>
#include <QAbstractTableModel>
#include <QStyledItemDelegate>
#include <QItemDelegate>
#include <QProcess>
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
    void getHomePath(QString,QString);
private slots:
    void on_openAxPrjBtn_clicked();
    void openLineHtmlFile();
    void deleteLineHtmlFile();
    void switchLineHtmlFile();
    void startSwitchFiles();
    void slot_QtVisonGroupClicked(int);
    void slot_DirPathGroupClicked(int);
    void on_deletListBtn_clicked();
    void on_selectDirBtn_clicked();
    void deletFileData(QString filename);
    void appendErrorRecord(QString record);
    void appendWarningRecord(QString record);
    void appendRecord(QString record);
    void getNewProjectPath(QString);

private:
    void innitSwitchNeedMsgStru();
    void getFolderMsg(QString path);
    void getSuitableHtmlFile();
    void getSuitableFile(QString baseFileName, QString dirPath);
    void findFixedFiles();
    bool findHadHtmlFile(QString newfileName);
    void updateListInformation();
    //

private:
    Ui::ClientOperate *ui;
    QList<Global::DirInfo> m_dirList;
    QList<Global::FileInfo> m_fileList;
    QList<Global::FileInfo> m_suitableFileList;
    Global::Stru_SwitchNeedMsg g_switchNeedMsg;
    QButtonGroup* QtVsionButtonGroup;
    QButtonGroup* DirPathButtonGroup;
    bool m_css;
    bool m_image;
    bool radioBtnSign;
    ViewMoudel* m_model;
    ViewDelegate *m_viewDelegate;
    Widget *m_widget;
};
#endif // CLIENTOPERATE_H


