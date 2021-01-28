#ifndef MYFLOATINGWINDOW_H
#define MYFLOATINGWINDOW_H

#include <QWidget>

#include "abstractcodetemplate.h"
namespace CXX{

class MyFloatingWindow : public AbstractCodeTemplate
{
public:
    explicit MyFloatingWindow();

    void prepareOutput(CppGenerate *generate);

    void setFloatingWindowParameter(FloatingWindow * floatingWindowData);
signals:

public slots:

private:
    FloatingWindow *m_floattingWindow;
};
} //namespace CXX

#endif // MYFLOATINGWINDOW_H
