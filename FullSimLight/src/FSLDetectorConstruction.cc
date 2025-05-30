/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "FSLDetectorConstruction.hh"
#include "FSLDetectorMessenger.hh"
#include "RegionConfigurator.hh"
#include "FullSimLight/MagFieldPlugin.h"
#include "MassCalculator.hh"
#include "ClashDetector.hh"

// Geant4 includes
#include "G4Version.hh"
#include "globals.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"

#include "G4UniformMagField.hh"
#include "G4QuadrupoleMagField.hh"
#include "G4FieldManager.hh"
#include "G4TransportationManager.hh"
#include "G4VPhysicalVolume.hh"
#include "G4RunManager.hh"
#include "G4PVPlacement.hh"
#include "G4ChordFinder.hh"
#include "G4Mag_UsualEqRhs.hh"
#include "G4MagIntegratorStepper.hh"
#include "G4Version.hh"
#include "G4LogicalVolume.hh"
#include "G4UnitsTable.hh"
#include "G4GDMLParser.hh"
//#include "G4LogicalVolumeStore.hh"

#include <algorithm>
// Geant4 steppers
#if G4VERSION_NUMBER>=1040
#include "G4BogackiShampine23.hh"
#include "G4BogackiShampine45.hh"
#include "G4DoLoMcPriRK34.hh"
#include "G4DormandPrince745.hh"
#include "G4DormandPrinceRK56.hh"
#include "G4DormandPrinceRK78.hh"
#include "G4RK547FEq1.hh"
#include "G4RK547FEq2.hh"
#include "G4RK547FEq3.hh"
#include "G4TsitourasRK45.hh"
#include "G4VIntegrationDriver.hh"
#include "G4IntegrationDriver.hh"
#endif

#if G4VERSION_NUMBER>=1060
#include "G4InterpolationDriver.hh"
#endif

#include "G4CashKarpRKF45.hh"
#include "G4ClassicalRK4.hh"
#include "G4FieldManager.hh"
#include "G4HelixExplicitEuler.hh"
#include "G4HelixImplicitEuler.hh"
#include "G4HelixSimpleRunge.hh"
#include "G4NystromRK4.hh"
#include "G4RKG3_Stepper.hh"


// **** INCLUDES for GeoModel
#include "GeoModelRead/ReadGeoModel.h"
#include "GeoModel2G4/ExtParameterisedVolumeBuilder.h"
#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelKernel/GeoVGeometryPlugin.h"
#include "GeoModelKernel/GeoGeometryPluginLoader.h"
#include "GeoModelKernel/GeoVolumeCursor.h"
#include "GeoModelKernel/GeoAlignableTransform.h"
#include "GeoModelHelpers/defineWorld.h"

// For Sensitive Detector plugins:
#include "FullSimLight/FSLSensitiveDetectorPlugin.h"
#include "FSLSDPluginLoader.h"
#include "G4VSensitiveDetector.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4LogicalVolume.hh"
#include "G4SDManager.hh"


#include <iomanip>

G4AnalysisManager* FSLDetectorConstruction::fAnalysisManager = nullptr;

G4double FSLDetectorConstruction::gFieldValue = 0.0;

FSLDetectorConstruction::FSLDetectorConstruction() : fWorld(nullptr), fDetectorMessenger(nullptr)
{
  fFieldValue          = 0.0;
  fVerbosityFlag       = -1;
  fGmclashVerbosity    = false;
  fTolerance           = 0.0;
  fFieldConstant       = false;
  fDetectorMessenger   = new FSLDetectorMessenger(this);
  fRunOverlapCheck     = false;
  fRunMassCalculator   = false;
  fAddRegions          = false;
  fDumpGDML            = false;
  fReportFileName      = "gmclash_report.json";
  fMinStep             = 1.0e-2;
  fField.Put(0);
  fOutputGDMLFileName = "geometry.gdml";

}

FSLDetectorConstruction::~FSLDetectorConstruction()
{
  delete fDetectorMessenger;
}

