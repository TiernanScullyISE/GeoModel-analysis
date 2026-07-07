/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "GeoModelKernel/GeoVGeometryPlugin.h"

#include "GeoModelHelpers/StringUtils.h"
#include "GeoModelHelpers/FileUtils.h"
#include "GeoModelKernel/throwExcept.h"
#include "GeoModelKernel/GeoPhysVol.h"

#include "GeoModelXml/GmxInterface.h"
#include "GeoModelXml/Gmx2Geo.h"


#include <iostream>
#include <fstream>
#include <sstream>

class GMXPlugin : public GeoVGeometryPlugin  {

 public:

  // Constructor:
  GMXPlugin() = default;

  // Constructor with name to provide to publisher  
  GMXPlugin(std::string name):GeoVGeometryPlugin(name){}

  // Destructor:
  ~GMXPlugin() = default;

  // Creation of geometry:
  virtual void create(GeoVPhysVol *world, bool publish) override;

 private:

  // Illegal operations:
  const GMXPlugin & operator=(const GMXPlugin &right)=delete;
  GMXPlugin(const GMXPlugin &right) = delete;

};




//## Other Operations (implementation)
void GMXPlugin::create(GeoVPhysVol *world, bool publish) {  
  std::cout<< "This is GMXPlugin: creating a GeoModelXml detector "<<std::endl;
  std::vector<std::string> filesToParse = GeoStrUtils::tokenize(GeoStrUtils::resolveEnviromentVariables("${GMX_FILES}"), ":");
  
  if (filesToParse.empty()) {
    filesToParse.push_back("gmx.xml");
  }
  
  
  bool matman{false};  
  const std::string matManager = GeoStrUtils::resolveEnviromentVariables("${GMX_USE_MATMANAGER}");
  /// Flags to perform the deduplication
  const bool deDuplicateShapes = GeoStrUtils::atoi(GeoStrUtils::resolveEnviromentVariables("${GMX_USE_SHAPEDEDUPL}"));
  const bool deDuplicateLogVol = GeoStrUtils::atoi(GeoStrUtils::resolveEnviromentVariables("${GMX_USE_LOGVOLDEDUPL}"));
  const bool deDuplicatePhysVol = GeoStrUtils::atoi(GeoStrUtils::resolveEnviromentVariables("${GMX_USE_PHYSVOLDEDUPL}"));
  const bool deDuplicateTrfVol = GeoStrUtils::atoi(GeoStrUtils::resolveEnviromentVariables("${GMX_USE_TRANSFDEDUPL}"));
  
  const std::string setLogLevel = GeoStrUtils::resolveEnviromentVariables("${GMX_LOG_LEVEL}");
  


  if (matManager.size()){
  	std::cout<<" Environment variable GMX_USE_MATMANAGER set to "<<matManager<<std::endl;
    matman = GeoStrUtils::atoi(matManager);
    std::cout<<"matman set to "<<matman<<std::endl;
  }

  std::string levelMaps= GeoStrUtils::resolveEnviromentVariables("${GMX_DUMP_LEVEL_MAPS}");

  for (auto f: filesToParse) {
    if (!GeoFileUtils::doesFileExist(f)) {
    	THROW_EXCEPTION("GDMLtoGeo: input file "<<f<<" does not exist.");   	  
    }
    
    GmxInterface gmxInterface;
    gmxInterface.enableMaterialManager(matman);
    gmxInterface.enableShapeDeDuplication(deDuplicateShapes);
    gmxInterface.enableLogVolDeDuplication(deDuplicateLogVol);
    gmxInterface.enablePhysVolDeDuplication(deDuplicatePhysVol);
    gmxInterface.enableTransformDeDuplication(deDuplicateTrfVol);

    //Could be simpler if we used an int to label the log levels but  prefer to make it explicit
    //so that the environment variable can be more intuitive

    if(setLogLevel == "FATAL") gmxInterface.setLogLevel(LogLevel::FATAL);
    else if(setLogLevel == "ERROR") gmxInterface.setLogLevel(LogLevel::ERROR);
    else if(setLogLevel == "WARNING") gmxInterface.setLogLevel(LogLevel::WARNING);
    else if(setLogLevel == "INFO") gmxInterface.setLogLevel(LogLevel::INFO);
    else if(setLogLevel == "DEBUG") gmxInterface.setLogLevel(LogLevel::DEBUG);
    else if(setLogLevel == "VERBOSE") gmxInterface.setLogLevel(LogLevel::VERBOSE);
    else if(setLogLevel == "ALWAYS") gmxInterface.setLogLevel(LogLevel::ALWAYS);

    //If we want to write the SQLite, pass a publisher through to fill Aux tables
    //(needed for ReadoutGeometry)
    if(publish) gmxInterface.setPublisher(getPublisher());

    if (levelMaps.size()){
        std::string mapname = f.substr(0, f.length() - 4);
        mapname+="_levelMap.txt";
        std::cout<<"Dumping Copy Number Level Map to "<<mapname<<std::endl;
        Gmx2Geo gmx2Geo(f, world, gmxInterface, 0 , mapname);
    } else Gmx2Geo gmx2Geo(f, world, gmxInterface, 0);
  }

  if (matman) MaterialManager::getManager()->printAll();

}

extern "C" GMXPlugin *createGMXPlugin() {
  return new GMXPlugin("GeoModelXML");
}
