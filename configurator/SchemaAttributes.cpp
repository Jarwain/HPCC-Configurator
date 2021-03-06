#include "jptree.hpp"
#include "jarray.hpp"
#include "SchemaCommon.hpp"
#include "SchemaAttributes.hpp"
#include "SchemaAppInfo.hpp"
#include "SchemaSimpleType.hpp"
#include "DocumentationMarkup.hpp"
#include "DojoJSMarkup.hpp"
#include "ConfigSchemaHelper.hpp"
#include "DojoHelper.hpp"
#include "QMLMarkup.hpp"
#include "ExceptionStrings.hpp"
#include "SchemaMapManager.hpp"
#include "ConfiguratorMain.hpp"
#include "jlog.hpp"
#include "SchemaKey.hpp"
#include "SchemaKeyRef.hpp"
#include "SchemaSimpleType.hpp"

CAttribute::~CAttribute()
{
    //CConfigSchemaHelper::getInstance()->getSchemaMapManager()->removeMapOfXPathToAttribute(this->getEnvXPath());
}

bool CAttribute::isHidden()
{
    if(this->getAnnotation()->getAppInfo() != NULL && !stricmp(this->getAnnotation()->getAppInfo()->getViewType(),"hidden"))
    {
        return true;
    }
    return false;
}

const char* CAttribute::getTitle() const
{
    if (this->m_pAnnotation != NULL && this->m_pAnnotation->getAppInfo() != NULL && this->m_pAnnotation->getAppInfo()->getTitle() != NULL && this->m_pAnnotation->getAppInfo()->getTitle()[0] != 0)
    {
        return this->m_pAnnotation->getAppInfo()->getTitle();
    }
    else
    {
        return this->getName();
    }
}

const char* CAttribute::getXML(const char* pComponent)
{
    if (m_strXML.length() == 0)
    {
        m_strXML.append(getName()).append("=\"").append(getDefault()).append("\"");
    }

    return m_strXML.str();
}

void CAttribute::dump(std::ostream& cout, unsigned int offset) const
{
    offset+= STANDARD_OFFSET_1;

    QuickOutHeader(cout,XSD_ATTRIBUTE_STR, offset);

    QUICK_OUT(cout, Name,   offset);
    QUICK_OUT(cout, Type,   offset);
    QUICK_OUT(cout, Default,offset);
    QUICK_OUT(cout, Use,    offset);
    QUICK_OUT(cout, XSDXPath,  offset);
    QUICK_OUT(cout, EnvXPath,  offset);
    QUICK_OUT(cout, EnvValueFromXML,  offset);

    if (m_pAnnotation != NULL)
    {
        m_pAnnotation->dump(cout, offset);
    }

    if (m_pSimpleTypeArray != NULL)
    {
        m_pSimpleTypeArray->dump(cout, offset);
    }

    QuickOutFooter(cout,XSD_ATTRIBUTE_STR, offset);
}

void CAttribute::getDocumentation(StringBuffer &strDoc) const
{
    const char *pName = this->getTitle();
    const char *pToolTip = NULL;
    const char *pDefaultValue = this->getDefault();
    const char *pRequired = this->getUse();

    if (m_pAnnotation != NULL && m_pAnnotation->getAppInfo() != NULL)
    {
        const CAppInfo *pAppInfo = m_pAnnotation->getAppInfo();
        const char* pViewType = pAppInfo->getViewType();

        if (pViewType != NULL && stricmp("hidden", pViewType) == 0)
        {
            return; // HIDDEN
        }
        else
        {
            pToolTip = pAppInfo->getToolTip();
            /*if (pAppInfo->getTitle() != NULL && (pAppInfo->getTitle())[0] != 0)
            {
                pName = pAppInfo->getTitle();  // if we have a title, then we use it instead of the attribute name
            }*/
        }
    }

    strDoc.appendf("<%s>\n", DM_TABLE_ROW);
    strDoc.appendf("<%s>%s</%s>\n", DM_TABLE_ENTRY, pName, DM_TABLE_ENTRY);
    strDoc.appendf("<%s>%s</%s>\n", DM_TABLE_ENTRY, pToolTip, DM_TABLE_ENTRY);

    if (m_pSimpleTypeArray == NULL)
    {
        strDoc.appendf("<%s>%s</%s>\n", DM_TABLE_ENTRY, pDefaultValue, DM_TABLE_ENTRY);
    }
    else
    {
        StringBuffer strDocTemp(pDefaultValue);

        m_pSimpleTypeArray->getDocumentation(strDocTemp);

        strDoc.appendf("<%s>%s</%s>\n", DM_TABLE_ENTRY, strDocTemp.str(), DM_TABLE_ENTRY);
    }

    if (m_pAnnotation != NULL && m_pAnnotation->getAppInfo() != NULL && m_pAnnotation->getAppInfo()->getDocLineBreak() == true)
    {
        strDoc.appendf("<%s>%s%s</%s>\n", DM_TABLE_ENTRY, DM_LINE_BREAK2, pRequired, DM_TABLE_ENTRY);
    }
    else
    {
        strDoc.appendf("<%s>%s</%s>\n", DM_TABLE_ENTRY, pRequired, DM_TABLE_ENTRY);
    }
    strDoc.appendf("</%s>\n", DM_TABLE_ROW);
}

