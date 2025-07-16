/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// GeoPhysVolHelper.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#include "GeoModelValidation/GeoPhysVolHelper.h"
#include "GeoModelValidation/GeoMaterialHelper.h"

// GeoModelKernel
#include "GeoModelKernel/GeoMaterial.h"
#include "GeoModelKernel/GeoElement.h"
#include "GeoModelKernel/Units.h"
#include "GeoModelKernel/GeoPhysVol.h"
///#include "GeoModelUtilities/GeoVisitVolumes.h"
#include "GeoModelDBManager/GMDBManager.h"
#include "GeoModelWrite/WriteGeoModel.h"
#include "GeoModelRead/ReadGeoModel.h" 
#include "GeoModelKernel/GeoShape.h"
#include "GeoModelKernel/GeoShapeShift.h"
#include "GeoModelKernel/GeoShapeSubtraction.h"
#include "GeoModelKernel/GeoShapeUnion.h"
#include "GeoModelKernel/GeoShapeIntersection.h"
#include "GeoModelKernel/GeoTubs.h"
#include "GeoModelKernel/GeoTube.h"
#include "GeoModelKernel/GeoPcon.h"
#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoCons.h"
#include "GeoModelKernel/GeoSimplePolygonBrep.h"
#include "GeoModelKernel/GeoTrd.h"
#include "GeoModelKernel/GeoTrap.h"
#include "GeoModelKernel/GeoPgon.h"
#include "GeoModelKernel/GeoPara.h"

#include "GeoModelHelpers/GeoShapeSorter.h"
#include "GeoModelHelpers/TransformSorter.h"

#include "GeoModelKernel/Units.h"
#define SYSTEM_OF_UNITS GeoModelKernelUnits

#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>

