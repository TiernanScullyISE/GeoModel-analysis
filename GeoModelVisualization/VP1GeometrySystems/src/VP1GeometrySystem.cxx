/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////////////
//                                                                     
//  Implementation of class VP1GeometrySystem                          
//                                                                     
//  Author: Thomas Kittelmann <Thomas.Kittelmann@cern.ch>              
//                                                                     
//  Derived from V-atlas geometry system by Joe Boudreau.              
//  Origins of initial version dates back to ~1996, initial VP1        
//  version by TK (May 2007) and almost entirely rewritten Oct 2007.   
//  Major refactoring october 2008.                                    
//                                                                     
//  Updates:                                                           
//  - Aug 2019, Riccardo Maria Bianchi @ CERN                          
//  - Aug 2020, Riccardo Maria Bianchi @ CERN                          
//  - Jul 2021, Riccardo Maria Bianchi <riccardo.maria.bianchi@cern.ch> 
//              * Added the 'filter volumes' tool
//              * Added signal/slot to update transparency type in the 3D window
//  - Jun 2024, Rui Xue  <r.xue@cern.ch><rux23@pitt.edu>
//              * Added methods to print out virtual surface information                 
/////////////////////////////////////////////////////////////////////////

// local includes
#include "VP1GeometrySystems/VP1GeometrySystem.h"
#include "VP1GeometrySystems/GeoSysController.h"
#include "VP1GeometrySystems/VP1GeoTreeView.h"
#include "VP1GeometrySystems/VolumeHandle.h"
#include "VP1GeometrySystems/VolumeHandleSharedData.h"
#include "VP1GeometrySystems/VolumeTreeModel.h"
#include "VP1GeometrySystems/VP1GeomUtils.h"
#include "VP1GeometrySystems/VisAttributes.h"
#include "VP1GeometrySystems/DumpShape.h"
#include "VP1GeometrySystems/PhiSectorManager.h"
#include "VP1Base/VP1CameraHelper.h"
#include "VP1Base/VP1QtInventorUtils.h"
#include "VP1Base/VP1Serialise.h"
#include "VP1Base/VP1Deserialise.h"
#include "VP1Base/VP1Msg.h"


// Coin/OpenInventor includes
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoSwitch.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoTexture2.h>
#include <Inventor/events/SoKeyboardEvent.h>
#include <Inventor/nodes/SoPickStyle.h>
#include <Inventor/nodes/SoEventCallback.h>
#include <Inventor/nodes/SoCamera.h>
#include <Inventor/nodes/SoCylinder.h>
#include <Inventor/nodes/SoFont.h>
#include <Inventor/nodes/SoText2.h>
#include <Inventor/nodes/SoTranslation.h>
#include <Inventor/nodes/SoLineSet.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoDrawStyle.h>
#include <Inventor/nodes/SoLightModel.h>
#include <Inventor/nodes/SoTransform.h>
#include <Inventor/nodes/SoScale.h>
#include <Inventor/nodes/SoSelection.h>
#include <Inventor/nodes/SoFaceSet.h>
// GeoModelCore includes
#include "GeoModelKernel/GeoVolumeCursor.h"
#include "GeoModelKernel/GeoPrintGraphAction.h"
#include "GeoModelKernel/GeoMaterial.h"
#include "GeoModelKernel/GeoElement.h"
#include "GeoModelKernel/GeoVPhysVol.h"
#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoCountVolAction.h"
#include "GeoModelKernel/GeoAccessVolumeAction.h"
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelKernel/GeoVGeometryPlugin.h"
#include "GeoModelKernel/Units.h"
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelKernel/GeoGeometryPluginLoader.h"
#include "GeoModelKernel/GeoPublisher.h"

// GeoModelIO includes
#include "GeoModelDBManager/GMDBManager.h"
#include "GeoModelWrite/WriteGeoModel.h"
#include "GeoModelRead/ReadGeoModel.h"

// WARNING: classes making use of 'Persistifier' should be included AFTER 'GeoModelIO/ReadGeoModel'
#include "GeoModelKernel/GeoShapeUnion.h"
#include "GeoModelKernel/GeoShapeShift.h"
#include "GeoModelHelpers/defineWorld.h"

// Qt includes
#include <QStack>
#include <QString>
#include <QDebug>
#include <QRegExp>
#include <QByteArray>
#include <QTimer>
#include <QHeaderView>
#include <QApplication>
#include <QCheckBox>
#include <QMessageBox>
#include <QFileInfo>
#include <QFileDialog>
#include <QPushButton>

// C++ includes
#include <map>
#include <unistd.h>
#include <stdexcept>

#define SYSTEM_OF_UNITS GeoModelKernelUnits // --> 'GeoModelKernelUnits::cm'

static QString m_existingGeoInput;



class VP1GeometrySystem::Imp {
public:
  Imp (VP1GeometrySystem*gs)
    : theclass(gs), sceneroot(0),
      detVisAttributes(0), matVisAttributes(0), volVisAttributes(0),
      controller(0),phisectormanager(0),
      volumetreemodel(0),kbEvent(0),m_textSep(0) {}


  VP1GeometrySystem * theclass;
  SoSeparator * sceneroot;

  std::map<SoSeparator*,VolumeHandle*> sonodesep2volhandle;
  //Might be needed later:  std::map<PVConstLink,VolumeHandle*> pv2volhandle;

  PVLink getGeometry();
  PVLink createTheWorld(PVConstLink world = nullptr);
  PVLink getGeometryFromLocalDB();

  SoTexture2* getDummyTexture();
  SoMaterial* getDummyMaterial();

  QString selectGeometryFile();

  class SubSystemInfo {
  public:
    // "geomodellocation" contains name of tree tops,
    // or possible a bit more complex info in case of muons.
    SubSystemInfo( QCheckBox* cb, const std::string &systemName,
		  VP1GeoFlags::SubSystemFlag the_flag,
		   const std::string& the_matname)

      : isbuilt(false), checkbox(cb),
        systemName(systemName),
        matname(the_matname), flag(the_flag), soswitch(0)
    {
    }

    //Info needed to define the system (along with the checkbox pointer in the map below):
    bool isbuilt;
    VolumeHandle::VolumeHandleList vollist;
    QCheckBox* checkbox;

    std::string systemName;      //For picking the geomodel treetops

    std::string matname; //if nonempty, use this from detvisattr instead of the top volname.
    VP1GeoFlags::SubSystemFlag flag;


    //Information needed to initialise the system:
    class TreetopInfo { public:
      TreetopInfo() {}
      PVConstLink pV;
      GeoTrf::Transform3D xf;// = av.getTransform();
      std::string volname;
    };
    std::vector<TreetopInfo> treetopinfo;

    //Switch associated with the system - it is initialised only if the system has info available:
    SoSwitch * soswitch;
  };

  QList<SubSystemInfo*> subsysInfoList;//We need to keep and ordered version also (since wildcards in regexp might match more than one subsystem info).
  void addSubSystem(const VP1GeoFlags::SubSystemFlag&, const std::string & systemName, const std::string& matname="");

  DetVisAttributes *detVisAttributes;
  MatVisAttributes *matVisAttributes;
  VolVisAttributes *volVisAttributes;
  void ensureInitVisAttributes() {
    if (!detVisAttributes) detVisAttributes = new DetVisAttributes();
    if (!matVisAttributes) matVisAttributes = new MatVisAttributes();
    if (!volVisAttributes) volVisAttributes = new VolVisAttributes();
  }


  void buildSystem(SubSystemInfo*);
  GeoSysController * controller;
  PhiSectorManager * phisectormanager;
  VolumeTreeModel * volumetreemodel;

  //Helpers used for printouts://FIXME: To VolumeHandle!!
  static double exclusiveMass(const PVConstLink& pv);
  static double inclusiveMass(const PVConstLink& pv);
  static double volume(const PVConstLink& pv);

  //Basically for the zapping, and to ignore all clicks with ctrl/shift:
  static void catchKbdState(void *userData, SoEventCallback *CB);
  const SoKeyboardEvent *kbEvent;

  void changeStateOfVisibleNonStandardVolumesRecursively(VolumeHandle*,VP1GeoFlags::VOLSTATE);
  void changeStateOfAllVolumesRecursively(VolumeHandle*,VP1GeoFlags::VOLSTATE);
  void expandVisibleVolumesRecursively(VolumeHandle*,const QRegExp&,bool bymatname);
  bool filterVolumesRec(VolumeHandle* vol, QRegExp selregexp, bool bymatname, bool stopAtFirst, bool visitChildren, bool resetView, bool &zapAll, bool &matchFound, unsigned &nFound, int maxIter = 1, unsigned int iter = 0);

  SoSeparator* m_textSep;//!< Separator used to hold all visible labels.


  QMap<quint32,QByteArray> restoredTopvolstates;
  void applyTopVolStates(const QMap<quint32,QByteArray>&, bool disablenotif = false);
  SoSwitch    *axesSwitch;
  SoScale     *axesScale;
  SoTransform *axesTransform;
};

//_____________________________________________________________________________________
VP1GeometrySystem::VP1GeometrySystem(QString name )
  : IVP13DSystemSimple(name,
		   "This system displays the geometry as defined in the GeoModel tree.",
           "Riccardo.Maria.Bianchi@cern.ch"),
    m_d(new Imp(this))
{

}


