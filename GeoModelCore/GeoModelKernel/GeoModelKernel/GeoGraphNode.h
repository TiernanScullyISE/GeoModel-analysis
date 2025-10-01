/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEOMODELKERNEL_GEOGRAPHNODE_H
#define GEOMODELKERNEL_GEOGRAPHNODE_H

/**
 * @class GeoGraphNode
 *
 * @brief Represents Nodes that can be assembled together to make
 *      a graph.  Subclasses (for now):
 *
 *      * GeoVTransform
 *           *GeoFixedTransform.     .. for fixed transforms
 *           *GeoVariableTransform  .. for variable transforms.
 *
 *      * GeoVPhysVol
 *           *GeoPhysVol       .. for normal recursive access.
 *           *GeoRAPhysVol  .. for random access
 */

#include "GeoModelKernel/RCBase.h"

class GeoNodeAction;
class GeoVPhysVol;

enum class GeoGraphNodeType{
  UNKNOWN, NodeEndpoint, GeoTransform
};



class GeoGraphNode : public RCBase {
 public:
  GeoGraphNode () = default;
  
  //	Executes a GeoNodeAction.
  virtual void exec (GeoNodeAction *action) const;
  
  //	When a node is added to a parent in the graph, the node
  //	is always notified.  What happens at that time is up to
  //	the node.  Most nodes do not need to do anything.
  //	Some--the alignable transforms in particular--need to
  //	take some actions, such as adding the parent volume to a
  //	list.
  virtual void dockTo (GeoVPhysVol* );

  //    Ability to Query the type of the node for specific logic cases
  virtual constexpr bool typeQuery(GeoGraphNodeType in) const { return in == GeoGraphNodeType::UNKNOWN; }
  
 protected:
  virtual ~GeoGraphNode() = default;
  
};

#endif
