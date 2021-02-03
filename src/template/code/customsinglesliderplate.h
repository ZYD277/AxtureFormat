#ifndef CUSTOMSINGLESLIDERPLATE_H
#define CUSTOMSINGLESLIDERPLATE_H

#include <QWidget>

#include "abstractcodetemplate.h"

namespace CXX{

class CustomSingleSliderPlate : public AbstractCodeTemplate
{
public:
    CustomSingleSliderPlate();

    void prepareOutput(CppGenerate *generate);

    void setScrollBarParameter(SingleSlidingBlockData *srollBarData);

private:
    SingleSlidingBlockData *m_srollBarData;
};
} //namespace CXX

#endif // CUSTOMSINGLESLIDERPLATE_H