G4VPhysicalVolume *FSLDetectorConstruction::Construct()
{
    fTimer.Start();

    PVConstLink world{};
    G4LogicalVolume* envelope;
    if (fGeometryFileName.contains(".dylib") || fGeometryFileName.contains(".so"))
    {
        std::cout<< "Bulding the detector from a plugin: "<<fGeometryFileName<<std::endl;
        GeoGeometryPluginLoader loader;
        GeoVGeometryPlugin *factory=loader.load(fGeometryFileName.data());
        if (!factory) {
            std::cout<<"Error!Cannot load geometry from factory. Exiting!"<<std::endl;
            exit(0);

        }

        GeoIntrusivePtr<GeoPhysVol> worldTmp{createGeoWorld()};
        factory->create(worldTmp);
        GeoIntrusivePtr<GeoPhysVol> world = resizeGeoWorld(worldTmp);
        G4cout << "World resized correctly." << G4endl;
        
        G4cout << "ReadGeoModel::buildGeoModel() done." << G4endl;
        fTimer.Stop();
        G4cout << "First step done. GeoModelTree built from the SQLite file." << G4endl;
        G4cout << "*** Real time elapsed   : " <<fTimer.GetRealElapsed()   << G4endl;
        G4cout << "*** User time elapsed   : " <<fTimer.GetUserElapsed()   << G4endl;
        G4cout << "*** System time elapsed : " <<fTimer.GetSystemElapsed() << G4endl;

        fTimer.Start();
        // build the Geant4 geometry and get an hanlde to the world' volume
        ExtParameterisedVolumeBuilder* builder = new ExtParameterisedVolumeBuilder("Detector");

        std::cout << "Building G4 geometry."<<std::endl;
        envelope = builder->Build(world);

        G4VPhysicalVolume* physWorld= new G4PVPlacement(0,G4ThreeVector(),envelope,envelope->GetName(),0,false,0,false);

        fWorld = physWorld;
        fWorld->GetLogicalVolume()->SetVisAttributes(G4VisAttributes::GetInvisible());

        if (fWorld == 0) {
            G4ExceptionDescription ed;
            ed << "World volume not set properly check your setup selection criteria or input files!" << G4endl;
            G4Exception("FSLDetectorConstruction::Construct()", "FULLSIMLIGHT_0000", FatalException, ed);
        }
        
//        std::vector<G4LogicalVolume*>* lvStore =  G4LogicalVolumeStore::GetInstance();
//        std::size_t nlv=lvStore->size();
//        std::cout<<"G4LogicalVolume store size: "<<nlv<<std::endl;
//        for (std::size_t i=0;  i<nlv; ++i) {
//            G4LogicalVolume* lv = (*lvStore)[i];
//            std::cout<<"G4LogicalVolume store:: "<<lv->GetName()<<std::endl;
//
//        }
        
        G4cout << "Second step done. Geant4 geometry created from GeoModeltree "<<G4endl;
        G4cout << "Detector Construction from the plugin file " << fGeometryFileName.data() <<", done!"<<G4endl;

    }
    else if (fGeometryFileName.contains(".db")){
        G4cout << "Building the detector from the SQLite file: "<<fGeometryFileName<<G4endl;

        // open the DB
        auto db = std::make_unique<GMDBManager>(fGeometryFileName.data());
        /* Open database */
        if (db->checkIsDBOpen()) {
            G4cout << "Database is open!" << G4endl;
        }
        else{
            G4cout << "ERROR! Database is not open." << G4endl;
            // return;
            throw;

        }

        // -- testing the input database
        //std::cout << "Printing the list of all GeoMaterial nodes" << std::endl;
        //db->printAllMaterials();
        /* setup the GeoModel reader */
        GeoModelIO::ReadGeoModel readInGeo{db.get()};
        G4cout << "ReadGeoModel set.";


        /* build the GeoModel geometry */
        world = readInGeo.buildGeoModel(); // builds the whole GeoModel tree in memory and get an handle to the 'world' volume
        G4cout << "ReadGeoModel::buildGeoModel() done." << G4endl;
        fTimer.Stop();
        G4cout << "First step done. GeoModelTree built from the SQLite file." << G4endl;
        G4cout << "*** Real time elapsed   : " <<fTimer.GetRealElapsed()   << G4endl;
        G4cout << "*** User time elapsed   : " <<fTimer.GetUserElapsed()   << G4endl;
        G4cout << "*** System time elapsed : " <<fTimer.GetSystemElapsed() << G4endl;

        fTimer.Start();
        // build the Geant4 geometry and get an hanlde to the world' volume
        ExtParameterisedVolumeBuilder* builder = new ExtParameterisedVolumeBuilder("ATLAS");

        std::cout << "Building G4 geometry."<<std::endl;
        envelope = builder->Build(world);
        G4VPhysicalVolume* physWorld= new G4PVPlacement(0,G4ThreeVector(),envelope,envelope->GetName(),0,false,0,false);

        fWorld = physWorld;
        fWorld->GetLogicalVolume()->SetVisAttributes(G4VisAttributes::GetInvisible());

        if (fWorld == 0) {
            G4ExceptionDescription ed;
            ed << "World volume not set properly check your setup selection criteria or GDML input!" << G4endl;
            G4Exception("FSLDetectorConstruction::Construct()", "FULLSIMLIGHT_0000", FatalException, ed);
        }
        G4cout << "Second step done. Geant4 geometry created from GeoModeltree "<<G4endl;
        G4cout << "Detector Construction from the SQLite file " << fGeometryFileName.data() <<", done!"<<G4endl;
        
//        std::vector<G4LogicalVolume*>* lvStore =  G4LogicalVolumeStore::GetInstance();
//        std::size_t nlv=lvStore->size();
//        std::cout<<"G4LogicalVolume store size: "<<nlv<<std::endl;
//        for (std::size_t i=0;  i<nlv; ++i) {
//            G4LogicalVolume* lv = (*lvStore)[i];
//            std::cout<<"G4LogicalVolume store:: "<<lv->GetName()<<std::endl;
//
//        }
    }

    else if (fGeometryFileName.contains(".gdml")){
        G4cout << "Building the detector from the GDML file: "<<fGeometryFileName<<G4endl;
        //fParser.SetOverlapCheck(true);
        fParser.Read(fGeometryFileName, false); // turn off schema checker
        fWorld = (G4VPhysicalVolume *)fParser.GetWorldVolume();
        fWorld->GetLogicalVolume()->SetVisAttributes(G4VisAttributes::GetInvisible());

        //RecursivelyCheckOverlap(fWorld->GetLogicalVolume());
        envelope = fWorld->GetLogicalVolume();

        if (fWorld == 0) {
            G4ExceptionDescription ed;
            ed << "World volume not set properly! Check your setup selection criteria or the GDML input!" << G4endl;
            G4Exception("FSLDetectorConstruction::Construct()", "FULLSIMLIGHT_0001", FatalException, ed);
        }
        G4cout << "Detector Construction from the GDML file " << fGeometryFileName.data() <<", done!"<<G4endl;


    }
    else{
        std::cout<< "Error! Geometry format file not supported! Please use one of the following format: .db/.gdml/.so/.dylib. Exiting. "<<std::endl;
        exit(-1);
    }


    fTimer.Stop();


    G4cout << "**** Real time elapsed   : " <<fTimer.GetRealElapsed()   << G4endl;
    G4cout << "**** User time elapsed   : " <<fTimer.GetUserElapsed()   << G4endl;
    G4cout << "**** System time elapsed : " <<fTimer.GetSystemElapsed() << G4endl;

    if (fRunOverlapCheck){
        G4cout << "\n ===================  Starting Clashes Detection  =================== \n" << G4endl;
        fTimer.Start();
        ClashDetector cd = ClashDetector(fWorld);
        cd.SetGMClashVerbosity(fGmclashVerbosity);
        cd.SetTolerance(fTolerance);
        cd.CheckOverlapsInTree(fWorld);
        fTimer.Stop();
        G4cout << G4endl;
        G4cout << "**** Real time elapsed   : " <<fTimer.GetRealElapsed()   << G4endl;
        G4cout << "**** User time elapsed   : " <<fTimer.GetUserElapsed()   << G4endl;
        G4cout << "**** System time elapsed : " <<fTimer.GetSystemElapsed() << G4endl;

        std::cout<<"\n**** Writing out the clashes report file: "<<fReportFileName<<std::endl;
        cd.PrintOutReport(fReportFileName);

        G4cout<<"\n=================== Clashes Detection done! =================== "<<G4endl;
        exit(0);
    }

    if(fRunMassCalculator)
    {
        G4cout<<"\n=================== Starting mass calculation...   =================== "<<G4endl;
        
        MassCalculator mc = MassCalculator();
        mc.SetPrefixLogicalVolume(fPrefixLogicalVolume);
        mc.SetMaterial(fMaterial);
        mc.SetVerbosity(fVerbosityFlag);
        std::cout<<"Calculating the mass of the total detector... "<<fGeometryFileName<<std::endl;
        fTimer.Start();
        std::vector<json> jlist;
        mc.recursiveMassCalculation(fWorld,world, jlist);


        fTimer.Stop();
        G4cout << "\n**** Real time elapsed for mass calculation  : "<<fTimer.GetRealElapsed()/60   << " min. "<< G4endl;
        G4cout << "**** User time elapsed for mass calculation   : " <<fTimer.GetUserElapsed()/60   << " min. "<<G4endl;
        G4cout << "**** System time elapsed for mass calculation : " <<fTimer.GetSystemElapsed()/60 << " min. "<<G4endl;

        json jReport={{"MassReport",jlist}};
        std::cout<<"\n**** Writing out the mass report file: "<<fReportFileName<<std::endl;
        std::ofstream outJsonFile(fReportFileName);
        outJsonFile << std::setw(4) << jReport << std::endl;
        outJsonFile.close();
        G4cout<<"\n=================== Mass calculation...DONE!   =================== "<<G4endl;
    }

    // trying to add detector regions with the configurations given in the RegionConfigurator
    if(Regions.size()>0)
    {
        fAddRegions = true;
    }
    
    if (fAddRegions)  {
        G4cout << "\n ===================  Trying to add detector regions ... ================== \n" << G4endl;
        RegionConfigurator::Instance().CreateRegions(Regions, RootLVNames, electron_cut,
                                                     proton_cut, positron_cut, gamma_cut,1);
        G4cout << "\n ===================  Adding detector regions is DONE!  ================== \n" << G4endl;
    }

    if (fDumpGDML){

        G4cout << "\n ===================  Dump geometry in GDML format  =================== \n" << G4endl;
        //G4GDMLParser parser;
        //fParser.SetRegionExport(true);
        //fParser.SetEnergyCutsExport(true);
        PullUnidentifiedVolumes(fWorld->GetLogicalVolume());
        G4cout << "\n" << G4endl;
        fParser.Write(fOutputGDMLFileName, fWorld->GetLogicalVolume());
        G4cout << "\n =================== Geometry exported in GDML, DONE!  =================== \n" << G4endl;
        exit(0);
    }

    return fWorld;
}