void CAttribute::getDojoJS(StringBuffer &strJS) const
{
    assert(this->getConstParentNode() != NULL);

    const char* pViewType = NULL;
    const char* pColumnIndex = NULL;
    const char* pXPath = NULL;
    const CXSDNodeBase *pGrandParentNode =this->getConstAncestorNode(2);
    const CElement *pNextHighestElement = dynamic_cast<const CElement*>(this->getParentNodeByType(XSD_ELEMENT));

    if (m_pAnnotation != NULL && m_pAnnotation->getAppInfo() != NULL)
    {
        const CAppInfo *pAppInfo = NULL;
        pAppInfo = m_pAnnotation->getAppInfo();
        pViewType = pAppInfo->getViewType();
        pColumnIndex = (pAppInfo->getColIndex() != NULL && pAppInfo->getColIndex()[0] != 0) ? pAppInfo->getColIndex() : NULL;
        pXPath = (pAppInfo->getXPath() != NULL && pAppInfo->getXPath()[0] != 0) ? pAppInfo->getXPath() : NULL;
    }

    if (pViewType != NULL && stricmp("hidden", pViewType) == 0)
    {
        return; // HIDDEN
    }

    if ((pColumnIndex != NULL && pColumnIndex[0] != 0) || (pXPath != NULL && pXPath[0] != 0) || (pGrandParentNode != NULL && pGrandParentNode->getNodeType() != XSD_ATTRIBUTE_GROUP && pGrandParentNode->getNodeType() != XSD_COMPLEX_TYPE && pGrandParentNode->getNodeType() != XSD_ELEMENT) || (pGrandParentNode->getNodeType() == XSD_ELEMENT && stricmp( (dynamic_cast<const CElement*>(pGrandParentNode))->getMaxOccurs(), "unbounded") == 0) || (pNextHighestElement != NULL && pNextHighestElement->getMaxOccurs() != NULL && pNextHighestElement->getMaxOccurs()[0] != 0))
    {
        strJS.append(DJ_LAYOUT_CONCAT_BEGIN);
        strJS.append(createDojoColumnLayout(this->getTitle(), getRandomID()));
        strJS.append(DJ_LAYOUT_CONCAT_END);
    }
    else //if (this->getDefault() != NULL && this->getDefault()[0] != 0)
    {
        StringBuffer id("ID_");
        id.append(getRandomID());

        StringBuffer strToolTip(DJ_TOOL_TIP_BEGIN);

        strToolTip.append(DJ_TOOL_TIP_CONNECT_ID_BEGIN);
        strToolTip.append(id.str());
        strToolTip.append(DJ_TOOL_TIP_CONNECT_ID_END);
        DEBUG_MARK_STRJS;

        if (this->getAnnotation()->getAppInfo() != NULL) // check for tooltip
        {
            StringBuffer strTT(this->getAnnotation()->getAppInfo()->getToolTip());
            strTT.replaceString("\"","\\\"");

            strToolTip.append(DJ_TOOL_TIP_LABEL_BEGIN).append(strTT.str()).append(DJ_TOOL_TIP_LABEL_END);
            strToolTip.append(DJ_TOOL_TIP_END);
            DEBUG_MARK_STRJS;
        }

        if (this->m_pSimpleTypeArray->length() == 0)
        {
            strJS.append(DJ_TABLE_ROW_PART_1).append(this->getTitle()).append(DJ_TABLE_ROW_PART_PLACE_HOLDER).append(this->getDefault()).append(DJ_TABLE_ROW_PART_ID_BEGIN).append(id.str()).append(DJ_TABLE_ROW_PART_ID_END);
        }
        else
        {
            m_pSimpleTypeArray->getDojoJS(strJS);

            if (this->getAnnotation()->getAppInfo() != NULL) // check for tooltip
            {
                CConfigSchemaHelper::getInstance()->addToolTip(strToolTip.str());
            }
        }

        if (this->getAnnotation() != NULL && this->getAnnotation()->getAppInfo() != NULL && this->getAnnotation() != NULL && this->getAnnotation()->getAppInfo()->getToolTip() != NULL && this->getAnnotation() != NULL && this->getAnnotation()->getAppInfo()->getToolTip()[0] != 0)
        {
            strJS.append(DJ_ADD_CHILD);
            DEBUG_MARK_STRJS;

            CConfigSchemaHelper::getInstance()->addToolTip(strToolTip.str());
        }
    }
  /*  else
    {
        strJS.append(DJ_TABLE_ROW_PART_1).append(this->getTitle()).append(DJ_TABLE_ROW_PART_2);
    }*/

/*    if (m_pSimpleTypeArray == NULL)
    {

    }
    else
    {
        m_pSimpleTypeArray->getDojoJS(strJS);
    }*/
}

#ifdef _USE_OLD_GET_QML_
void CAttribute::getQML(StringBuffer &strQML, int idx) const
{
    assert(this->getConstParentNode() != NULL);

    const char* pViewType = NULL;

    if (m_pAnnotation != NULL && m_pAnnotation->getAppInfo() != NULL)
    {
        const CAppInfo *pAppInfo = NULL;
        pAppInfo = m_pAnnotation->getAppInfo();
        pViewType = pAppInfo->getViewType();
    }

    if (pViewType != NULL && stricmp("hidden", pViewType) == 0)
    {
        return; // HIDDEN
    }

    if (CQMLMarkupHelper::isTableRequired(this) == true)
    {
        CQMLMarkupHelper::getTableViewColumn(strQML, this->getTitle(), this->getEnvXPath());
        DEBUG_MARK_QML;
    }
    else
    {
        if (this->m_pSimpleTypeArray->length() == 0)
        {
            strQML.append(QML_ROW_BEGIN).append(QML_RECTANGLE_DEFAULT_COLOR_SCHEME_1_BEGIN);
            DEBUG_MARK_QML;

            strQML.append(QML_TEXT_BEGIN_2).append("\"  ").append(this->getTitle()).append("\"").append(QML_TEXT_END_2);
            DEBUG_MARK_QML;

            strQML.append(QML_RECTANGLE_LIGHT_STEEEL_BLUE_END);
            DEBUG_MARK_QML;

            strQML.append(QML_TEXT_FIELD_BEGIN);

            StringBuffer strTextArea("textarea");
            CQMLMarkupHelper::getRandomID(&strTextArea);

            strQML.append(QML_APP_DATA_GET_VALUE_BEGIN).append(this->getEnvXPath()).append(QML_APP_DATA_GET_VALUE_END);

            strQML.append(QML_ON_ACCEPTED);
            strQML.append(QML_APP_DATA_SET_VALUE_BEGIN).append(this->getEnvXPath()).append("\", ").append(strTextArea.str()).append(QML_APP_DATA_SET_VALUE_END);

            strQML.append(QML_TEXT_FIELD_ID_BEGIN).append(strTextArea).append(QML_TEXT_FIELD_ID_END);
            DEBUG_MARK_QML;

            strQML.append(QML_TEXT_FIELD_PLACE_HOLDER_TEXT_BEGIN);
            strQML.append("\"").append(this->getDefault()).append("\"");
            strQML.append(QML_TEXT_FIELD_PLACE_HOLDER_TEXT_END);
            DEBUG_MARK_QML;

            if (this->getAnnotation()->getAppInfo() != NULL) // check for tooltip
            {
                CQMLMarkupHelper::getToolTipQML(strQML, this->getAnnotation()->getAppInfo()->getToolTip(), strTextArea.str());
            }

            strQML.append(QML_TEXT_FIELD_END);
            DEBUG_MARK_QML;

            strQML.append(QML_ROW_END);
            DEBUG_MARK_QML;
        }
        else
        {
            m_pSimpleTypeArray->getQML(strQML);

            if (this->getAnnotation()->getAppInfo() != NULL) // check for tooltip
            {
            }
        }

        if (this->getAnnotation() != NULL && this->getAnnotation()->getAppInfo() != NULL && this->getAnnotation() != NULL && this->getAnnotation()->getAppInfo()->getToolTip() != NULL && this->getAnnotation() != NULL && this->getAnnotation()->getAppInfo()->getToolTip()[0] != 0)
        {

        }
    }
}
#else // _USE_OLD_GET_QML_

