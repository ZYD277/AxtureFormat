#include "mcontrolimproves.h"
#include "mcontrolimprove.h"

namespace RQt{

McontrolImproves::McontrolImproves()
{

}

McontrolImproves::~McontrolImproves()
{
    for(int i = 0; i < m_controlImproves.size(); i++){
        delete m_controlImproves.at(i);
    }
}

void McontrolImproves::write(QXmlStreamWriter &writer, QString tagName)
{
    writer.writeStartElement(tagName.isEmpty() ? "customwidgets" : tagName);

    foreach(McontrolImprove * controlImprove,m_controlImproves){
		controlImprove->write(writer,"customwidget");
    }

    writer.writeEndElement();
}

void McontrolImproves::addCustomControl(McontrolImprove *controlImprove)
{
    if(controlImprove)
        m_controlImproves.append(controlImprove);
}

} //namespace RQt
