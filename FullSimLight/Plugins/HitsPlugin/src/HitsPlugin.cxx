//
// Plugin to Generate Hits
//
#include "H5Cpp.h"
#include "FullSimLight/FSLUserActionPlugin.h"
#include "G4UserSteppingAction.hh"
#include "G4UserEventAction.hh"
#include "G4UserRunAction.hh"
#include <iostream>
#include <fstream>
#include "G4Step.hh"
#include "G4Run.hh"
#include <string>
#include <G4Event.hh>
#include <G4String.hh>
#include <map>
#include <mutex>
#include <filesystem>

//----------------------------------------------------------------------//

struct Hit{
  float x;
  float y;
  float z;
  unsigned int  id;
};

std::map<G4String, unsigned int> particle_ids { {"gamma", 1}, {"e-", 2}, {"e+", 2}, {"mu-", 3}, {"mu+", 3}, };


//----------------------------------------------------------------------//


class GenerateHitsStep:
public G4UserSteppingAction
{
public:
    
    
    // Constructor:
    GenerateHitsStep();
     
    // Destructor:
    ~GenerateHitsStep();
     
    //Overriding Function
    void UserSteppingAction(const G4Step* step) override;
    
    //Hits vector
    std::vector<Hit> hits;
    
    //Clear Hits
    void clearhits(){hits.clear();}
    

 };
                            
GenerateHitsStep::GenerateHitsStep(){}

GenerateHitsStep::~GenerateHitsStep() {}


void GenerateHitsStep::UserSteppingAction(const G4Step* step){
 
    Hit hit_inst;
    hit_inst.x = step->GetPreStepPoint()->GetPosition()[0];
    hit_inst.y = step->GetPreStepPoint()->GetPosition()[1];
    hit_inst.z = step->GetPreStepPoint()->GetPosition()[2];
    
    //Checking to see if particle is in particle map defined above
    if (particle_ids.count(step->GetTrack()->GetParticleDefinition()->GetParticleName())>0)
    {
    hit_inst.id = particle_ids.find(step->GetTrack()->GetParticleDefinition()->GetParticleName())->second;
    }
    
    //Otherwise assigning it a default particle id of 9 for now.
    else
    {
    hit_inst.id = 9;
    }
    hits.push_back(hit_inst);

}

//----------------------------------------------------------------------//

class GenerateHitsEvent:
public G4UserEventAction
 {
public:
    
     // Constructor:
     GenerateHitsEvent();
     
     // Destructor:
    ~GenerateHitsEvent();
     
     //Overriding Function
     void EndOfEventAction(const G4Event* evt) override;
     
     //Set Stepping Action
     void SetSteppingAction(GenerateHitsStep* stepact){step = stepact;}
     
     //Set file
     void assignfile(H5::H5File *filename){file = filename;}
     
     //Set DataType
     void assignDataType(H5::CompType &data_type){datatype = &data_type;}
     
     

 private:
     
     GenerateHitsStep* step;
     unsigned int event_ID;
     std::vector<Hit> hits;
     H5::H5File* file;
     H5::CompType* datatype;
     std::mutex mutex;
     
     
     
 };



GenerateHitsEvent::GenerateHitsEvent(){}

GenerateHitsEvent::~GenerateHitsEvent(){}

void GenerateHitsEvent::EndOfEventAction(const G4Event* evt)
{
    event_ID = evt->GetEventID();
    hits = step->hits;
    mutex.lock();
    hsize_t numberOfHits= hits.size();
    H5::DataSpace dataspace(1,&numberOfHits,nullptr);
    const std::string datasetName="EVENT-"+std::to_string(event_ID);
    H5::DataSet dataset=file->createDataSet(datasetName, *datatype, dataspace);
    dataset.write(hits.data(),*datatype);
    step->clearhits();
    mutex.unlock();
}


//----------------------------------------------------------------------//




class GenerateHitsRun:
public G4UserRunAction
 {
public:
    
     // Constructor:
     GenerateHitsRun();
     
     // Destructor:
    ~GenerateHitsRun();
     
     //Overriding Function
     virtual void BeginOfRunAction(const G4Run*) override final;
     virtual void EndOfRunAction(const G4Run*) override final;
     
     //Setting other User Actions
     void SetEventAction(GenerateHitsEvent* evt) {event = evt;}
     


     
 private:
     
     GenerateHitsEvent* event;
     static H5::CompType datatype;
     static std::string path;
     static H5::H5File *file;
     

 };

std::string GenerateHitsRun::path = "hits.h5";
H5::H5File *GenerateHitsRun::file{nullptr};
H5::CompType GenerateHitsRun::datatype = sizeof(Hit);

GenerateHitsRun::GenerateHitsRun(){
  if (!file) {
    std::filesystem::path hitsPath(path);
    if (std::filesystem::exists(hitsPath)) std::filesystem::remove(hitsPath);
    file=new H5::H5File(path, H5F_ACC_TRUNC);
  }
}
GenerateHitsRun::~GenerateHitsRun(){
  delete file;
  file=nullptr;
}


void GenerateHitsRun::BeginOfRunAction(const G4Run*)
{
    if(IsMaster())
   {
        
        
        datatype.insertMember("X", HOFFSET(Hit,x),H5::PredType::NATIVE_FLOAT);
        datatype.insertMember("Y", HOFFSET(Hit,y),H5::PredType::NATIVE_FLOAT);
        datatype.insertMember("Z", HOFFSET(Hit,z),H5::PredType::NATIVE_FLOAT);
        datatype.insertMember("ID", HOFFSET(Hit,id),H5::PredType::NATIVE_UINT);

    }
    
    event->assignfile(file);
    event->assignDataType(datatype);
    
}

void GenerateHitsRun::EndOfRunAction(const G4Run*)
{
    if(IsMaster())
    {
        std::cout << "Hits file written at: " << path << std::endl;
    }
    
}











//----------------------------------------------------------------------//


class GenerateHitsPlugin:public FSLUserActionPlugin {
    
public:
    
    GenerateHitsPlugin();
    virtual G4UserSteppingAction *getSteppingAction() const final override;
    virtual G4UserEventAction *getEventAction() const final override;
    virtual G4UserRunAction *getRunAction() const final override;
    GenerateHitsEvent* eventaction = new GenerateHitsEvent();
    GenerateHitsStep* stepaction = new GenerateHitsStep();
    GenerateHitsRun*  runaction = new GenerateHitsRun();

};

GenerateHitsPlugin::GenerateHitsPlugin()
{
    
eventaction->SetSteppingAction(stepaction);
runaction->SetEventAction(eventaction);

}

G4UserSteppingAction *GenerateHitsPlugin::getSteppingAction() const {
  return stepaction;
}

G4UserEventAction *GenerateHitsPlugin::getEventAction() const {
    return eventaction;
    
}

G4UserRunAction *GenerateHitsPlugin::getRunAction() const {
    return runaction;
    
}


extern "C" GenerateHitsPlugin *createGenerateHitsPlugin() {
    return new GenerateHitsPlugin();
}