void CAttribute::getQML(StringBuffer &strQML, int idx) const
{
    assert(this->getConstParentNode() != NULL && this->getConstParentNode()->getNodeType() == XSD_ATTRIBUTE_ARRAY);


    // HPCC-Specific  TODO: Remove HPCC specific stuff
    const char* pViewType = NULL;

    if (m_pAnnotation != NULL && m_pAnnotation->getAppInfo() != NULL)
    {
        const CAppInfo *pAppInfo = NULL;

        pAppInfo = m_pAnnotation->getAppInfo();
        pViewType = pAppInfo->getViewType();
    }

    if (pViewType != NULL && stricmp("hidden", pViewType) == 0) // HPCC-Specific
    {
        return; // HIDDEN
    }
    // HPCC-Specifc end

    const CElement *pElement = dynamic_cast<const CElement*>(this->getParentNodeByType(XSD_ELEMENT));

    if (pElement != NULL && stricmp(pElement->getMaxOccurs(), TAG_UNBOUNDED) == 0)
    {
        CQMLMarkupHelper::getTableViewColumn(strQML, this->getTitle(), this->getEnvXPath());
        DEBUG_MARK_QML;
    }
    else
    {
        strQML.append(QML_ROW_BEGIN).append(QML_RECTANGLE_DEFAULT_COLOR_SCHEME_1_BEGIN);
        DEBUG_MARK_QML;

        strQML.append(QML_TEXT_BEGIN_2).append("\"  ").append(this->getTitle()).append("\"").append(QML_TEXT_END_2);
        DEBUG_MARK_QML;

        strQML.append(QML_RECTANGLE_LIGHT_STEEEL_BLUE_END);
        DEBUG_MARK_QML;

        strQML.append(QML_TEXT_FIELD_BEGIN);

        StringBuffer strTextArea("textarea");
        CQMLMarkupHelper::getRandomID(&strTextArea);

        strQML.append(QML_APP_DATA_GET_VALUE_BEGIN).append(this->getEnvXPath()).append(QML_APP_DATA_GET_VALUE_END);

        strQML.append(QML_ON_ACCEPTED);
        strQML.append(QML_APP_DATA_SET_VALUE_BEGIN).append(this->getEnvXPath()).append("\", ").append(strTextArea.str()).append(QML_APP_DATA_SET_VALUE_END);

        strQML.append(QML_TEXT_FIELD_ID_BEGIN).append(strTextArea).append(QML_TEXT_FIELD_ID_END);
        DEBUG_MARK_QML;

        strQML.append(QML_TEXT_FIELD_PLACE_HOLDER_TEXT_BEGIN);
        strQML.append("\"").append(this->getDefault()).append("\"");
        strQML.append(QML_TEXT_FIELD_PLACE_HOLDER_TEXT_END);
        DEBUG_MARK_QML;

        if (this->getAnnotation()->getAppInfo() != NULL) // check for tooltip
        {
            CQMLMarkupHelper::getToolTipQML(strQML, this->getAnnotation()->getAppInfo()->getToolTip(), strTextArea.str());
        }

        strQML.append(QML_TEXT_FIELD_END);
        DEBUG_MARK_QML;

        strQML.append(QML_ROW_END);
        DEBUG_MARK_QML;
    }
}

#endif // _USE_OLD_GET_QML_

void CAttribute::getQML2(StringBuffer &strQML, int idx) const
{
    if (this->getParentNode()->getUIType() == QML_UI_TABLE_CONTENTS)
    {
        this->setUIType(QML_UI_TABLE_CONTENTS);
        CQMLMarkupHelper::getTableViewColumn(strQML, this->getTitle(), this->getEnvXPath());
        DEBUG_MARK_QML;
    }
    else if (this->getParentNode()->getUIType() == QML_UI_TAB_CONTENTS)
    {
        this->setUIType(QML_UI_TEXT_FIELD);
        strQML.append(QML_ROW_BEGIN).append(QML_RECTANGLE_DEFAULT_COLOR_SCHEME_1_BEGIN);
        DEBUG_MARK_QML;

        strQML.append(QML_TEXT_BEGIN_2).append("\"  ").append(this->getTitle()).append("\"").append(QML_TEXT_END_2);
        DEBUG_MARK_QML;

        strQML.append(QML_RECTANGLE_LIGHT_STEEEL_BLUE_END);
        DEBUG_MARK_QML;

        strQML.append(QML_TEXT_FIELD_BEGIN);

        StringBuffer strTextArea("textarea");
        CQMLMarkupHelper::getRandomID(&strTextArea);

        strQML.append(QML_APP_DATA_GET_VALUE_BEGIN).append(this->getEnvXPath()).append(QML_APP_DATA_GET_VALUE_END);

        strQML.append(QML_ON_ACCEPTED);
        strQML.append(QML_APP_DATA_SET_VALUE_BEGIN).append(this->getEnvXPath()).append("\", ").append(strTextArea.str()).append(QML_APP_DATA_SET_VALUE_END);

        strQML.append(QML_TEXT_FIELD_ID_BEGIN).append(strTextArea).append(QML_TEXT_FIELD_ID_END);
        DEBUG_MARK_QML;

        strQML.append(QML_TEXT_FIELD_PLACE_HOLDER_TEXT_BEGIN);
        strQML.append("\"").append(this->getDefault()).append("\"");
        strQML.append(QML_TEXT_FIELD_PLACE_HOLDER_TEXT_END);
        DEBUG_MARK_QML;

        if (this->getAnnotation()->getAppInfo() != NULL) // check for tooltip
        {
            CQMLMarkupHelper::getToolTipQML(strQML, this->getAnnotation()->getAppInfo()->getToolTip(), strTextArea.str());
        }

        strQML.append(QML_TEXT_FIELD_END);
        DEBUG_MARK_QML;

        strQML.append(QML_ROW_END);
        DEBUG_MARK_QML;
    }
    else
    {
        assert(!"what am i?");
    }
}

void CAttribute::getQML3(StringBuffer &strQML, const char * role, int idx) const
{
    /*
    Sample QML Returned by Attribute
        value: ListElement {
            value: [ListElement{value: "A Masterpiece"}] // Or multiple for ComboBoxes
            type: "field" 
            tooltip:"Hey"
            placeholder: "kay" 
        }
     */
    DEBUG_MARK_QML;
    StringBuffer name(role == NULL || role[0] == '\0' ? this->getName() : role);
    StringArray values;
    values.append(this->getEnvXPath());
    if(this->getAnnotation()->getAppInfo() != NULL)
    {
        const char * viewType = this->getAnnotation()->getAppInfo()->getViewType();
        if( viewType != NULL)
        {
            if(!stricmp(viewType,"readonly"))
                viewType = "text";
            else
                viewType = "field";
        } else viewType = "field";
        CQMLMarkupHelper::buildRole(strQML, name, values, viewType, this->getAnnotation()->getAppInfo()->getToolTip(), this->getDefault());
    } else
    {
        CQMLMarkupHelper::buildRole(strQML, name, values, "field");
    }
    DEBUG_MARK_QML
}

