#include "logoutputer.h"
#include "ui_logoutputer.h"

#include <QFocusEvent>
#include <QDebug>

LogOutputer::LogOutputer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LogOutputer)
{
    ui->setupUi(this);

    ui->textEdit->installEventFilter(this);
    ui->textEdit->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->textEdit,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(showContextMenu(QPoint)));
}

LogOutputer::~LogOutputer()
{
    delete ui;
}

void LogOutputer::appendLog(QString oneline)
{
    ui->textEdit->append(oneline);
}

void LogOutputer::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    ui->textEdit->setFocus();
}

bool LogOutputer::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == ui->textEdit){
        if(event->type() == QEvent::FocusOut){
            QFocusEvent * eve = dynamic_cast<QFocusEvent *>(event);
            if(eve && eve->reason() == Qt::MouseFocusReason){
                this->hide();
            }
            return true;
        }
    }
    return QWidget::eventFilter(watched,event);
}

void LogOutputer::showContextMenu(QPoint point)
{
    Q_UNUSED(point);

    QMenu * menu = new QMenu(ui->textEdit);
    QAction * clearAction = new QAction();
    clearAction->setIcon(QIcon(":/icon/image/clear.png"));
    clearAction->setText(QStringLiteral("清除"));
    connect(clearAction,SIGNAL(triggered(bool)),this,SLOT(clearText(bool)));
    menu->addAction(clearAction);

    menu->exec(QCursor::pos());
    menu->deleteLater();
}

void LogOutputer::clearText(bool flag)
{
    Q_UNUSED(flag);
    ui->textEdit->clear();
}