//_____________________________________________________________________________________
void VP1GeometrySystem::systemuncreate()
{

  m_d->volumetreemodel->cleanup();
  delete m_d->matVisAttributes; m_d->matVisAttributes = 0;
  delete m_d->detVisAttributes; m_d->detVisAttributes = 0;
  delete m_d->volVisAttributes; m_d->volVisAttributes = 0;

  foreach (Imp::SubSystemInfo * subsys, m_d->subsysInfoList)
    delete subsys;
  m_d->subsysInfoList.clear();

}

//_____________________________________________________________________________________
VP1GeometrySystem::~VP1GeometrySystem()
{
  delete m_d;
  m_d = 0;
}

//_____________________________________________________________________________________
void VP1GeometrySystem::setGeometrySelectable(bool b) {
   ensureBuildController();
   m_d->controller->setGeometrySelectable(b);
}

//_____________________________________________________________________________________
void VP1GeometrySystem::setZoomToVolumeOnClick(bool b) {
   ensureBuildController();
   m_d->controller->setZoomToVolumeOnClick(b);
}


//_____________________________________________________________________________________
void VP1GeometrySystem::Imp::addSubSystem(const VP1GeoFlags::SubSystemFlag& f,
					  const std::string & systemName,
					  const std::string& matname)
{
  QCheckBox * cb = controller->subSystemCheckBox(f);
  if (!cb) {
    theclass->message(("Error: Problems retrieving checkbox for subsystem "+f).c_str());
    return;
  }
  //int counter(0); // RMB: removed to stop the 'unused variable' warning
  for (auto i=subsysInfoList.begin();i!=subsysInfoList.end();i++) {
    if (systemName==(*i)->systemName) {
      //std::cout << "Very severe warning. You create duplicate systems! " << std::endl;
      //... er, just bail out.  Subsystem exists, everything fine.
      return;
    }
  }

  subsysInfoList << new SubSystemInfo(cb,systemName,f,matname);
  //FIXME: DELETE!!!
}

//_____________________________________________________________________________________
QWidget * VP1GeometrySystem::buildController()
{
  message("VP1GeometrySystem::buildController");

  m_d->controller = new GeoSysController(this);

  m_d->phisectormanager = new PhiSectorManager(m_d->controller->phiSectionWidget(),this,this);


  connect(m_d->controller,SIGNAL(showVolumeOutLinesChanged(bool)),this,SLOT(setShowVolumeOutLines(bool)));
  setShowVolumeOutLines(m_d->controller->showVolumeOutLines());
  connect(m_d->controller,SIGNAL(saveMaterialsToFile(QString,bool)),this,SLOT(saveMaterialsToFile(QString,bool)));
  connect(m_d->controller,SIGNAL(loadMaterialsFromFile(QString)),this,SLOT(loadMaterialsFromFile(QString)));

  connect(m_d->controller,SIGNAL(transparencyChanged(float)),this,SLOT(updateTransparency()));

  connect (m_d->controller,SIGNAL(volumeStateChangeRequested(VolumeHandle*,VP1GeoFlags::VOLSTATE)), this,SLOT(volumeStateChangeRequested(VolumeHandle*,VP1GeoFlags::VOLSTATE)));
  connect (m_d->controller,SIGNAL(volumeResetRequested(VolumeHandle*)), this,SLOT(volumeResetRequested(VolumeHandle*)));
 
 
  connect(m_d->controller,SIGNAL(autoExpandByVolumeOrMaterialName(bool,QString)),this,SLOT(autoExpandByVolumeOrMaterialName(bool,QString)));
  connect(m_d->controller,SIGNAL(signalFilterVolumes(QString, bool, int, bool, bool, bool)),this,SLOT(filterVolumes(QString, bool, int, bool, bool, bool)));
  
  connect(m_d->controller->requestOutputButton(), SIGNAL(clicked()), this, SLOT(saveTrees()));
  connect(m_d->controller,SIGNAL(displayLocalAxesChanged(int)), this, SLOT(toggleLocalAxes(int)));
  connect(m_d->controller,SIGNAL(axesScaleChanged(int)), this, SLOT(setAxesScale(int)));

  //Setup models/views for volume tree browser and zapped volumes list:
  m_d->volumetreemodel = new VolumeTreeModel(m_d->controller->volumeTreeBrowser());
  m_d->controller->volumeTreeBrowser()->header()->hide();
  m_d->controller->volumeTreeBrowser()->uniformRowHeights();
  m_d->controller->volumeTreeBrowser()->setModel(m_d->volumetreemodel);

  return m_d->controller;
}


//_____________________________________________________________________________________
void VP1GeometrySystem::systemcreate(StoreGateSvc*)
{
  m_d->ensureInitVisAttributes();
}

//_____________________________________________________________________________________
void VP1GeometrySystem::Imp::catchKbdState(void *address, SoEventCallback *CB) {
  VP1GeometrySystem::Imp *This=static_cast<VP1GeometrySystem::Imp*>(address);
  if (This)
    This->kbEvent = static_cast<const SoKeyboardEvent *>(CB->getEvent());
}

//_____________________________________________________________________________________
void VP1GeometrySystem::buildPermanentSceneGraph(StoreGateSvc*/*detstore*/, SoSeparator *root)
{
  m_d->sceneroot = root;


  PVConstLink world(m_d->getGeometry());
  if (!world) return;

  if (!m_d->m_textSep) {
    // FIXME!
    //    std::cout<<"Making new Text sep"<<std::endl;
    m_d->m_textSep = new SoSeparator;
    m_d->m_textSep->setName("TextSep");
    m_d->m_textSep->ref();
  }
  m_d->sceneroot->addChild(m_d->m_textSep);

  // FIXME - what if font is missing?
  SoFont *myFont = new SoFont;
  myFont->name.setValue("Arial");
  myFont->size.setValue(12.0);
  m_d->m_textSep->addChild(myFont);

  bool save = root->enableNotify(false);

  //Catch keyboard events:
  SoEventCallback *catchEvents = new SoEventCallback();
  catchEvents->addEventCallback(SoKeyboardEvent::getClassTypeId(),Imp::catchKbdState, m_d);
  root->addChild(catchEvents);

  root->addChild(m_d->controller->drawOptions());
  root->addChild(m_d->controller->pickStyle());

  {
    GeoVolumeCursor av(world);
    while (!av.atEnd()) {
      m_d->addSubSystem(av.getName(), av.getName().c_str());
      av.next();
    }
  }

  foreach (Imp::SubSystemInfo * subsys, m_d->subsysInfoList) {
    connect(subsys->checkbox,SIGNAL(toggled(bool)),this,SLOT(checkboxChanged()));
  }


  if(VP1Msg::debug()){
    qDebug() << "Looping on volumes from the input GeoModel...";
  }
  GeoVolumeCursor av(world);
  while (!av.atEnd()) {

    std::string name = av.getName();

    //Let us see if we recognize this volume:
    bool found = false;
    foreach (Imp::SubSystemInfo * subsys, m_d->subsysInfoList) {
      if (subsys->systemName==name){
	{

	  found = true;
	  //We did... now, time to extract info:
    if(av.getVolume() == nullptr) {
      THROW_EXCEPTION(" \n \n ERROR!!! \n Virtual Surface Cannot Serve As TreeTop Info \n \n");
    }
	  subsys->treetopinfo.resize(subsys->treetopinfo.size()+1);
	  subsys->treetopinfo.back().pV = av.getVolume();
	  subsys->treetopinfo.back().xf = av.getTransform();
	  subsys->treetopinfo.back().volname = av.getName();

	  //Add a switch for this system (turned off for now):
	  SoSwitch * sw = new SoSwitch();
	  //But add a separator on top of it (for caching):
	  subsys->soswitch = sw;
	  if (sw->whichChild.getValue() != SO_SWITCH_NONE)
	    sw->whichChild = SO_SWITCH_NONE;
	  SoSeparator * sep = new SoSeparator;
	  sep->addChild(sw);
	  root->addChild(sep);
	  //Enable the corresponding checkbox:
	  subsys->checkbox->setEnabled(true);
	  subsys->checkbox->setToolTip("Toggle the display of the "+subsys->checkbox->text()+" sub system");
	}
      }
    }
    av.next(); // increment volume cursor.
  }
  //Build the geometry for those (available) subsystems that starts out being turned on:
  foreach (Imp::SubSystemInfo * subsys, m_d->subsysInfoList) {
    if (!subsys->soswitch||!subsys->checkbox->isChecked())
      continue;
    m_d->buildSystem(subsys);
    //Enable in 3D view:
    if (subsys->soswitch->whichChild.getValue() != SO_SWITCH_ALL)
      subsys->soswitch->whichChild = SO_SWITCH_ALL;
    //Enable in tree browser:
    m_d->volumetreemodel->enableSubSystem(subsys->flag);
  }
  if (!m_d->restoredTopvolstates.isEmpty()) {
    m_d->applyTopVolStates(m_d->restoredTopvolstates,false);
    m_d->restoredTopvolstates.clear();
  }
  m_d->phisectormanager->updateRepresentationsOfVolsAroundZAxis();


  // Build some axes for the display of the local coordinate system.
  {
    m_d->axesSwitch = new SoSwitch();
    SoSeparator *axesSeparator = new SoSeparator;
    m_d->axesScale     = new SoScale;
    m_d->axesTransform = new SoTransform;
    m_d->axesSwitch->whichChild=SO_SWITCH_NONE ;

    SoLightModel *lModel=new SoLightModel;
    lModel->model=SoLightModel::BASE_COLOR;
    SoDrawStyle *drawStyle=new SoDrawStyle;
    drawStyle->style=SoDrawStyle::LINES;
    drawStyle->lineWidth=5;
    axesSeparator->addChild(lModel);
    axesSeparator->addChild(drawStyle);
    axesSeparator->addChild(m_d->axesTransform);
    axesSeparator->addChild(m_d->axesScale);
    axesSeparator->addChild(m_d->axesSwitch);


    std::string label[]={"X", "Y", "Z"};

    for (int i=0;i<3;i++) {
      SoSeparator *sep = new SoSeparator;
      SoCoordinate3 *coord = new SoCoordinate3;
      SoLineSet *lineSet=new SoLineSet;
      SoMaterial *material=new SoMaterial;
      coord->point.set1Value(0,SbVec3f(0,0,0));
      coord->point.set1Value(1,SbVec3f((i==0 ? 1500:0), (i==1? 1500: 0), (i==2 ? 1500: 0)));
      material->diffuseColor.setValue((i==0 ? 1:0), (i==1? 1:0), (i==2 ? 1:0));
      m_d->axesSwitch->addChild(sep);
      sep->addChild(material);
      sep->addChild(coord);
      sep->addChild(lineSet);
      SoTranslation *translation = new SoTranslation;
      translation->translation.setValue((i==0 ? 1520:0), (i==1? 1520: 0), (i==2 ? 1520: 0));
      SoFont  *font= new SoFont;
      font->size=32;

      SoText2 *text = new SoText2;
      text->string=label[i].c_str();
      sep->addChild(translation);
      sep->addChild(font);
      sep->addChild(text);
     }
    m_d->sceneroot->addChild(axesSeparator);
  }

  root->enableNotify(save);
  if (save)
    root->touch();


}

