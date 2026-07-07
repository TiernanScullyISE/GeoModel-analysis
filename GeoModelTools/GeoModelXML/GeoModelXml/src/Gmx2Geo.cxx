/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "GeoModelXml/Gmx2Geo.h"
#include "OutputDirector.h"

#include <string>
#include <iomanip>
#include <sstream>
#include <stdlib.h>

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMLSParser.hpp>
#include <xercesc/util/PlatformUtils.hpp>

#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoVPhysVol.h"

#include "GeoModelKernel/GeoVolumeTagCatalog.h"

#include "ExpressionEvaluator/IEvaluator.h"

#include "xercesc/util/XMLString.hpp"
#include "GeoModelXml/GeoNodeList.h"
#include "GeoModelXml/ElementProcessor.h"
#include "GeoModelXml/GmxUtil.h"
#include "GeoModelXml/GmxInterface.h"
#include "GeoModelXml/createdomdocument.h"
#include "GeoModelHelpers/MaterialManager.h"
#include "GeoModelKernel/throwExcept.h"

using namespace std;
using namespace xercesc;

Gmx2Geo::Gmx2Geo(const string& xmlFile, 
                 GeoVPhysVol *addHere, 
                 GmxInterface &gmxInterface, 
                 unsigned int flags,                  
                 std::string levelMapName, 
                 const processorList& procs) {
//
//    Create the xml tree (DOMDocument)
//


    DOMLSParser *parser = 0;
    DOMDocument *doc = createDOMDocument(xmlFile, parser, flags);
    if (!doc) {// Parsed badly
        XMLPlatformUtils::Terminate();
       THROW_EXCEPTION("Error in xml file " << xmlFile << ". Exiting program." );
    }

//
//    Set up the CLHEP evaluator and the xml-tag processors, and store the GmxInterface:
//
    GmxUtil gmxUtil(gmxInterface); 
    if (gmxInterface.useMaterialManager()) gmxUtil.matManager=MaterialManager::getManager();

    // add any additional ElementProcessor
    for (auto pr: procs)
        gmxUtil.processorRegistry.enregister(pr.first,pr.second);

    //
    //    Process the xml tree, creating all the GeoModel items and adding to the GeoModel tree.
    //
    const DOMElement *root = doc->getDocumentElement();
//
//    info message: name of detector
//
    if (flags & 0x1) {
        msglog << MSG::INFO << "Set up detector geometry from db-string \nwhich adds sub-detector ";
    }
    else {
        msglog << MSG::INFO << "Set up detector geometry from file " << xmlFile << " which adds sub-detector ";
    }
    const DOMElement *element = dynamic_cast<const DOMElement*>(root);
    XMLCh * name_tmp = XMLString::transcode("name");
    const XMLCh *attribute = element->getAttribute(name_tmp);
    msglog << XMLString::transcode(attribute) << endmsg;

    if(gmxUtil.gmxInterface().msgLvl(LogLevel::NIL)){
        msglog << MSG::INFO << "Invalid log level set " << (int) gmxUtil.gmxInterface().logLevel() <<endmsg;
        gmxUtil.gmxInterface().setLogLevel(LogLevel::INFO);
        msglog << MSG::INFO << "Setting logging to INFO level " <<endmsg;
    }


    if(gmxUtil.gmxInterface().msgLvl(LogLevel::INFO)){
         msglog << MSG::INFO << "LogLevel set to " << (int) gmxUtil.gmxInterface().logLevel() <<endmsg;
    }

// 
// if the material manager is set, create a namespace
// 

    if (gmxUtil.matManager) gmxUtil.matManager->setMaterialNamespace(XMLString::transcode(attribute));

//
//    Add all constant definitions to the evaluator, so they are ready if needed.
//
    doDefines(doc, gmxUtil.eval);
//
//    Set up all sensitive detector types (readout geometry)
//
    doReadoutGeometry(doc, gmxUtil);
//
//    Add all index names
//
    doPositionIndex(doc, gmxUtil, levelMapName);
//
//   The xml file ends with an addbranch element. We start there, process it, 
//   and it fills in the list of things to be added to the GeoModel graph. 
//   There is one and only one addbranch element according to the DTD.
//
    XMLCh * addbranch_tmp = XMLString::transcode("addbranch"); 
    xercesc::DOMNodeList *addbranchs = doc->getElementsByTagName(addbranch_tmp);
    const DOMElement *addbranch = dynamic_cast<const DOMElement *> (addbranchs->item(0)); 

//  ADA - check if an envelope is requested, in case we take is and fill it up with the addbranch's contents.

    XMLCh * envelope_tmp=XMLString::transcode("envelope");
    GeoVPhysVol* physVol=addHere;
    if (addbranch->hasAttribute(envelope_tmp))
    {
        char *toRelease=XMLString::transcode(addbranch->getAttribute(envelope_tmp));
	std::cout<<" envelope requested "<<toRelease<<std::endl;
        std::string envel=toRelease;
	XMLString::release(&toRelease);
	GeoVPhysVol* tmpVol=GeoVolumeTagCatalog::VolumeTagCatalog()->getTaggedVolume("Envelope",envel);
	if (tmpVol) 
	{
		msglog << MSG::INFO<< " Volume "<<envel<<" found in the envelope catalog"<<endmsg;
		physVol=tmpVol;
	}
    }

    GeoNodeList toAdd;
    gmxUtil.processorRegistry.find("addbranch")->process(addbranch, gmxUtil, toAdd);
    for (GeoNodeList::iterator node = toAdd.begin(); node != toAdd.end(); ++node) {
        physVol->add(*node);
    }

    XMLString::release(&name_tmp);
    XMLString::release(&addbranch_tmp);
    XMLPlatformUtils::Terminate();
}

