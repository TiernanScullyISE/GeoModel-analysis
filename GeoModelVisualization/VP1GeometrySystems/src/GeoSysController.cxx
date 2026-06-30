/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/


////////////////////////////////////////////////////////////////
//                                                           
//  Implementation of class GeoSysController                  
//                                                            
//  Author: Thomas H. Kittelmann (Thomas.Kittelmann@cern.ch)  
//  Initial version: October 2008
//  
//  Major updates:
// 
//  - Riccardo Maria BIANCHI (riccardo.maria.bianchi@cern.ch)  
//    2019
//
//  - Riccardo Maria BIANCHI (riccardo.maria.bianchi@cern.ch)  
//    Jul 2021: Added support to filter volumes based on names
//                                                            
////////////////////////////////////////////////////////////////

#define VP1IMPVARNAME m_d

#include "VP1GeometrySystems/GeoSysController.h"
#include "VP1GeometrySystems/ZappedVolumeListModel.h"
#include "VP1Base/VP1Serialise.h"
#include "VP1Base/VP1Deserialise.h"
#include "VP1Base/VP1Settings.h"
#include "VP1Base/IVP1System.h"
#include "ui_geometrysystemcontroller.h"
#include "ui_settings_display_form.h"
#include "ui_settings_iconisedvols_form.h"
#include "ui_settings_interactions_form.h"
#include "ui_settings_misc_form.h"
#include "ui_settings_treebrowser_form.h"

#include <Inventor/nodes/SoPickStyle.h>

#include <QFileDialog>

#include <map>


//____________________________________________________________________
class GeoSysController::Imp {
public:
  GeoSysController * theclass;
  Ui::GeometrySystemControllerForm ui;
  Ui::GeoSysSettingsDisplayForm ui_disp;
  Ui::GeoSysSettingsIconisedVolumesForm ui_iconisedvols;
  Ui::GeoSysSettingsInteractionsForm ui_int;
  Ui::GeoSysSettingsMiscForm ui_misc;
  Ui::GeoSysSettingsTreeBrowserForm ui_treebrowser;
  std::map<VP1GeoFlags::SubSystemFlag,QCheckBox*> subSysCheckBoxMap;
  float last_transparency;
  bool last_showVolumeOutLines;
  int last_labels; //!< needed for POSSIBLECHANGE_IMP macro.
  QList<int> last_labelPosOffset; //!< needed for  POSSIBLECHANGE_IMP macro.
  SoPickStyle * pickStyle;
  VolumeHandle * lastSelectedVolHandle;
  ZappedVolumeListModel * zappedVolumeListModel;

  QString lastSaveMaterialsFile;
  QString lastLoadMaterialsFile;
};


