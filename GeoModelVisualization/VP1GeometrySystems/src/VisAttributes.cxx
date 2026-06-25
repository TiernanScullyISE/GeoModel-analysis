/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

// local includes
#include "VP1GeometrySystems/VisAttributes.h"
#include "VP1Base/VP1QtInventorUtils.h"

// Coin3D includes
#include <Inventor/nodes/SoMaterial.h>

// Qt includes
#include <QBuffer>

// nlohmann_json includes
#include "nlohmann/json.hpp"

// C++ includes
#include <iostream>
#include <fstream>
#include <iomanip>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <map>

// utility functions to get preprocessor variables' values
#define STR_VALUE(arg) #arg
#define STR_NAME(name) STR_VALUE(name)

using namespace nlohmann;


/////////////////////////////////////// Base class ///////////////////////////////////////

namespace GeoModelVis {

  struct Triplet {
    double R;
    double G;
    double B;
  };

  struct Material {
    std::vector<std::string> name;
    Triplet     diffuse;
    Triplet     specular;
    Triplet     ambient;
    Triplet     emissive;
    double      shininess;
    double      transparency;
  };

  void to_json(json& j, const Triplet & t) {
    j=json{{"R",t.R},{"G",t.G},{"B",t.B}};
  }

  void to_json(json& j, const Material & m) {
    j=json{{"name",m.name},
	   {"diffuse",m.diffuse},
	   {"specular",m.specular},
	   {"ambient",m.ambient},
	   {"emissive",m.emissive},
	   {"shininess",m.shininess},
	   {"transparency",m.transparency}};
  }

  void from_json(const json & j, Triplet & t) {
    j.at("R").get_to(t.R);
    j.at("G").get_to(t.G);
    j.at("B").get_to(t.B);
  }

  void from_json(const json& j, Material & m) {
    j.at("name").get_to(m.name);
    j.at("diffuse").get_to(m.diffuse);
    j.at("specular").get_to(m.specular);
    j.at("ambient").get_to(m.ambient);
    j.at("emissive").get_to(m.emissive);
    j.at("shininess").get_to(m.shininess);
    j.at("transparency").get_to(m.transparency);
  }
} // namespace GeoModelVis





class VisAttributes::Imp {
public:
  // The material map is here:
  std::map< std::string, SoMaterial *> _map;

  QMap<QString,QByteArray> currentState() const;
  QMap<QString,QByteArray> initialState;
};

//____________________________________________________________________
void VisAttributes::init()
{
  m_d->initialState = m_d->currentState();


  //
  // This block of code is now skipped. It was originally used to generate
  // json files from memory. I keep it here in case it is ever needed.
  //
  if (0) {
    std::vector<json> jlist;
    for (auto m=m_d->_map.begin();m!=m_d->_map.end();m++) {
      std::string name = m->first;
      SoMaterial  *material=m->second;
      GeoModelVis::Material gMaterial;
      gMaterial.name.push_back(name);
      gMaterial.diffuse=GeoModelVis::Triplet{(*material->diffuseColor.getValues(0))[0],
					     (*material->diffuseColor.getValues(0))[1],
					     (*material->diffuseColor.getValues(0))[2]};
      gMaterial.specular=GeoModelVis::Triplet{(*material->specularColor.getValues(0))[0],
					      (*material->specularColor.getValues(0))[1],
					      (*material->specularColor.getValues(0))[2]};
      gMaterial.ambient=GeoModelVis::Triplet{(*material->ambientColor.getValues(0))[0],
					     (*material->ambientColor.getValues(0))[1],
					     (*material->ambientColor.getValues(0))[2]};
      gMaterial.emissive=GeoModelVis::Triplet{(*material->emissiveColor.getValues(0))[0],
					      (*material->emissiveColor.getValues(0))[1],
					      (*material->emissiveColor.getValues(0))[2]};
      gMaterial.shininess   =*material->shininess.getValues(0);
      gMaterial.transparency=*material->transparency.getValues(0);
      json jMat;
      to_json(jMat,gMaterial);
      jlist.push_back(jMat);
    }
    static int i=0;

    json jReport={{"DetVisAttributes",jlist}};
    std::ofstream outJsonFile("/tmp/visReport"+std::to_string(i++)+".json");
    outJsonFile << std::setw(4) << jReport << std::endl;
    outJsonFile.close();
  }
  // End of disabled part.


}