void CAttribute::populateEnvXPath(StringBuffer strXPath, unsigned int index)
{
    assert(this->getName() != NULL);

    const char *pChar = strrchr(strXPath.str(),'[');

    assert(pChar != NULL && strlen(pChar) >= 3);

    strXPath.setLength(strXPath.length()-strlen(pChar));  // remove [N] from XPath;
    strXPath.appendf("[%d]", index);

    strXPath.append("/").append("[@").append(this->getName()).append("]");

    this->setEnvXPath(strXPath.str());

    PROGLOG("Mapping attribute with XPATH of %s to %p", this->getEnvXPath(), this);

    //if (index > 1) //if not added as part of the initial XSD
    {
        CConfigSchemaHelper::getInstance()->getSchemaMapManager()->addMapOfXPathToAttribute(this->getEnvXPath(), this);
    }

    CConfigSchemaHelper::getInstance()->appendAttributeXPath(this->getEnvXPath());

    if (this->m_pSimpleTypeArray != NULL)
    {
        m_pSimpleTypeArray->populateEnvXPath(strXPath);
    }
}

void CAttribute::loadXMLFromEnvXml(const IPropertyTree *pEnvTree)
{
    assert(this->getEnvXPath() != NULL);
    assert(this->getConstParentNode()->getEnvXPath() != NULL);

    StringBuffer strXPath(this->getConstParentNode()->getEnvXPath());

    if (pEnvTree->hasProp(strXPath.str()) == true)
    {
        StringBuffer strAttribName("@");
        strAttribName.append(this->getName());
        this->setEnvValueFromXML(pEnvTree->queryPropTree(strXPath.str())->queryProp(strAttribName.str()));

        setInstanceAsValid();

        if (this->m_pSimpleTypeArray != NULL)
        {
            m_pSimpleTypeArray->loadXMLFromEnvXml(pEnvTree);
        }
    }
    else if (stricmp(this->getUse(), XML_ENV_VALUE_REQUIRED) == 0) // check if this a required attribute
    {
        assert(false);  // required value missing
        throw MakeExceptionFromMap(EX_STR_MISSING_REQUIRED_ATTRIBUTE);
    }
}

void CAttribute::setEnvValueFromXML(const char *p)
{
    if (p == NULL)
    {
        if (this->getDefault() != NULL && *(this->getDefault()) != 0)
        {
            this->setInstanceValue(this->getDefault());
            this->setInstanceAsValid(true);
            return;
        }

        if (this->getUse() != NULL && stricmp(this->getUse(), TAG_REQUIRED) != 0)
        {
            return;
        }
        else
        {
            this->setInstanceAsValid(false);
            assert (!"Missing attribute property, property not marked as optional");

            return;
        }
    }

    if (this->getTypeNode() != NULL)
    {
        const CXSDBuiltInDataType *pNodeBuiltInType = dynamic_cast<const CXSDBuiltInDataType*>(this->getTypeNode());

        if (pNodeBuiltInType != NULL && pNodeBuiltInType->checkConstraint(p) == false)
        {
            this->setInstanceAsValid(false);
            assert (!"Invalid value for data type");

            return;
        }

        if (this->getTypeNode()->getNodeType() == XSD_SIMPLE_TYPE)
        {
            const CSimpleType *pNodeSimpleType = dynamic_cast<const CSimpleType*>(this->getTypeNode());

            //if (pNodeSimpleType != NULL && pNodeSimpleType->)
        }

    }

    if (this->m_ReverseKeyRefArray.length() > 0)
    {
        for (int idx = 0; this->m_ReverseKeyRefArray.length(); idx++)
        {
           CKeyRef *pKeyRef = static_cast<CKeyRef*>((this->m_ReverseKeyRefArray.item(idx)));

            assert(pKeyRef != NULL);

            if (pKeyRef->checkConstraint(p) == false)
            {
                this->setInstanceAsValid(false);
                assert (!"Constraint Violated");

                return;
            }
        }
    }

    this->setInstanceValue(p);
    this->setInstanceAsValid(true);
}

void CAttribute::appendReverseKeyRef(const CKeyRef *pKeyRef)
{
    assert(pKeyRef != NULL);

    if (pKeyRef != NULL)
    {
        this->m_ReverseKeyRefArray.append(static_cast<void*>(const_cast<CKeyRef*>(pKeyRef)));
    }
}

void CAttribute::appendReverseKey(const CKey *pKey)
{
    assert(pKey != NULL);

    if (pKey != NULL)
    {
        this->m_ReverseKeyArray.append(static_cast<void*>(const_cast<CKey*>(pKey)));
    }
}

CAttribute* CAttribute::load(CXSDNodeBase* pParentNode, const IPropertyTree *pSchemaRoot, const char* xpath)
{
    assert(pSchemaRoot != NULL);

    if (pSchemaRoot == NULL)
    {
        return NULL;
    }

    CAttribute *pAttribute = new CAttribute(pParentNode);

    pAttribute->setXSDXPath(xpath);

    Owned<IAttributeIterator> iterAttrib = pSchemaRoot->queryPropTree(xpath)->getAttributes(true);

    ForEach(*iterAttrib)
    {
        if (strcmp(iterAttrib->queryName(), XML_ATTR_NAME) == 0)
        {
            pAttribute->setName(iterAttrib->queryValue());
        }
        else if (strcmp(iterAttrib->queryName(), XML_ATTR_DEFAULT) == 0)
        {
            pAttribute->setDefault(iterAttrib->queryValue());
        }
        else if (strcmp(iterAttrib->queryName(), XML_ATTR_USE) == 0)
        {
            pAttribute->setUse(iterAttrib->queryValue());
        }
        else if (strcmp(iterAttrib->queryName(), XML_ATTR_TYPE) == 0)
        {
            const char *pType = iterAttrib->queryValue();

            assert(pType != NULL && *pType != 0);

            if (pType != NULL && *pType != 0)
            {
                pAttribute->setType(pType);
                CConfigSchemaHelper::getInstance()->addNodeForTypeProcessing(pAttribute);
            }
        }
    }

    const char *pType = pSchemaRoot->queryPropTree(xpath)->queryProp(XML_ATTR_TYPE);
/*
    // special case for naming.
    if (pType != NULL && stricmp(pType, TAG_COMPUTERTYPE) == 0)
    {
        pAttribute->setName(TAG_NAME);
    }
*/
    StringBuffer strXPathExt(xpath);

    strXPathExt.append("/").append(XSD_TAG_ANNOTATION);

    CAnnotation *pAnnotation = CAnnotation::load(pAttribute, pSchemaRoot, strXPathExt.str());

    if (pAnnotation != NULL)
    {
        pAttribute->setAnnotation(pAnnotation);
    }

    strXPathExt.clear().append(xpath);

    strXPathExt.append("/").append(XSD_TAG_SIMPLE_TYPE);

    CSimpleTypeArray *pSimpleTypeArray = CSimpleTypeArray::load(pAttribute, pSchemaRoot, strXPathExt.str());

    if (pSimpleTypeArray != NULL)
    {
        pAttribute->setSimpleTypeArray(pSimpleTypeArray);
    }

    return pAttribute;
}