int Gmx2Geo::doDefines(DOMDocument *doc, GeoModelTools::IEvaluator &eval) {
//
//    Add all defines to the CLHEP::Evaluator
//
//
//    It is better to store the evaluated number, not the expression 
//      1: it avoids the parenthesis problem sum = a + b; 2 * sum != 2(a + b)
//      2: it finds undefined constants sooner (helps debugging)
//      3: anyway, everything is a constant so we might as well
//    ...This means we need everything already defined before it is introduced
//    ...Which means we must do matrices and vectors (which are only numbers) before vars.
//    in case the user uses a vector-element when defining a var element
//

const DOMElement *element;
//
//-------------------------------------------------------------------------------------------
//


    XMLCh * defines_tmp = XMLString::transcode("defines");
    XMLCh * vector_tmp = XMLString::transcode("vector");
    XMLCh * name_tmp = XMLString::transcode("name");
    XMLCh * value_tmp = XMLString::transcode("value");
    XMLCh * var_tmp = XMLString::transcode("var");
    XMLCh * matrix_tmp = XMLString::transcode("matrix");
    XMLCh * coldim_tmp = XMLString::transcode("coldim");
    
    DOMNodeList *defines = doc->getElementsByTagName(defines_tmp);
    int nDefines = defines->getLength();
    for (int i = 0; i < nDefines; ++i) {
        element = dynamic_cast<DOMElement *>(defines->item(i));
        DOMNodeList *vars;
        DOMElement *var;
        int nVars;
//
//   Vectors: a list of variables. Names subscripted by _k; values white-space separated numbers.
//
        //msglog << "\n\n    Vectors\n    =======\n\n";

        vars = element->getElementsByTagName(vector_tmp);
        nVars = vars->getLength();
        for (int j = 0; j < nVars; ++j) {
            var = dynamic_cast<DOMElement*>(vars->item(j));
            char *name = XMLString::transcode(var->getAttribute(name_tmp));
            char *val = XMLString::transcode(var->getAttribute(value_tmp));
            string fullname(name);
            fullname += '_';
            istringstream list(val);
            double dble;
            int k = 0;
            do  {
                list >> dble;
                //msglog << fullname + to_string(k) << " = " << dble << endl; 
                eval.setVariable((fullname + to_string(k++)).c_str(), dble);
            } while(list.good());
            XMLString::release(&name);
            XMLString::release(&val);
        }
//
//   Matrices: a matrix of variables. Names subscripted by _j_k; values: white-space separated numbers.
//
        //msglog << "\n\n    Matrices\n    ========\n\n";

        vars = element->getElementsByTagName(matrix_tmp);
        nVars = vars->getLength();
        for (int j = 0; j < nVars; ++j) {
            var = dynamic_cast<DOMElement*>(vars->item(j));
            char *name = XMLString::transcode(var->getAttribute(name_tmp));
            char *val = XMLString::transcode(var->getAttribute(value_tmp));
            int coldim;
            istringstream(XMLString::transcode(var->getAttribute(coldim_tmp))) >> coldim;
            string fullname(name);
            fullname += '_';
            istringstream list(val);
            double dble;
            int k = 0;
            do  {
                for (int col = 0; col < coldim; ++col) {
                        list >> dble;
                        //msglog << fullname + to_string(k) + '_' + to_string(col) << " = " << dble << endl; 
                        eval.setVariable((fullname + to_string(k) + '_' + to_string(col)).c_str(), dble);
                        if (!list.good()) break;
                }
                k++;
            } while(list.good());
            XMLString::release(&name);
            XMLString::release(&val);
        }
//
//    Vars: single variable
//
        //msglog << "\n\n    Single variables\n    ================\n\n";

        vars = element->getElementsByTagName(var_tmp);
        nVars = vars->getLength();
        for (int j = 0; j < nVars; ++j) {
            var = dynamic_cast<DOMElement*>(vars->item(j));
            char *name = XMLString::transcode(var->getAttribute(name_tmp));
            char *val = XMLString::transcode(var->getAttribute(value_tmp));
            double evaluated = eval.evaluate(val);
            if (eval.status() != GeoModelTools::IEvaluator::OK) {
                msglog << MSG::FATAL << "GeoModelXml Error processing CLHEP Evaluator expression. Error name " <<
                eval.error_name() << endl << "Message: ";
                eval.print_error();
                msglog << val << endl;
                msglog << string(eval.error_position(), '-') << '^' << '\n';
                msglog << "Exiting program." << endmsg;
		std::abort();
            }
            eval.setVariable(name, evaluated);
            //msglog << name << "\t\t" << val << " = " << setprecision(10) << evaluated << endl;
            XMLString::release(&name);
            XMLString::release(&val);
        }
    }

    XMLString::release(&defines_tmp);
    XMLString::release(&vector_tmp);
    XMLString::release(&name_tmp);
    XMLString::release(&value_tmp);
    XMLString::release(&var_tmp);
    XMLString::release(&matrix_tmp);
    XMLString::release(&coldim_tmp);

    //msglog << endmsg;
    return 1;
}

