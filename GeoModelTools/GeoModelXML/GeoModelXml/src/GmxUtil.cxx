/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "GeoModelXml/GmxUtil.h"

#include "OutputDirector.h"

#include "GeoModelKernel/GeoElement.h"
#include "GeoModelKernel/GeoMaterial.h"
#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoLogVol.h"
#include "GeoModelXml/GmxInterface.h"
#include "GeoModelKernel/throwExcept.h"

#include <cstdlib>
#include <sstream>
#include <cctype>
#include <limits>

using namespace std;

GmxUtil::GmxUtil(GmxInterface &gmxInterface):
    m_gmxInterface{gmxInterface} {

    geoItemRegistry.enableLogVolDeDuplication(gmxInterface.doLogVolDeDuplication());
    processorRegistry.enableLogVolDeDuplication(gmxInterface.doLogVolDeDuplication());

    geoItemRegistry.enableShapeDeDuplication(gmxInterface.doShapeDeDuplication());
    processorRegistry.enableShapeDeDuplication(gmxInterface.doShapeDeDuplication());

    geoItemRegistry.enableTransformDeDuplication(gmxInterface.doTransformDeDuplication());
    processorRegistry.enableTransformDeDuplication(gmxInterface.doTransformDeDuplication());

    geoItemRegistry.enablePhysVolDeDuplication(gmxInterface.doPhysVolDeDpulication());
    processorRegistry.enablePhysVolDeDuplication(gmxInterface.doPhysVolDeDpulication());

//
//    Initialise the CLHEP::Evaluator
//
    eval.setStdMath();
//
//    Fetch the material special::HyperUranium and make any shape with it to make a special logvol.
//    When Geo2G4 finds any physvol using a logvol with material special::HyperUranium, it creates a G4Assembly
//    instead of a G4Volume.
//
    m_assemblyLV = makeAssemblyLV();
//
//   Register tag handlers that produce a vector of items to add to the tree.
//
    processorRegistry.enregister("addbranch", &tagHandler.addbranch);
    processorRegistry.enregister("logvol", &tagHandler.logvol);
    processorRegistry.enregister("logvolref", &tagHandler.logvolref);
    processorRegistry.enregister("assembly", &tagHandler.assembly);
    processorRegistry.enregister("assemblyref", &tagHandler.assemblyref);
    processorRegistry.enregister("transform", &tagHandler.transform);
    processorRegistry.enregister("multicopy", &tagHandler.multicopy);
    processorRegistry.enregister("replicaX", &tagHandler.replicaX);
    processorRegistry.enregister("replicaY", &tagHandler.replicaY);
    processorRegistry.enregister("replicaZ", &tagHandler.replicaZ);
    processorRegistry.enregister("replicaRPhi", &tagHandler.replicaRPhi);
    processorRegistry.enregister("replicaXYarrays", &tagHandler.replicaXYArrays);
    processorRegistry.enregister("index", &tagHandler.index);
    ///
    ///   Register tag handlers that produce GeoNodes. Only useful for those tags which
    ///   can appear in a long list in any order. So certainly all shapes; maybe others.
    ///
    geoItemRegistry.enregister("simplepolygonbrep",  &tagHandler.simplepolygonbrep);
    geoItemRegistry.enregister("twistedtrap",  &tagHandler.twistedtrap);
    geoItemRegistry.enregister("ellipticaltube",  &tagHandler.ellipticaltube);
    geoItemRegistry.enregister("torus",  &tagHandler.torus);
    geoItemRegistry.enregister("box",  &tagHandler.box);
    geoItemRegistry.enregister("cons",  &tagHandler.cons);
    geoItemRegistry.enregister("generictrap",  &tagHandler.generictrap);
    geoItemRegistry.enregister("para", &tagHandler.para);
    geoItemRegistry.enregister("pcon", &tagHandler.pcon);
    geoItemRegistry.enregister("pgon", &tagHandler.pgon);
    geoItemRegistry.enregister("trap", &tagHandler.trap);
    geoItemRegistry.enregister("trd",  &tagHandler.trd);
    geoItemRegistry.enregister("tube", &tagHandler.tube);
    geoItemRegistry.enregister("tubs", &tagHandler.tubs);
    geoItemRegistry.enregister("intersection", &tagHandler.intersection);
    geoItemRegistry.enregister("union",  &tagHandler.onion); // union is reserved
    geoItemRegistry.enregister("subtraction",  &tagHandler.subtraction);
    geoItemRegistry.enregister("shaperef",  &tagHandler.shaperef);
    geoItemRegistry.enregister("transformation",  &tagHandler.transformation);
    geoItemRegistry.enregister("transformationref",  &tagHandler.transformationref);
    geoItemRegistry.enregister("shapeshift", &tagHandler.shapeshift);
    
    tagHandler.addplane.gmxUtil=this;
}

