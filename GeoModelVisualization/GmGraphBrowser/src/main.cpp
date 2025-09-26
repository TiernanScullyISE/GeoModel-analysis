#include "mainwindow.h"
#include <QApplication>
#include "GeoModelIOHelpers/GMIO.h"
#include "GeoModelRead/ReadGeoModel.h"
#include <string>
#include <filesystem>
#include <format> 
int main(int argc, char *argv[])
{
  // Parse command line
  std::string usage=std::string("usage: ") + argv[0] + " InputFile [InputFile2]";
  if (argc<2 || argc>3) {
    std::cerr << usage << std::endl;
    return 1;
  }

  // Check if input files exists.
  if (!std::filesystem::exists(argv[1])) {
    std::cerr << std::format("Cannot open file {}; exiting \n.",argv[1]);  
    return 2;
  }

  // Check if input files exists.
  if (argc==3 && !std::filesystem::exists(argv[2])) {
    std::cerr << std::format("Cannot open file {}; exiting \n.",argv[1]);  
    return 3;
  }

  PVConstLink rootVol1;
  {
    std::cout << "Opening file " << argv[1] << std::endl;
    GeoModelIO::ReadGeoModel  inputReader=GeoModelIO::IO::getReaderDB(argv[1]);
    rootVol1=inputReader.buildGeoModel();
  }
  PVConstLink rootVol2;
  if (argc==3) 
  {
    std::cout << "Opening file " << argv[2] << std::endl;
    GeoModelIO::ReadGeoModel  inputReader=GeoModelIO::IO::getReaderDB(argv[2]);
    rootVol2=inputReader.buildGeoModel();
  }

  
  QApplication a(argc, argv);
  MainWindow w;
  w.setTopLevel(rootVol1,0);  
  w.setTopLevel(rootVol2,1);
  if (argc>1) w.setHeaderLabel(argv[1],0);
  if (argc>2) w.setHeaderLabel(argv[2],1);
  w.show();
  return a.exec();

}