int Gmx2Geo::doPositionIndex(xercesc::DOMDocument *doc, GmxUtil &gmxUtil, std::string levelMapName) {
    XMLCh * positionindex_tmp = XMLString::transcode("positionindex");
    XMLCh * addindex_tmp = XMLString::transcode("addindex");
    XMLCh * name_tmp = XMLString::transcode("name");

    DOMNodeList *posIndex = doc->getElementsByTagName(positionindex_tmp);
    int nPosIndex = posIndex->getLength(); // Probably limited to 0 or 1 by DTD
    for (int i = 0; i < nPosIndex; ++i) {
        const DOMElement *element = dynamic_cast<DOMElement *>(posIndex->item(i));
        DOMNodeList *addindexs = element->getElementsByTagName(addindex_tmp);
        int nIndexs = addindexs->getLength();
        for (int j = 0; j < nIndexs; ++j) {
            DOMElement *addindex = dynamic_cast<DOMElement*>(addindexs->item(j));
            string name = string(XMLString::transcode(addindex->getAttribute(name_tmp)));
            gmxUtil.positionIndex.addIndex(name);
            if(levelMapName!="") gmxUtil.positionIndex.setLevelMapName(levelMapName);
        }
    }
    XMLString::release(&positionindex_tmp);
    XMLString::release(&addindex_tmp);
    XMLString::release(&name_tmp);
    return 1;
}