void GeoModelTools::GeoPhysVolHelper::compareDB( std::string dbref_path, std::string dbtest_path, std::string branch_ref, std::string branch_test, bool printFullInfo, float tolerance) {

  const PVConstLink db_ref = retrieveFromDb(dbref_path);
  const PVConstLink db_test = retrieveFromDb(dbtest_path);

  if (!db_ref) std::cout << "invalid input file "<<dbref_path<<", reference db not retrieved, exiting" << std::endl;  
  if (!db_test) std::cout << "invalid input file "<<dbtest_path<<",  test db not retrieved, exiting" << std::endl;  

  PVConstLink top_ref = branch_ref!="" ?  findBranch(db_ref, branch_ref) : db_ref;  
  PVConstLink top_test = db_test;
  if ( branch_test!="" )   top_test = findBranch(db_test, branch_test); 
  else if ( branch_ref!="" )   top_test = findBranch(db_test, branch_ref); 

  if (!top_ref) {
    std::cout << "required ref branch not found "<<branch_ref<<", trying to resolve from top" << std::endl; 
    top_ref = db_ref;
  }  
  if (!top_test) {
    std::cout << "required test branch not found "<<branch_test<<"/"<<branch_ref<<",  trying to resolve from top" << std::endl;  
    top_test = db_test;
  }

  // =====================================================================  
  // print out a short overview of code used 
  std::cout <<"//======================================================"<< std::endl;
  if ( !printFullInfo ) {
    std::cout<<"Printing out light-weight info about branch content and transform matching. " << std::endl;  
    std::cout<<"Returns the code of first non-trivial difference found: run printFullInfo=True to get the whole list of discrepancies." << std::endl;  
  } else {
    std::cout<<"Printing out detailed info about branch content and transform matching. " << std::endl;  
  }
  std::cout<<"Return value:  level*1000 + code  OR  level*1000 + childNo*10 + code " << std::endl;
  std::cout<<"            code: 0  no discrepancy found" << std::endl;
  std::cout<<"            code: 1  name change ( considered trivial ) " << std::endl;
  std::cout<<"            code: 2  material difference " << std::endl;
  std::cout<<"            code: 3  shape type change" << std::endl;
  std::cout<<"            code: 4  shape definition change" << std::endl;
  std::cout<<"            code: 5  number of child volumes differs" << std::endl;
  std::cout<<"            code: 6  position (translation) differs" << std::endl;
  std::cout<<"            code: 7  orientation (rotation) differs" << std::endl;
  std::cout<<"Position of alignable objects which show discrepancies is printed out to facilitate interpretation." <<std::endl; 
  std::cout <<"======================================================//"<< std::endl;

  // =====================================================================
 
  std::cout << "Validation strategy A ( assume identity )"<<std::endl;
  // prepare arrays to follow matching progress, detect missing objects and duplicates
  bool identicalTreeHierarchy = true;

  unsigned int nctest = top_test->getNChildVols();
  unsigned int ncref = top_ref->getNChildVols();
  std::cout << "number of child volumes (ref:test)" << ncref<<":" << nctest<<std::endl;
  if (nctest!=ncref) identicalTreeHierarchy = false;
  std::string cname,cnameref;

  // test -> ref 
  int match_code = 0;
  for (unsigned int ic = 0; ic < nctest; ic++) {
    const GeoVPhysVol* cv = &(*(top_test->getChildVol(ic)));
    GeoModelTools::GM_valid mcode( cv, 0, 1, -1,-1);
    if (ic>ncref-1) {
      m_test2db.push_back(mcode);
      continue;
    }
    GeoTrf::Transform3D tr_test = top_test->getXToChildVol(ic);
    GeoTrf::Transform3D tr_ref = top_ref->getXToChildVol(ic);
    const GeoVPhysVol* cvref = &(*(top_ref->getChildVol(ic)));
    cname = cv->getLogVol()->getName();
    cnameref = cvref->getLogVol()->getName();
    
    if (cname!=cnameref) {
      m_test2db.push_back(mcode);
      identicalTreeHierarchy = false;
      continue;
    }
    
    // TODO: match alignables with nearest AND resolve identification 

    mcode.valid_position = compareTransforms(tr_test,tr_ref, tolerance);  

    if (mcode.valid_position==6 && printFullInfo)  {
      std::cout <<ic<<":"<<cv->getLogVol()->getName()<<" position differs:translation:"<<std::endl;
      printTranslationDiff( tr_test, tr_ref, tolerance);
    }

    if (mcode.valid_position==7 && printFullInfo)   {
      std::cout <<ic<<":"<<cv->getLogVol()->getName()<<" position differs:rotation:"<<std::endl;
      printRotationDiff( tr_test, tr_ref, tolerance);
    }
 
    mcode.ref_branch = cvref;
    mcode.valid_branch = compareGeoVolumes(cv,cvref,tolerance,printFullInfo,0);
 
    // short printout if no difference found
    if (mcode.valid_branch<2 && mcode.valid_position==0)
      std::cout << ic<<":"<<cv->getLogVol()->getName()<<":branch content:"<<mcode.valid_branch<<":branch transforms:"<<mcode.valid_position<<std::endl;
    else
      std::cout << ic<<":"<<cv->getLogVol()->getName()<<":branch content:"<<mcode.valid_branch<<":branch transforms:"<<mcode.valid_position<<" at position:x:y:z:"<<tr_test.translation().x()<<":"<<tr_test.translation().y()<<":"<< tr_test.translation().z()<<std::endl;

     m_test2db.push_back(mcode);  

 }

  // run a cross-check:  ref->test to spot duplicates; collect non-resolved for more efficient search
  std::cout <<" running ref->test check ( search optimization )" << std::endl; 
  std::vector<const GeoVPhysVol*> ref_nonresolved;
  for (unsigned int ic = 0; ic < ncref; ic++) {
    const GeoVPhysVol* cvref = &(*(top_ref->getChildVol(ic)));
    GeoModelTools::GM_valid mcode( cvref, 0, 1, -1,-1);
    if (ic>nctest-1) {
      mcode.ref_branch = findBranch(top_test,cvref->getLogVol()->getName(),true,1);
      if (mcode.ref_branch) std::cout<<"cross reference found:"<< cvref->getLogVol()->getName() << std::endl;
      else ref_nonresolved.push_back(cvref);
      m_db2test.push_back(mcode);
      continue;
    }
    const GeoVPhysVol* cv = &(*(top_test->getChildVol(ic)));
    cname = cv->getLogVol()->getName();
    cnameref = cvref->getLogVol()->getName();
    
    if (cname!=cnameref) {
      mcode.ref_branch = findBranch(top_test,cvref->getLogVol()->getName(),true,1);
      if (mcode.ref_branch) std::cout<<"cross reference found:"<< cvref->getLogVol()->getName() << std::endl;
      else ref_nonresolved.push_back(cvref);
      m_db2test.push_back(mcode);
      continue;
    }
    mcode.ref_branch = cv;
    mcode.valid_position = m_test2db[ic].valid_position;
    mcode.valid_branch = m_test2db[ic].valid_branch;
    m_db2test.push_back(mcode);  
 }

  if ( identicalTreeHierarchy && nctest==ncref )   return;  //  looks like no further step is needed

  for (auto nonres : ref_nonresolved) std::cout << "non-resolved reference branch:"<< nonres->getLogVol()->getName()<<std::endl;   

  //==========================================================================
      
  std::cout << "Validation strategy B ( resolve alignable stations which were moved in the tree and / or renamed )"<<std::endl;

  for (unsigned int ic = 0; ic < nctest; ic++) {
    if (m_test2db[ic].ref_branch) continue;
    const GeoVPhysVol* cv = top_test->getChildVol(ic);
    const GeoVFullPhysVol* cvf = dynamic_cast<const GeoVFullPhysVol*> (cv);
    if (!cvf) continue;
    cname = cv->getLogVol()->getName();
    GeoTrf::Transform3D tr_test = cvf->getAbsoluteTransform();
    const GeoVPhysVol* cvref = 0;
    for (auto nonres : ref_nonresolved) {
      cvref = findAlignableBranch(nonres,cname,tr_test);    // full matching name search
      if (!cvref) cvref = findAlignableBranch(nonres,cname.substr(m_pnami,m_pnaml),tr_test, false);  // partial name matching
      if (cvref) break;
    }   
    if (!cvref) cvref = findAlignableBranch(top_ref,cname,tr_test);    // full matching name search
    if (!cvref) cvref = findAlignableBranch(top_ref,cname.substr(m_pnami,m_pnaml),tr_test, false);   // partial name matching
    
    m_test2db[ic].valid_strategy = 2;  
    
    if (!cvref) {
      std::cout << "unmatched alignable branch :" <<ic <<":"<< cname << std::endl;
      continue;
    }	      
    
    const GeoVFullPhysVol* cvfref = dynamic_cast<const GeoVFullPhysVol*> (cvref);
    GeoTrf::Transform3D tr_ref = cvfref->getAbsoluteTransform();
    
    m_test2db[ic].valid_position = compareTransforms(tr_test,tr_ref, tolerance);  
    
    if (m_test2db[ic].valid_position==6 && printFullInfo )  {
      std::cout <<ic<<":"<<cv->getLogVol()->getName()<<" position differs:translation:"<<std::endl;
      printTranslationDiff( tr_test, tr_ref, tolerance);
    }
    
    if (m_test2db[ic].valid_position==7 && printFullInfo )   {
      std::cout <<ic<<":"<<cv->getLogVol()->getName()<<" position differs:rotation:"<<std::endl;
      printRotationDiff( tr_test, tr_ref, tolerance );
    }
    
    m_test2db[ic].ref_branch = cvref;
    m_test2db[ic].valid_branch = compareGeoVolumes(cv,cvref,tolerance,printFullInfo,0);
    // short printout if no difference found ( or simple renaming )
    if ( m_test2db[ic].valid_branch<2 && m_test2db[ic].valid_position==0 ) 
      std::cout << ic<<":"<<cv->getLogVol()->getName()<<":branch content:"<<m_test2db[ic].valid_branch<<":branch transforms:"<<m_test2db[ic].valid_position<<std::endl;
    else  // print the position as well
      std::cout << ic<<":"<<cv->getLogVol()->getName()<<":branch content:"<<m_test2db[ic].valid_branch<<":branch transforms:"<<m_test2db[ic].valid_position<<" at position:x:y:z:"<<tr_test.translation().x()<<":"<<tr_test.translation().y()<<":"<< tr_test.translation().z()<<std::endl;
    
  } // end loop over tree content
  
    // TODO add symmetric loop (db2test) to spot missing stations or duplicates
  
 //==========================================================================
      
  std::cout << "Validation strategy C ( resolve inert material )"<<std::endl;

  for (unsigned int ic = 0; ic < nctest; ic++) {
    if (m_test2db[ic].ref_branch) continue;
    const GeoVPhysVol* cv = top_test->getChildVol(ic);
    const GeoVFullPhysVol* cvf = dynamic_cast<const GeoVFullPhysVol*> (cv);
    if (cvf) continue;
    cname = cv->getLogVol()->getName();
    m_test2db[ic].valid_strategy = 3;

    PVConstLink cvref = findBranch(top_ref,cname,true,0);      // top level search
    if (!cvref)  for (auto nonres : ref_nonresolved) {            // search in non-resolved ref branches
      cvref = findBranch(nonres,cname);   
      if (cvref) break;
    }   
    if (!cvref) cvref = findBranch(top_ref,cname);      // full search ( time consuming )

    if (cvref) {
      m_test2db[ic].ref_branch = cvref;
      m_test2db[ic].valid_branch = compareGeoVolumes(cv,cvref,tolerance,printFullInfo,0);
      std::cout << ic<<":"<<cv->getLogVol()->getName()<<":branch content:"<<m_test2db[ic].valid_branch<<std::endl;
    } else  { 
      // branch does not exist : if logical volume, try to resolve child volumes
      m_test2db[ic].valid_strategy = -3;    // flip sign to signal abandon of top level object  ( TODO: make it recursive for dummy materials ) 
      for (unsigned int id = 0; id < cv->getNChildVols(); id++) {
	const GeoVPhysVol* dv = &(*(cv->getChildVol(id)));
	std::string dname = dv->getLogVol()->getName();
	const GeoVPhysVol* dvref = findBranch(top_ref,dname);
	if (dvref) {   
	  GeoModelTools::GM_valid mcode( dv, dvref, 3, -1,-1);
	  mcode.valid_branch = compareGeoVolumes(dv,dvref,tolerance,printFullInfo,0);
	  std::cout << ic<<":"<<cname<<":resolved in child branch:"<<id<<":"<<dname<<":content:"<<mcode.valid_branch<<std::endl;
          m_test2db.push_back(mcode); // add the resolved child branch to the list for further checks ( material )
	} else {
	  std::cout << ic<<":"<<cname<<":child branch:"<<dname<<": reference branch not found" << std::endl;
	}
      } // end loop over child volumes
    } // end non-alignable
  } // end loop over tree content

   // TODO add symmetric loop (db2test) to spot missing branches or duplicates

//==========================================================================

  return;    //  the following is blocked till prototyping scheme is resolved to speed up the processing & check identification 
      
  std::cout << "Validation strategy D ( inclusive mass per branch )"<<std::endl;

  GeoModelTools::GeoMaterialHelper mat_helper;

  for (unsigned int ic = 0; ic < nctest; ic++) {
  
    if ( m_test2db[ic].ref_branch) 
    
    std::cout <<ic<<m_test2db[ic].test_branch->getLogVol()->getName()<<":inclusive mass of test branch:"<<mat_helper.evaluateMass(m_test2db[ic].test_branch)<<":ref branch:"<< mat_helper.evaluateMass(m_test2db[ic].ref_branch) <<std::endl;
        
    else std::cout <<ic<<m_test2db[ic].test_branch->getLogVol()->getName()<<":inclusive mass of test branch:"<<mat_helper.evaluateMass(m_test2db[ic].test_branch)<<":ref branch not found:"<<std::endl;
  
  }

  return;
}
 
