#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QList>

#include "html/htmlparser.h"
#include "css/cssparser.h"
#include "qui/qtoutput.h"

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

signals:
    void updateProgress(QString record);

private slots:
    void chooseAxtureProject();
    void appendRecord(QString record);

private:
    void showWarnings(QString content);
    void showInfomation(QString content);

    bool checkJsCssExisted(QString path, bool isSinglePage = true);
    QPair<QString,QString> getJsCssFile(QString path,bool isSinglePage = true);

private:
    Ui::Widget *ui;

    const QString jsBaseFileName;
    const QString cssBaseFileName;
    const QString jsSinglePageFileName;

    Html::HtmlParser m_htmlParser;
    RQt::QtOutput m_qtOutput;
};

#endif // WIDGET_H
