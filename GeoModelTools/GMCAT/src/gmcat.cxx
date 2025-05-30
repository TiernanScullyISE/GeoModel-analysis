/*
 *   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration   
*/

#include "GeoModelKernel/GeoVGeometryPlugin.h"
#include "GeoModelDBManager/GMDBManager.h"
#include "GeoModelRead/ReadGeoModel.h"
#include "GeoModelWrite/WriteGeoModel.h"
#include "GeoModelHelpers/defineWorld.h"

#include "GeoModelKernel/GeoGeometryPluginLoader.h"
#include "GeoModelKernel/GeoVolumeCursor.h"
#include "GeoModelKernel/GeoMaterial.h"
#include "GeoModelKernel/GeoElement.h"
#include "GeoModelKernel/GeoVPhysVol.h"
#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoCountVolAction.h"
#include "GeoModelKernel/GeoAccessVolumeAction.h"
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelKernel/GeoPublisher.h"

#include <iostream>
#include <string>
#include <vector>
#include <cstdio>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>

#ifdef __APPLE__
const std::string shared_obj_extension=".dylib";
#else
const std::string shared_obj_extension=".so";
#endif


void publishMetaData( GMDBManager & db,
                      const std::string& repoPath,
                      const std::vector<std::string> &inputFiles,
                      const std::vector<std::string> &inputPlugins,
                      const std::vector<std::string> &pluginNames,
                      const std::string              &outputFile);