int GeoModelTools::GeoPhysVolHelper::compareGeoVolumes( const GeoVPhysVol* gv1, const GeoVPhysVol* gv2, float tolerance, bool dumpInfo, int level , int returnCode )   {

  m_diff = returnCode;

  // CASE 1: naming difference 
  if (gv1->getLogVol()->getName() != gv2->getLogVol()->getName())  {
    m_diff = 1000*level+1;
     if (dumpInfo) {
      std::cout <<"CASE 1: names differ at level:"<<level<<":"<<gv1->getLogVol()->getName() <<":"<<gv2->getLogVol()->getName() << std::endl; 
    }
     //  else return m_diff;    // the naming change is harmless and can mask more serious problems, continue the loop
  }  
  // CASE 2: material type difference
  if (gv1->getLogVol()->getMaterial()->getName() != gv2->getLogVol()->getMaterial()->getName())  {
    m_diff = 1000*level+2;
    if (dumpInfo) {
      std::cout <<"CASE 2: material types differ for volume:"<<gv1->getLogVol()->getName()<<":at level:"<<level<<":"<<gv1->getLogVol()->getMaterial()->getName() <<":differs from:"<<gv2->getLogVol()->getMaterial()->getName() << std::endl; 
     }
    else return m_diff;
  }
  //  CASE 3: shape type difference
  if (gv1->getLogVol()->getShape()->type() != gv2->getLogVol()->getShape()->type() ) {
    m_diff = 1000*level+3;
    if (dumpInfo) {
      std::cout <<"CASE 3: shape types differ at level:"<<level<<":volume name:"<<gv1->getLogVol()->getName() <<":shape:"<<gv1->getLogVol()->getShape()->type() <<":shape ref:"<<gv2->getLogVol()->getShape()->type() << std::endl; 
    } 
    else  return m_diff;
  }
  //  CASE 4: difference in shape definition   
  if ( !compareShapes( gv1->getLogVol()->getShape(), gv2->getLogVol()->getShape(), tolerance ) ) {
    m_diff = 1000*level+4;
    if (dumpInfo) {
      std::cout <<"CASE 4: shape definition differ at level:"<<level<<":decoding:"<< std::endl; 
      std::cout <<"(a) decoding shape definition for child volume:"<<gv1->getLogVol()->getName()<< std::endl;
      decodeShape(gv1->getLogVol()->getShape());
      std::cout <<"(b) decoding shape definition for child volume:"<<gv2->getLogVol()->getName()<< std::endl;
      decodeShape(gv2->getLogVol()->getShape());
      // extract objects for visual inspection
      if (m_dump_diff_path!="" ) {
	saveToDb(gv1, m_dump_diff_path+"/"+gv1->getLogVol()->getName()+"_test.db",true);
	saveToDb(gv2, m_dump_diff_path+"/"+gv2->getLogVol()->getName()+"_ref.db",true);
      }
    }
    else return m_diff;
  }
  // CASE 5: difference in the number of child volumes
  if (gv1->getNChildVols() != gv2->getNChildVols())  {
    m_diff = 1000*level+5;
    if (dumpInfo) { 
      std::cout <<"CASE 5: number of child vols differ at level:"<<level<< ":volume name:"<<gv1->getLogVol()->getName()<<":nChildVols:" <<gv1->getNChildVols()<<":nChildVols ref:"<<gv2->getNChildVols()<< std::endl;
    } 
    else return m_diff;  
  }

  // CASE 6 & 7: transform to child difference 
  for (unsigned int ic = 0; ic < gv1->getNChildVols() ; ic++) {
    GeoTrf::Transform3D transf1 = gv1->getXToChildVol(ic);
    GeoTrf::Transform3D transf2 = gv2->getXToChildVol(ic);

    const GeoVPhysVol* cv1 = &(*(gv1->getChildVol(ic)));
    const GeoVPhysVol* cv2 = &(*(gv2->getChildVol(ic)));

    if ((transf1.translation()-transf2.translation()).norm()>tolerance) {
      m_diff = 1000*level + 10*ic + 6;
      if (dumpInfo) {
	std::cout <<"CASE 6: translation differs at level:"<<level<<": between mother and child:"<<gv1->getLogVol()->getName()<<":"<< cv1->getLogVol()->getName()<<std::endl;
        printTranslationDiff(transf1,transf2,tolerance);
      } 
      else return m_diff; 
    }
    GeoTrf::RotationMatrix3D rot = transf1.rotation()*transf2.rotation().inverse();
    if ( fabs(rot(0,1))>tolerance || fabs(rot(0,2))>tolerance || fabs(rot(1,2))>tolerance)  {
      m_diff = 1000*level + 10*ic + 7;
      if (dumpInfo) {
	std::cout <<"CASE 7: rotation differs at level:"<<level<<":between mother and child:"<<gv1->getLogVol()->getName()<<":"<< cv1->getLogVol()->getName()<<std::endl;
        printRotationDiff(transf1,transf2,tolerance);
      }
     else return m_diff; 
    }
 
    int child_comp = compareGeoVolumes(cv1,cv2,tolerance,dumpInfo,level+1, m_diff); 
    if (child_comp != m_diff) std::cout <<"WARNING: faulty logic in return code"<<m_diff<<":"<<child_comp << std::endl;
  } 

  return m_diff;
}