//_____________________________________________________________________________________
PVLink VP1GeometrySystem::Imp::getGeometry()
{
  return getGeometryFromLocalDB(); // for production
}

//_____________________________________________________________________________________
PVLink VP1GeometrySystem::Imp::createTheWorld(PVConstLink world)
{
  if (world == nullptr) {

    world = createGeoWorld();
  }
  return const_pointer_cast(world);
}


QString VP1GeometrySystem::Imp::selectGeometryFile() {
  std::cout << "selectGeometryFile()" << std::endl;
  QString path;
  char buffer[1024];
  char *wd=getcwd(buffer,1024);
  path = QFileDialog::getOpenFileName(nullptr, tr("Open Geometry File"),
				      wd,
				      tr("Geometry files (*.db *.so* *.dylib)"),0,QFileDialog::DontUseNativeDialog);
  return path;
}

//_____________________________________________________________________________________
PVLink VP1GeometrySystem::Imp::getGeometryFromLocalDB()
{

  QString path;
  std::cout << "m_existingGeoInput:  " << m_existingGeoInput.toStdString() << std::endl;
  if (m_existingGeoInput.size() > 0 ) {
    path = m_existingGeoInput;
  } else {

    char *pEnv=getenv("GX_GEOMETRY_FILE0");
    if (pEnv) {
      path=pEnv;
      unsetenv("GX_GEOMETRY_FILE0");
    }
    else {
      path=selectGeometryFile();
    }

    m_existingGeoInput = path;
  }



  if (path=="") return nullptr;

  PVLink world = getenv("GX_GEOMETRY_FILE1") ? createTheWorld(nullptr) : nullptr;

  int g=0;
  while (path!="") {
    // check if DB file exists. If not, return
    if (! QFileInfo(path).exists() ) {
      QMessageBox::warning(0, "Error!","Warning, geometry input does not exist. Exiting.",QMessageBox::Ok,QMessageBox::Ok);
      exit(0);
    }
    if (path.contains(".db")) {
      // open the DB
      auto db = std::make_unique<GMDBManager>(path.toStdString());
      if (!db->checkIsDBOpen()) throw std::runtime_error ("Error, database is not open ");

      /* set the GeoModel reader */
      GeoModelIO::ReadGeoModel readInGeo{std::move(db)};

      /* build the GeoModel geometry */
      PVConstLink dbPhys = readInGeo.buildGeoModel(); // builds the whole GeoModel tree in memory

      if (world) {

	//world->add(dbPhys);
	GeoVolumeCursor aV(dbPhys);

	while (!aV.atEnd()) {
	  world->add(make_intrusive<GeoNameTag>(aV.getName()));
	  world->add(make_intrusive<GeoTransform>(aV.getTransform()));
	  world->add(const_pointer_cast(aV.getVolume()));
	  aV.next();
	}

      }
      else {
	world = createTheWorld(dbPhys);
      }

    }
    else if (path.contains(".dylib") || path.contains(".so")) {

      std::cout << "path=" << path.toStdString() << std::endl;
      GeoGeometryPluginLoader loader;
      GeoVGeometryPlugin *factory=loader.load(path.toStdString());
      if (!factory) {
	QMessageBox::warning(0, "Error!","The factory could not be created (hint: probably the plugin is missing. Check that). Cannot load geometry from factory. Exiting.",QMessageBox::Ok,QMessageBox::Ok);
	exit(0);
      }

      if (!world) world=createTheWorld(nullptr);
    
      // TODO: check this with simple and more complex plugins! Also, get lists of FPV and AXF and show them or highlight them in the viewer somehow...
     
      factory->create(world);
      //factory->create(world, true); // if we want to get lists of nodes published by plugins, if any

    }

    g++;
    char *pEnv=getenv((std::string("GX_GEOMETRY_FILE")+std::to_string(g)).c_str());
    if (pEnv) {
      path=pEnv;
      unsetenv((std::string("GX_GEOMETRY_FILE")+std::to_string(g)).c_str());
    }
    else {
      path="";
    }
  }

  return world;
}

//_____________________________________________________________________________________
void VP1GeometrySystem::toggleLocalAxes(int i) {
  if (i==0) m_d->axesSwitch->whichChild=SO_SWITCH_NONE;
  else m_d->axesSwitch->whichChild=SO_SWITCH_ALL;
}

//_____________________________________________________________________________________
void VP1GeometrySystem::setAxesScale(int i) {
  double x=(i-50)/25.0;
  double scale = pow(10,x);
  m_d->axesScale->scaleFactor.setValue(scale,scale,scale);
}

//_____________________________________________________________________________________
void VP1GeometrySystem::checkboxChanged()
{
  QCheckBox * cb = static_cast<QCheckBox*>(sender());
  Imp::SubSystemInfo * subsys(0);
  foreach (Imp::SubSystemInfo * ss, m_d->subsysInfoList) {
    if (cb==ss->checkbox) {
      subsys=ss;
      break;
    }
  }
  if (!subsys) {
    message("ERROR: Unknown checkbox");
    return;
  }
  SoSwitch * sw = subsys->soswitch;
  assert(sw);
  if (cb->isChecked()) {
    SbBool save = sw->enableNotify(false);
    if (sw->getNumChildren()==0) {
      m_d->buildSystem(subsys);
      m_d->phisectormanager->updateRepresentationsOfVolsAroundZAxis();//
    }
    if (sw->whichChild.getValue() != SO_SWITCH_ALL)
      sw->whichChild = SO_SWITCH_ALL;
    sw->enableNotify(save);
    //Enable in tree browser:
    m_d->volumetreemodel->enableSubSystem(subsys->flag);
    if (save)
      sw->touch();
  } else {
    if (sw->whichChild.getValue() != SO_SWITCH_NONE)
      sw->whichChild = SO_SWITCH_NONE;
    m_d->volumetreemodel->disableSubSystem(subsys->flag);
  }
}