int Gmx2Geo::doReadoutGeometry(xercesc::DOMDocument *doc, GmxUtil &gmxUtil) {
//
//    We keep three parameter lists (each is a map of (parameter name, parameter value)), one at each
//    level (readoutgeometry, sensorclass, sensortype). Currently just created new, copying previous one into
//    it. In this way the sensortype parameter list has all parameters in it. Not efficient, but the list is always short?
//
char *name2release;
string tagName;
//
//    Loop over all readoutgeometry elements
//

XMLCh * readoutgeometry_tmp = XMLString::transcode("readoutgeometry");
XMLCh * name_tmp = XMLString::transcode("name");
 
DOMNodeList *rgs = doc->getElementsByTagName(readoutgeometry_tmp);
    int nRG = rgs->getLength(); 
    for (int i = 0; i < nRG; ++i) {
        map<string, string> rgParams; // New empty list
        const DOMElement *rg = dynamic_cast<DOMElement *>(rgs->item(i));
        
//
//    Loop over readoutgeometry children
//

        for (DOMNode *rgChild = rg->getFirstChild(); rgChild != 0; rgChild = rgChild->getNextSibling()) {
            if (rgChild->getNodeType() != DOMNode::ELEMENT_NODE) continue; // Skip char data
            name2release = XMLString::transcode(rgChild->getNodeName());
            tagName = name2release;
            XMLString::release(&name2release);
            
            if (tagName == "param") {
                addParam(rgChild, rgParams);
            }

            else if (tagName == "sensorclass") {
                map<string, string> scParams(rgParams); // Initialised with all previous params
                const DOMElement *sensorClass = dynamic_cast<DOMElement *>(rgChild);
                name2release = XMLString::transcode(sensorClass->getAttribute(name_tmp));
                string clas(name2release); // class is reserved word
                XMLString::release(&name2release);

                //This is how we publish a table per sensor class;
                // not clear if a table per type or class is best?
                std::vector<std::string> colNames;
                std::vector<std::string> colTypes;
                std::vector<std::vector<std::variant<int,long,float,double,std::string>>> tableData; 
                
                colNames.push_back(clas+"_data_id");
                colTypes.push_back("INT");

                colNames.push_back("SensorType");
                colTypes.push_back("STRING");
                bool columsDefined = false;
//
//    Loop over sensorclass children
//
                int sensorIndex = 0;
                for (DOMNode *scChild = sensorClass->getFirstChild(); scChild != 0; scChild = scChild->getNextSibling()) {
                    if (scChild->getNodeType() != DOMNode::ELEMENT_NODE) continue; // Skip char data
                    name2release = XMLString::transcode(scChild->getNodeName());
                    tagName = name2release;
                    XMLString::release(&name2release);
                    std::vector<std::variant<int,long,float,double,std::string>> data;
                    if (tagName == "param") {
                        addParam(scChild, scParams);
                    }
                    else if (tagName == "sensortype") {
                        map<string, string> stParams(scParams); // Initialised with all previous params
                        const DOMElement *sensorType = dynamic_cast<DOMElement *>(scChild);
                        name2release = XMLString::transcode(sensorType->getAttribute(XMLString::transcode("name")));
                        string name(name2release);
                        XMLString::release(&name2release);
                        sensorIndex++;
                        std::vector<std::variant<int,long,float,double,std::string>> data;
                        data.push_back(sensorIndex);
                        data.push_back(name);

                    

//
//    Loop over sensortype parameters
//
                        for (DOMNode *stChild = scChild->getFirstChild(); stChild != 0; stChild = stChild->getNextSibling()) {
                            if (stChild->getNodeType() != DOMNode::ELEMENT_NODE) continue; // Skip char data
                            addParam(stChild, stParams);
                        }
                        fillTable(stParams,colNames,colTypes,data,columsDefined);
//
//    Call the user's call back routine to add this sensor type with its specific parameters
//
                        tableData.push_back(data);
                        gmxUtil.gmxInterface().addSensorType(clas, name, stParams);

                    //finished loop over first sensorType, so we now should have all the columns for this type
                    columsDefined = true;
                    }

                }
                gmxUtil.gmxInterface().publish(clas,colNames,colTypes,tableData);
            }
        }

    }
    
    XMLString::release(&readoutgeometry_tmp);
    XMLString::release(&name_tmp);

    return 1;
}

void Gmx2Geo::addParam(DOMNode *node, map<string, string> &params) {
  XMLCh * name_tmp = XMLString::transcode("name");
  XMLCh * value_tmp = XMLString::transcode("value_tmp");

    const DOMElement *param = dynamic_cast<DOMElement *>(node);
    char *name2release = XMLString::transcode(param->getAttribute(XMLString::transcode("name")));
    string name(name2release);
    XMLString::release(&name2release);
    name2release = XMLString::transcode(param->getAttribute(XMLString::transcode("value")));
    string value(name2release);
    XMLString::release(&name2release);
    params[name] = value;

    XMLString::release(&name_tmp);
    XMLString::release(&value_tmp);
}

void Gmx2Geo::fillTable(map<string, string> &params, std::vector<std::string>& colNames, std::vector<std::string>& colTypes, std::vector<std::variant<int,long,float,double,std::string>>& data, bool& columnsDefined) {
for (auto param:params){
        //only do this for the first iteration; after that the columns are already defined
        //so we only need to fill the matching data for the other instances
        if(!columnsDefined){
            colNames.push_back(param.first);
            colTypes.push_back("STRING");
        }
        data.push_back(param.second); 
    }
}
