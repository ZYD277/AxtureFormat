#ifndef QTPARSEMETHOD_H
#define QTPARSEMETHOD_H

#include <QMap>
#include <QSharedPointer>
#include "../html/htmlstruct.h"
#include "../css/cssstruct.h"
#include "../util/fileutils.h"
#include "qtstruct.h"

namespace RQt{

class DocWrapper{
public:
    DocWrapper(QDomDocument * doc):m_doc(doc){}

    QDomElement data(){return dataElement;}

    DocWrapper & createElement(QString tagName,bool rootNode = false){
        dataElement = m_doc->createElement(tagName);
        if(rootNode)
            m_doc->appendChild(dataElement);
        return * this;
    }

    DocWrapper & setAttribute(QString name,QString value){
        if(!dataElement.isNull())
            dataElement.setAttribute(name,value);
        return *this;
    }

    DocWrapper & setData(QString text){
        if(!dataElement.isNull())
            dataElement.setNodeValue(text);
        return *this;
    }

    DocWrapper & createChildElement(QString tagName){
        if(!dataElement.isNull()){
            QDomElement child = m_doc->createElement(tagName);
            dataElement.appendChild(child);
            dataElement = child;
        }
        return *this;
    }

    DocWrapper & createTextNode(QString text){
        if(!dataElement.isNull()){
            QDomText textNode = m_doc->createTextNode(text);
            dataElement.appendChild(textNode);
        }
        return *this;
    }

    DocWrapper & parent(){
        if(!isNull() && !(dataElement.parentNode().isNull())){
            dataElement = dataElement.parentNode().toElement();
        }
        return *this;
    }

    bool isNull(){return dataElement.isNull();}

private:
    QDomDocument * m_doc;
    QDomElement dataElement;
};

class QtParseMethod : public RXmlParseMethod
{
public:
    QtParseMethod();

    void setDataSource(DomHtmlPtr ptr);
    void setCssMap(CSS::CssMap globalCss, CSS::CssMap pageCss);
    void setResFile(QString resFile);

    bool  startSave(QDomDocument & doc) override;

    QStringList getOriginalResources(){return m_originalResources;}
    QStringList getResources(){return m_resources;}

private:
    void generateDomClass(QDomDocument &doc, QDomElement parent, QString nodeText);
    void generateResources(QDomDocument &doc, QDomElement parent);
    void generateNodeToUI(QDomDocument &doc, QDomElement parent, Html::DomNode * node, QRect parentGlobalRect);

    void generateRect(QDomDocument &doc,QDomElement parent,QRect rect);
    QMap<QString, QString> extractCssRule(Html::DomNode * node);

    int removePxUnit(QString valueWithUnit);

    QDomElement createChildElement(PropertyType ptype,PropValueType vtype,QDomDocument &doc,QString prpoName, QString value);

private:
    DomHtmlPtr m_dataSrc;
    CSS::CssMap m_globalCss;
    CSS::CssMap m_pageCss;

    QString m_resFileName;

    QStringList m_originalResources;
    QStringList m_resources;

    QMap<Html::NodeType,QString> m_htmlMapToQtWidget;
};

} //namespace RQt

#endif // QTPARSEMETHOD_H