//____________________________________________________________________
GeoSysController::GeoSysController(IVP1System * sys)
  : VP1Controller(sys,"GeoSysController"), m_d(new Imp)
{
  m_d->theclass = this;
  m_d->ui.setupUi(this);

  initDialog(m_d->ui_disp, m_d->ui.pushButton_settings_display);
  initDialog(m_d->ui_iconisedvols, m_d->ui.pushButton_settings_iconisedvols);
  initDialog(m_d->ui_int, m_d->ui.pushButton_settings_interactions);
  initDialog(m_d->ui_misc, m_d->ui.pushButton_settings_misc);
  initDialog(m_d->ui_treebrowser, m_d->ui.pushButton_settings_treebrowser);

  //  m_d->ui_disp.widget_drawOptions->setLineWidthsDisabled();
  m_d->ui_disp.widget_drawOptions->setPointSizesDisabled();
//   m_d->ui_disp.widget_drawOptions->setBaseLightingDisabled();
  m_d->ui_disp.widget_drawOptions->setComplexity(1.0); // default 'curved surface accuracy' value

  m_d->pickStyle = new SoPickStyle;
  m_d->pickStyle->ref();

 


  m_d->zappedVolumeListModel = new ZappedVolumeListModel(m_d->ui_iconisedvols.listView_iconisedvolumes);
  m_d->ui_iconisedvols.listView_iconisedvolumes->setUniformItemSizes(true);
  m_d->ui_iconisedvols.listView_iconisedvolumes->setSelectionMode(QAbstractItemView::ExtendedSelection);
  m_d->ui_iconisedvols.listView_iconisedvolumes->setModel(m_d->zappedVolumeListModel);

  connect(m_d->ui_iconisedvols.listView_iconisedvolumes,SIGNAL(activated(const QModelIndex&)),
	  m_d->zappedVolumeListModel,SLOT(activated(const QModelIndex&)));

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //  Setup connections which monitor changes in the controller so that we may emit signals as appropriate:  //
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////

  addUpdateSlot(SLOT(updatePickStyle()));
  connectToLastUpdateSlot(m_d->ui_int.checkBox_geomselectable);

  addUpdateSlot(SLOT(possibleChange_showVolumeOutLines()));
  connectToLastUpdateSlot(m_d->ui_disp.checkBox_showVolumeOutLines);

  addUpdateSlot(SLOT(possibleChange_transparency()));
  connectToLastUpdateSlot(m_d->ui_disp.spinBox_transp);

  initLastVars();

  //Connections for state-less signals:

  connect(m_d->ui_disp.pushButton_saveChangedMaterials,SIGNAL(clicked()),this,SLOT(saveMaterialsRequested()));
  connect(m_d->ui_disp.pushButton_loadMaterials,SIGNAL(clicked()),this,SLOT(loadMaterialsRequested()));


  connect(m_d->ui_misc.lineEdit_expand_vols_matname,SIGNAL(returnPressed()),this,SLOT(emit_autoExpandByVolumeOrMaterialName()));
  connect(m_d->ui_misc.pushButton_expand_vols_matname,SIGNAL(clicked()),this,SLOT(emit_autoExpandByVolumeOrMaterialName()));
  connect(m_d->ui_misc.lineEdit_expand_vols_volname,SIGNAL(returnPressed()),this,SLOT(emit_autoExpandByVolumeOrMaterialName()));
  connect(m_d->ui_misc.pushButton_expand_vols_volname,SIGNAL(clicked()),this,SLOT(emit_autoExpandByVolumeOrMaterialName()));

  // filter volumes
  connect(m_d->ui_misc.lineEdit_filter_logvolname,SIGNAL(returnPressed()),this,SLOT(emit_autoExpandByVolumeOrMaterialName()));
  connect(m_d->ui_misc.pushButton_filter_logvolname,SIGNAL(clicked()),this,SLOT(emit_autoExpandByVolumeOrMaterialName()));
  connect(m_d->ui_misc.pushButton_filter_reset,SIGNAL(clicked()),this,SLOT(emit_autoExpandByVolumeOrMaterialName()));


  connect(m_d->ui_int.checkBox_localAxes, SIGNAL(stateChanged(int)), this, SIGNAL(displayLocalAxesChanged(int)));
  connect(m_d->ui_int.slider_AxesScale, SIGNAL(valueChanged(int)), this, SIGNAL(axesScaleChanged(int)));

  setLastSelectedVolume(0);
}

//____________________________________________________________________
GeoSysController::~GeoSysController()
{
  m_d->pickStyle->unref();
  delete m_d;
}

//____________________________________________________________________
void GeoSysController::setGeometrySelectable(bool b)
{
  m_d->ui_int.checkBox_geomselectable->setChecked(b);
  updatePickStyle();
}

//____________________________________________________________________
void GeoSysController::setComplexity(double c)
{
  m_d->ui_disp.widget_drawOptions->setComplexity(c);
}

//____________________________________________________________________
void GeoSysController::setZoomToVolumeOnClick(bool b)
{
  m_d->ui_int.checkBox_zoomToVolumes->setChecked(b);
}

//____________________________________________________________________
void GeoSysController::setLastSelectedVolume(VolumeHandle*vh)
{
  m_d->lastSelectedVolHandle = vh;
  m_d->ui_disp.matButton_lastSel->setEnabled(vh!=0);
  if (vh) {
    QList<SoMaterial*> mats = m_d->ui_disp.matButton_lastSel->handledMaterials();
    if (mats.isEmpty()||mats.at(0)!=vh->material()) {
      m_d->ui_disp.matButton_lastSel->clearHandledMaterials();
      m_d->ui_disp.matButton_lastSel->setMaterial(vh->material());
    }
  } else {
    m_d->ui_disp.matButton_lastSel->clearHandledMaterials();
  }
}

//____________________________________________________________________
VolumeHandle* GeoSysController::lastSelectedVolume() const
{
  return m_d->lastSelectedVolHandle;
}

