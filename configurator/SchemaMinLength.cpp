#include "SchemaMinLength.hpp"

CMinLength* CMinLength::load(CXSDNodeBase* pParentNode, const IPropertyTree *pSchemaRoot, const char* xpath)
{
    assert(pSchemaRoot != NULL);

    if (pSchemaRoot == NULL)
    {
        return NULL;
    }

    CMinLength *pMinLength = new CMinLength(pParentNode);

    pMinLength->setXSDXPath(xpath);

    if (xpath != NULL && *xpath != 0)
    {
        IPropertyTree* pTree = pSchemaRoot->queryPropTree(xpath);

        if (pTree == NULL)
        {
            return pMinLength;
        }

        const char* pValue = pTree->queryProp(XML_ATTR_VALUE);

        if (pValue != NULL && *pValue != 0)
        {
            pMinLength->setMinLength(atoi(pValue));
            pMinLength->setValue(pValue);
        }

        if (pMinLength->setMinLength() < 0)  // not set or bad length value
        {
            delete pMinLength;
            pMinLength = NULL;

            throw MakeExceptionFromMap(EX_STR_LENGTH_VALUE_MUST_BE_GREATER_THAN_OR_EQUAL_TO_ZERO , EACTION_MIN_LENGTH_BAD_LENGTH);
        }
    }

    return pMinLength;
}
