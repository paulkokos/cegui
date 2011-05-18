/***********************************************************************
    filename:   CEGUITinyXMLParser.cpp
    created:    Sun Mar 13 2005
    author:     Paul D Turner
*************************************************************************/
/***************************************************************************
 *   Copyright (C) 2004 - 2006 Paul D Turner & The CEGUI Development Team
 *
 *   Permission is hereby granted, free of charge, to any person obtaining
 *   a copy of this software and associated documentation files (the
 *   "Software"), to deal in the Software without restriction, including
 *   without limitation the rights to use, copy, modify, merge, publish,
 *   distribute, sublicense, and/or sell copies of the Software, and to
 *   permit persons to whom the Software is furnished to do so, subject to
 *   the following conditions:
 *
 *   The above copyright notice and this permission notice shall be
 *   included in all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *   IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 *   OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 *   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *   OTHER DEALINGS IN THE SOFTWARE.
 ***************************************************************************/
#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif

#include "CEGUITinyXMLParser.h"
#include "CEGUIResourceProvider.h"
#include "CEGUISystem.h"
#include "CEGUIXMLHandler.h"
#include "CEGUIXMLAttributes.h"
#include "CEGUILogger.h"
#include "CEGUIExceptions.h"
#include CEGUI_TINYXML_H

// Start of CEGUI namespace section
namespace CEGUI
{
    class TinyXMLDocument : public CEGUI_TINYXML_NAMESPACE::TiXmlDocument
    {
    public:
        TinyXMLDocument(XMLHandler& handler, const RawDataContainer& source, const String& schemaName);
        ~TinyXMLDocument()
        {}
    protected:
        void processElement(const CEGUI_TINYXML_NAMESPACE::TiXmlElement* element);

    private:
        XMLHandler* d_handler;
    };

    TinyXMLDocument::TinyXMLDocument(XMLHandler& handler, const RawDataContainer& source, const String& /*schemaName*/)
    {
        d_handler = &handler;

        // Create a buffer with extra bytes for a newline and a terminating null
        size_t size = source.getSize();
        char* buf = new char[size + 2];
        memcpy(buf, source.getDataPtr(), size);
        // PDT: The addition of the newline is a kludge to resolve an issue
        // whereby parse returns 0 if the xml file has no newline at the end but
        // is otherwise well formed.
        buf[size] = '\n';
        buf[size+1] = 0;

        // Parse the document
        CEGUI_TINYXML_NAMESPACE::TiXmlDocument doc;
        if (!doc.Parse((const char*)buf))
        {
            // error detected, cleanup out buffers
            delete[] buf;

            // throw exception
            CEGUI_THROW(FileIOException("TinyXMLParser: an error occurred while "
                "parsing the XML document - check it for potential errors!."));
        }

        const CEGUI_TINYXML_NAMESPACE::TiXmlElement* currElement = doc.RootElement();
        if (currElement)
        {
            CEGUI_TRY
            {
                // function called recursively to parse xml data
                processElement(currElement);
            }
            CEGUI_CATCH(...)
            {
                delete [] buf;

                CEGUI_RETHROW;
            }
        } // if (currElement)

        // Free memory
        delete [] buf;
    }

    void TinyXMLDocument::processElement(const CEGUI_TINYXML_NAMESPACE::TiXmlElement* element)
    {
        // build attributes block for the element
        XMLAttributes attrs;

        const CEGUI_TINYXML_NAMESPACE::TiXmlAttribute *currAttr = element->FirstAttribute();
        while (currAttr)
        {
            attrs.add((encoded_char*)currAttr->Name(), (encoded_char*)currAttr->Value());
            currAttr = currAttr->Next();
        }

        // start element
        d_handler->elementStart((encoded_char*)element->Value(), attrs);

        // do children
        const CEGUI_TINYXML_NAMESPACE::TiXmlNode* childNode = element->FirstChild();
        while (childNode)
        {
            switch(childNode->Type())
            {
            case CEGUI_TINYXML_NAMESPACE::TiXmlNode::ELEMENT:
                processElement(childNode->ToElement());
                break;
            case CEGUI_TINYXML_NAMESPACE::TiXmlNode::TEXT:
                if (childNode->ToText()->Value() != '\0')
                    d_handler->text((encoded_char*)childNode->ToText()->Value());
                break;

                // Silently ignore unhandled node type
            };
            childNode = childNode->NextSibling();
        }

        // end element
        d_handler->elementEnd((encoded_char*)element->Value());
    }

    TinyXMLParser::TinyXMLParser(void)
    {
        // set ID string
        d_identifierString = "CEGUI::TinyXMLParser - Official tinyXML based parser module for CEGUI";
    }

    TinyXMLParser::~TinyXMLParser(void)
    {}

    void TinyXMLParser::parseXML(XMLHandler& handler, const RawDataContainer& source, const String& schemaName)
    {
      TinyXMLDocument doc(handler, source, schemaName);
    }

    bool TinyXMLParser::initialiseImpl(void)
    {
        // This used to prevent deletion of line ending in the middle of a text.
        // WhiteSpace cleaning will be available throught the use of String methods directly
        //CEGUI_TINYXML_NAMESPACE::TiXmlDocument::SetCondenseWhiteSpace(false);
        return true;
    }

    void TinyXMLParser::cleanupImpl(void)
    {}

} // End of  CEGUI namespace section