#ifndef HTMLPARSEMETHOD_H
#define HTMLPARSEMETHOD_H

#include <QSharedPointer>
#include "../util/fileutils.h"
#include "htmlstruct.h"

namespace Html{

class DomWrapper{
public:
    DomWrapper(QDomElement & element):dataElement(element){}

    QDomElement data(){return dataElement;}

    DomWrapper & firstChild(){
        dataElement = getIndexChildElement(dataElement,0);
        return *this;
    }

    DomWrapper & secondChild(){
        dataElement = getIndexChildElement(dataElement,1);
        return *this;
    }

    DomWrapper & parent(){
        if(!isNull() && !(dataElement.parentNode().isNull())){
            dataElement = dataElement.parentNode().toElement();
        }
        return *this;
    }

    QString attribute(QString attr){
        if(!isNull() && !attr.isEmpty())
            return dataElement.attribute(attr);
        return QString();
    }

    bool isNull(){return dataElement.isNull();}

private:
    QDomElement getIndexChildElement(const QDomElement & element,int index);

private:
    QDomElement dataElement;
};

class HtmlParseMethod : public RXmlParseMethod
{
public:
    HtmlParseMethod();

    bool  startParse(QDomNode & rootNode) override;

    DomHtmlPtr getParsedResult(){return m_result;}

private:
    void parseBody(const QDomElement &bodyElement, DomHtml *parentNode);
    void parseDiv(const QDomElement &divElement, DomNode *parentNode);

    NodeType getNodeType(const QDomElement &element, QDomElement parentElement);
    bool hasChilds(const QDomElement & element){return element.childNodes().size() > 0;}

    QDomElement firstChildElement(const QDomElement & element);
    QDomElement secondChildElement(const QDomElement & element);
    QDomElement getIndexChildElement(const QDomElement & element,int index);

    inline void establishRelation(DomNode *parentNode,DomNode *childNode);

    void printBody(DomNode *node);

    void parseNodeData(QDomElement & element, NodeType type, DomNode *node);
    void parseButtonNodeData(QDomElement &element, DomNode *node);
    void parseRadioButtonNodeData(QDomElement &element, DomNode *node);
    void parserdynamicPanelNodeData(QDomElement &element, DomNode *parentNode);
    void parseLabelNodeData(QDomElement &element, DomNode *node);
    void parseTextFieldNodeData(QDomElement &element, DomNode *node);
    void parseImageNodeData(QDomElement &element, DomNode *node);
    void parseTableNodeData(QDomElement &element, DomNode *node);
    void parseTreeNodeData(QDomElement &element, DomNode *node);
    void parseTableCellNodeData(QDomElement &element, DomNode *parentNode);

    void parseGroupNodeData(QDomElement &element, DomNode *node);

private:
    NodeHtml m_nodeType;
    DomHtmlPtr m_result;
    int m_rdynamic;
    QString m_classInfo;
};

} //namespace Html

#endif // HTMLPARSEMETHOD_H