// temporary, to be replaced by GMIO.h  
void GeoModelTools::GeoPhysVolHelper::saveToDb(const GeoVPhysVol* pv, std::string filename, bool silentMode )  { 

  std::ifstream infile(filename.c_str());

  if (infile.good() ) {
    if (silentMode )  return;
    std::cout << "Warning : file :"<< filename <<" exists already: dump cancelled " << std::endl;
  }

  // open the DB connection
  GMDBManager db(filename);
  
  // check the DB connection
  if (db.checkIsDBOpen()) {
    std::cout << "OK! Database is open!" << std::endl;
  } else {
    std::cout << "Database ERROR!! Exiting..."<< filename << std::endl;
    return;
  }
 
  // Dump the tree volumes to a local file
  std::cout << "Dumping the GeoModel geometry to the DB file..." << std::endl;
  GeoModelIO::WriteGeoModel dumpGeoModelGraph(db); // init the GeoModel node action
  pv->exec(&dumpGeoModelGraph); // visit all GeoModel nodes
  dumpGeoModelGraph.saveToDB(); // save to the SQlite DB file
  std::cout << "DONE. Geometry saved into "<<filename <<std::endl;
  
  return;
}

// temporary, to be replaced by GMIO.h
PVConstLink GeoModelTools::GeoPhysVolHelper::retrieveFromDb(const std::string& filename) const  { 

  // open the DB
  auto db = std::make_unique<GMDBManager>(filename);

  if (!db->checkIsDBOpen()) { 
    std::cout << "error in database readout, no GeoVolume retrieved"<< std::endl;
    return nullptr;
  }
 
   // setup the GeoModel reader 
  GeoModelIO::ReadGeoModel readInGeo{std::move(db)};
  
  return readInGeo.buildGeoModel(); // builds the GeoModel tree in memory
}


