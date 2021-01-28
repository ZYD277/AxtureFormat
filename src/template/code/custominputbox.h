#ifndef CUSTOMINPUTBOX_H
#define CUSTOMINPUTBOX_H

#include <QWidget>
#include "abstractcodetemplate.h"

namespace CXX{

class CustomInputBox : public AbstractCodeTemplate
{

public:
    CustomInputBox();

    void prepareOutput(CppGenerate *generate);

    void setInputBoxParameter(KeyBoardInputBoxData * inputBoxData);

signals:

public slots:

private:
    KeyBoardInputBoxData *m_inputBoxData;

};
} //namespace CXX

#endif // CUSTOMINPUTBOX_H