// NOTE: before arriving here, events are intercepted by VP1Base/SoCooperativeSelection
// and VP1Gui/GXExecutionScheduler, in this order.
//_____________________________________________________________________________________
void VP1GeometrySystem::userPickedNode(SoNode* , SoPath *pickedPath)
{

  //////////////////////////////////////////////////////////////////////////
  //  We want to find the volumehandle for the volume. To do so, we look  //
  //  for the SoSeparator identifying the actual picked shape node, and   //
  //  use it to look up the handle:                                       //
  //////////////////////////////////////////////////////////////////////////

  //Looking for the identifying "nodesep", there are three scenarios
  //for the type signatures of the final part of the path:
  //
  // 1) Most shapes:
  //    nodesep -> pickedNode (but we must pop to a group node in case of SoCylinders)
  //
  // 2) Volumes Around Z (all phi sectors enabled):
  //    nodesep -> switch -> pickedNode
  //
  // 3) Volumes Around Z (only some phi sectors enabled):
  //    nodesep -> switch -> sep -> pickedNode
  //
  // In the third scenario we also have to pop the path, in order for
  // all phi-slices of the part gets highlighted (since more than one
  // soshape node represents the volume).

	VP1Msg::messageDebug("VP1GeometrySystem::userPickedNode()");

  if (pickedPath->getNodeFromTail(0)->getTypeId()==SoCylinder::getClassTypeId())
    pickedPath->pop();

  if (pickedPath->getLength()<5) {
    message("Path too short");
    return;
  }

  SoSeparator * nodesep(0);
  
  if (pickedPath->getNodeFromTail(0)->getTypeId()==SoFaceSet::getClassTypeId()
   && pickedPath->getNodeFromTail(1)->getTypeId()==SoSeparator::getClassTypeId()
   && pickedPath->getNodeFromTail(2)->getTypeId()==SoSelection::getClassTypeId()){

    //Scenario 0: this is a virtual surface
      nodesep = static_cast<SoSeparator*>(pickedPath->getNodeFromTail(3));
      pickedPath->pop();
      pickedPath->pop();
      pickedPath->pop();
  }  
  else if (pickedPath->getNodeFromTail(1)->getTypeId()==SoSeparator::getClassTypeId()
      && pickedPath->getNodeFromTail(2)->getTypeId()==SoSwitch::getClassTypeId()
      && pickedPath->getNodeFromTail(3)->getTypeId()==SoSeparator::getClassTypeId())
  {
    //Scenario 3:
    nodesep = static_cast<SoSeparator*>(pickedPath->getNodeFromTail(3));
    pickedPath->pop();//To get highlighting of siblings also.
  }
  else if (pickedPath->getNodeFromTail(1)->getTypeId()==SoSwitch::getClassTypeId()
      && pickedPath->getNodeFromTail(2)->getTypeId()==SoSeparator::getClassTypeId())
  {
    //Scenario 2:
    nodesep = static_cast<SoSeparator*>(pickedPath->getNodeFromTail(2));
  }
  else if (pickedPath->getNodeFromTail(1)->getTypeId()==SoSeparator::getClassTypeId()) {
    //Scenario 1 (normal):
    nodesep = static_cast<SoSeparator*>(pickedPath->getNodeFromTail(1));
  }
  if (!nodesep) {
    message("Unexpected picked path");
    return;
  }
  if ( (!(nodesep)) || (m_d->sonodesep2volhandle.find(nodesep) == m_d->sonodesep2volhandle.end()) ) {
    message("Problems finding volume handle");
    return;
  }
  VolumeHandle * volhandle = m_d->sonodesep2volhandle[nodesep];
  if (!volhandle) {
    message("Found NULL volume handle");
    return;
  }

  /////////////////////////////////////////////////////////////////////////
  //  Next thing to do is to check whether volume was clicked on with a  //
  //  modifier of SHIFT/CTRL/Z. If so, we have to change the state on    //
  //  the volume handle. Otherwise, we need to print some information:   //
  /////////////////////////////////////////////////////////////////////////

  //For focus reason, and since Qt doesn't allow standard keys such as
  //'z' as modifiers, we check for keypress states using a combination
  //of the inventor and Qt way

  bool shift_isdown = (Qt::ShiftModifier & QApplication::keyboardModifiers());
  bool ctrl_isdown  = (Qt::ControlModifier & QApplication::keyboardModifiers());
  bool alt_isdown  = (Qt::AltModifier & QApplication::keyboardModifiers());

  if (ctrl_isdown && shift_isdown) {
    //Volume should be put in ZAPPED state.
    deselectAll();
    volhandle->setState(VP1GeoFlags::ZAPPED);
    message("===> Zapping Node: "+volhandle->getName());
    return;
  }

  else if (shift_isdown) {
    //Parent of volume should be put in CONTRACTED state.
    deselectAll();

    VolumeHandle * parent=volhandle->parent();
    if (parent) parent->setState(VP1GeoFlags::CONTRACTED);
    m_d->phisectormanager->updateRepresentationsOfVolsAroundZAxis();
    return;
  }


  else if (ctrl_isdown) {
    //Volume should be put in EXPANDED state if it has children.
    deselectAll();
    if (volhandle->nChildren()>0) {
      volhandle->setState(VP1GeoFlags::EXPANDED);
    }
    m_d->phisectormanager->updateRepresentationsOfVolsAroundZAxis();
    return;
  }


  else if (alt_isdown) {
    deselectAll();
#ifdef __APPLE__
    char buffer[1024];
    char *wd=getcwd(buffer,1024);

    QString path = QFileDialog::getSaveFileName(nullptr, tr("Save Geometry File"),
						wd,
						tr("Geometry files (*.db)"),0,QFileDialog::DontUseNativeDialog);
#else
    QString path = QFileDialog::getSaveFileName(nullptr, tr("Save Geometry File"),
						get_current_dir_name(),
						tr("Geometry files (*.db)"),0,QFileDialog::DontUseNativeDialog);
#endif
    if (path.isEmpty()) return;

    GMDBManager db(path.toStdString());

    // check the DB connection
    if (db.checkIsDBOpen()) {
      std::cout << "OK! Database is created: " << path.toStdString() << std::endl;
    } else {
      std::cout << "ERROR! Database could not be created! Returning..." << std::endl;
      return;
    }

    GeoModelIO::WriteGeoModel dumpGeoModelGraph(db);
    PVConstLink pV=volhandle->geoPVConstLink();
    SbMatrix sbMtx=volhandle->getGlobalTransformToVolume();
    GeoTrf::Transform3D::MatrixType  mtx;
    for (int i=0;i<4;i++) {
      for (int j=0;j<4;j++) {
	mtx(j,i)=sbMtx[i][j];
      }
    }
    GeoTrf::Transform3D tx;
    tx.matrix()=mtx;
    GeoTransform *xf=new GeoTransform(tx);
    PVLink world= createGeoWorld();
    GeoNameTag   *nameTag = new GeoNameTag(volhandle->getName().toStdString());
    world->add(nameTag);
    world->add(xf);
    GeoPhysVol *pVMutable=(GeoPhysVol *) &*pV;
    world->add(pVMutable);
    world->exec(&dumpGeoModelGraph);
    dumpGeoModelGraph.saveToDB();
    world->unref();
    return;

  }
  //////////////////////////////////////////////////////////////////
  //  Depending on settings, we are to realign the camera if the  //
  //  clicked volume is (daughter of) a muon chamber              //
  //////////////////////////////////////////////////////////////////

  bool orientedView(false);

  //////////////////////
  //  Zoom to volume  //
  //////////////////////

  if (!orientedView&&m_d->controller->zoomToVolumeOnClick()) {
    if (m_d->sceneroot&&volhandle->nodeSoSeparator()) {
      std::set<SoCamera*> cameras = getCameraList();
      std::set<SoCamera*>::iterator it,itE = cameras.end();
      for (it=cameras.begin();it!=itE;++it) {
	    VP1CameraHelper::animatedZoomToSubTree(*it,m_d->sceneroot,volhandle->nodeSoSeparator(),2.0,1.0);
      }
    }
  }


  if (m_d->controller->displayLocalAxesOnClick()) {
    SbMatrix mtx=volhandle->getGlobalTransformToVolume();
    m_d->axesSwitch->whichChild=SO_SWITCH_ALL;
    m_d->axesTransform->setMatrix(mtx);
  }


  ///////////////////////////////////
  //  Update last-select controls  //
  ///////////////////////////////////

  m_d->controller->setLastSelectedVolume(volhandle);

  /////////////////////////////////////////////////////////
  //  OK, time to print some information for the volume  //
  /////////////////////////////////////////////////////////

  message("===> Selected Node: "+volhandle->getName());
  if (m_d->controller->printInfoOnClick_Shape()) {
    if(volhandle->geoPVConstLink()){
      foreach (QString str, DumpShape::shapeToStringList(volhandle->geoPVConstLink()->getLogVol()->getShape()))
        message(str);}
    else{
      foreach (QString str, DumpShape::shapeToStringList(volhandle->geoVSConstLink()->getShape()))
        message(str);}
  }       

  if (m_d->controller->printInfoOnClick_Material()) {
      message("===> Material:");
      if(volhandle->geoPVConstLink()){
	  const GeoMaterial* mat = volhandle->geoMaterial();
	  
	  QStringList out;
	  out << VP1GeomUtils::geoMaterialToStringList(mat);
	  message("     "+out.join(" "));
	  unsigned int nEl = mat->getNumElements();
	  if (nEl) {
		  for (unsigned int i=0; i<nEl; ++i) {
			  QStringList out;
			  out << QString::number(i+1)+")"
			  	  << "fraction:" << QString::number(mat->getFraction(i)) << "-";
			  out <<  VP1GeomUtils::geoElementToStringList(mat->getElement(i));
			  message("          " + out.join(" "));
		  }
	  } else {
		  message("          (the material has no elements defined)");
	  }
      }
      else{
          message("     Virtual Surface has no material.");
      }
  }

  if ( m_d->controller->printInfoOnClick_CopyNumber() ) {
    int cn = volhandle->copyNumber();
    message("===> CopyNo : "+(cn>=0?QString::number(cn):QString(cn==-1?"Invalid":"Error reconstructing copynumber")));
  }

  if ( m_d->controller->printInfoOnClick_Transform() ) {
    float translation_x, translation_y, translation_z, rotaxis_x, rotaxis_y, rotaxis_z, rotangle_radians;
    {
      SbMatrix mtx=volhandle->getLocalTransformToVolume();
      SbVec3f    t,s,axis;
      SbRotation r,so;
      float      angle;
      mtx.getTransform(t,r,s,so); // translation, rotation, scaling, scale orientation -- in this order
      translation_x=t[0];
      translation_y=t[1];
      translation_z=t[2];
      r.getValue(axis,angle);
      rotaxis_x=axis[0];
      rotaxis_y=axis[1];
      rotaxis_z=axis[2];
      rotangle_radians=angle;
    }
    message("===> Local Translation:");
    message("        x = "+QString::number(translation_x/SYSTEM_OF_UNITS::mm)+" mm");
    message("        y = "+QString::number(translation_y/SYSTEM_OF_UNITS::mm)+" mm");
    message("        z = "+QString::number(translation_z/SYSTEM_OF_UNITS::mm)+" mm");
    message("===> Local Rotation:");
    message("        axis x = "+QString::number(rotaxis_x));
    message("        axis y = "+QString::number(rotaxis_y));
    message("        axis z = "+QString::number(rotaxis_z));
    message("        angle = "+QString::number(rotangle_radians*180.0/M_PI)+" deg");

    {
      SbMatrix mtx=volhandle->getGlobalTransformToVolume();
      SbVec3f    t,s,axis;
      SbRotation r,so;
      float      angle;
      mtx.getTransform(t,r,s,so);
      translation_x=t[0];
      translation_y=t[1];
      translation_z=t[2];
      r.getValue(axis,angle);
      rotaxis_x=axis[0];
      rotaxis_y=axis[1];
      rotaxis_z=axis[2];
      rotangle_radians=angle;
    }
    message("===> Global Translation:");
    message("        x = "+QString::number(translation_x/SYSTEM_OF_UNITS::mm)+" mm");
    message("        y = "+QString::number(translation_y/SYSTEM_OF_UNITS::mm)+" mm");
    message("        z = "+QString::number(translation_z/SYSTEM_OF_UNITS::mm)+" mm");
    message("===> Global Rotation:");
    message("        axis x = "+QString::number(rotaxis_x));
    message("        axis y = "+QString::number(rotaxis_y));
    message("        axis z = "+QString::number(rotaxis_z));
    message("        angle = "+QString::number(rotangle_radians*180.0/M_PI)+" deg");
  }

  if (m_d->controller->printInfoOnClick_Tree()) {
    message("===> Tree:");
    if(volhandle->geoPVConstLink()){
        std::ostringstream str;
        GeoPrintGraphAction pg(str);
        volhandle->geoPVConstLink()->exec(&pg);
        foreach (QString line, QString(str.str().c_str()).split("\n"))
          message("     "+line);
    }
    else{
        message("        Virtual Surface has no child node.");
    }    
  }

  if (m_d->controller->printInfoOnClick_Mass()) {
    //FIXME: Move the mass calculations to the volume handles, and let
    //the common data cache some of the volume information by
    //logVolume).
    if(volhandle->geoPVConstLink()){
        message("===> Total Mass &lt;===");
        message("Inclusive "+QString::number(Imp::inclusiveMass(volhandle->geoPVConstLink())/SYSTEM_OF_UNITS::kilogram)+" kg");
        message("Exclusive "+QString::number(Imp::exclusiveMass(volhandle->geoPVConstLink())/SYSTEM_OF_UNITS::kilogram)+" kg");
    }
    else{
        message("===> Total Mass: ");
        message("        Virtual Surface has no mass.");
    }
  }

}