const char* CAttributeArray::getXML(const char* /*pComponent*/)
{
    if (m_strXML.length() == 0)
    {
        int length = this->length();

        for (int idx = 0; idx < length; idx++)
        {
            CAttribute &Attribute = this->item(idx);

            m_strXML.append(" ").append(Attribute.getXML(NULL));

            if (idx+1 < length)
            {
                m_strXML.append("\n");
            }
        }
    }

    return m_strXML.str();
}

CAttributeArray* CAttributeArray::load(const char* pSchemaFile)
{
    assert(pSchemaFile != NULL);

    if (pSchemaFile == NULL)
    {
        return NULL;
    }

    Linked<IPropertyTree> pSchemaRoot;

    StringBuffer schemaPath;
    schemaPath.appendf("%s%s", DEFAULT_SCHEMA_DIRECTORY, pSchemaFile);
    pSchemaRoot.setown(createPTreeFromXMLFile(schemaPath.str()));

    StringBuffer strXPathExt("./");
    strXPathExt.append(XSD_TAG_ATTRIBUTE);

    return CAttributeArray::load(NULL, pSchemaRoot, strXPathExt.str());
}

CAttributeArray* CAttributeArray::load(CXSDNodeBase* pParentNode, const IPropertyTree *pSchemaRoot, const char* xpath)
{
    assert(pSchemaRoot != NULL);

    if (pSchemaRoot == NULL || xpath == NULL)
    {
        return NULL;
    }

    StringBuffer strXPathExt(xpath);

    CAttributeArray *pAttribArray = new CAttributeArray(pParentNode);

    pAttribArray->setXSDXPath(xpath);

    Owned<IPropertyTreeIterator> attributeIter = pSchemaRoot->getElements(xpath, ipt_ordered);

    int count = 1;
    ForEach(*attributeIter)
    {
        strXPathExt.clear().append(xpath).appendf("[%d]",count);

        CAttribute *pAttrib = CAttribute::load(pAttribArray, pSchemaRoot, strXPathExt.str());

        if (pAttrib != NULL)
        {
            pAttribArray->append(*pAttrib);
        }

        count++;
    }

    if (pAttribArray->length() == 0)
    {
        delete pAttribArray;
        return NULL;
    }

    return pAttribArray;
}

void CAttributeArray::dump(std::ostream &cout, unsigned int offset) const
{
    offset+= STANDARD_OFFSET_1;

    QuickOutHeader(cout, XSD_ATTRIBUTE_ARRAY_STR, offset);

    QUICK_OUT(cout, XSDXPath,  offset);
    QUICK_OUT(cout, EnvXPath,  offset);
    QUICK_OUT_ARRAY(cout, offset);

    QuickOutFooter(cout, XSD_ATTRIBUTE_ARRAY_STR, offset);
}


void CAttributeArray::getDocumentation(StringBuffer &strDoc) const
{
    assert(this->getConstParentNode() != NULL);

    strDoc.append(DM_SECT4_BEGIN);

    if (this->getConstParentNode()->getNodeType() == XSD_COMPLEX_TYPE && this->getConstParentNode()->getConstParentNode()->getNodeType() != XSD_COMPLEX_TYPE)
    {
        strDoc.appendf("%s%s%s", DM_TITLE_BEGIN, "Attributes", DM_TITLE_END);  // Attributes is hard coded default
        DEBUG_MARK_STRDOC;
    }
    else
    {
        strDoc.append(DM_TITLE_BEGIN).append(DM_TITLE_END);
        DEBUG_MARK_STRDOC;
    }

    const CComplexType *pParentComplexType = dynamic_cast<const CComplexType*>(this->getConstParentNode());
    const CAttributeGroup *pParentAttributeGroup = dynamic_cast<const CAttributeGroup*>(this->getConstParentNode());

    strDoc.append(DM_TABLE_BEGIN);
    DEBUG_MARK_STRDOC;
    strDoc.append(DM_TABLE_ID_BEGIN);
    DEBUG_MARK_STRDOC;

    if (pParentComplexType != NULL && pParentComplexType->getAnnotation() != NULL && pParentComplexType->getAnnotation()->getAppInfo() != NULL && pParentComplexType->getAnnotation()->getAppInfo()->getDocTableID() != NULL)
    {
        strDoc.append(pParentComplexType->getAnnotation()->getAppInfo()->getDocTableID());
        DEBUG_MARK_STRDOC;
    }
    else if  (pParentAttributeGroup != NULL && pParentAttributeGroup->getAnnotation() != NULL && pParentAttributeGroup->getAnnotation()->getAppInfo() != NULL && pParentAttributeGroup->getAnnotation()->getAppInfo()->getDocTableID() != NULL)
    {
        strDoc.append(pParentAttributeGroup->getAnnotation()->getAppInfo()->getDocTableID());
        DEBUG_MARK_STRDOC;
    }
    else
    {
        static unsigned undefined_counter = 1;
        strDoc.append(DM_TABLE_ID_UNDEFINED).append("-").append(undefined_counter++);
        DEBUG_MARK_STRDOC;
    }

    strDoc.append(DM_TABLE_ID_END);
    DEBUG_MARK_STRDOC;

    strDoc.append(DM_TGROUP4_BEGIN);
    strDoc.append(DM_COL_SPEC4);
    strDoc.append(DM_TBODY_BEGIN);

    DEBUG_MARK_STRDOC;
    QUICK_DOC_ARRAY(strDoc);
    DEBUG_MARK_STRDOC;

    strDoc.append(DM_TBODY_END);
    strDoc.append(DM_TGROUP4_END);
    strDoc.append(DM_TABLE_END);

    strDoc.append(DM_SECT4_END);
}

