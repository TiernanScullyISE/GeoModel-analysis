#include "GeoIntegrityCheckAction.h"
#include "GeoModelRead/ReadGeoModel.h"
#include "GeoModelDBManager/GMDBManager.h"
#include "GeoModelIOHelpers/GMIO.h"
#include <string>
#include <filesystem>
#include <format> 

#include <iostream>
#include <sqlite3.h>

std::vector<std::string> fetchTables (const std::string db_path, const std::string & prefix="") {
    sqlite3* db;
    sqlite3_stmt* stmt;
    std::vector<std::string> tableVector;
    // Open the database
    if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
	return tableVector;
    }

    // Prepare SQL statement to list tables
    const char* sql = "SELECT name FROM sqlite_master WHERE type='table';";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
	return tableVector;
    }


    // Execute and iterate over results
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* tableName = sqlite3_column_text(stmt, 0);
	std::string tnS=reinterpret_cast<const char *> (tableName);
	if (prefix.empty() || tnS.starts_with(prefix)) {
	  tableVector.push_back(tnS.substr(prefix.length()));
	}
    }

    // Clean up
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return tableVector;
}


int main(int argc, char *argv[])
{
  // Parse command line
  std::string usage=std::string("usage: ") + argv[0] + " InputFile";
  if (argc!=2) {
    std::cerr << usage << std::endl;
    return 1;
  }

  // Check if input files exists.
  if (!std::filesystem::exists(argv[1])) {
    std::cerr << std::format("Cannot open file {}; exiting \n.",argv[1]);  
    return 2;
  }


  using FullPhysVolMap         = std::map<std::string, GeoFullPhysVol*>;
  using AlignableTransformMap  = std::map<std::string, GeoAlignableTransform*>;


  // Parallel vectors;
  std::vector<std::string>           fpvTables=fetchTables(argv[1],"PublishedFullPhysVols_");
  std::vector<FullPhysVolMap>        fpvMaps;

  std::vector<std::string>           alxTables=fetchTables(argv[1],"PublishedAlignableTransforms_");
  std::vector<AlignableTransformMap> alxMaps;
  

  
  
  // This strange procedure insures that reference counts are not augmented.
  const GeoVPhysVol *root{nullptr};
  {
    std::cout << "Opening file " << argv[1] << std::endl;
    GeoModelIO::ReadGeoModel  inputReader=GeoModelIO::IO::getReaderDB(argv[1]);
    PVConstLink rootVol;
    rootVol=inputReader.buildGeoModel();
    root=rootVol.get();
    root->ref();
    for (std::string & s : fpvTables)  fpvMaps.emplace_back(inputReader.getPublishedNodes<std::string, GeoFullPhysVol*>(s));
    for (std::string & s : alxTables)  alxMaps.emplace_back(inputReader.getPublishedNodes<std::string, GeoAlignableTransform*>(s));
  }
  // End of "strange procedure"

  
  GeoIntegrityCheckAction integrityCheckAction;
  root->exec(&integrityCheckAction);
  const std::set<const GeoFullPhysVol *>        fullPhysVolList=integrityCheckAction.getFullPhysicalVolumes();
  const std::set<const GeoAlignableTransform *> alignableTransformList=integrityCheckAction.getAlignableTransforms();

  {
    std::cout << "\nChecking for multiply-published full physical volumes \n\n";
    std::set<GeoFullPhysVol *> fpvSet;
    for (int i=0;i<fpvTables.size();i++) {
      std::cout << "    Checking " << fpvTables[i] <<  std::endl;
      FullPhysVolMap & mapFPV = fpvMaps[i];
      for ( auto const& [key, vol] : mapFPV ) {
	if (fpvSet.find(vol)==fpvSet.end()) {
	  fpvSet.insert(vol);
	}
	else {
	  std::cout << "INFO: the volume " << key << " in system " << fpvTables[i] <<  " is multiply published."  << std::endl;
	}
      }
    }

    std::set<GeoAlignableTransform *> alxSet;
    std::cout << "\nChecking for multiply-published alignable transforms \n\n";
    for (int i=0;i<alxTables.size();i++) {
      std::cout << "    Checking " << alxTables[i] <<  std::endl;
      AlignableTransformMap &mapALX = alxMaps[i];
      for ( auto const& [key, alx] : mapALX ) {
	if (alxSet.find(alx)==alxSet.end()) {
	  alxSet.insert(alx);
	}
	else {
	  std::cout << "INFO: the transform " << key << " in system " << alxTables[i] <<  " is multiply published."  << std::endl;
	}
      }
    }
  }

  std::cout << "\nChecking shared instances of alignable transform (permitted, but unusual) \n\n";
  for (const GeoAlignableTransform *x: alignableTransformList) {
    unsigned int refCount=x->refCount();
    if (refCount!=1) {
      if (refCount!=1) std::cout << "Detected alignable transform with share count " << refCount << std::endl;
      bool published{false};
      for (int i=0;i<alxTables.size();i++) {
	AlignableTransformMap &mapALX = alxMaps[i];
	for ( auto const& [key, alx] : mapALX ) {
	  if (alx==x) {
	    std::cout << "    Published as " << key << " under system " << alxTables[i] << std::endl;
	    published=true;
	  }
	}
      }
      if (!published) std::cout << "   Unpublished" << std::endl;
      
    }
  }


  std::cout << "\nChecking shared instances of full physical volumes (forbidden) \n\n";

    for (const GeoFullPhysVol *v: fullPhysVolList) {
    unsigned int refCount=v->refCount();
    if (refCount!=1) {
      if (refCount!=1) std::cout << "Detected full physical volume  with share count " << refCount << std::endl;
      bool published{false};
      for (int i=0;i<fpvTables.size();i++) {
	FullPhysVolMap &mapFPV = fpvMaps[i];
	for ( auto const& [key, fpv] : mapFPV ) {
	  if (fpv==v) {
	    std::cout << "    Published as " << key << " under system " << fpvTables[i] << std::endl;
	    published=true;
	  }
	}
      }
      if (!published) std::cout << "   Unpublished" << std::endl;
      
    }
  }

  
  return 0;
}