PVConstLink GeoModelTools::GeoPhysVolHelper::findBranch( const GeoVPhysVol* top, std::string name, bool fullName, int searchDepth ) const {

  PVConstLink branch{nullptr};
  if (fullName && name == top->getLogVol()->getName() ) return top;
  if (!fullName && top->getLogVol()->getName().find(name)!=std::string::npos ) return top;
  GeoModelTools::GeoMaterialHelper mat_helper;
  if (!mat_helper.dummy_material(top) && searchDepth>-2) return branch;  // stop short of splitting material branch 
  unsigned int nc = top->getNChildVols();
  std::string cname;  
  // proceed level by level
  for (unsigned int ic = 0; ic < nc; ic++) {
    const GeoVPhysVol* cv = top->getChildVol(ic);
    const GeoLogVol* clv = cv->getLogVol();
    if (fullName && name == clv->getName() )  return cv;
    if (!fullName && clv->getName().find(name)!=std::string::npos ) return cv;
  }
  if (searchDepth==0) return branch;    //  stop search 
  int newSearchDepth = ( searchDepth<0 ) ? searchDepth : searchDepth - 1;
  for (unsigned int ic = 0; ic < nc; ic++) {
    const GeoVPhysVol* cv = top->getChildVol(ic);
    const GeoLogVol* clv = cv->getLogVol();
    branch = findBranch(cv, name, fullName, newSearchDepth);
    if (branch) return branch;   
  }
  return branch;
} 