//____________________________________________________________________
void GeoSysController::saveMaterialsRequested()
{
  const bool ctrl_isdown = (Qt::ControlModifier & QApplication::keyboardModifiers());
  const bool onlyChangedMaterials = !ctrl_isdown;

  QString filename
    = QFileDialog::getSaveFileName(this, "Select geometry material file to save",
				   (m_d->lastSaveMaterialsFile.isEmpty()?
				    VP1Settings::defaultFileSelectDirectory()
				    :m_d->lastSaveMaterialsFile),
				   "VP1 geometry material files (*.vp1geomat)",
				   0,QFileDialog::DontResolveSymlinks);

  if(filename.isEmpty())
    return;

  if (!filename.endsWith(".vp1geomat"))
    filename += ".vp1geomat";

  m_d->lastSaveMaterialsFile = filename;

  messageVerbose("Emitting saveMaterialsToFile(\""+filename+"\","+str(onlyChangedMaterials)+")");
  emit saveMaterialsToFile(filename,onlyChangedMaterials);

}

//____________________________________________________________________
void GeoSysController::loadMaterialsRequested()
{
  QString filename = QFileDialog::getOpenFileName(this, "Select geometry material file to load",
						  (m_d->lastLoadMaterialsFile.isEmpty()?
						   VP1Settings::defaultFileSelectDirectory()
						   :m_d->lastLoadMaterialsFile),
						  "VP1 geometry material files (*.vp1geomat)",
						  0,QFileDialog::DontResolveSymlinks);
  if(filename.isEmpty())
    return;

  m_d->lastLoadMaterialsFile = filename;

  messageVerbose("Emitting loadMaterialsFromFile(\""+filename+"\")");
  emit loadMaterialsFromFile(filename);
}
#include <iostream>
//____________________________________________________________________
QCheckBox * GeoSysController::subSystemCheckBox(VP1GeoFlags::SubSystemFlag f) const
{
  std::map<VP1GeoFlags::SubSystemFlag,QCheckBox*>::const_iterator it = m_d->subSysCheckBoxMap.find(f);
  if (it==m_d->subSysCheckBoxMap.end()) {
    //std::cout << "Adding " << f << std::endl;
    QCheckBox *checkBox=new QCheckBox(m_d->ui.groupBox_Subsystems);
    checkBox->setText(f.c_str());
    checkBox->setEnabled(false);
    m_d->ui.groupBox_Subsystems->layout()->addWidget(checkBox);
    it=m_d->subSysCheckBoxMap.insert(it,std::make_pair(f,checkBox));
    checkBox->adjustSize();
  }
  return  it!= m_d->subSysCheckBoxMap.end() ?  it->second : nullptr;
}

//____________________________________________________________________
SoGroup * GeoSysController::drawOptions() const
{
  return m_d->ui_disp.widget_drawOptions->drawOptionsGroup();
}

//____________________________________________________________________
SoPickStyle * GeoSysController::pickStyle() const
{
  return m_d->pickStyle;
}

//____________________________________________________________________
VP1GeoTreeView * GeoSysController::volumeTreeBrowser() const
{
  return m_d->ui_treebrowser.treeView_volumebrowser;
}

QPushButton * GeoSysController::requestOutputButton () const {
  return m_d->ui.pushButton_settings_persistify;
}

QPushButton * GeoSysController::enableAllSubSystemsButton() const
{
  return m_d->ui.pushButton_enableAllSubsystems;
}

//____________________________________________________________________
PhiSectionWidget * GeoSysController::phiSectionWidget() const
{
  return m_d->ui_disp.phisectionwidget;
}

//____________________________________________________________________
ZappedVolumeListModel * GeoSysController::zappedVolumeListModel() const
{
  return m_d->zappedVolumeListModel;
}

//____________________________________________________________________
bool GeoSysController::displayLocalAxesOnClick() const {
  return m_d->ui_int.checkBox_localAxes->isChecked();
}


//____________________________________________________________________
bool GeoSysController::zoomToVolumeOnClick() const
{
  return m_d->ui_int.checkBox_zoomToVolumes->isChecked();
}

//____________________________________________________________________
bool GeoSysController::printInfoOnClick_Shape() const
{
  return m_d->ui_int.checkBox_print_shape->isChecked();
}

//____________________________________________________________________
bool GeoSysController::printInfoOnClick_Material() const
{
  return m_d->ui_int.checkBox_print_material->isChecked();
}