//____________________________________________________________________
QByteArray VisAttributes::getState(bool onlyChangedMaterials)
{
  //Figure out states to store
  QMap<QString,QByteArray> storedstates, statesnow = m_d->currentState();
  if (onlyChangedMaterials) {
    QMap<QString,QByteArray>::const_iterator it, itE(statesnow.constEnd());
    QMap<QString,QByteArray>::const_iterator itOrig, itOrigE(m_d->initialState.constEnd());
    for (it = statesnow.constBegin(); it!=itE; ++it) {
      itOrig = m_d->initialState.constFind(it.key());
      if (itOrig==itOrigE||it.value()!=itOrig.value())
	storedstates.insert(it.key(),it.value());
    }
  } else {
    storedstates = statesnow;
  }

  //Put map in bytearray and return:
  QByteArray byteArray;
  QBuffer buffer(&byteArray);
  buffer.open(QIODevice::WriteOnly);
  QDataStream out(&buffer);
  out << qint32(0);//version
  out << storedstates;
  buffer.close();
  return byteArray;

}

//____________________________________________________________________
void VisAttributes::applyState(QByteArray ba)
{
  //Get map out:
  QMap<QString,QByteArray> storedstates;
  QBuffer buffer(&ba);
  buffer.open(QIODevice::ReadOnly);
  QDataStream state(&buffer);
  qint32 version;
  state >> version;
  if (version!=0)
    return;//ignore silently
  state >> storedstates;
  buffer.close();

  std::map< std::string, SoMaterial *>::iterator itMat,itMatE(m_d->_map.end());

  //Apply states from map:
  QMap<QString,QByteArray>::const_iterator it, itE(storedstates.constEnd());
  for (it = storedstates.constBegin(); it!=itE; ++it) {
    itMat = m_d->_map.find(it.key().toStdString());
    if (itMat!=itMatE) {
      QByteArray b(it.value());
      VP1QtInventorUtils::deserialiseSoMaterial(b,itMat->second);
    }
  }

}

//____________________________________________________________________
QMap<QString,QByteArray> VisAttributes::Imp::currentState() const
{
  QMap<QString,QByteArray> outmap;
  std::map< std::string, SoMaterial *>::const_iterator it(_map.begin()), itE(_map.end());
  for (;it!=itE;++it)
    outmap.insert(QString(it->first.c_str()),VP1QtInventorUtils::serialiseSoMaterial(it->second));
  return outmap;
}

//____________________________________________________________________
VisAttributes::VisAttributes() : m_d(new Imp) {
}

//____________________________________________________________________
VisAttributes::~VisAttributes() {

  std::map<std::string, SoMaterial *>::iterator m,e=m_d->_map.end();
  for (m=m_d->_map.begin();m!=e;m++)
    (*m).second->unref();

  delete m_d;
}

SoMaterial *VisAttributes::get (const std::string & name) const {
  std::map <std::string, SoMaterial *>::const_iterator m = m_d->_map.find(name);
  if (m!=m_d->_map.end()) {
    return (*m).second;
  } else {
    return NULL;
  }
}

void VisAttributes::add(const std::string & name, SoMaterial *material) {
  if (m_d->_map.find(name)!=m_d->_map.end()) {
    std::cout<<"VisAttributes::add ERROR: Material " <<name<<" already added!"<<std::endl;
    return;
  }
  material->ref();
  m_d->_map[name]=material;
  if (material->transparency.getNum()!=1)
    std::cout<<"VisAttributes::add Warning: Found #transparency values different from 1 in material "<<name<<std::endl;
  if (material->transparency[0]!=0.0)
    std::cout<<"VisAttributes::add Warning: Found transparency value different from 0 in material "<<name<<std::endl;
}

void VisAttributes::overrideTransparencies(float transpfact)
{
  std::map< std::string, SoMaterial *>::iterator it, itE = m_d->_map.end();
  for (it=m_d->_map.begin();it!=itE;++it)
    it->second->transparency.set1Value( 0, transpfact );
}

//////////////////////////////// Attributes for detectors ////////////////////////////////

