#include "widget.h"

#include <QApplication>
#include <QTextCodec>

#include "clientoperate.h"
#include "util/windumpfile.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ApplicationCrashHandler);

#if QT_VERSION > 0x050000
    QTextCodec * codec = QTextCodec::codecForName("utf-8");
    QTextCodec::setCodecForLocale(codec);
#endif

    ClientOperate w;
    w.show();

    return a.exec();
}
