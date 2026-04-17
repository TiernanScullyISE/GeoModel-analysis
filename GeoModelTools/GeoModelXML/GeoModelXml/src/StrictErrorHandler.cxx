/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "GeoModelXml/StrictErrorHandler.h"
#include "GeoModelXml/StringWrappers.h"
#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOMError.hpp>
#include <xercesc/dom/DOMLocator.hpp>
#include "OutputDirector.h"


using namespace xercesc;

bool StrictErrorHandler::handleError(const xercesc::DOMError &domError) {
    msglog << MSG::ERROR << "StrictErrorHandler: Handle an error\n";
    m_SawErrors = true;
    //
    switch (domError.getSeverity()) {
    case (xercesc::DOMError::DOM_SEVERITY_WARNING):
        msglog << "\nWarning at file \n";
        break;
    case (xercesc::DOMError::DOM_SEVERITY_ERROR):
        msglog << "\nError at file \n";
        break;
    default:
        msglog << "\nFatal Error at file \n";
    }
    const auto* loc = domError.getLocation();
    if (loc) {
      const auto * uri = loc->getURI();
      if (uri){
        msglog << GeoXML::xml2Str(uri);
      } else {
        msglog << "<UNKNOWN URI>";
      }
      msglog << GeoXML::xml2Str(loc->getURI());
      msglog << " - line " << loc->getLineNumber() << ", char " << loc->getColumnNumber()<<"\n";
    } else {
      msglog << "<UNKNOWN XML LOCATION>\n";
    }
    msglog << "\n  Message: " << GeoXML::xml2Str(domError.getMessage()) << std::endl;

    return true;
}
