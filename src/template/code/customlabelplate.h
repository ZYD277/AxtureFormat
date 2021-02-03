#ifndef CUSTOMLABELPLATE_H
#define CUSTOMLABELPLATE_H

#include <QWidget>

#include "abstractcodetemplate.h"

namespace CXX{

class CustomLabelPlate : public AbstractCodeTemplate
{
public:
    CustomLabelPlate();

    void prepareOutput(CppGenerate *generate);

    void setCustomLabelInfo(CustomLabelData *customLabelInfo);

private:

    CustomLabelData *m_customLabelInfo;
};
} //namespace CXX

#endif // CUSTOMLABELPLATE_H