void FSLDetectorConstruction::ConstructSDandField()
{
 // if (std::abs(fFieldValue) > 0.0) {

    for (unsigned int i=0;i<sensitiveDetectorPluginName.size();i++) {
      FSLSDPluginLoader loader;
      FSLSensitiveDetectorPlugin *plugin=loader.load(sensitiveDetectorPluginName[i]);
      G4VSensitiveDetector *sensitiveDetector=plugin->getSensitiveDetector();
      sensitiveDetector->Activate(true);
      G4SDManager *sdm=G4SDManager::GetSDMpointer();
      sdm->AddNewDetector(sensitiveDetector);
      G4LogicalVolumeStore *logVolStore=G4LogicalVolumeStore::GetInstance();
      for (auto lv: *logVolStore) {
	for (auto v=plugin->beginLogVolumeNames();v!=plugin->endLogVolumeNames();v++) {
	  if (*v==lv->GetName()) {
	    std::cout << "Sensitive Detector " << sensitiveDetector->GetName() << " attached to log vol " << lv->GetName() << " active = " << sensitiveDetector->isActive() << std::endl;
	    lv->SetSensitiveDetector(sensitiveDetector);
	  }
	}
      }
    }




  
    if (fFieldConstant && std::abs(fFieldValue) > 0.0){
    // Apply a global uniform magnetic field along the Z axis.
    // Notice that only if the magnetic field is not zero, the Geant4
    // transportation in field gets activated.
    auto uniformMagField     = new G4UniformMagField(G4ThreeVector(0.0, 0.0, fFieldValue));
    G4FieldManager *fieldMgr = G4TransportationManager::GetTransportationManager()->GetFieldManager();
    fieldMgr->SetDetectorField(uniformMagField);
    fieldMgr->CreateChordFinder(uniformMagField);
    G4cout << G4endl << " *** SETTING UNIFORM MAGNETIC FIELD : fieldValue = " << fFieldValue / tesla << " Tesla *** " << G4endl
           << G4endl;

    }
    else if (fFieldConstant && fFieldValue == 0.0 ){
        G4cout << G4endl << " *** MAGNETIC FIELD IS OFF  *** " << G4endl << G4endl;
    }
    else // if (!fFieldConstant)
        {
          G4cout << G4endl << " *** MAGNETIC FIELD SET FROM PLUGIN  *** " << G4endl << G4endl;
          if (fField.Get() == 0)
          {
              {
              GeoPluginLoader<MagFieldPlugin> loader;
              MagFieldPlugin *plugin=loader.load(mag_field_plugin_path);
              G4MagneticField *g4Field=plugin->getField();
              delete plugin;
              if(g4Field==nullptr) std::cout<<"Error, g4Field is null!"<<std::endl;
              fField.Put(g4Field);
              }

          //This is thread-local
          G4FieldManager* fieldMgr =
          G4TransportationManager::GetTransportationManager()->GetFieldManager();
          G4cout<< "DeltaStep "<<fieldMgr->GetDeltaOneStep()/mm <<"mm" <<G4endl;
          //G4ChordFinder *pChordFinder = new G4ChordFinder(FSLmagField);

//#if G4VERSION_NUMBER < 1040
//
//        auto stepper = getStepper(m_integratorStepper, field);
//        G4MagInt_Driver* magDriver = fieldMgr->GetChordFinder()->GetIntegrationDriver();
//        magDriver->RenewStepperAndAdjust(stepper);
//#else
//
//        auto chordFinder = fieldMgr->GetChordFinder();
//        auto driver = createDriverAndStepper(m_integratorStepper, field);
//        chordFinder->SetIntegrationDriver(driver);

          fieldMgr->SetDetectorField(fField.Get());
          fieldMgr->CreateChordFinder(fField.Get());
//#endif

      }


  }
}
#if G4VERSION_NUMBER>=1040
//=============================================================================
// Create the driver with a stepper
//=============================================================================
G4VIntegrationDriver*
FSLDetectorConstruction::createDriverAndStepper(std::string stepperType) const
{


    G4Mag_EqRhs* eqRhs(nullptr);
//        if (!m_equationOfMotion.empty())
//        {
//            eqRhs = m_equationOfMotion->makeEquationOfMotion(field);
//            //ATH_MSG_INFO("Configuring alternative equation of motion using " <<
//                         m_equationOfMotion.name() );
//        }
//        else
//        {
//            //ATH_MSG_VERBOSE("Using G4Mag_UsualEqRhs as the equation of motion.");
//            eqRhs = new G4Mag_UsualEqRhs(field);
//        }
    G4VIntegrationDriver* driver = nullptr;
    if (stepperType=="HelixImplicitEuler") {
            G4HelixImplicitEuler* stepper = new G4HelixImplicitEuler(eqRhs);
            driver = new G4IntegrationDriver<G4HelixImplicitEuler>(
                                                                   fMinStep, stepper, stepper->GetNumberOfVariables());
        } else if (stepperType=="HelixSimpleRunge") {
            G4HelixSimpleRunge* stepper = new G4HelixSimpleRunge(eqRhs);
            driver = new G4IntegrationDriver<G4HelixSimpleRunge>(
                                                                 fMinStep, stepper, stepper->GetNumberOfVariables());
        } else if (stepperType=="HelixExplicitEuler") {
            G4HelixExplicitEuler* stepper = new G4HelixExplicitEuler(eqRhs);
            driver = new G4IntegrationDriver<G4HelixExplicitEuler>(
                                                                   fMinStep, stepper, stepper->GetNumberOfVariables());
        } else if (stepperType=="NystromRK4") {
            G4NystromRK4* stepper = new G4NystromRK4(eqRhs);
            driver = new G4IntegrationDriver<G4NystromRK4>(
                                                           fMinStep, stepper, stepper->GetNumberOfVariables());
        } else if (stepperType=="ClassicalRK4") {
            G4ClassicalRK4* stepper = new G4ClassicalRK4(eqRhs);
            driver = new G4IntegrationDriver<G4ClassicalRK4>(
                                                             fMinStep, stepper, stepper->GetNumberOfVariables());
        }  else if (stepperType=="CashKarpRKF45") {
            G4CashKarpRKF45* stepper = new G4CashKarpRKF45(eqRhs);
            driver = new G4IntegrationDriver<G4CashKarpRKF45>(
                                                              fMinStep, stepper, stepper->GetNumberOfVariables());
        }
        else if (stepperType=="DoLoMcPriRK34") {
            G4DoLoMcPriRK34* stepper = new G4DoLoMcPriRK34(eqRhs);
        driver = new G4IntegrationDriver<G4DoLoMcPriRK34>(
                                                          fMinStep, stepper, stepper->GetNumberOfVariables());

        } else if (stepperType=="BogackiShampine23") {
            G4BogackiShampine23* stepper = new G4BogackiShampine23(eqRhs);
            driver = new G4IntegrationDriver<G4BogackiShampine23>(fMinStep, stepper, stepper->GetNumberOfVariables());

        } else if (stepperType=="BogackiShampine45") {
            G4BogackiShampine45* stepper = new G4BogackiShampine45(eqRhs);
            driver = new G4IntegrationDriver<G4BogackiShampine45>(
                                                              fMinStep, stepper, stepper->GetNumberOfVariables());

        } else if (stepperType=="DormandPrince745") {
            G4DormandPrince745* stepper = new G4DormandPrince745(eqRhs);
            driver = new G4IntegrationDriver<G4DormandPrince745>(
                                                                 fMinStep, stepper, stepper->GetNumberOfVariables());
        } else if (stepperType=="DormandPrinceRK56") {
            G4DormandPrinceRK56* stepper = new G4DormandPrinceRK56(eqRhs);
            driver = new G4IntegrationDriver<G4DormandPrinceRK56>(
                                                                  fMinStep, stepper, stepper->GetNumberOfVariables());
        } else if (stepperType=="DormandPrinceRK78") {
            G4DormandPrinceRK78* stepper = new G4DormandPrinceRK78(eqRhs);
            driver = new G4IntegrationDriver<G4DormandPrinceRK78>(
                                                                  fMinStep, stepper, stepper->GetNumberOfVariables());
        } else if (stepperType=="RK547FEq1") {
            G4RK547FEq1* stepper = new G4RK547FEq1(eqRhs);
            driver = new G4IntegrationDriver<G4RK547FEq1>(
                                                          fMinStep, stepper, stepper->GetNumberOfVariables());
        } else if (stepperType=="RK547FEq2") {
            G4RK547FEq2* stepper = new G4RK547FEq2(eqRhs);
            driver = new G4IntegrationDriver<G4RK547FEq2>(
                                                          fMinStep, stepper, stepper->GetNumberOfVariables());
        } else if (stepperType=="RK547FEq3") {
            G4RK547FEq3* stepper = new G4RK547FEq3(eqRhs);
            driver = new G4IntegrationDriver<G4RK547FEq3>(
                                                          fMinStep, stepper, stepper->GetNumberOfVariables());
        }
        else if (stepperType=="TsitourasRK45") {
            G4TsitourasRK45* stepper = new G4TsitourasRK45(eqRhs);
            driver = new G4IntegrationDriver<G4TsitourasRK45>(
                                                          fMinStep, stepper, stepper->GetNumberOfVariables());

        }
        else if (stepperType=="RKG3_Stepper") {
            G4RKG3_Stepper* stepper = new G4RKG3_Stepper(eqRhs);
            driver = new G4IntegrationDriver<G4RKG3_Stepper>(
                                                             fMinStep, stepper, stepper->GetNumberOfVariables());
        }
#if G4VERSION_NUMBER>=1060
        else if (stepperType=="DormandPrince745Int") {
            G4DormandPrince745* stepper = new G4DormandPrince745(eqRhs);
            driver = new G4InterpolationDriver<G4DormandPrince745>(
                                                                   fMinStep, stepper, stepper->GetNumberOfVariables());
        }
#endif
        else {
            std::cout<<"Stepper " << stepperType << " not available! returning NystromRK4!"<<std::endl;
            G4NystromRK4* stepper = new G4NystromRK4(eqRhs);
            driver = new G4IntegrationDriver<G4NystromRK4>(
                                                           fMinStep, stepper, stepper->GetNumberOfVariables());
        }
        return driver;

    }