GmxInterface& GmxUtil::gmxInterface() {
    return m_gmxInterface;
}

double GmxUtil::evaluate(const std::string& strExpression) {
    static thread_local string lastGoodExpression("none"); // It is helpful for debugging to get some idea of where we had got to...

    bool isWhiteSpace = true;
    for(unsigned int i = 0; i < strExpression.length(); ++i){
        if(!std::isspace(strExpression[i])) {
            isWhiteSpace = false;
            break;
        }
    }
    if (isWhiteSpace) { // Catch a common error early and give best message possible
       THROW_EXCEPTION("evaluate: empty or white space expression. Last good expression was " + lastGoodExpression);
    }
//
//    Process any []s. Contents are evaluated to in integer, then the [...] are replaced by 
//    string representation of that integer. Idea from Joakim Gradin. Re-coded by Nigel.
//    Allows array elements to be accessed as array_[col-expression]_[row-expression]. 
//
    string noBrackets = debracket(strExpression);
//
//   And evaluate the result
//
    double result = eval.evaluate(noBrackets.c_str());
    if (eval.status() != GeoModelTools::Evaluator::OK) {
        THROW_EXCEPTION("evaluate: invalid expression <"<<noBrackets
                     <<">. Last good expression was <" + lastGoodExpression + ">");
    }
    lastGoodExpression = strExpression;
    return result;
}

std::string GmxUtil::debracket(std::string expression) {
//
//    Get the last brackets contents and evaluate it
//
    size_t lastOpen = expression.find_last_of('[');
    if (lastOpen == string::npos) { // None found
        return expression; // Done
    }
    size_t nextClose = expression.find_first_of(']', lastOpen);
    if (nextClose == string::npos) {
        return expression;
    }
    string toEvaluate = expression.substr(lastOpen + 1, nextClose - lastOpen - 1);
//
//    Evaluate it
//
    stringstream result;
    result << (int) eval.evaluate(toEvaluate.c_str());
//
//    Replace [sub-expr] with result
//
    string newExpression = expression.substr(0, lastOpen) + result.str() + 
                           expression.substr(nextClose + 1, string::npos);

    return debracket(newExpression);
}
GeoIntrusivePtr<GeoLogVol>  GmxUtil::makeAssemblyLV() {

    GeoIntrusivePtr<const GeoMaterial> assembly_material{nullptr};
    
    if (matManager) {
        assembly_material = matManager->getMaterial("special::HyperUranium");
    }
    else {
        GeoMaterial* temp_material = new GeoMaterial("special::HyperUranium", 1.e-20);
        GeoElement *vacuum = new GeoElement("vacuum", "Mt", 1, 1);
        temp_material->add(vacuum, 1.0);
        temp_material->lock();
        assembly_material=temp_material;
    }

    GeoBox *box = new GeoBox(1., 1., 1.); // Simplest shape; it is irrelevant
    GeoIntrusivePtr<GeoLogVol> lv{new GeoLogVol(string("AssemblyLV"), box, assembly_material)};
    return lv;    
}