//____________________________________________________________________
bool GeoSysController::printInfoOnClick_CopyNumber() const
{
  return m_d->ui_int.checkBox_print_copyno->isChecked();
}

//____________________________________________________________________
bool GeoSysController::printInfoOnClick_Transform() const
{
  return m_d->ui_int.checkBox_print_transform->isChecked();
}

//____________________________________________________________________
bool GeoSysController::printInfoOnClick_Tree() const
{
  return m_d->ui_int.checkBox_print_tree->isChecked();
}

//____________________________________________________________________
bool GeoSysController::printInfoOnClick_Mass() const
{
  return m_d->ui_int.checkBox_print_mass->isChecked();
}

//_____________________________________________________________________________________
void GeoSysController::updatePickStyle()
{
  m_d->pickStyle->style = m_d->ui_int.checkBox_geomselectable->isChecked()
    ? SoPickStyle::SHAPE : SoPickStyle::UNPICKABLE;
}

//____________________________________________________________________
float GeoSysController::transparency() const
{
  int v(m_d->ui_disp.spinBox_transp->value());
  return (v>=100?1.0:(v<=0?0.0:v/100.0));
}

//____________________________________________________________________
void GeoSysController::setTransparency(float value) const
{
  assert( 0. <= value && value <= 100.);
  m_d->ui_disp.spinBox_transp->setValue(value);
  return;
}

//____________________________________________________________________
bool GeoSysController::isTranspLocked() const
{
      return m_d->ui_misc.lockTransp->isChecked();
}

//____________________________________________________________________
bool GeoSysController::showVolumeOutLines() const
{
  return m_d->ui_disp.checkBox_showVolumeOutLines->isChecked();
}


//____________________________________________________________________
void GeoSysController::emit_autoExpandByVolumeOrMaterialName()
{
  // if we filter on visible objects, then...
	if (sender()==m_d->ui_misc.pushButton_expand_vols_volname
			|| sender()==m_d->ui_misc.lineEdit_expand_vols_volname)
      {
          // check if a name is set in the 'volume' field
          bool volname(sender()==m_d->ui_misc.pushButton_expand_vols_volname
				      ||sender()==m_d->ui_misc.lineEdit_expand_vols_volname);

          // get volume's or material's name
		      QString name(volname ? m_d->ui_misc.lineEdit_expand_vols_volname->text()
				      : m_d->ui_misc.lineEdit_expand_vols_matname->text());

          // return if name is empty
          if (name.isEmpty()) {
			       return;
          }

          // emit the signal
		      messageVerbose("emitting autoExpandByVolumeOrMaterialName("+str(!volname)+", "+name+")");
		      emit autoExpandByVolumeOrMaterialName(!volname,name);
	}
  // if we use the 'filter volumes' feature, then...
	else if (sender()==m_d->ui_misc.pushButton_filter_logvolname
			|| sender()==m_d->ui_misc.lineEdit_filter_logvolname
            || sender()==m_d->ui_misc.pushButton_filter_reset)
  {
        // handle as "filter" if was sent by the 'filter volume' field or 'Apply' button...
        bool filter(sender()==m_d->ui_misc.pushButton_filter_logvolname
				      || sender()==m_d->ui_misc.lineEdit_filter_logvolname);

        // handle as "reset the view" if sent by the 'reset filters' button...
        bool resetView(sender()==m_d->ui_misc.pushButton_filter_reset);
       
        // get additional options
        int maxDepth(m_d->ui_misc.spinBox_maxDepth->value()==-1 ? 9999 : m_d->ui_misc.spinBox_maxDepth->value());
        bool stopAtFirst(m_d->ui_misc.radioButton_StopAtFirst->isChecked() ? true : false );
        bool visitChildren(m_d->ui_misc.radioButton_DoNotVisitChildren->isChecked() ? false : true);

        bool bymatname = false; // TODO: set this in the UI!
        QString nameRegEx(m_d->ui_misc.lineEdit_filter_logvolname->text());

        // return if name is empty
        if (nameRegEx.isEmpty()) {
			     return;
        }

        // clear regex if we are resetting the filters
        if (resetView) {
            m_d->ui_misc.radioButton_StopAtFirst->setChecked(true);
            m_d->ui_misc.radioButton_DoNotVisitChildren->setChecked(true);
            m_d->ui_misc.lineEdit_filter_logvolname->clear();
        }

        messageDebug("emitting signalFilterVolumes("+nameRegEx + ", " + str(bymatname) + ", " + str(maxDepth) + ", " + str(stopAtFirst)+ ", " + str(visitChildren) + ", " + str(resetView)+")");
        emit signalFilterVolumes(nameRegEx, bymatname, maxDepth, stopAtFirst, visitChildren, resetView);
	} 
	return;
}

