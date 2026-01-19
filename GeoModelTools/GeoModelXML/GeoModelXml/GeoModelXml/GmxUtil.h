/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEO_MODEL_XML_GMXUTIL_H
#define GEO_MODEL_XML_GMXUTIL_H
//
//    Convenience class to hold Evaluator and registries. Allows one parameter to be passed
//    to element handlers instead of a (changing??) list.
//
#include <string>
#include "ExpressionEvaluator/Evaluator.h"
#include "GeoModelHelpers/MaterialManager.h"
#include "GeoModelXml/ProcessorRegistry.h"
#include "GeoModelXml/Element2GeoItemRegistry.h"

#include "GeoModelXml/AddbranchProcessor.h"
#include "GeoModelXml/LogvolProcessor.h"
#include "GeoModelXml/LogvolrefProcessor.h"
#include "GeoModelXml/AssemblyProcessor.h"
#include "GeoModelXml/AssemblyrefProcessor.h"
#include "GeoModelXml/TransformProcessor.h"
#include "GeoModelXml/MulticopyProcessor.h"
#include "GeoModelXml/ReplicaXProcessor.h"
#include "GeoModelXml/ReplicaYProcessor.h"
#include "GeoModelXml/ReplicaZProcessor.h"
#include "GeoModelXml/ReplicaRPhiProcessor.h"
#include "GeoModelXml/ReplicaXYArraysProcessor.h"
#include "GeoModelXml/IndexProcessor.h"

#include "GeoModelXml/MakeElement.h"
#include "GeoModelXml/MakeMaterial.h"
#include "GeoModelXml/MakeTransformation.h"
#include "GeoModelXml/MakeTransformationref.h"
#include "GeoModelXml/MakeTranslation.h"
#include "GeoModelXml/MakeRotation.h"

#include "GeoModelXml/shape/MakeSimplePolygonBrep.h"
#include "GeoModelXml/shape/MakeEllipticalTube.h"
#include "GeoModelXml/shape/MakeTwistedTrap.h"
#include "GeoModelXml/shape/MakeTorus.h"
#include "GeoModelXml/shape/MakeBox.h"
#include "GeoModelXml/shape/MakeCons.h"
#include "GeoModelXml/shape/MakeGenericTrap.h"
#include "GeoModelXml/shape/MakePara.h"
#include "GeoModelXml/shape/MakePcon.h"
#include "GeoModelXml/shape/MakePgon.h"
#include "GeoModelXml/shape/MakeTrap.h"
#include "GeoModelXml/shape/MakeTrd.h"
#include "GeoModelXml/shape/MakeTube.h"
#include "GeoModelXml/shape/MakeTubs.h"
#include "GeoModelXml/shape/MakeIntersection.h"
#include "GeoModelXml/shape/MakeUnion.h"
#include "GeoModelXml/shape/MakeSubtraction.h"
#include "GeoModelXml/shape/MakeShaperef.h"
#include "GeoModelXml/shape/AddPlane.h"
#include "GeoModelXml/shape/MakeShapeShift.h"


#include "GeoModelXml/PositionIndex.h"
#include "GeoModelXml/GmxInterface.h"

class GmxUtil {
public:
    GmxUtil(GmxInterface &gmxInterface);
    virtual ~GmxUtil() = default;
    GmxInterface& gmxInterface();
    double evaluate(const std::string& expression);
    std::string debracket(std::string expression);
    GeoModelTools::Evaluator eval;
    PositionIndex positionIndex;
    MaterialManager* matManager{nullptr};
    ProcessorRegistry processorRegistry;
    Element2GeoItemRegistry geoItemRegistry;
    GeoLogVol * getAssemblyLV() {return m_assemblyLV;};
    
    
    struct TagHandler {
        //
        //    Things creating a vector of nodes to be added to the tree
        //
        AddbranchProcessor addbranch;
        LogvolProcessor logvol;
        LogvolrefProcessor logvolref;
        AssemblyProcessor assembly;
        AssemblyrefProcessor assemblyref;
        TransformProcessor transform;
        MulticopyProcessor multicopy;
        ReplicaXProcessor replicaX;
        ReplicaYProcessor replicaY;
        ReplicaZProcessor replicaZ;
        ReplicaRPhiProcessor replicaRPhi;
        ReplicaXYarraysProcessor replicaXYArrays;
        IndexProcessor index;
//
//    Things creating an RCBase *
//
        MakeElement element;
        MakeMaterial material;
        MakeSimplePolygonBrep simplepolygonbrep;
        MakeEllipticalTube ellipticaltube;
        MakeTwistedTrap twistedtrap;
        MakeTorus torus;
        MakeBox box;
        MakeCons cons;
        MakeGenericTrap generictrap;
        MakePara para;
        MakePcon pcon;
        MakePgon pgon;
        MakeTrap trap;
        MakeTrd trd;
        MakeTube tube;
        MakeTubs tubs;
        MakeIntersection intersection;
        MakeUnion onion; // union is reserved
        MakeSubtraction subtraction;
        MakeShaperef shaperef;
        MakeShapeShift shapeshift;
        MakeTransformation transformation;
        MakeTransformationref transformationref;
//
//    Things creating HEP transforms
//
        MakeTranslation translation;
        MakeRotation rotation;
//
//    Other things
//
        AddPlane addplane;
    } tagHandler;
//    SensitiveId sensitiveId;
private:
    GeoIntrusivePtr<GeoLogVol> makeAssemblyLV();
    GeoIntrusivePtr<GeoLogVol> m_assemblyLV{}; // Special logvol to be turned into an assembly-physical-volume. 
                             // Achieved by filling it with special::ether material.
    GmxInterface& m_gmxInterface; 

};

#endif // GMXUTIL_H