int main(int argc, char ** argv) {

  bool verbose{false};

  //
  // Usage message:
  //

  std::string gmcat= argv[0];
  std::string usage= "usage: " + gmcat + " [OPTIONS] [plugin1"+shared_obj_extension
    + "] [plugin2" + shared_obj_extension
    + "] ...[file1.db] [file2.db].. -o outputFile]\n"
    + "Options:\n"
    + "\t-v Print verbose output to the screen (default: direct verbose output to /tmp)\n"
    + "\t-g Path to the local GeoModelATLAS repository (default: .)";
  //
  // Print usage message if no args given:
  //
  if (argc==1) {
    std::cerr << usage << std::endl;
    return 0;
  }
  //
  // Parse the command line:
  //
  std::vector<std::string> inputFiles{}, inputPlugins{};
  std::string outputFile;
  std::string gmAtlasDir{"."};
  bool outputFileSet = false;
  for (int argi=1;argi<argc;argi++) {
      std::string argument=argv[argi];
      if (argument.find("-o")!=std::string::npos) {
          argi++;
          if (argi>=argc) {
              std::cerr << usage << std::endl;
              return 1;
          }
          outputFile=argv[argi];
          outputFileSet = true;
      }
      else if (argument.find("-v")!=std::string::npos) {
          setenv("GEOMODEL_GEOMODELIO_VERBOSE", "1", 1); // does overwrite
          verbose=true;
      }
      else if (argument.find("-g")!=std::string::npos) {
          gmAtlasDir = std::string(argv[++argi]);
      }
      else if (argument.find(shared_obj_extension)!=std::string::npos) {
          inputPlugins.push_back(argument);
      }
      else if (argument.find(".db")!=std::string::npos) {
          inputFiles.push_back(argument);
      }
      else {
          std::cerr << "Unrecognized argument " << argument << std::endl;
          std::cerr << usage << std::endl;
          return 2;
      }
  }
  if( !outputFileSet ) {
      std::cerr << "\nERROR! You should set an output file.\n" << std::endl;
      std::cerr << usage << std::endl;
      return 3;
  }

  //
  // Check that we can access the output file
  //
  if (access(outputFile.c_str(),F_OK)==0) {
    if (!access(outputFile.c_str(),W_OK)) {
      if (system(("rm -f "+ outputFile).c_str())) {
          std::cerr << "gmcat -- Error, cannot overwrite existing file " << outputFile << std::endl;
          return 3;
      }
    }
    else {
      std::cerr << "gmcat -- Error, cannot overwrite existing file " << outputFile << " (permission denied)" << std::endl;
      return 4;
    }
  }


  //
  // Create the "GeoWorld" volume, which is the container:
  //
  GeoIntrusivePtr<GeoPhysVol> world{createGeoWorld()};
  

  //
  // Loop over plugins, create the geometry and put it under the world:
  //
  std::ofstream file;
  std::string verboseOutput{"/tmp/gmcat-"+std::to_string(getpid())};
  std::streambuf *coutBuff=std::cout.rdbuf();
  std::streambuf *fileBuff=file.rdbuf();
  if (!verbose) {
    file.open(verboseOutput.c_str());
    std::cout.rdbuf(fileBuff); 
  }
  
  std::vector<GeoPublisher*> vecPluginsPublishers; // caches the stores from all plugins
  std::vector<std::unique_ptr<GeoVGeometryPlugin>> pluginInstances{};
  for (const std::string & plugin : inputPlugins) {
    GeoGeometryPluginLoader loader;

    if(!verbose) {
      std::cout.rdbuf(coutBuff);
      std::cout << "Building geometry using the plugin " << plugin << " ..." << std::endl;
      std::cout.rdbuf(fileBuff);
    }

    std::unique_ptr<GeoVGeometryPlugin> factory{loader.load(plugin)};
    if (!factory) {
      std::cerr << "gmcat -- Could not load plugin " << plugin << std::endl;
      return 5;
    }
    
    // NOTE: we want the plugin to publish lits FPV and AXF nodes, 
    // if it is intended to do that, and store them in the DB. 
    // For that, we pass a true to the plugin's `create()` method, 
    // which will use it to publish nodes, 
    // and then we get from the plugin the pointer to the GeoPublisher instance 
    // and we cache it for later, to dump the published nodes into the DB.
    factory->create(world, true);
    if( nullptr != factory->getPublisher() ) {
        vecPluginsPublishers.push_back( factory->getPublisher() ); // cache the publisher, if any, for later
    }
    if(!verbose) {
      std::cout.rdbuf(coutBuff);
      std::cout << "\t ... DONE!" << std::endl;
      std::cout.rdbuf(fileBuff);
    }
    pluginInstances.emplace_back(std::move(factory));
  }

  //
  // Loop over files, create the geometry and put it under the world:
  //
  for (const std::string & file : inputFiles) {
    if(!verbose) {
      std::cout.rdbuf(coutBuff);
      std::cout << "Reading geometry from the file " << file << " ..." << std::endl;
      std::cout.rdbuf(fileBuff);
    }

    auto db = std::make_unique<GMDBManager>(file);
    if (!db->checkIsDBOpen()){
      std::cerr << "gmcat -- Error opening the input file: " << file << std::endl;
      return 6;
    }

    /* set the GeoModel reader */
    GeoModelIO::ReadGeoModel readInGeo{db.get()};

    /* build the GeoModel geometry */
   PVConstLink dbPhys{readInGeo.buildGeoModel()}; // builds the whole GeoModel tree in memory

    /* get an handle on a Volume Cursor, to traverse the whole set of Volumes */
    GeoVolumeCursor aV(dbPhys);

    /* loop over the Volumes in the tree */
    while (!aV.atEnd()) {
        if (aV.getName()!="ANON") {
          world->add(make_intrusive<GeoNameTag>(aV.getName()));
        }
        world->add(make_intrusive<GeoTransform>(aV.getTransform()));
        world->add(const_pointer_cast(aV.getVolume()));
        aV.next();
    }
    if(!verbose) {
      std::cout.rdbuf(coutBuff);
      std::cout << "\t ... DONE!" << std::endl;
      std::cout.rdbuf(fileBuff);
    }
  }
  //
  // Open a new database:
  //
  GMDBManager db(outputFile);
  //
  // check the DB connection
  //
  if (!db.checkIsDBOpen()) {
    std::cerr << "gmcat -- Error opening the output file: " << outputFile << std::endl;
    return 7;
  }
    
  //resize the world volume to the needed one
  GeoIntrusivePtr<GeoPhysVol> resizedWorld = resizeGeoWorld(world);

  GeoModelIO::WriteGeoModel dumpGeoModelGraph(db);
  resizedWorld->exec(&dumpGeoModelGraph);

  if(!verbose) {
    std::cout.rdbuf(coutBuff);
    std::cout << "Writing auxiliary tables to the output database ..." << std::endl;
    std::cout.rdbuf(fileBuff);
  }
  if (vecPluginsPublishers.size() > 0) {
    dumpGeoModelGraph.saveToDB(vecPluginsPublishers);
  } else {
    dumpGeoModelGraph.saveToDB();
  }
  if(!verbose) {
    std::cout.rdbuf(coutBuff);
    std::cout << "\t ... DONE!" << std::endl;
    std::cout.rdbuf(fileBuff);
  }

  if(!verbose) {
    std::cout.rdbuf(coutBuff);
    std::cout << "Writing metadata to the output database ..." << std::endl;
    std::cout.rdbuf(fileBuff);
  }
  std::vector<std::string> systemNames{};
  for (auto& plugin : pluginInstances) {
    if(plugin->getName().size()){
      systemNames.push_back(plugin->getName());
    }
  }
  std::sort(systemNames.begin(), systemNames.end());

  try {
    publishMetaData(db, gmAtlasDir, inputFiles, inputPlugins, systemNames, outputFile);
  } catch(const std::runtime_error& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return 1;
  }
  if(!verbose) {
    std::cout.rdbuf(coutBuff);
    std::cout << "\t ... DONE!" << std::endl;
    std::cout.rdbuf(fileBuff);
  }

  std::cout.rdbuf(coutBuff);
  std::cout << "SUCCESS!" << std::endl;
  std::cout << "(verbose output at " << verboseOutput << ")" << std::endl;
  return 0;
}