DetVisAttributes::DetVisAttributes() {

  //
  // TODO: document this!!
  // We look for `gmexDetVisAttributes.json` in these folder and in this order:
  // 1) ${GXSHAREDIR}/gmexDetVisAttributes.json
  // 2) `pwd`
  // 3) ~/.gmexDetVisAttributes.json  (hiden file inside the user's home folder)
  // 4) ${GEOMODEL_INSTALL_PREFIX}/share/gmex/gmexDetVisAttributes.json
  // 5) /usr/local/share/gmex/gmexDetVisAttributes.json
  // 6) /usr/share/gmex/gmexDetVisAttributes.json
  //


#ifdef __APPLE__
  char buffer[1024];
  char *wdLocation=getcwd(buffer,1024);
#else
  char *wdLocation=get_current_dir_name();
#endif

  // struct passwd *pw = getpwuid(getuid());
  // std::string homedir = pw->pw_dir;
  // std::vector<std::string> locations={homedir+"/.","/usr/local/share/gmex/", "/usr/share/gmex/"};
  // char * gxShareDir=getenv("GXSHAREDIR");
  // if (wdLocation) locations.insert(locations.begin(), wdLocation+std::string("/"));
  // if (gxShareDir) locations.insert(locations.begin(), gxShareDir+std::string("/"));

  // TODO: this snippet could go in one single place and used from all three places
  struct passwd *pw = getpwuid(getuid());
  std::string homedir = pw->pw_dir;
  // Init with case 3)
  std::vector<std::string> locations= {homedir+"/."};
  char * gxShareDir=getenv("GXSHAREDIR");
  // Add 2)
  if (wdLocation) locations.insert(locations.begin(), wdLocation+std::string("/"));
  // Add 1)
  if (gxShareDir) locations.insert(locations.begin(), gxShareDir+std::string("/"));
  // Add 4)
  //const char * installPath = STR_NAME( GEOMODEL_INSTALL_PREFIX ); // old CMake setup
  std::string installedShare = std::string(GEOMODEL_INSTALL_PREFIX) + "/share/gmex/";
  locations.push_back(installedShare);
  // Add 5) and 6)
  locations.push_back("/usr/local/share/gmex/");
  locations.push_back("/usr/share/gmex/");


  for (const std::string & s : locations ) {
    std::string userFile=s+std::string("gmexDetVisAttributes.json");
    if (access(userFile.c_str(),R_OK)==0) {
      std::cout << "Reading detector vis attributes from file " << userFile << std::endl;
      std::ifstream in(userFile);
      auto j=json::parse(in);
      for (const json & element : j["DetVisAttributes"]) {
	GeoModelVis::Material mat;
	GeoModelVis::from_json(element, mat);
	SoMaterial *m = new SoMaterial;
	m->ambientColor.setValue(mat.ambient.R, mat.ambient.G, mat.ambient.B);
	m->diffuseColor.setValue(mat.diffuse.R, mat.diffuse.G, mat.diffuse.B);
	m->specularColor.setValue(mat.specular.R, mat.specular.G, mat.specular.B);
	m->emissiveColor.setValue(mat.emissive.R, mat.emissive.G, mat.emissive.B);
	m->shininess.setValue(mat.shininess);
	m->transparency.setValue(mat.transparency);
	for (const std::string & s  : mat.name) add(s,m);
      }
      break;
    }
  }

  init();


}

//////////////////////////////// Attributes for materials ////////////////////////////////

MatVisAttributes::MatVisAttributes() {
  {
    //NB: Must be here!!
    SoMaterial *m = new SoMaterial;
    add("DEFAULT",m);
  }
#ifdef __APPLE__
  char buffer[1024];
  char *wdLocation=getcwd(buffer,1024);
#else
  char *wdLocation=get_current_dir_name();
#endif

  //
  // TODO: document this!!
  // We look for `gmexMatVisAttributes.json` in these folder and in this order:
  // 1) ${GXSHAREDIR}/gmexMatVisAttributes.json
  // 2) `pwd`
  // 3) ~/.gmexMatVisAttributes.json  (hiden file inside the user's home folder)
  // 4) ${GEOMODEL_INSTALL_PREFIX}/share/gmex/gmexMatVisAttributes.json
  // 5) /usr/local/share/gmex/gmexMatVisAttributes.json
  // 6) /usr/share/gmex/gmexMatVisAttributes.json
  //

  // TODO: this snippet could go in one single place and used from all three places
  struct passwd *pw = getpwuid(getuid());
  std::string homedir = pw->pw_dir;
  // Init with case 3)
  std::vector<std::string> locations= {homedir+"/."};
  char * gxShareDir=getenv("GXSHAREDIR");
  // Add 2)
  if (wdLocation) locations.insert(locations.begin(), wdLocation+std::string("/"));
  // Add 1)
  if (gxShareDir) locations.insert(locations.begin(), gxShareDir+std::string("/"));
  // Add 4)
  //const char * installPath = STR_NAME( GEOMODEL_INSTALL_PREFIX ); // old CMake setup
  std::string installedShare = std::string(GEOMODEL_INSTALL_PREFIX) + "/share/gmex/";
  locations.push_back(installedShare);
  // Add 5) and 6)
  locations.push_back("/usr/local/share/gmex/");
  locations.push_back("/usr/share/gmex/");

  for (const std::string & s : locations ) {
    std::string userFile=s+std::string("gmexMatVisAttributes.json");
    if (access(userFile.c_str(),R_OK)==0) {
      std::cout << "Reading material vis attributes from file " << userFile << std::endl;
      std::ifstream in(userFile);
      auto j=json::parse(in);
      for (const json & element : j["MatVisAttributes"]) {
      	GeoModelVis::Material mat;
      	GeoModelVis::from_json(element, mat);
      	SoMaterial *m = new SoMaterial;
      	m->ambientColor.setValue(mat.ambient.R, mat.ambient.G, mat.ambient.B);
      	m->diffuseColor.setValue(mat.diffuse.R, mat.diffuse.G, mat.diffuse.B);
      	m->specularColor.setValue(mat.specular.R, mat.specular.G, mat.specular.B);
      	m->emissiveColor.setValue(mat.emissive.R, mat.emissive.G, mat.emissive.B);
      	m->shininess.setValue(mat.shininess);
      	m->transparency.setValue(mat.transparency);
      	for (const std::string & s  : mat.name) add(s,m);
      }
      break;
    }
  }


//////////////
  init();
}