PVConstLink GeoModelTools::GeoPhysVolHelper::findAlignableBranch( const GeoVPhysVol* top, std::string name, GeoTrf::Transform3D transf, bool fullName, int searchDepth ) const {

  PVConstLink branch{};
  GeoModelTools::GeoMaterialHelper mat_helper;
  if (!mat_helper.dummy_material(top) && searchDepth>-2) return branch;  // stop short of splitting material branch 
  unsigned int nc = top->getNChildVols();
  std::string cname;  double dist;
  // compare distance to name-matching objects at the same level 
  std::pair<PVConstLink,double> bestMatch(0,-1.);
  for (unsigned int ic = 0; ic < nc; ic++) {
    const GeoVPhysVol* cv = top->getChildVol(ic);
    const GeoLogVol* clv = cv->getLogVol();
    if ((fullName && name == clv->getName()) || (!fullName && clv->getName().find(name)!=std::string::npos)  ) {
      const GeoVFullPhysVol* cvf = dynamic_cast<const GeoVFullPhysVol*> (cv);
      if (cvf) { 
	GeoTrf::Transform3D trf = cvf->getAbsoluteTransform();
	dist = (trf.translation()-transf.translation()).norm();
        if (bestMatch.second<0. || dist<bestMatch.second ) bestMatch=std::make_pair(cv,dist);
      }
    }
  }
  if ( bestMatch.first) return bestMatch.first;

  // if no match, test next level  
  if (searchDepth==0) return 0;    //  stop search 
 int newSearchDepth = ( searchDepth<0 ) ? searchDepth : searchDepth - 1;
  for (unsigned int ic = 0; ic < nc; ic++) {
    const GeoVPhysVol* cv = &(*(top->getChildVol(ic)));
    branch = findAlignableBranch(cv, name, transf, fullName, newSearchDepth);
    if (branch) return branch;   
 }

  return bestMatch.first;
} 