//_____________________________________________________________________________________
SoTexture2* VP1GeometrySystem::Imp::getDummyTexture()
{
  SoTexture2* texture = new SoTexture2;
  // texture->diffuseColor.setValue(0.40,0.60,0.40);
  // texture->ambientColor.setValue(0.57,0.57,0.57);
  // texture->specularColor.setValue(0.27,0.27,0.27);
  // texture->shininess.setValue(.80);
  return texture;
}
//_____________________________________________________________________________________
SoMaterial* VP1GeometrySystem::Imp::getDummyMaterial()
{
  SoMaterial* material = new SoMaterial;
  material->diffuseColor.setValue(0.40,0.60,0.40);
  material->ambientColor.setValue(0.57,0.57,0.57);
  material->specularColor.setValue(0.27,0.27,0.27);
  material->shininess.setValue(.80);
  return material;
}


//_____________________________________________________________________________________
void VP1GeometrySystem::Imp::buildSystem(SubSystemInfo* si)
{

  if (!si||si->isbuilt) {
    return;
  }
  si->isbuilt = true;
  int ichild(0);

  ensureInitVisAttributes();

  assert(si->soswitch);
  SoSeparator * subsystemsep = new SoSeparator;
  phisectormanager->registerSubSystemSeparator(si->flag,subsystemsep);
  phisectormanager->largeChangesBegin();

  SbBool save = si->soswitch->enableNotify(false);

  {
    //Loop over the treetop's that we previously selected:

    std::vector<SubSystemInfo::TreetopInfo>::const_iterator it, itE = si->treetopinfo.end();
    for (it=si->treetopinfo.begin(); it!=itE; ++it)
      {
	VP1Msg::messageDebug("-- toptree vol: " + QString(it->volname.c_str()) );

	// Get the material name from the top volume and search for it
	// in the list of "default" materials for the sub-detectors:
	QString topMaterialName = QString::fromStdString(it->pV->getLogVol()->getMaterial()->getName());
	VP1Msg::messageDebug("topMaterial: " + topMaterialName);
	SoMaterial* topMaterial = detVisAttributes->get(it->volname);

	// replace with special Dummy material if user uses this
	if (topMaterialName == "Dummy") {
	  VP1Msg::messageWarning("the volume uses the 'Dummy' material!");
	  topMaterial = getDummyMaterial();
	}

	VolumeHandleSharedData* volhandle_subsysdata = new VolumeHandleSharedData(controller,
										  si->flag, 
                                          &sonodesep2volhandle, 
                                          it->pV, 
                                          phisectormanager,
										  topMaterial, 
                                          matVisAttributes, 
                                          volVisAttributes,
										  controller->zappedVolumeListModel(),
										  controller->volumeTreeBrowser(),
										  m_textSep);
	const GeoTrf::Transform3D::MatrixType & mtx=it->xf.matrix();
	SbMatrix matr(mtx(0,0),mtx(1,0),mtx(2,0),mtx(3,0),  // Beware, Eigen and SoQt have different conventions
		      mtx(0,1),mtx(1,1),mtx(2,1),mtx(3,1),  // For the matrix of homogenous transformations.
		      mtx(0,2),mtx(1,2),mtx(2,2),mtx(3,2),
		      mtx(0,3),mtx(1,3),mtx(2,3),mtx(3,3));

	VolumeHandle * vh = new VolumeHandle(volhandle_subsysdata,0,it->pV,ichild++,matr);
	si->vollist.push_back(vh);
      }
  }


  volumetreemodel->addSubSystem( si->flag, si->vollist );

  //NB: We let the destructor of volumetreemodel take care of deleting
  //our (top) volume handles, since it has to keep a list of them
  //anyway.


  //Perform auto expansion of all ether volumes (needed for muon dead material):
  VolumeHandle::VolumeHandleListItr it, itE(si->vollist.end());
  for (it = si->vollist.begin(); it!=itE; ++it){
    (*it)->expandMothersRecursivelyToNonEther();
  }


  phisectormanager->updateRepresentationsOfVolsAroundZAxis();
  phisectormanager->largeChangesEnd();
  si->soswitch->addChild(subsystemsep);
  si->soswitch->enableNotify(save);
  if (save)
    si->soswitch->touch();
  VP1Msg::messageDebug("END of VP1GeometrySystem::Imp::buildSystem() " );
}

//_____________________________________________________________________________________
double VP1GeometrySystem::Imp::exclusiveMass(const PVConstLink& pv) {
  const GeoLogVol* lv       = pv->getLogVol();
  const GeoMaterial      *material = lv->getMaterial();
  double density = material->getDensity();
  return density*volume(pv);
}


//_____________________________________________________________________________________
double VP1GeometrySystem::Imp::volume(const PVConstLink& pv) {
  const GeoLogVol        * lv       = pv->getLogVol();
  const GeoShape         *shape    = lv->getShape();
  return shape->volume();
}


//_____________________________________________________________________________________
double VP1GeometrySystem::Imp::inclusiveMass(const PVConstLink& pv) {

  const GeoLogVol*        lv       = pv->getLogVol();
  const GeoMaterial      *material = lv->getMaterial();
  double density = material->getDensity();

  double mass = exclusiveMass(pv);

  GeoVolumeCursor av(pv);
  while (!av.atEnd()) {
    if(av.getVolume()){
      std::string materialName=av.getVolume()->getLogVol()->getMaterial()->getName();    
      if (QString(materialName.c_str()).endsWith("Ether") || QString(materialName.c_str()).endsWith("HyperUranium")) {
        // Do nothing.  These are not real volumes. 
      }
      else {
        double delta =  inclusiveMass(av.getVolume()) -  (volume(av.getVolume())*density);
        mass += delta;
      }
    }    
    av.next();
  }
  return mass;
}

