#include "SchemaFractionDigits.hpp"
#include "XMLTags.h"
#include "jptree.hpp"
#include <cstdlib>

CFractionDigits* CFractionDigits::load(CXSDNodeBase* pParentNode, const IPropertyTree *pSchemaRoot, const char* xpath)
{
    assert(pSchemaRoot != NULL);

    if (pSchemaRoot == NULL)
    {
        return NULL;
    }

    CFractionDigits *pFractionDigits = new CFractionDigits(pParentNode);

    pFractionDigits->setXSDXPath(xpath);

    if (xpath != NULL && *xpath != 0)
    {
        IPropertyTree* pTree = pSchemaRoot->queryPropTree(xpath);

        if (pTree == NULL)
        {
            return pFractionDigits;
        }

        const char* pValue = pTree->queryProp(XML_ATTR_VALUE);

        if (pValue != NULL && *pValue != 0)
        {
            pFractionDigits->setMaxFractionDigits(pValue);
            pFractionDigits->setValue(pValue);
        }

        if (pFractionDigits->getMaxFractionalDigits() < 0)  // not set or bad length value
        {
            delete pFractionDigits;
            pFractionDigits = NULL;

            throw MakeExceptionFromMap(EX_STR_LENGTH_VALUE_MUST_BE_GREATER_THAN_OR_EQUAL_TO_ZERO , EACTION_FRACTION_DIGITS_HAS_BAD_LENGTH);
        }
    }

    return pFractionDigits;
}
