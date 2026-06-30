/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/


////////////////////////////////////////////////////////////////
//                                                            
//  Header file for class GeoSysController                    
//                                                            
//  Description: Controller for the geometry system.          
//                                                            
//  Author: Thomas H. Kittelmann (Thomas.Kittelmann@cern.ch)  
//  Initial version: October 2008                             
//
//  Major updates:
//   
//  - Riccardo Maria BIANCHI (riccardo.maria.bianchi@cern.ch)  
//    Jul 2021: Added support to filter volumes based on names
//
////////////////////////////////////////////////////////////////

#ifndef GEOSYSCONTROLLER_H
#define GEOSYSCONTROLLER_H

#include "VP1Base/VP1Controller.h"
#include "VP1GeometrySystems/VP1GeoFlags.h"
class VolumeHandle;
class SoGroup;
class SoPickStyle;
class VP1GeoTreeView;
class PhiSectionWidget;
class ZappedVolumeListModel;

class GeoSysController : public VP1Controller {

  Q_OBJECT

public:

  GeoSysController(IVP1System * sys);
  virtual ~GeoSysController();

  //Change defaults (call right after constructor):
  void setGeometrySelectable(bool);
  void setZoomToVolumeOnClick(bool);
  void setComplexity(double);

  void actualRestoreSettings(VP1Deserialise&);
  int  currentSettingsVersion() const;
  void actualSaveSettings(VP1Serialise&) const;

  void setLastSelectedVolume(VolumeHandle*);
  VolumeHandle* lastSelectedVolume() const;

  QCheckBox * subSystemCheckBox(VP1GeoFlags::SubSystemFlag) const;//Checkbox will be disabled
                                                                        //if corresponding geometry
                                                                        //is not present in job.

  SoGroup * drawOptions() const;
  SoPickStyle * pickStyle() const;
  VP1GeoTreeView * volumeTreeBrowser() const;
  QPushButton * requestOutputButton () const;
  QPushButton * enableAllSubSystemsButton() const;
  PhiSectionWidget * phiSectionWidget() const;
  ZappedVolumeListModel * zappedVolumeListModel() const;

  ///////////////////////////////////
  //  Access Methods for settings  //
  ///////////////////////////////////

  //Those only queried on-demand:
  bool printInfoOnClick_Shape() const;
  bool printInfoOnClick_Material() const;
  bool printInfoOnClick_CopyNumber() const;
  bool printInfoOnClick_Transform() const;
  bool printInfoOnClick_Tree() const;
  bool printInfoOnClick_Mass() const;
  bool displayLocalAxesOnClick() const;
  bool zoomToVolumeOnClick() const;

  //Settings with change signals:

  float transparency() const;
  void setTransparency(float value) const;
  bool isTranspLocked() const;

  bool showVolumeOutLines() const;
  int labels() const;
  QList<int> labelPosOffset() ; //!< Offset in x,y,z

  ///////////////////////////////////////
  //  Signals for changes in settings  //
  ///////////////////////////////////////
signals:

  //Change signals:
  void transparencyChanged(float);
  void showVolumeOutLinesChanged(bool);

  //Signals without state:
  void resetSubSystems(VP1GeoFlags::SubSystemFlag);
  void autoExpandByVolumeOrMaterialName(bool, QString); //volname: (false,namestr), matname: (true,namestr)
  void volumeStateChangeRequested(VolumeHandle*,VP1GeoFlags::VOLSTATE);//Might not be used atm.
  void volumeResetRequested(VolumeHandle*);
  void signalFilterVolumes(QString, bool, int, bool, bool, bool);
  void saveMaterialsToFile(QString,bool);//(filename,onlyChangedMaterials)
  void loadMaterialsFromFile(QString);//filename
  void displayLocalAxesChanged(int);
  void axesScaleChanged(int);

  
private:

  class Imp;
  Imp * m_d;

private slots:
  void possibleChange_transparency();
  void possibleChange_showVolumeOutLines();
  void updatePickStyle();
  void saveMaterialsRequested();
  void loadMaterialsRequested();
  void emit_autoExpandByVolumeOrMaterialName();
};

#endif