//_____________________________________________________________________________________
QByteArray VP1GeometrySystem::saveState() {

  ensureBuildController();

  VP1Serialise serialise(7/*version*/,this);
  serialise.save(IVP13DSystemSimple::saveState());

  //Controller:
  serialise.save(m_d->controller->saveSettings());

  //Subsystem checkboxes:
  QMap<QString,bool> subsysstate;
  foreach (Imp::SubSystemInfo * subsys, m_d->subsysInfoList) {
    serialise.widgetHandled(subsys->checkbox);
    subsysstate.insert(subsys->checkbox->text(),subsys->checkbox->isChecked());
  }
  serialise.save(subsysstate);

  //Volume states:
  QMap<quint32,QByteArray> topvolstates;
  foreach (Imp::SubSystemInfo * subsys, m_d->subsysInfoList) {
    VolumeHandle::VolumeHandleListItr it(subsys->vollist.begin()),itE(subsys->vollist.end());
    for (;it!=itE;++it)
      topvolstates.insert((*it)->hashID(),(*it)->getPersistifiableState());
  }
  serialise.save(topvolstates);

  m_d->ensureInitVisAttributes();
  serialise.save(m_d->detVisAttributes->getState());//version 7+
  serialise.save(m_d->matVisAttributes->getState());//version 7+
  serialise.save(m_d->volVisAttributes->getState());//version 7+

  serialise.disableUnsavedChecks();//We do the testing in the controller
  return serialise.result();
}

//_____________________________________________________________________________________
void VP1GeometrySystem::restoreFromState(QByteArray ba) {
  VP1Deserialise state(ba,this);
  if (state.version()<0||state.version()>7) {
    message("Warning: State data in .vp1 file is in wrong format - ignoring!");
    return;
  }
  if (state.version()<=5) {
    message("Warning: State data in .vp1 file is in obsolete format - ignoring!");
    return;
  }

  ensureBuildController();
  IVP13DSystemSimple::restoreFromState(state.restoreByteArray());

  //Controller:
  m_d->controller->restoreSettings(state.restoreByteArray());

  //Subsystem checkboxes:
  VP1GeoFlags::SubSystemFlags flags(0);
  QMap<QString,bool> subsysstate = state.restore<QMap<QString,bool> >();
  foreach (Imp::SubSystemInfo * subsys, m_d->subsysInfoList) {
    state.widgetHandled(subsys->checkbox);
    if (subsysstate.contains(subsys->checkbox->text())&&subsysstate[subsys->checkbox->text()])
      flags.push_back(subsys->flag);
  }

  //Volume states:
  QMap<quint32,QByteArray> topvolstates;
  topvolstates = state.restore<QMap<quint32,QByteArray> >();
  if (m_d->sceneroot)//(for some reason) we already have been in buildPermanentScenegraph
    m_d->applyTopVolStates(topvolstates,true);
  else
    m_d->restoredTopvolstates = topvolstates;//save until buildPermanentScenegraph

  if (state.version()>=7) {
    m_d->ensureInitVisAttributes();
    m_d->detVisAttributes->applyState(state.restoreByteArray());
    m_d->matVisAttributes->applyState(state.restoreByteArray());
    m_d->volVisAttributes->applyState(state.restoreByteArray());
  }

  state.disableUnrestoredChecks();//We do the testing in the controller

}

//_____________________________________________________________________________________
void VP1GeometrySystem::Imp::applyTopVolStates(const QMap<quint32,QByteArray>&topvolstates,bool disablenotif)
{
  if (disablenotif)
    phisectormanager->largeChangesBegin();
  QMap<quint32,QByteArray>::const_iterator topvolstatesItr;
  foreach (Imp::SubSystemInfo * subsys, subsysInfoList) {
    VolumeHandle::VolumeHandleListItr it(subsys->vollist.begin()),itE(subsys->vollist.end());
    for (;it!=itE;++it) {
      topvolstatesItr = topvolstates.find((*it)->hashID());
      if (topvolstatesItr!=topvolstates.end())
	(*it)->applyPersistifiableState(topvolstatesItr.value());
    }
  }
  if (disablenotif) {
    phisectormanager->updateRepresentationsOfVolsAroundZAxis();
    phisectormanager->largeChangesEnd();
  }
}

//_____________________________________________________________________________________
void VP1GeometrySystem::setCurvedSurfaceRealism(int val)
{
   ensureBuildController();
   if (val<0||val>100) {
     message("setCurvedSurfaceRealism Error: Value "+str(val)+"out of range!");
     return;
   }
   m_d->controller->setComplexity(val==100?1.0:(val==0?0.0:val/100.0));
}



//_____________________________________________________________________________________
void VP1GeometrySystem::updateTransparency()
{
  ensureBuildController();

  float transparency = m_d->controller->transparency();

  VolumeHandle* lastSelVol = m_d->controller->lastSelectedVolume();
  m_d->controller->setLastSelectedVolume(0);
  m_d->ensureInitVisAttributes();
  m_d->detVisAttributes->overrideTransparencies(transparency);
  m_d->matVisAttributes->overrideTransparencies(transparency);
  m_d->volVisAttributes->overrideTransparencies(transparency);
  m_d->controller->setLastSelectedVolume(lastSelVol);
}

//_____________________________________________________________________________________
void VP1GeometrySystem::resetSubSystems(VP1GeoFlags::SubSystemFlags f)
{
  if (f.empty()) {
    return;
  }

  deselectAll();
  foreach(Imp::SubSystemInfo*si,m_d->subsysInfoList) {
    if (std::find(f.begin(),f.end(),si->flag)!=f.end()) {
        if (!si->isbuilt) {
	        continue;
        }
        VolumeHandle::VolumeHandleListItr it(si->vollist.begin()),itE(si->vollist.end());
        for (;it!=itE;++it) {
            messageDebug("resetting volume --> " + (*it)->getName() );
	        (*it)->reset();
        }
    }
  }
}

//_____________________________________________________________________________________
void VP1GeometrySystem::autoExpandByVolumeOrMaterialName(bool bymatname,QString targetname)
{
  if (targetname.isEmpty()) {
	  VP1Msg::messageDebug("targetname is empty.");
    return;
  }

    messageVerbose("Auto expansions of **visible** volumes requested. Target all volumes with "
		 +str(bymatname?"material name":"name")+" matching "+targetname);

  //QRegExp selregexp(targetname,Qt::CaseSensitive,QRegExp::Wildcard);
  QRegExp selregexp(targetname, Qt::CaseSensitive, QRegExp::RegExp);
  VP1Msg::messageDebug("RegExp pattern: " + selregexp.pattern() );

  std::vector<std::pair<VolumeHandle::VolumeHandleListItr,VolumeHandle::VolumeHandleListItr> > roothandles;
  m_d->volumetreemodel->getRootHandles(roothandles);
  VolumeHandle::VolumeHandleListItr it, itE;

  bool save = m_d->sceneroot->enableNotify(false);
  m_d->phisectormanager->largeChangesBegin();

  if (roothandles.size() == 0) {
    VP1Msg::messageWarningRed("No root nodes selected! Please select at least a root node before trying to apply filters. Ideally, you should turn on only the root node containing the volumes you are interested in; e.g., turn on the 'Endcap MDT' root node, if you are looking for an EIL chamber.");
  } else {
    bool save = m_d->sceneroot->enableNotify(false);
    m_d->phisectormanager->largeChangesBegin();

    deselectAll();

    for (unsigned i = 0; i<roothandles.size();++i) {
      it = roothandles.at(i).first;
      itE = roothandles.at(i).second;
      bool matchFound = false;

      for(;it!=itE;++it) {
        VolumeHandle* vol = *it;
        VP1Msg::messageDebug("Looking inside the root node [name: " + vol->getName() + ", material: " + QString::fromStdString(vol->geoMaterial()->getName()) + "]" );
      	  m_d->expandVisibleVolumesRecursively(vol, selregexp, bymatname);
      }
    }

    m_d->phisectormanager->updateRepresentationsOfVolsAroundZAxis();
    m_d->phisectormanager->largeChangesEnd();
    if (save) {
      m_d->sceneroot->enableNotify(true);
      m_d->sceneroot->touch();
    }
  }
}

//_____________________________________________________________________________________
void VP1GeometrySystem::Imp::expandVisibleVolumesRecursively(VolumeHandle* handle,const QRegExp& selregexp,bool bymatname)
{
  //if (handle->state()==VP1GeoFlags::ZAPPED)
  //  return;
  VP1Msg::messageDebug("VP1GeometrySystem::Imp::expandVisibleVolumesRecursively()");
  if (handle->state()==VP1GeoFlags::ZAPPED) {
	VP1Msg::messageDebug(handle->getName() +" is ZAPPED. Returning...");
    return;
  }

  if (handle->state()==VP1GeoFlags::CONTRACTED) {
    VP1Msg::messageDebug(handle->getName() +" is CONTRACTED. Going on...");
    //See if we match (and have children) - if so, update state.
    if (handle->nChildren()>0
	&& selregexp.exactMatch(bymatname?QString(handle->geoMaterial()->getName().c_str()):handle->getName())) 
    {
      VP1Msg::messageDebug(handle->getName() +" has >0 children: OK! Setting it to 'EXPANDED'.");
      handle->setState(VP1GeoFlags::EXPANDED);
    }
    VP1Msg::messageDebug(handle->getName() +" - Now returning...");
    return;
  }
  
  //Must be expanded: Let us call on any (initialised) children instead.
  if (handle->nChildren()==0||!handle->childrenAreInitialised()) {
    VP1Msg::messageDebug(handle->getName() +" has NO children or they are not initialized. Returning...");
    return;
  }

  //TODO: does the code comes here ever??? Check!
  VP1Msg::messageDebug(handle->getName() +" - Now looping over children...");
  VolumeHandle::VolumeHandleListItr it(handle->childrenBegin()), itE(handle->childrenEnd());
  for(;it!=itE;++it) {
    expandVisibleVolumesRecursively(*it,selregexp,bymatname);
  }
}