void CAttributeArray::getDojoJS(StringBuffer &strJS) const
{
    assert(this->getConstParentNode() != NULL);
    assert(this->getConstAncestorNode(2) != NULL);


    if (this->getConstParentNode()->getNodeType() == XSD_COMPLEX_TYPE && this->getConstAncestorNode(3) != NULL && this->getConstAncestorNode(3)->getNodeType() == XSD_ELEMENT && CDojoHelper::IsElementATab(dynamic_cast<const CElement*>(this->getConstAncestorNode(3))) == true)
    {
        const char *pName = NULL;

        pName = dynamic_cast<const CElement*>(this->getConstAncestorNode(3))->getName();

        assert(pName != NULL);
        assert(pName[0] != 0);


        genTabDojoJS(strJS, pName);
        DEBUG_MARK_STRJS;


        const CComplexType *pComplexType = dynamic_cast<const CComplexType*>(this->getConstParentNode());

        if (pComplexType->getSequence() == NULL)
        {
            strJS.append(DJ_TABLE_PART_1);
            DEBUG_MARK_STRJS;
        }

        QUICK_DOJO_JS_ARRAY(strJS);

        if (pComplexType->getSequence() != NULL)
        {
            strJS.append(DJ_GRID);
            DEBUG_MARK_STRJS;
        }

        const CElementArray *pElemArray = dynamic_cast<const CElementArray*>(this->getParentNodeByType(XSD_ELEMENT_ARRAY));
        if (pElemArray != NULL && pElemArray->getConstParentNode()->getNodeType() == XSD_CHOICE)
        {
            strJS.append(DJ_LAYOUT_END);
            DEBUG_MARK_STRJS;
        }

        strJS.append(DJ_TABLE_PART_2);
        DEBUG_MARK_STRJS;
    }
    else
    {
        if (this->getConstParentNode()->getNodeType() == XSD_COMPLEX_TYPE && this->getConstAncestorNode(2)->getNodeType() != XSD_COMPLEX_TYPE && CDojoHelper::IsElementATab(dynamic_cast<const CElement*>(this->getConstAncestorNode(2))) == true)
        {
    //        genTabDojoJS(strJS, dynamic_cast<const CElement*>(this->getConstAncestorNode(3))->getName());
            DEBUG_MARK_STRJS;
        }
        else if (this->getConstParentNode()->getNodeType() == XSD_ATTRIBUTE_GROUP)
        {
            const CAttributeGroup *pAttributeGroup = dynamic_cast<const CAttributeGroup*>(this->getConstParentNode());

            assert(pAttributeGroup != NULL);

            if (pAttributeGroup != NULL)
            {
                genTabDojoJS(strJS, pAttributeGroup->getName());
                DEBUG_MARK_STRJS;
            }
        }
        else if (CDojoHelper::IsElementATab(dynamic_cast<const CElement*>(this->getConstAncestorNode(2))) == false && CElement::isAncestorTopElement(this) == true)
        {
            genTabDojoJS(strJS, "Attributes");
            DEBUG_MARK_STRJS;
        }

        strJS.append(DJ_TABLE_PART_1);
        DEBUG_MARK_STRJS;

        QUICK_DOJO_JS_ARRAY(strJS);

        strJS.append(DJ_TABLE_PART_2);
        strJS.append(DJ_TABLE_END);
    }
}

#ifdef _USE_OLD_GET_QML_
void CAttributeArray::getQML(StringBuffer &strQML, int idx) const
{
    assert(this->getConstParentNode() != NULL);
    assert(this->getConstAncestorNode(2) != NULL);


    if (this->getConstParentNode()->getNodeType() == XSD_COMPLEX_TYPE && this->getConstAncestorNode(3) != NULL && this->getConstAncestorNode(3)->getNodeType() == XSD_ELEMENT && CDojoHelper::IsElementATab(dynamic_cast<const CElement*>(this->getConstAncestorNode(3))) == true  && *(dynamic_cast<const CElement*>(this->getConstAncestorNode(3)))->getMaxOccurs() != 0)
    {
        const char *pName = NULL;

        pName = dynamic_cast<const CElement*>(this->getConstAncestorNode(3))->getName();

        assert(pName != NULL);
        assert(pName[0] != 0);

        strQML.append(QML_ROW_BEGIN);
        DEBUG_MARK_QML;

        strQML.append(QML_TABLE_VIEW_BEGIN);
        DEBUG_MARK_QML;

        strQML.append(QML_MODEL).append(modelNames[CConfigSchemaHelper::getInstance(0)->getNumberOfTables()]).append(QML_STYLE_NEW_LINE);
        DEBUG_MARK_QML;

        const CElement *pElement = dynamic_cast<const CElement*>(this->getParentNodeByType(XSD_ELEMENT));
        assert(pElement != NULL);

        strQML.append(QML_PROPERTY_STRING_TABLE_BEGIN).append(modelNames[CConfigSchemaHelper::getInstance(0)->getNumberOfTables()]).append(QML_PROPERTY_STRING_TABLE_PART_1).append(pElement->getXSDXPath()).append(QML_PROPERTY_STRING_TABLE_END);
        DEBUG_MARK_QML;

        CConfigSchemaHelper::getInstance(0)->incTables();

        QUICK_QML_ARRAY2(strQML);
        DEBUG_MARK_QML;

        strQML.append(QML_TABLE_VIEW_END);
        DEBUG_MARK_QML;

        strQML.append(QML_ROW_END);
        DEBUG_MARK_QML;
    }
    else
    {
        if (this->getConstParentNode()->getNodeType() == XSD_ATTRIBUTE_GROUP)
        {
            const CAttributeGroup *pAttributeGroup = dynamic_cast<const CAttributeGroup*>(this->getConstParentNode());

            assert(pAttributeGroup != NULL);

            if (pAttributeGroup != NULL)
            {
                CQMLMarkupHelper::getTabQML(strQML, pAttributeGroup->getName());
                DEBUG_MARK_QML;

                strQML.append(QML_GRID_LAYOUT_BEGIN);
                DEBUG_MARK_QML;

                QUICK_QML_ARRAY(strQML);

                strQML.append(QML_GRID_LAYOUT_END);
                DEBUG_MARK_QML;

                strQML.append(QML_FLICKABLE_END);
                DEBUG_MARK_QML;

                strQML.append(QML_TAB_END);
                DEBUG_MARK_QML;
            }
        }
        else if (CDojoHelper::IsElementATab(dynamic_cast<const CElement*>(this->getConstAncestorNode(2))) == false && CElement::isAncestorTopElement(this) == true)
        {
            CQMLMarkupHelper::getTabQML(strQML, "Attributes");
            DEBUG_MARK_QML;

            strQML.append(QML_GRID_LAYOUT_BEGIN);
            DEBUG_MARK_QML;

            QUICK_QML_ARRAY(strQML);

            strQML.append(QML_GRID_LAYOUT_END);
            DEBUG_MARK_QML;

            strQML.append(QML_FLICKABLE_END);
            DEBUG_MARK_QML;

            strQML.append(QML_TAB_END);
            DEBUG_MARK_QML;
        }
        else if (CQMLMarkupHelper::isTableRequired(&(this->item(0))) == true)
        {
            strQML.append(QML_ROW_BEGIN);
            DEBUG_MARK_QML;

            strQML.append(QML_TABLE_VIEW_BEGIN);
            DEBUG_MARK_QML;

            //strQML.append(QML_MODEL).append(QML_TABLE_DATA_MODEL);
            //CConfigSchemaHelper::getInstance(0)->incTables();
            //DEBUG_MARK_QML;
            strQML.append(QML_MODEL).append(modelNames[CConfigSchemaHelper::getInstance(0)->getNumberOfTables()]);
            DEBUG_MARK_QML;

            const CElementArray *pElementArray = dynamic_cast<const CElementArray*>(this->getParentNodeByType(XSD_ELEMENT_ARRAY));
            assert(pElementArray != NULL);

            const CElement *pElement = dynamic_cast<const CElement*>(this->getParentNodeByType(XSD_ELEMENT));
            assert(pElement != NULL);

            //strQML.append(QML_PROPERTY_STRING_TABLE_BEGIN).append(pElement->getXSDXPath()).append(QML_PROPERTY_STRING_TABLE_END);
            strQML.append(QML_PROPERTY_STRING_TABLE_BEGIN).append(modelNames[CConfigSchemaHelper::getInstance(0)->getNumberOfTables()]).append(QML_PROPERTY_STRING_TABLE_PART_1).append(pElement->getXSDXPath()).append(QML_PROPERTY_STRING_TABLE_END);
            DEBUG_MARK_QML;

            CConfigSchemaHelper::getInstance(0)->incTables();

            QUICK_QML_ARRAY2(strQML);

            strQML.append(QML_TABLE_VIEW_END);
            DEBUG_MARK_QML;

            strQML.append(QML_ROW_END);
            DEBUG_MARK_QML;
        }
        else
        {
         //   strQML.append(QML_TAB_VIEW_HEIGHT).append((this->length()+1)*25);
        //            DEBUG_MARK_QML;
            CQMLMarkupHelper::setImplicitHeight(((this->length()) * 26)+30);
            DEBUG_MARK_QML;

            //strQML.append(QML_GRID_LAYOUT_BEGIN);
            //DEBUG_MARK_QML;

            QUICK_QML_ARRAY2(strQML);
            DEBUG_MARK_QML;

            strQML.append(QML_TAB_END);
            DEBUG_MARK_QML;
        }
    }
}
#else // _USE_OLD_GET_QML_