//////////////////////////////// Attributes for volumes ////////////////////////////////

VolVisAttributes::VolVisAttributes() {
  //
  // TODO: document this!!
  // We look for `gmexMatVisAttributes.json` in these folder and in this order:
  // 1) ${GXSHAREDIR}/gmexVolVisAttributes.json
  // 2) `pwd`
  // 3) ~/.gmexVolVisAttributes.json  (hiden file inside the user's home folder)
  // 4) ${GEOMODEL_INSTALL_PREFIX}/share/gmex/gmexVolVisAttributes.json
  // 5) /usr/local/share/gmex/gmexVolVisAttributes.json
  // 6) /usr/share/gmex/gmexVolVisAttributes.json
  //
#ifdef __APPLE__
  char buffer[1024];
  char *wdLocation=getcwd(buffer,1024);
#else
  char *wdLocation=get_current_dir_name();
#endif

  // struct passwd *pw = getpwuid(getuid());
  // std::string homedir = pw->pw_dir;
  // std::vector<std::string> locations= {homedir+"/.","/usr/local/share/gmex/", "/usr/share/gmex/"};
  // char * gxShareDir=getenv("GXSHAREDIR");
  // if (wdLocation) locations.insert(locations.begin(), wdLocation+std::string("/"));
  // if (gxShareDir) locations.insert(locations.begin(), gxShareDir+std::string("/"));

  // TODO: this snippet could go in one single place and used from all three places
  struct passwd *pw = getpwuid(getuid());
  std::string homedir = pw->pw_dir;
  // Init with case 3)
  std::vector<std::string> locations= {homedir+"/."};
  char * gxShareDir=getenv("GXSHAREDIR");
  // Add 2)
  if (wdLocation) locations.insert(locations.begin(), wdLocation+std::string("/"));
  // Add 1)
  if (gxShareDir) locations.insert(locations.begin(), gxShareDir+std::string("/"));
  // Add 4)
  //const char * installPath = STR_NAME( GEOMODEL_INSTALL_PREFIX ); // old CMake setup
  std::string installedShare = std::string(GEOMODEL_INSTALL_PREFIX) + "/share/gmex/";
  locations.push_back(installedShare);
  // Add 5) and 6)
  locations.push_back("/usr/local/share/gmex/");
  locations.push_back("/usr/share/gmex/");


  for (const std::string & s : locations ) {
    std::string userFile=s+std::string("gmexVolVisAttributes.json");
    if (access(userFile.c_str(),R_OK)==0) {
      std::cout << "Reading volume vis attributes from file " << userFile << std::endl;
      std::ifstream in(userFile);
      auto j=json::parse(in);
      for (const json & element : j["VolVisAttributes"]) {
	GeoModelVis::Material mat;
	GeoModelVis::from_json(element, mat);
	SoMaterial *m = new SoMaterial;
	m->ambientColor.setValue(mat.ambient.R, mat.ambient.G, mat.ambient.B);
	m->diffuseColor.setValue(mat.diffuse.R, mat.diffuse.G, mat.diffuse.B);
	m->specularColor.setValue(mat.specular.R, mat.specular.G, mat.specular.B);
	m->emissiveColor.setValue(mat.emissive.R, mat.emissive.G, mat.emissive.B);
	m->shininess.setValue(mat.shininess);
	m->transparency.setValue(mat.transparency);
	for (const std::string & s  : mat.name) add(s,m);
      }
      break;
    }
  }

  init();
}