//_____________________________________________________________________________________
void VP1GeometrySystem::volumeStateChangeRequested(VolumeHandle*vh,VP1GeoFlags::VOLSTATE state)
{
  //might not use this slot presently...
  if (!vh)
    return;
  deselectAll();
  vh->setState(state);
  m_d->phisectormanager->updateRepresentationsOfVolsAroundZAxis();
}


//_____________________________________________________________________________________
void VP1GeometrySystem::volumeResetRequested(VolumeHandle*vh)
{
  if (!vh)
    return;
  deselectAll();
  vh->reset();
  m_d->phisectormanager->updateRepresentationsOfVolsAroundZAxis();
}


//_____________________________________________________________________________________
void VP1GeometrySystem::setShowVolumeOutLines(bool b)
{
  std::map<SoSeparator*,VolumeHandle*>::iterator it,itE(m_d->sonodesep2volhandle.end());
  for (it =m_d->sonodesep2volhandle.begin();it!=itE;++it)
    VolumeHandleSharedData::setShowVolumeOutlines(it->first,b);
}


//_____________________________________________________________________________________
void VP1GeometrySystem::saveMaterialsToFile(QString filename,bool onlyChangedMaterials)
{
  if (filename.isEmpty())
    return;

  //If file exists, ask to overwrite.
  QFileInfo fi(filename);
  if (fi.exists()) {
    if (!fi.isWritable()) {
      QMessageBox::critical(0, "Error - could not save to file "+filename,
			    "Could not save to file: <i>"+filename+"</i>"
			    +"<br/><br/>Reason: File exists already and is write protected",QMessageBox::Ok,QMessageBox::Ok);
      return;
    }
  }

  QFile file(filename);
  if (!file.open(QIODevice::WriteOnly)) {
      QMessageBox::critical(0, "Error - problems writing to file "+filename,
			    "Problems writing to file: <i>"+filename+"</i>",QMessageBox::Ok,QMessageBox::Ok);
      return;
  }

  m_d->ensureInitVisAttributes();
  VP1Serialise s(0/*version*/,this);
  //Save some file ID info!!
  s.save(QString("VP1GeoMaterialsBegin"));
  s.save(m_d->detVisAttributes->getState(onlyChangedMaterials));
  s.save(m_d->matVisAttributes->getState(onlyChangedMaterials));
  s.save(m_d->volVisAttributes->getState(onlyChangedMaterials));
  s.save(QString("VP1GeoMaterialsEnd"));
  s.disableUnsavedChecks();

  QDataStream outfile(&file);
  outfile<<qCompress(s.result()).toBase64();

}

//_____________________________________________________________________________________
void VP1GeometrySystem::loadMaterialsFromFile(QString filename)
{
  if (filename.isEmpty())
    return;
  QFileInfo fi(filename);
  if (!fi.exists()) {
    QMessageBox::critical(0, "Error - file does not exists: "+filename,
			  "File does not exists: <i>"+filename+"</i>",QMessageBox::Ok,QMessageBox::Ok);
    return;
  }
  if (!fi.isReadable()) {
    QMessageBox::critical(0, "Error - file is not readable: "+filename,
			  "File is not readable: <i>"+filename+"</i>",QMessageBox::Ok,QMessageBox::Ok);
    return;
  }
  //open file
  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly)) {
      QMessageBox::critical(0, "Error - problems opening file "+filename,
			    "Problems opening file: <i>"+filename+"</i>",QMessageBox::Ok,QMessageBox::Ok);
      return;
  }
  QByteArray byteArray64;
  QDataStream infile(&file);
  infile >> byteArray64;
  QByteArray byteArray = qUncompress(QByteArray::fromBase64(byteArray64));

  VP1Deserialise s(byteArray,this);
  if (s.version()!=0) {
    QMessageBox::critical(0, "Error - File in wrong format "+filename,
			  "File in wrong format: <i>"+filename+"</i>",QMessageBox::Ok,QMessageBox::Ok);
    return;
  }
  QString txtbegin, txtend;
  QByteArray baDet, baMat, baVol;
  txtbegin = s.restoreString();
  baDet = s.restoreByteArray();
  baMat = s.restoreByteArray();
  baVol = s.restoreByteArray();
  txtend = s.restoreString();
  s.disableUnrestoredChecks();
  if (txtbegin!="VP1GeoMaterialsBegin"||txtend!="VP1GeoMaterialsEnd") {
    QMessageBox::critical(0, "Error - File in wrong format "+filename,
			  "File in wrong format: <i>"+filename+"</i>",QMessageBox::Ok,QMessageBox::Ok);
    return;
  }

  m_d->ensureInitVisAttributes();
  m_d->detVisAttributes->applyState(baDet);
  m_d->matVisAttributes->applyState(baMat);
  m_d->volVisAttributes->applyState(baVol);

  VolumeHandle* lastsel = m_d->controller->lastSelectedVolume();
  m_d->controller->setLastSelectedVolume(0);
  m_d->controller->setLastSelectedVolume(lastsel);
}



//_____________________________________________________________________________________
void VP1GeometrySystem::saveTrees() {
#ifdef __APPLE__
  char buffer[1024];
  char *wd=getcwd(buffer,1024);

  QString path = QFileDialog::getSaveFileName(nullptr, tr("Save Geometry File"),
					      wd,
					      tr("Geometry files (*.db)"),0,QFileDialog::DontUseNativeDialog);
#else
  QString path = QFileDialog::getSaveFileName(nullptr, tr("Save Geometry File"),
					      get_current_dir_name(),
					      tr("Geometry files (*.db)"),0,QFileDialog::DontUseNativeDialog);
#endif
  if (path.isEmpty()) return;

  GMDBManager db(path.toStdString());

  // check the DB connection
  if (db.checkIsDBOpen())
    std::cout << "OK! Local GeoModel database has been created: " << path.toStdString() << std::endl;
  else {
    std::cout << "GeoModel database ERROR!! Exiting...";
    return;
  }
  GeoModelIO::WriteGeoModel dumpGeoModelGraph(db);


  PVLink world=createGeoWorld();

  foreach (Imp::SubSystemInfo * subsys, m_d->subsysInfoList) {
    if (subsys->checkbox->isChecked()){
      std::vector<Imp::SubSystemInfo::TreetopInfo> & ttInfo=subsys->treetopinfo;
      foreach (const Imp::SubSystemInfo::TreetopInfo & treeTop, ttInfo) {

	GeoNameTag   *nameTag = new GeoNameTag(treeTop.volname);
	GeoTransform *transform=new GeoTransform(treeTop.xf);
	world->add(transform);
	world->add(nameTag);
	GeoPhysVol *pV=(GeoPhysVol *) &*treeTop.pV;
	world->add(pV);
      }
    }
  }
  world->exec(&dumpGeoModelGraph);
  dumpGeoModelGraph.saveToDB();
  //  world->unref();
}

//_____________________________________________________________________________________

