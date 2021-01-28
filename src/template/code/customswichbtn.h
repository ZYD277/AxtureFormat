#ifndef CUSTOMSWICHBTN_H
#define CUSTOMSWICHBTN_H

#include <QWidget>

#include "abstractcodetemplate.h"

namespace CXX{

class CustomSwichBtn : public AbstractCodeTemplate
{
public:
    explicit CustomSwichBtn();

    void prepareOutput(CppGenerate *generate);

    void setCustomSwitchBtnData(customSwitchButton *customSwitchBtnInfo);

signals:

public slots:

private:
    customSwitchButton *m_customSwitchBtnInfo;

};

} //namespace CXX

#endif // CUSTOMSWICHBTN_H
