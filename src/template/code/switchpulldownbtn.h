#ifndef SWITCHPULLDOWNBTN_H
#define SWITCHPULLDOWNBTN_H

#include <QWidget>

#include "abstractcodetemplate.h"

namespace CXX{

class SwitchPullDownBtn : public AbstractCodeTemplate
{
public:
    SwitchPullDownBtn();

    void prepareOutput(CppGenerate * generate);
    void setIds(SwitchPullDownButtonData *switchPullDownBtnData);

private:
    SwitchPullDownButtonData *m_switchPullDownBtnData;
};

} //namespace CXX

#endif // SWITCHPULLDOWNBTN_H