//_____________________________________________________________________________________
void VP1GeometrySystem::filterVolumes(QString targetname, bool bymatname, int maxDepth = 1, bool stopAtFirst = true, bool visitChildren = false, bool resetView = false)
{

  // initialize the regular expression
  QRegExp selregexp(targetname, Qt::CaseSensitive, QRegExp::RegExp);
  // VP1Msg::messageDebug("RegExp pattern: " + selregexp.pattern()  );
  
  QStringList ll;
  ll << "VP1GeometrySystem::filterVolumes" << "RegExp pattern:" << selregexp.pattern() 
     << "- maxDepth:" << QString::number(maxDepth)
     << "- stopAtFirst:" << QString::number(stopAtFirst) 
     << "- visitChildren:" << QString::number(visitChildren) 
     << "- resetView:" << QString::number(resetView);
  VP1Msg::messageDebug(ll.join(" "));

   
   // get root handles list
  std::vector<std::pair<VolumeHandle::VolumeHandleListItr,VolumeHandle::VolumeHandleListItr> > roothandles;
  m_d->volumetreemodel->getRootHandles(roothandles);
  VolumeHandle::VolumeHandleListItr it, itE;
  
  bool save = m_d->sceneroot->enableNotify(false);
  m_d->phisectormanager->largeChangesBegin();
  deselectAll();

  bool zapAll = false; // TODO: do we need that?
  bool matchFound = false;
  unsigned nFound = 0;
  
  // loop over root handles
  for (unsigned i = 0; i<roothandles.size();++i) {
    it = roothandles.at(i).first;
    itE = roothandles.at(i).second;

    // loop over root volumes
    for(;it!=itE;++it) {

        VolumeHandle* handle = (*it);

        VP1Msg::messageDebug("Looking at the root node [name: " + handle->getName() + ", mat: " + QString::fromStdString(handle->geoMaterial()->getName()) + "]" );
        m_d->filterVolumesRec(handle, selregexp, bymatname, stopAtFirst, visitChildren, resetView, zapAll, matchFound, nFound, maxDepth);
        VP1Msg::messageDebug("matchFound: " + QString::number(matchFound));
        
        if (resetView) {
            VP1Msg::messageDebug("'expanding' and 'contracting' the root volume...");
            // in the call to 'filterVolumesRec' above, when resetting the view, 
            // we have 'contracted' the root volume and all the children we had 'zapped' before.
            // So, now we need to open ('expand'), and then close ('contract') the root volume again,
            // in order to get rid of the manually-contracted volumes.
            handle->setState(VP1GeoFlags::EXPANDED); 
            handle->setState(VP1GeoFlags::CONTRACTED); 
        }

    } // end loop over root volumes
  } // loop over root handles

  // give feedback to the user
  message("[filter volumes] # of matching volumes: " + QString::number(nFound));
  
  //if user chose to visit children of matching volumes,
  //then we change transparency type to 'Sorted Object Blend', usually better to visualize nested volumes
  if (visitChildren && (nFound>0) ) {
      message(  QString("[filter volumes] NOTE: to show both matching mother and matching daughter volumes, ") 
              + QString("transparency has been set to 50% and transparency type has been changed to 'Sorted Object Blend', ")
              + QString("which is usually better to visualize nested geometry volumes. ")
              + QString("You can disable this auto-setting by checking the 'lock'.") );
      if ( ! m_d->controller->isTranspLocked()) {
          m_d->controller->setTransparency(50);
          emit updateTransparencyType( VP1QtInventorUtils::transparencyTypeToInt(SoGLRenderAction::SORTED_OBJECT_BLEND) );
      }
  }


  VP1Msg::messageDebug("largeChangesEnding...");
  m_d->phisectormanager->updateRepresentationsOfVolsAroundZAxis();
  m_d->phisectormanager->largeChangesEnd();
  if (save) {
    m_d->sceneroot->enableNotify(true);
    m_d->sceneroot->touch();
  }

}
//_____________________________________________________________________________________
bool VP1GeometrySystem::Imp::filterVolumesRec(VolumeHandle* handle, QRegExp selregexp, bool bymatname, bool stopAtFirst, bool visitChildren, bool resetView, bool &zapAll, bool &matchFound, unsigned &nFound, int maxIter, unsigned int iter)
{
      if (!zapAll) {
        VP1Msg::messageDebug2("iteration: " + QString::number(iter) + ", maxIter: " + QString::number(maxIter));
        VP1Msg::messageDebug2("looking into volume: " + handle->getName());
      }
      
      // zap the current volume by default...
      handle->setState(VP1GeoFlags::ZAPPED);

      VP1Msg::messageDebug2("filtering: " + handle->getName() );

      if ( !zapAll ) {
        if ( !resetView ) {
          //... then unzap the volume if it matches the filter regex
          if (selregexp.exactMatch( bymatname ? QString(handle->geoMaterial()->getName().c_str()) : handle->getName()) ) 
          {
            matchFound = true;
            ++nFound;
            VP1Msg::messageDebug(handle->getName() +" - **MATCH!** - 'Contracting' it (-->make it visible)...");
            handle->setState(VP1GeoFlags::CONTRACTED); // match, make the matching volume visible
            // if a matching volume was found and 'stop at first' option was choosen, then stop here
            //VP1Msg::messageDebug2("matchFound: " + QString::number(matchFound) + " - stopAtFirst: " + QString::number(stopAtFirst));
            
            if (stopAtFirst && matchFound) {
              VP1Msg::messageDebug("\tYou chose to show only the first matching volume, so we stop here - exiting from the inner children loop...");
              //return matchFound;
              zapAll = true;
            } 
          } else {
            VP1Msg::messageDebug2("not matching --> zapping it");
            handle->setState(VP1GeoFlags::ZAPPED); // no match, open the volume to show its children
          }
        }
        if (resetView) {
          if(handle->state()!=VP1GeoFlags::CONTRACTED) {
              VP1Msg::messageDebug2("resetView --> contracting it");
              handle->setState(VP1GeoFlags::CONTRACTED); // open the volume to show its children
          }
        }
      } // end if !zapAll


      // check iterations, a.k.a., the 'maxDepth', i.e., the number of layers of daughter volumes visited 
      if( maxIter != -1 && iter == maxIter) {
          VP1Msg::messageDebug("maxIter [" + QString::number(iter) + "] reached, returning from the recursive method and go to the next upper volume...");
          return matchFound;
      }
       // increment the iteration number
      iter++;
      
      // if no match yet, 
      // or if user asked to visit children of matching volumes too, 
      // then loop over children
      if ( !matchFound || (matchFound && visitChildren) ) {
          if (!zapAll) {
              VP1Msg::messageDebug2("No match, or you chose to inspect all child volumes, so we look into the children...");
          }
          if (handle->nChildren()>0) {
              // get children list
              handle->initialiseChildren();
              VolumeHandle::VolumeHandleListItr itChl(handle->childrenBegin()),itChlE(handle->childrenEnd());
              for (;itChl!=itChlE;++itChl) {

                  VolumeHandle* child = (*itChl);

                  // filter children volumes recursively
                  filterVolumesRec(child, selregexp, bymatname, stopAtFirst, visitChildren, resetView, zapAll, matchFound, nFound, maxIter, iter);
                  if (stopAtFirst && matchFound) {
                      //return matchFound;
                      zapAll = true;
                  }
              } // end loop over children
          
              if (resetView) {
                 // in the call to 'filterVolumesRec' above, when resetting the view, 
                 // we have 'contracted' the root volume and all the children we had 'zapped' before.
                 // So, now we need to open ('expand'), and then close ('contract') the root volume again,
                 // in order to get rid of the manually-contracted volumes.
                 handle->setState(VP1GeoFlags::EXPANDED); 
                 handle->setState(VP1GeoFlags::CONTRACTED); 
              }
          }
      }

      return matchFound;
}











// =========================

////////
// Methods which come from VP1 and have been rplaced by other methods, 
// but they are useful for later use,
// when we try to act on all volumes
////////

// Not used at the moment, but useful
//_____________________________________________________________________________________
void VP1GeometrySystem::Imp::changeStateOfAllVolumesRecursively(VolumeHandle*handle,VP1GeoFlags::VOLSTATE target)
{
    handle->initialiseChildren();
    handle->setState(target);
    VolumeHandle::VolumeHandleListItr itChl(handle->childrenBegin()),itChlE(handle->childrenEnd());

    // loop over second level children 
    for (;itChl!=itChlE;++itChl) {
        changeStateOfAllVolumesRecursively(*itChl, target);
    }
}


// Not used at the moment, but useful
//_____________________________________________________________________________________
void VP1GeometrySystem::Imp::changeStateOfVisibleNonStandardVolumesRecursively(VolumeHandle*handle,VP1GeoFlags::VOLSTATE target)
{
  assert(target!=VP1GeoFlags::CONTRACTED);
  if (handle->isAttached()) {
    //The volume is visible, so ignore daughters
    if (handle->isInitialisedAndHasNonStandardShape()) {
      if (target!=VP1GeoFlags::EXPANDED||handle->nChildren()>0)
	handle->setState(target);
    }
    return;
  } else if (handle->state()==VP1GeoFlags::ZAPPED)
    return;
  //Must be expanded: Let us call on any (initialised) children instead.
  if (handle->nChildren()==0||!handle->childrenAreInitialised())
    return;
  VolumeHandle::VolumeHandleListItr it(handle->childrenBegin()), itE(handle->childrenEnd());
  for(;it!=itE;++it)
    changeStateOfVisibleNonStandardVolumesRecursively(*it, target);
}

// Not used at the moment, but useful
//_____________________________________________________________________________________
void VP1GeometrySystem::actionOnAllVolumes(bool zap, bool standardVolumes /* default: true*/)
{
  //VP1GeoFlags::VOLSTATE target = zap ? VP1GeoFlags::ZAPPED : VP1GeoFlags::EXPANDED;
  VP1GeoFlags::VOLSTATE target = zap ? VP1GeoFlags::ZAPPED : VP1GeoFlags::CONTRACTED;
  messageVerbose("Action on volumes with non-standard VRML representations. Target state is "+VP1GeoFlags::toString(target));

  std::vector<std::pair<VolumeHandle::VolumeHandleListItr,VolumeHandle::VolumeHandleListItr> > roothandles;
  m_d->volumetreemodel->getRootHandles(roothandles);
  VolumeHandle::VolumeHandleListItr it, itE;

  bool save = m_d->sceneroot->enableNotify(false);
  m_d->phisectormanager->largeChangesBegin();

  deselectAll();

  for (unsigned i = 0; i<roothandles.size();++i) {
    it = roothandles.at(i).first;
    itE = roothandles.at(i).second;
    for(;it!=itE;++it) {
      if (standardVolumes) {
        m_d->changeStateOfAllVolumesRecursively(*it, target);
      } else {
        m_d->changeStateOfVisibleNonStandardVolumesRecursively(*it, target);
      }
    }
  }

  m_d->phisectormanager->updateRepresentationsOfVolsAroundZAxis();
  m_d->phisectormanager->largeChangesEnd();
  if (save) {
    m_d->sceneroot->enableNotify(true);
    m_d->sceneroot->touch();
  }
}