#endif

//=============================================================================
// Create the stepper (Geant4 < 10.4)
//=============================================================================
G4MagIntegratorStepper*
FSLDetectorConstruction::CreateStepper(std::string name, G4MagneticField* field) const
{
    std::cout<<"Stepper type is : "<<name<<std::endl;
    //ATH_MSG_DEBUG("getStepper");
    G4Mag_EqRhs* eqRhs(nullptr);
    //if (!m_equationOfMotion.empty())

   //{
    //    eqRhs = m_equationOfMotion->makeEquationOfMotion(field);
    //    ATH_MSG_INFO("Configuring alternative equation of motion using " <<
    //                 m_equationOfMotion.name() );
    //}
    //else
    //{
    //    ATH_MSG_VERBOSE("Using G4Mag_UsualEqRhs as the equation of motion.");
    // @TODO Check that this is the  default equation of motion in ATLAS
    eqRhs = new G4Mag_UsualEqRhs(field);
    //}
    // @TODO Add some way of confirming that the choices of Equation of
    // motion and stepper are compatible.
    // @TODO consider moving the stepper choice into a tool as well?
    // Only worthwhile if we envisage adding more stepper choices in the
    // future.
    if (name=="HelixImplicitEuler") return new G4HelixImplicitEuler(eqRhs);
    else if (name=="HelixSimpleRunge") return new G4HelixSimpleRunge(eqRhs);
    else if (name=="HelixExplicitEuler") return new G4HelixExplicitEuler(eqRhs);
    else if (name=="NystromRK4") return new G4NystromRK4(eqRhs);
    else if (name=="ClassicalRK4") return new G4ClassicalRK4(eqRhs);
    else if (name=="AtlasRK4") {
        std::string g4tag = G4VERSION_TAG;
        if (g4tag.find("atlas") != std::string::npos){

            std::cout<<"Setting AtlasRK4 stepper, in the OLD style"<<std::endl;
            std::cout<<"CAVEAT: only available with atlas patches of Geant4!"<<std::endl;
#ifdef G4ATLAS
            return new G4AtlasRK4(eqRhs);
#endif
        } else {

            std::cout<<"ERROR! AtlasRK4 is only available with atlas patches of Geant4!"<<std::endl;
            std::cout<<"Returning ClassicalRK4!"<<std::endl;
            return new G4ClassicalRK4(eqRhs);

        }
    }
#if G4VERSION_NUMBER>=1040
    else if (name=="BogackiShampine23") return new G4BogackiShampine23(eqRhs);
    else if (name=="BogackiShampine45") return new G4BogackiShampine45(eqRhs);
    else if (name=="CashKarpRKF45") return new G4CashKarpRKF45(eqRhs);
    else if (name=="DoLoMcPriRK34") return new G4DoLoMcPriRK34(eqRhs);
    else if (name=="DormandPrince745") return new G4DormandPrince745(eqRhs);
    else if (name=="DormandPrinceRK56") return new G4DormandPrinceRK56(eqRhs);
    else if (name=="DormandPrinceRK78") return new G4DormandPrinceRK78(eqRhs);
    else if (name=="RK547FEq1") return new G4RK547FEq1(eqRhs);
    else if (name=="RK547FEq2") return new G4RK547FEq2(eqRhs);
    else if (name=="RK547FEq3") return new G4RK547FEq3(eqRhs);
    else if (name=="RKG3_Stepper") return new G4RKG3_Stepper(eqRhs);
    else if (name=="TsitourasRK45") return new G4TsitourasRK45(eqRhs);
#endif
    std::cout<<"ERROR: Stepper " << name << " not available! returning NystromRK4!"<<std::endl;
    return new G4NystromRK4(eqRhs);
}