void CAttributeArray::getQML(StringBuffer &strQML, int idx) const
{
    assert(strQML.length() > 0);  // can't start the qml

    const CElement *pElem = dynamic_cast<const CElement*>(this->getConstAncestorNode(3));

    assert(pElem != NULL);
    assert(pElem->getNodeType() == XSD_ELEMENT);

    if (pElem->isTopLevelElement() == true)
    {
        //strQML.append(QML_TAB_VIEW_BEGIN);
        //DEBUG_MARK_QML;
        CQMLMarkupHelper::getTabQML(strQML, "Attributes");
        DEBUG_MARK_QML;
        strQML.append(QML_GRID_LAYOUT_BEGIN_1);
        DEBUG_MARK_QML;
        QUICK_QML_ARRAY2(strQML);
        DEBUG_MARK_QML;
        strQML.append(QML_GRID_LAYOUT_END);
        DEBUG_MARK_QML;
        strQML.append(QML_FLICKABLE_HEIGHT).append(CQMLMarkupHelper::getImplicitHeight() * 1.5);
        DEBUG_MARK_QML;
        strQML.append(QML_FLICKABLE_END);
        DEBUG_MARK_QML;
        strQML.append(QML_TAB_END);
        DEBUG_MARK_QML;
        strQML.append(QML_TAB_VIEW_END);
        DEBUG_MARK_QML;
    }
    else if (pElem->isATab() == true)
    {
        //strQML.append(QML_GRID_LAYOUT_BEGIN);
        //DEBUG_MARK_QML;

        QUICK_QML_ARRAY2(strQML);
        DEBUG_MARK_QML;

        //strQML.append(QML_GRID_LAYOUT_END);
        DEBUG_MARK_QML;

        //strQML.append(QML_FLICKABLE_END);
        //DEBUG_MARK_QML;
    }
    else if (pElem->getMaxOccursInt() != 1 || pElem->getMinOccursInt() > 1)  // table
    {
        const char *pName = pElem->getName();

        assert(pName != NULL);
        assert(pName[0] != 0);

        strQML.append(QML_ROW_BEGIN);
        DEBUG_MARK_QML;

        strQML.append(QML_TABLE_VIEW_BEGIN);
        DEBUG_MARK_QML;

        strQML.append(QML_MODEL).append(modelNames[CConfigSchemaHelper::getInstance(0)->getNumberOfTables()]).append(QML_STYLE_NEW_LINE);
        DEBUG_MARK_QML;

        strQML.append(QML_PROPERTY_STRING_TABLE_BEGIN).append(modelNames[CConfigSchemaHelper::getInstance(0)->getNumberOfTables()]).append(QML_PROPERTY_STRING_TABLE_PART_1).append(pElem->getXSDXPath()).append(QML_PROPERTY_STRING_TABLE_END);
        DEBUG_MARK_QML;

        CConfigSchemaHelper::getInstance(0)->incTables();

        QUICK_QML_ARRAY2(strQML);
        DEBUG_MARK_QML;

        strQML.append(QML_TABLE_VIEW_END);
        DEBUG_MARK_QML;

        strQML.append(QML_ROW_END);
        DEBUG_MARK_QML;
    }
    else
    {
        //assert(!"add more logic");
        //QUICK_QML_ARRAY2(strQML);

        strQML.append(QML_ROW_BEGIN);
        DEBUG_MARK_QML;

        strQML.append(QML_TABLE_VIEW_BEGIN);
        DEBUG_MARK_QML;

        strQML.append(QML_MODEL).append(modelNames[CConfigSchemaHelper::getInstance(0)->getNumberOfTables()]).append(QML_STYLE_NEW_LINE);
        DEBUG_MARK_QML;

        strQML.append(QML_PROPERTY_STRING_TABLE_BEGIN).append(modelNames[CConfigSchemaHelper::getInstance(0)->getNumberOfTables()])\
                .append(QML_PROPERTY_STRING_TABLE_PART_1).append(pElem->getXSDXPath()).append(QML_PROPERTY_STRING_TABLE_END);
        DEBUG_MARK_QML;

        CConfigSchemaHelper::getInstance(0)->incTables();

        for (int idx=0; idx < this->length(); idx++)
        {
            (this->item(idx)).getQML(strQML);
            DEBUG_MARK_QML;
        }
        DEBUG_MARK_QML;

        strQML.append(QML_TABLE_VIEW_END);
        DEBUG_MARK_QML;

        strQML.append(QML_ROW_END);
        DEBUG_MARK_QML;

        //strQML.append(QML_TAB_END);
        //DEBUG_MARK_QML;
    }

}