void GeoModelTools::GeoPhysVolHelper::printInfo(const GeoVPhysVol* pv, int historyDepth) const {

  const GeoLogVol* lv = pv->getLogVol();
  std::cout <<"printInfo : new Object:" << lv->getName() << ", made of " << lv->getMaterial()->getName() << " x0 "
	    << lv->getMaterial()->getRadLength() << "," << lv->getShape()->type()<<std::endl;
  //shapeCnv.decodeShape(lv->getShape());
  int level = 0;
  printChildren(pv,level,historyDepth);
}

void GeoModelTools::GeoPhysVolHelper::printChildren(const GeoVPhysVol* pv, int level, int historyDepth) const {

  // subcomponents
  unsigned int nc = pv->getNChildVols();
  level++;  std::string cname; if (historyDepth>0 && level > historyDepth) return; 
  for (unsigned int ic = 0; ic < nc; ic++) {
    GeoTrf::Transform3D transf = pv->getXToChildVol(ic);

    const GeoVPhysVol* cv = &(*(pv->getChildVol(ic)));
    const GeoFullPhysVol* cfv = dynamic_cast<const GeoFullPhysVol*> (cv);
    const GeoLogVol* clv = cv->getLogVol();
    if (ic==0) cname = clv->getName();
    if (cfv) {
      //const std::string&  fname = new GeoFullPhysVol(*cfv)->getAbsoluteName();
      //std::cout <<"GeoFullPhysVol:"<<clv->getName()<<":identifier:"<<cfv->getId()<< std::endl;
      std::cout <<"GeoFullPhysVol:"<<clv->getName()<< std::endl;
    }
    std::cout <<"subcomponent:" << level<<":"<<ic << ":" << clv->getName() << ", made of " << clv->getMaterial()->getName()
                                    << " x0 " << clv->getMaterial()->getRadLength() << " , "
                                    << clv->getShape()->type() << "," << transf.translation().x() << " "
	      << transf.translation().y() << " " << transf.translation().z()<<": nChild:"<< cv->getNChildVols() << std::endl;;

    // shapeCnv.decodeShape(clv->getShape());

    if (ic==0 || clv->getName()!=cname) printChildren(cv,level,historyDepth);
 
    cname = clv->getName();
  }
}

bool GeoModelTools::GeoPhysVolHelper::compareShapes( const GeoShape* sh1, const GeoShape* sh2, float /*tol*/ ) const {
  GeoShapeSorter sorter{};
  return sorter.compare(sh1, sh2) != 0;
}