//____________________________________________________________________
int GeoSysController::currentSettingsVersion() const
{
  return 6;
}

//____________________________________________________________________
void GeoSysController::actualSaveSettings(VP1Serialise&s) const
{
  s.save(m_d->ui_disp.phisectionwidget);//version <=1 saved in old format
  s.save(m_d->ui_disp.spinBox_transp);
  s.save(m_d->ui_disp.widget_drawOptions);
  s.save(m_d->ui_misc.lineEdit_expand_vols_matname);
  s.save(m_d->ui_misc.lineEdit_expand_vols_volname);
  s.save(m_d->ui_int.checkBox_geomselectable);
  s.save(m_d->ui_int.checkBox_print_shape);
  s.save(m_d->ui_int.checkBox_print_material);
  s.save(m_d->ui_int.checkBox_print_transform);
  s.save(m_d->ui_int.checkBox_print_mass);
  s.save(m_d->ui_int.checkBox_print_copyno);
  s.save(m_d->ui_int.checkBox_print_tree);
  s.save(m_d->ui_int.checkBox_zoomToVolumes);
  s.save(m_d->ui_disp.checkBox_showVolumeOutLines);//version 1+

  
  s.ignoreWidget(m_d->ui_disp.matButton_lastSel);
  std::map<VP1GeoFlags::SubSystemFlag,QCheckBox*>::const_iterator it,itE(m_d->subSysCheckBoxMap.end());
  for (it=m_d->subSysCheckBoxMap.begin();it!=itE;++it)
    s.ignoreWidget(it->second);
}

//____________________________________________________________________
void GeoSysController::actualRestoreSettings(VP1Deserialise& s)
{
  if (s.version()<0||s.version()>currentSettingsVersion()) {
    message("Warning: State data in .vp1 file has unsupported version ("+str(s.version())+")");
    return;
  }

  if (s.version()<=1) {
    s.ignoreObsoletePhiSectionWidgetState();
    s.ignoreWidget(m_d->ui_disp.phisectionwidget);
  } else {
    s.restore(m_d->ui_disp.phisectionwidget);
  }
  s.restore(m_d->ui_disp.spinBox_transp);
  s.restore(m_d->ui_disp.widget_drawOptions);
  s.restore(m_d->ui_misc.lineEdit_expand_vols_matname);
  s.restore(m_d->ui_misc.lineEdit_expand_vols_volname);
  s.restore(m_d->ui_int.checkBox_geomselectable);
  s.restore(m_d->ui_int.checkBox_print_shape);
  s.restore(m_d->ui_int.checkBox_print_material);
  s.restore(m_d->ui_int.checkBox_print_transform);
  s.restore(m_d->ui_int.checkBox_print_mass);
  s.restore(m_d->ui_int.checkBox_print_copyno);
  s.restore(m_d->ui_int.checkBox_print_tree);
  s.restore(m_d->ui_int.checkBox_zoomToVolumes);
  if (s.version()>=1)
    s.restore(m_d->ui_disp.checkBox_showVolumeOutLines);

  s.ignoreWidget(m_d->ui_disp.matButton_lastSel);
  std::map<VP1GeoFlags::SubSystemFlag,QCheckBox*>::const_iterator it,itE(m_d->subSysCheckBoxMap.end());
  for (it=m_d->subSysCheckBoxMap.begin();it!=itE;++it)
    s.ignoreWidget(it->second);

  m_d->ui_int.groupBox_printOnSelection->setEnabled(m_d->ui_int.checkBox_geomselectable->isChecked());
  m_d->ui_int.groupBox_automaticViewReorientations->setEnabled(m_d->ui_int.checkBox_geomselectable->isChecked());

}

///////////////////////////////////////////////////////////////////////////
// Test for possible changes in values and emit signals as appropriate:
// (possibleChange_XXX() slots code provided by macros)
#define VP1CONTROLLERCLASSNAME GeoSysController
#include "VP1Base/VP1ControllerMacros.h"
POSSIBLECHANGE_IMP(transparency)
POSSIBLECHANGE_IMP(showVolumeOutLines)


