# Plugins Examples

`fullSimLight` provides a convenient mechanism for users to extend their simulations through plugins in the form of shared libraries. Plugins can be used to add:

- Geometry description
- User Actions
- Sensitive Detectors 
- Magnetic Field
- Physics Lists
- Event Generators

For a description of the available example plugins look at the following sections. 

##Geometry plugins

Some geometry example plugins are available at the [GeoModelATLAS](https://gitlab.cern.ch/atlas/geomodelatlas/GeoModelATLAS) repository, under the [Example Plugins](https://gitlab.cern.ch/atlas/geomodelatlas/GeoModelATLAS/-/tree/main/ExamplePlugins) folder. The `ToyGeometryPlugin` and the `AccordionPlugin` are the right examples to start with. 

##User Actions plugins

You can find some example user actions plugins under the FullSimLight/Plugins folder. In particular we provide a DummyUserAction plugin available [here](https://gitlab.cern.ch/GeoModelDev/GeoModel/-/tree/main/FullSimLight/Plugins/Examples/UserActionPlugins) and a [Hits plugin](https://gitlab.cern.ch/GeoModelDev/GeoModel/-/tree/main/FullSimLight/Plugins/HitsPlugin) and a [Track plugin](https://gitlab.cern.ch/GeoModelDev/GeoModel/-/tree/main/FullSimLight/Plugins/TracksPlugin). 

###Hits Plugin

The [Hits plugin](https://gitlab.cern.ch/GeoModelDev/GeoModel/-/tree/main/FullSimLight/Plugins/HitsPlugin) implements both `G4UserSteppingAction` and `G4UserEventAction`. At each step the particle position (x,y,z coordinates) is saved and at the end of the event the hits are saved into a `HDF5` output file. The same file can be in a second step visualized in `gmex`.

###Tracks Plugin

The [Tracks Plugin](https://gitlab.cern.ch/GeoModelDev/GeoModel/-/tree/main/FullSimLight/Plugins/TracksPlugin) implements `G4UserRunAction`, `G4UserEventAction`, `G4UserSteppingAction`, `G4UserTrackingAction`. For each track at each step the position is recorded and mapped into a map. At the end of the event the information is saved into a `HDF5` output file (the default name is `Tracks_data.h5`). The tracks file data can be in a second step visualized in `gmex`.

##Sensitive Detectors plugins

One sensitive detectors example plugin is provided [here](https://gitlab.cern.ch/GeoModelDev/GeoModel/-/blob/main/FullSimLight/Plugins/Examples/SensitiveDetectorPlugins/SDPlugin/src/SDPlugin.cxx). It's a dummy plugin that serves the purpose of showing how to implement the corresponding `fullSimLight` abstract interface. 

##Magnetic Field plugins

An example Magnetic field plugin can be found [here](https://gitlab.cern.ch/GeoModelDev/GeoModel/-/tree/main/FullSimLight/Plugins/Examples/MagneticFieldPlugins/UniformMagneticFieldPlugin). It implements a uniform magnetic field. 
An ATLAS specific Magnetic Field plugin is also available in the [ATLAS Extensions](https://gitlab.cern.ch/atlas/geomodelatlas/ATLASExtensions) repository. Please refer to the ATLAS Extensions page for further info.

##Physics list plugins

A physics list plugin example is available [here](https://gitlab.cern.ch/GeoModelDev/GeoModel/-/tree/main/FullSimLight/Plugins/Examples/PhysicsListPlugins/FSLTestPhysListPlugins). It implements a custom physics list that is based on the standard `FTFP_BERT`, adding the `G4OpticalPhysics` process. 

##Event Generators plugins

An Event generator example plugin is available [here](https://gitlab.cern.ch/GeoModelDev/GeoModel/-/tree/main/FullSimLight/Plugins/Examples/EventGeneratorPlugins/FSLExamplePrimaryGeneratorPlugin). 
It shows how to customize the use of the `G4ParticleGun`and set some properties as the particle type, name, energy, position, polarization, and momentum direction. 

## How to write a plugin: the Hits Plugin example

As a simple example, suppose you as the user want to keep track of the various particles and their positions as the simulation progresses. In particular, if you run the simulation with a certain number of events, then for each event you want to write to a file all the particles and their positions ***"hits"***.

Geant4 provides classes to assist with this task. Namely the `G4UserSteppingAction` class contains the function:    

```c++
virtual void UserSteppingAction (const G4Step *aStep);
```

By overriding this function we can get particle positions and names through the `G4Step` object. Now since we also want to write out our hits to a file for each event, we can use the `G4UserEventAction` class, which conveniently provides the function:
 
```c++
virtual void EndOfEventAction (const G4Event *anEvent);
```
which is called at the end of every event. We can also access the event ID through the `G4Event` object. Before proceeding let us set up our project. Navigating to the directory of choice we can run on the terminal
 
```bash
mkdir HitsPlugin
cd HitsPlugin
touch CMakeLists.txt
mkdir build
mkdir src
cd src
touch HitsPlugin.cxx
```
This creates the directory structure:

```bash
 HitsPlugin
   ├── CMakeLists.txt
   ├── build
   └── src
       └── HitsPlugin.cxx    
```

Opening up the CMakeLists.txt file, we call our project GenerateHitsPlugin and configure the file as follows:

```cmake
# Set up the project.
set(CMAKE_CXX_STANDARD 17)
project( "GenerateHitsPlugin" )

# Find and set the source file.
file( GLOB SOURCES src/*.cxx )
set(PROJECT_SOURCES ${SOURCES})

# Set up the library.
add_library(GenerateHitsPlugin SHARED ${SOURCES})

#FullSimLight
find_package(FullSimLight REQUIRED)

#Geant4
find_package(Geant4 REQUIRED)
include(${Geant4_USE_FILE})

#Link libraries and include directories
target_link_libraries ( GenerateHitsPlugin PUBLIC FullSimLight ${Geant4_LIBRARIES})
target_include_directories( GenerateHitsPlugin PUBLIC ${FullSimLight_INCLUDE_DIR})
```

Now we are ready to write our implementation of recording hits in `HitsPlugin.cxx`. This will require three classes and one additional function at the end of the file.

```c++
class GenerateHitsStep;
class GenerateHitsEvent;
class GenerateHitsPlugin;
extern "C" GenerateHitsPlugin * createGenerateHitsPlugin();
```

- The first two classes `GenerateHitsStep` & `GenerateHitsEvent` will inherit from `G4UserSteppingAction` and `G4UserEventAction` respectively, since these contain the methods that are relevant for our purpose as discussed earlier. In general you will need one class for every type of User Action you decide to use.

- The final class `GenerateHitsPlugin` is necessary for defining the plugin and ***must have the same name as the name specified within the add_library command in the CMakeLists.txt file.*** This class will inherit from the `FSLUserActionPlugin` class, which is the abstract class provided by FullSimLight to allow it to interface with our custom defined User Actions. This class must always be defined.

- Finally the function `createGenerateHitsPlugin` is neccesary for properly loading in the plugin to FullSimLight and ***must always have the name create + name of plugin class.*** This function must always be defined.

Now that we have an outline of what we need to do, let's include all the relevant header files based on the above discussion.

```c++
#include <iostream>
#include <fstream>
#include <map>
#include "FullSimLight/FSLUserActionPlugin.h"
#include "G4UserSteppingAction.hh"
#include "G4UserEventAction.hh"
#include "G4Step.hh"
#include <G4Event.hh>
```
Next let's define a `Hit` struct which provides a convenient way of storing a particle position and ID

```c++
struct Hit{
  float x;
  float y;
  float z;
  unsigned int  id;
};
```
Let us also create a map which associates various particles with an ID. This is useful as we can access particle names through the `G4Step` object and then associate those names with IDs for more efficient storage.

```c++
//Not a comprehensive list of all particles
std::map<G4String, unsigned int> particle_ids { {"gamma", 1}, 
{"e-", 2}, {"e+", 2}, {"mu-", 3}, {"mu+", 3}, };
```

We can now define our first class `GenerateHitsStep` which will inherit from `G4UserSteppingAction` and will record hits for us.

```c++
class GenerateHitsStep:
public G4UserSteppingAction
{
public:
    
    
    // Constructor:
    GenerateHitsStep();
     
    // Destructor:
    ~GenerateHitsStep();
     
    //Overriding virtual function with our implementation
    void UserSteppingAction(const G4Step* step) override;
    
    //A vector to store hits
    std::vector<Hit> hits;
    
    //A function to clear the hits vector when a new event starts
    void clearhits(){hits.clear();}
    

 };
                            
GenerateHitsStep::GenerateHitsStep(){}

GenerateHitsStep::~GenerateHitsStep() {}


void GenerateHitsStep::UserSteppingAction(const G4Step* step){
 
    //Creating a Hit instance
    Hit hit_inst;
    
    //Assigning the Hit position
    hit_inst.x = step->GetPreStepPoint()->GetPosition()[0];
    hit_inst.y = step->GetPreStepPoint()->GetPosition()[1];
    hit_inst.z = step->GetPreStepPoint()->GetPosition()[2];
    
    //Checking to see if particle is in particle map defined above, if so assigning the corresponding ID
    if (particle_ids.count(step->GetTrack()->GetParticleDefinition()->GetParticleName())>0)
    {
    hit_inst.id = particle_ids.find(step->GetTrack()->GetParticleDefinition()->GetParticleName())->second;
    }
    
    //Otherwise assigning it a default particle id of 9.
    else
    {
    hit_inst.id = 9;
    }
    //Adding hit to hits vector
    hits.push_back(hit_inst);

}

```
Next we define `GenerateHitsEvent` class which will inherit from `G4UserEventAction` and will write hits to a file at the end of every event.

```c++
class GenerateHitsEvent:
public G4UserEventAction
 {
public:
    
     // Constructor:
     GenerateHitsEvent();
     
     // Destructor:
    ~GenerateHitsEvent();
     
     //Overriding virtual function with our implementation. 
     //Function called at the end of every event.
     void EndOfEventAction(const G4Event* evt) override;
     
     //Function to set a Stepping Action
     void SetSteppingAction(GenerateHitsStep* stepact){step = stepact;}
     
 private:
     
     //Instance of stepping action to get hits from
     GenerateHitsStep* step;
     
     //Private member to store the event ID
     unsigned int event_ID;
     
     //Vector to store hits
     std::vector<Hit> hits;
     
     
     
 };


GenerateHitsEvent::GenerateHitsEvent(){}

GenerateHitsEvent::~GenerateHitsEvent(){}

void GenerateHitsEvent::EndOfEventAction(const G4Event* evt)
{
    //Get Event ID
    event_ID = evt->GetEventID();
    
    //Get hits from the stepping action
    hits = step->hits;

    /* Implement your own code to write
        to a file */
    
    //Clear hits vector in the stepping action as end of event is reached.
    step->clearhits();
}

```

Now we need to define the `GenerateHitsPlugin` class which will provide the interface with FullSimLight. This class will inherit from `FSLUserActionPlugin` class and override methods depending upon the User Actions used.

```c++
class GenerateHitsPlugin:public FSLUserActionPlugin {
    
public:
    
    //Constructor
    GenerateHitsPlugin();
    
    //overriding functions in FSLUserActionPlugin based on the User Actions we've used.
    //Since we used G4UserSteppingAction & G4UserEventAction we override getSteppingAction and getEventAction.
    //Similar procedure for other actions.
    //These overrided functions just return a new instance of the corresponding implementation class.
    //For example getEventAction would return an instance of GenerateHitsEvent in this case.
    //Similarly getSteppingAction would return an instance of GenerateHitsStep.
    //Similar procedure if other User Actions were used.
    virtual G4UserSteppingAction *getSteppingAction() const final override;
    virtual G4UserEventAction *getEventAction() const final override;

    //New Instances of implementation classes
    GenerateHitsEvent* eventaction = new GenerateHitsEvent();
    GenerateHitsStep* stepaction = new GenerateHitsStep();

};

GenerateHitsPlugin::GenerateHitsPlugin()
{
//Passing the stepping action into the event action as required by our implementation.
eventaction->SetSteppingAction(stepaction);
}

G4UserSteppingAction *GenerateHitsPlugin::getSteppingAction() const {
  return stepaction;
}

G4UserEventAction *GenerateHitsPlugin::getEventAction() const {
    return eventaction;
    
}
```

Finally we need to define the function `createGenerateHitsPlugin()` which will return a new instance of our plugin class.


```c++

extern "C" GenerateHitsPlugin *createGenerateHitsPlugin() {
    return new GenerateHitsPlugin();
}

```

With the implementation completed, we can now move into the build folder and run.

```bash
cmake ..
make
```

In the build folder you will see:

```bash
libGenerateHitsPlugin.dylib (Mac)
libGenerateHitsPlugin.so (Linux)
```

Our plugin is ready to use! We can now open up `fsl` by running the `fsl` command and can add our plugin in the User Actions tab as shown below:

{{ imgutils_image_caption('plugin.png', 
   alt='fsl', 
   cap='fsl User Actions tab',
   width ='700px',
   urlFix=False) 
}}

Once we have configured the rest of the simulation to our desire we can run FullSimLight and record hits.