void GeoModelTools::GeoPhysVolHelper::decodeShape(const GeoShape* sh) const {

  std::cout << "decoding shape:"<< sh->type() << std::endl;

  if ( sh->type()=="Trd") {
    const GeoTrd* trd = dynamic_cast<const GeoTrd*> (sh);
    std::cout << "dimensions:" << trd->getXHalfLength1() << ","
	      << trd->getXHalfLength2() << ","
	      << trd->getYHalfLength1() << ","
	      << trd->getYHalfLength2() << ","
	      << trd->getZHalfLength() << std::endl;
  }

  if ( sh->type()=="SimplePolygonBrep") {
    const GeoSimplePolygonBrep* spb = dynamic_cast<const GeoSimplePolygonBrep*> (sh);
 
    int nv = spb->getNVertices();
    std::cout <<"number of planes: dZ:"<<nv<<":"<<spb->getDZ()<< std::endl;
    
    for (unsigned int iv = 0; iv < nv; iv++) {

      std::cout <<"x,y:"<<spb->getXVertex(iv)<<","<<spb->getYVertex(iv)<< std::endl;
    }
  }


  if ( sh->type()=="Subtraction") {
    const GeoShapeSubtraction* sub = dynamic_cast<const GeoShapeSubtraction*> (sh);
    const GeoShape* sha = sub->getOpA();
    const GeoShape* shs = sub->getOpB();
    std::cout << "decoding subtracted shape:" << std::endl;
    decodeShape(sha);
    decodeShape(shs);
  }

  if ( sh->type()=="Union") {
    const GeoShapeUnion* sub = dynamic_cast<const GeoShapeUnion*> (sh);
    const GeoShape* shA = sub->getOpA();
    const GeoShape* shB = sub->getOpB();
    std::cout << "decoding shape A:" << std::endl;
    decodeShape(shA);
    std::cout << "decoding shape B:" << std::endl;
    decodeShape(shB);
  }

  if ( sh->type()=="Shift") {
    const GeoShapeShift* shift = dynamic_cast<const GeoShapeShift*> (sh);
    const GeoShape* shA = shift->getOp();
    const GeoTrf::Transform3D& transf = shift->getX();
    std::cout << "shifted by:x:y:z:" <<transf.translation().x()<<":"<<transf.translation().y()<<":"<<transf.translation().z()<<std::endl;
    std::cout <<"rotation diag:"<< transf(0,0)<<"," << transf(1,1) <<"," << transf(2,2) << std::endl;
    decodeShape(shA);
  }

}

int GeoModelTools::GeoPhysVolHelper::compareTransforms(GeoTrf::Transform3D tr_test, GeoTrf::Transform3D tr_ref, 
                                                        float /*tolerance*/) const {
  GeoTrf::TransformSorter sorter{};
  return sorter.compare(tr_test, tr_ref);
}
						   
void GeoModelTools::GeoPhysVolHelper::printTranslationDiff(GeoTrf::Transform3D tr_test, GeoTrf::Transform3D tr_ref, float tolerance) const {

  std::cout << std::fixed << std::setprecision(4);
  std::cout << "test translation:x:y:z:" << tr_test.translation().x() <<":" << tr_test.translation().y() <<":" << tr_test.translation().z() <<std::endl; 
  std::cout << " ref  translation:x:y:z:" << tr_ref.translation().x() <<":" << tr_ref.translation().y() <<":" << tr_ref.translation().z() <<std::endl; 
  std::cout <<" absolute shift :" <<	(tr_test.translation()-tr_ref.translation()).norm()<<": to be compared with the tolerance limit:"<<tolerance<< std::endl;
  
}

void GeoModelTools::GeoPhysVolHelper::printRotationDiff(GeoTrf::Transform3D tr_test, GeoTrf::Transform3D tr_ref, float tolerance) const {

  GeoTrf::RotationMatrix3D rotest = tr_test.rotation();
  GeoTrf::RotationMatrix3D rotref = tr_ref.rotation();
  GeoTrf::RotationMatrix3D rotid = rotest*rotref.inverse();

  std::cout << std::fixed << std::setprecision(4);
  std::cout << "test rotation:" << rotest(0,0) <<":" << rotest(0,1) <<":" << rotest(0,2) <<std::endl; 
  std::cout << "                   " << rotest(1,0) <<":" << rotest(1,1) <<":" << rotest(1,2) <<std::endl; 
  std::cout << "                   " << rotest(2,0) <<":" << rotest(2,1) <<":" << rotest(2,2) <<std::endl; 
  std::cout << " ref rotation:" << rotref(0,0) <<":" << rotref(0,1) <<":" << rotref(0,2) <<std::endl; 
  std::cout << "                   " << rotref(1,0) <<":" << rotref(1,1) <<":" << rotref(1,2) <<std::endl; 
  std::cout << "                   " << rotref(2,0) <<":" << rotref(2,1) <<":" << rotref(2,2) <<std::endl; 
  std::cout << "test*inv(ref):" << rotid(0,0) <<":" << rotid(0,1) <<":" << rotid(0,2) <<std::endl; 
  std::cout << "                   " << rotid(1,0) <<":" << rotid(1,1) <<":" << rotid(1,2) <<std::endl; 
  std::cout << "                   " << rotid(2,0) <<":" << rotid(2,1) <<":" << rotid(2,2) <<std::endl; 
 std::cout <<" identity check fails within the tolerance limit:"<<tolerance<< std::endl;
  
}