#endif //_USE_OLD_GET_QML_

void CAttributeArray::getQML2(StringBuffer &strQML, int idx) const
{
    if (this->getParentNode()->getUIType() == QML_UI_TABLE)
    {
        this->setUIType(QML_UI_TABLE_CONTENTS);

        for (int i = 0; i < this->length(); i++)
        {
            DEBUG_MARK_QML2(this);
            (this->item(i)).getQML2(strQML);
        }

        DEBUG_MARK_QML2(this);
    }
    else if (this->getParentNode()->getUIType() == QML_UI_TAB)
    {
        this->setUIType(QML_UI_TAB_CONTENTS);

        const CElement* pElem = dynamic_cast<const CElement*>(this->getParentNodeByType(XSD_ELEMENT));

        assert(pElem != NULL);

        if (pElem->isTopLevelElement() == true)
        {
            CQMLMarkupHelper::getTabQML(strQML, "Attributes");
            DEBUG_MARK_QML;
            strQML.append(QML_GRID_LAYOUT_BEGIN_1);
            DEBUG_MARK_QML;
        }

        for (int i = 0; i < this->length(); i++)
        {
            DEBUG_MARK_QML2(this);
            (this->item(i)).getQML2(strQML);
        }

        if (pElem->isTopLevelElement() == true)
        {
            strQML.append(QML_GRID_LAYOUT_END);
            DEBUG_MARK_QML;
            strQML.append(QML_FLICKABLE_HEIGHT).append(CQMLMarkupHelper::getImplicitHeight() * 1.5);
            DEBUG_MARK_QML;
            strQML.append(QML_FLICKABLE_END);
            DEBUG_MARK_QML;
            strQML.append(QML_TAB_END);
            DEBUG_MARK_QML;
        }

        DEBUG_MARK_QML2(this);
    }
    else
    {
        assert(!"what am i?");
    }

}

void CAttributeArray::getQML3(StringBuffer &strQML, int idx) const
{
    /*
        Sample QML returned by AttributeArray
        ListElement {
            value: ListElement {value: [ListElement{value: "A Masterpiece"}] type: "field"; tooltip:"Hey" }
            key: ListElement {value: [ListElement{value: "Gabriel"}]}
        }
     */
    DEBUG_MARK_QML;
    // If UI is LIST, each AttributeArray.getQML3() call needs to effectively return a row
    if(this->getUIType() == QML_UI_TABLE_LIST)
    {
        strQML.append(QML_TABLE_ROW_START);
        for(int i = 0; i < this->length(); i++)
        {
            (this->item(i)).getQML3(strQML);
        }
        strQML.append(QML_TABLE_ROW_END);
    // Otherwise, assume Key/Val and generate your own table
    } 
    else 
    {
        StringArray roles;
        StringArray titles;
        strQML.append(QML_TABLE_START);
        
        // Builds Columns
        roles.append("key");
        titles.append("key");
        roles.append("value");
        titles.append("value");
        CQMLMarkupHelper::buildColumns(strQML, roles, titles);
        strQML.append(QML_TABLE_CONTENT_START);
        // build Rows
        for (int i = 0; i < this->length(); i++)
        {
            if((this->item(i)).isHidden()) 
                continue;
            strQML.append(QML_TABLE_ROW_START);
            StringArray key;
            key.append(this->item(i).getTitle());
            CQMLMarkupHelper::buildRole(strQML, "key", key);
            (this->item(i)).getQML3(strQML, "value");
            strQML.append(QML_TABLE_ROW_END);
        }
    strQML.append(QML_DOUBLE_END_BRACKET);
    }
    DEBUG_MARK_QML;
}

void CAttributeArray::populateEnvXPath(StringBuffer strXPath, unsigned int index)
{
    //assert(index == 1);  // Only 1 array of elements per node

    this->setEnvXPath(strXPath);

 //   QUICK_ENV_XPATH(strXPath)
    /*for (int idx=0; idx < this->length(); idx++)
    {
        (this->item(idx)).populateEnvXPath(strXPath.str(), index);
    }*/
    QUICK_ENV_XPATH_WITH_INDEX(strXPath, index)
}

void CAttributeArray::loadXMLFromEnvXml(const IPropertyTree *pEnvTree)
{
    assert(pEnvTree != NULL);

    if (pEnvTree->hasProp(this->getEnvXPath()) == false)
    {
        throw MakeExceptionFromMap(EX_STR_XPATH_DOES_NOT_EXIST_IN_TREE);
    }
    else
    {
        try
        {
            QUICK_LOAD_ENV_XML(pEnvTree)
        }
        catch (IException *e)
        {
            if (e->errorCode() == EX_STR_MISSING_REQUIRED_ATTRIBUTE)
            {
                throw e;
            }
        }
    }
}

const CAttribute* CAttributeArray::findAttributeWithName(const char *pName, bool bCaseInsensitive) const
{
    assert (pName != NULL && pName[0] != 0);

    if (pName == NULL || pName[0] == 0 || this->length() == 0)
    {
        return NULL;
    }

    for (int idx = 0; idx < this->length(); idx++)
    {
        if (bCaseInsensitive == true)
        {
            if (stricmp(this->item(idx).getName(), pName) == 0)
            {
                return &(this->item(idx));
            }
        }
        else
        {
            if (strcmp(this->item(idx).getName(), pName) == 0)
            {
                return &(this->item(idx));
            }
        }
    }

    return NULL;
}
const void CAttributeArray::getAttributeNames(StringArray &names, StringArray &titles) const
{
    for(int i = 0; i < this->length(); i++)
    {
        if(!this->item(i).isHidden())
        {
            names.append(this->item(i).getName());
            titles.append(this->item(i).getTitle());
        }
    }
}

bool CAttributeArray::getCountOfValueMatches(const char *pValue) const
{

}
