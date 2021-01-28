#ifndef DROPDOWNBOX_H
#define DROPDOWNBOX_H

#include <QWidget>

#include "abstractcodetemplate.h"
namespace CXX{

class DropDownBox : public AbstractCodeTemplate
{
public:
    DropDownBox();

    void prepareOutput(CppGenerate * generate);

    void setIds(QString widgetID, QString buttonID, QStringList optionIdList);

private:
    QString m_widgetID;
    QString m_buttonID;
    QStringList m_optionIdList;
};
} //namespace CXX

#endif // DROPDOWNBOX_H

