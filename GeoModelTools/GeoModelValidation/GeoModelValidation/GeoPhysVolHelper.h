/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// GeoPhysVolHelper.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef GEOPHYSVOLHELPER_H
#define GEOPHYSVOLHELPER_H

#include <GeoModelKernel/GeoDefinitions.h>
#include <GeoModelKernel/GeoVPhysVol.h>
#include <vector>

class GeoVPhysVol;
class GeoPhysVol;
class GeoShape;

namespace GeoModelTools {     // TODO resolve prototype/clone, identification scheme

  struct GM_valid{
    const GeoVPhysVol*  test_branch;
    const GeoVPhysVol*  ref_branch;
    int                valid_strategy;
    int                                 valid_position; 
    int                                 valid_branch; 

    GM_valid(const GeoVPhysVol*  testvol, const GeoVPhysVol*  refvol, unsigned int strategy, int code_pos, int code_branch  ) :
    test_branch(testvol), ref_branch(refvol), valid_strategy(strategy), valid_position(code_pos), valid_branch(code_branch){}  
  };

  /**
    @class GeoPhysVolHelper
    
    A helper class that browses the GeoModel tree for validation purposes
        
    @author sarka.todorova@cern.ch
    */
    
    class GeoPhysVolHelper {
      
      public:
        /** Default constructor*/
        GeoPhysVolHelper() = default;
        /** Destructor*/
        virtual ~GeoPhysVolHelper() = default;

	/** Recursive comparison of trees/branches/volumes : 
	    in quiet mode (printFullInfo=False) , returns the indicator of first encountered difference ( 0 if none),
	    in verbose mode (printFullInfo=True), returns the indicator of last encountered difference ( 0 if none) */
 	int compareGeoVolumes( const GeoVPhysVol* gv1, const GeoVPhysVol* gv2, float tolerance, bool printFullInfo = false,  int level=0 , int returnCode=0); 

        /** Comparison of databases via sequential deployment of validation strategies to resolve modified tree hierarchy;
            naming change resolved for alignable(sensitive) branches; 
	    if no input branch name given, the comparison starts at the top level  */  
        void compareDB( std::string dbref_path, std::string dbtest_path, std::string branch_ref= "", std::string branch_test = "", bool printFullInfo = false, float tolerance = 1.e-4);

        /** Comparison of transforms : returns 0 code for identity, 6 for shift, 7 for change of orientation */
        int compareTransforms(GeoTrf::Transform3D trtest, GeoTrf::Transform3D trref, float tolerance) const;
 
	/** save tree/branch/volume into sqlite database file : to be replaced by GMIO.h */
        void saveToDb( const GeoVPhysVol* gv, std::string filename, bool silentMode=false); 

	/** retrieve tree/branch/volume from sqlite database file : to be replaced by GMIO.h */
        PVConstLink retrieveFromDb(const std::string& filename) const; 

	/** shape comparison */
	bool compareShapes( const GeoShape* gs1, const GeoShape* gv2, float tolerance ) const; 

        /** navigation through logical volumes */
        PVConstLink findBranch( const GeoVPhysVol* top, std::string name, bool fullName=true, int searchDepth=-1 ) const;
        PVConstLink findAlignableBranch( const GeoVPhysVol* top, std::string name, GeoTrf::Transform3D transf, bool fullName=true, int searchDepth=-1 ) const;

        /** define partial name */
        void set_partial_name(unsigned int i0, unsigned int len) {
	  m_pnami = i0; m_pnaml = len; 
	} 

        /** define output directory */
        void set_path_for_dump_diff( std::string path) { m_dump_diff_path = path; }
    
	/** dumps from GeoModel tree for debugging  */
	void printInfo(const GeoVPhysVol* pv, int historyDepth=-1) const;
	void printChildren(const GeoVPhysVol* pv, int level, int historyDepth) const;
	void decodeShape(const GeoShape* sh) const;

      private:
        mutable int m_diff;
        mutable std::vector< GM_valid >  m_test2db;
        mutable std::vector< GM_valid >  m_db2test;
        unsigned int m_pnami = 0;
        unsigned int m_pnaml = 2;
	std::string m_dump_diff_path = "";
	/** check of rotation invariance */
	bool identity_check(GeoTrf::RotationMatrix3D rotation, float tol) const;
        /** printout diff - unify output */
        void printTranslationDiff(GeoTrf::Transform3D trtest, GeoTrf::Transform3D trref, float tolerance) const;
        void printRotationDiff(GeoTrf::Transform3D trtest, GeoTrf::Transform3D trref, float tolerance) const;
   };
 
} // end of namespace GeoModelTools

#endif

