/*!
 *  @brief     日志输出窗口
 *  @details   支持显示按照不同颜色输出
 *  @author    wey
 *  @version   1.0
 *  @date      2020.03.21
 *  @warning
 *  @copyright NanJing RenGu.
 */
#ifndef LOGOUTPUTER_H
#define LOGOUTPUTER_H

#include <QWidget>
#include <QMenu>
#include <QAction>

namespace Ui {
class LogOutputer;
}

class LogOutputer : public QWidget
{
    Q_OBJECT

public:
    explicit LogOutputer(QWidget *parent = 0);
    ~LogOutputer();

    void appendLog(QString oneline);

protected:
    void showEvent(QShowEvent * event);
    bool eventFilter(QObject *watched, QEvent *event);

private slots:
    void showContextMenu(QPoint point);
    void clearText(bool flag);

private:
    Ui::LogOutputer *ui;
};

#endif // LOGOUTPUTER_H
