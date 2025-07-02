/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/
// Author: Joe Boudreau, repurposed from orginal work from
//              - Riccardo Maria Bianchi, Apr 2017
//              - Sebastian Andreas Merkt, 2018
// Update: Nov 2019
//
// Qt includes
#include <QString>
#include <QDir>
#include <QDebug>
#include <QCommandLineParser>

//
#include "VP1Gui/GXExecutionScheduler.h"

// C++ includes
#include <iostream>
#include <cstdlib>

// utility functions to get preprocessor variables' values
#define STR_VALUE(arg) #arg
#define STR_NAME(name) STR_VALUE(name)



int main(int argc, char** argv)
{

#ifndef __APPLE__
    
  setenv("LC_ADDRESS","en_US.UTF-8",1);
  setenv("LC_NAME","en_US.UTF-8",1);
  setenv("LC_MONETARY","en_US.UTF-8",1);
  setenv("LC_PAPER","en_US.UTF-8",1);
  setenv("LC_IDENTIFICATION","en_US.UTF-8",1);
  setenv("LC_TELEPHONE","en_US.UTF-8",1);
  setenv("LC_MEASUREMENT","en_US.UTF-8",1);
  setenv("LC_TIME","en_US.UTF-8",1);
  setenv("LC_NUMERIC","en_US.UTF-8",1);
  setenv("QT_QPA_PLATFORM","xcb",0);
#endif
  
  auto pManip= [] (const char * variable, const char *plus) {
		 const char *path=getenv(variable);
		 if (path) {
		   std::string newPath=std::string(path)+":"+plus;
		   setenv(variable, newPath.c_str(), true);
		 }
		 else {
		   setenv(variable, plus, true);
		 }
	       };
  
  std::string gxpluginpath= GXPLUGINDIR;
  pManip("GXPLUGINPATH",gxpluginpath.c_str());

  QStringList arguments;
  for (int i = 0; i<=argc; i++){
    arguments << argv[i];
  }

  QCommandLineParser parser;

  // Print help
  QCommandLineOption helpOption(QStringList() << "h" << "help", "Show help.");
  parser.addOption(helpOption);

  // Process the command line arguments given by the user
  parser.process(arguments);

  bool helpIsSet = parser.isSet(helpOption);

  QStringList inputList=parser.positionalArguments();

  // This handles the input geometry files and the plugins
  int NGeomFiles=0;
  for (int i=0;i<inputList.size()-1;i++) {
    std::string input=inputList[i].toStdString();
    if (input.find(".db")!=std::string::npos ||
	input.find(".so")!=std::string::npos ||
	input.find(".dylib")!=std::string::npos)
      {
      setenv((std::string("GX_GEOMETRY_FILE")+std::to_string(NGeomFiles++)).c_str(),inputList[i].toStdString().c_str(), true);
    }
  }

  // This handles the JSON files containing the geometry clash information
  int NJsonFiles=0;
  for (int i=0;i<inputList.size()-1;i++) {
    std::string input=inputList[i].toStdString();
    if (input.find(".json")!=std::string::npos){

      setenv((std::string("GX_JSON_FILE")+std::to_string(NJsonFiles++)).c_str(),inputList[i].toStdString().c_str(), true);
    }
  }


  //If help option is set, display help and exit gmex
  if(helpIsSet){
    qInfo() << "Usage";
    qInfo() << " ";
    qInfo() << "  gmex [options] [dbfile1] [dbfile2]... [sharedlib1] [sharedlib2] [sharedlib3]... [clash1.json] [clash2.json]...";
    qInfo() << "Options:";
    qInfo() << "  -h, --help                   = Show help.";
    return 0;
  }

  GXExecutionScheduler *scheduler=GXExecutionScheduler::init();
  while (scheduler->interact())
  GXExecutionScheduler::cleanup(scheduler);

}
