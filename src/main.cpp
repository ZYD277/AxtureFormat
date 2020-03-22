#include "widget.h"

#include <QApplication>
#include <QTextCodec>
#include <QFile>

#include "clientoperate.h"
#include "util/windumpfile.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ApplicationCrashHandler);

    QFile styleSheet(":/style/stylesheet.txt");
    if(styleSheet.open(QFile::ReadOnly)){
        a.setStyleSheet(styleSheet.readAll());
    }

    QTextCodec * codec = QTextCodec::codecForName("utf-8");
    QTextCodec::setCodecForLocale(codec);

    ClientOperate w;
    w.show();

    return a.exec();
}