void FSLDetectorConstruction::PullUnidentifiedVolumes( G4LogicalVolume* v ){

    if (v==0) return;
    std::vector<G4VPhysicalVolume*> pv_to_remove;
    for (size_t i=0;i<v->GetNoDaughters();++i){

        G4VPhysicalVolume * n_v = v->GetDaughter(i);

        if (n_v->GetName() == "LAr::EMEC::Pos::InnerWheel" ||
            n_v->GetName() == "LAr::EMEC::Neg::InnerWheel" ||
            n_v->GetName() == "LAr::EMEC::Pos::OuterWheel" ||
            n_v->GetName() == "LAr::EMEC::Neg::OuterWheel" ||
            n_v->GetName() == "LAr::EMEC::Pos::InnerCone"  ||
            n_v->GetName() == "LAr::EMEC::Neg::InnerCone"  ||
            n_v->GetName() == "LAr::EMEC::Pos::OuterFrontCone" ||
            n_v->GetName() == "LAr::EMEC::Neg::OuterFrontCone" ||
            n_v->GetName() == "LAr::EMEC::Pos::OuterBackCone"  ||
            n_v->GetName() == "LAr::EMEC::Neg::OuterBackCone"  ||
            n_v->GetName() == "LAr::EMEC::Pos::InnerSlice00"   ||
            n_v->GetName() == "LAr::EMEC::Neg::InnerSlice00"   ||
            n_v->GetName() == "LAr::EMEC::Pos::OuterSlice00"   ||
            n_v->GetName() == "LAr::EMEC::Neg::OuterSlice00"   ||
            n_v->GetName() == "UnidentifiedShape"){
            // This is one to remove
            std::cout<<" !REMOVING: "<< n_v->GetName()<<", shape is not supported in GDML!"<<std::endl;
            pv_to_remove.push_back(n_v);
        } else {
            // Recurse
            PullUnidentifiedVolumes( n_v->GetLogicalVolume() );
        }
    }
    for (unsigned int j=0;j<pv_to_remove.size();++j){
        v->RemoveDaughter( pv_to_remove[j] );
    }


}
