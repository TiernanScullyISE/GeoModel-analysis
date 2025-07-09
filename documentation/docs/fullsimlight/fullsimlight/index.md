# FullSimLight

FullSimLight package consists of different tools based on the [Geant4](https://geant4.web.cern.ch) toolkit, that can be run on multiple geometries. It contains: 

- **fullSimLight**: a light `Geant4` particle transport simulation (geometry, transport in magnetic field and basic physics scoring)
- **fsl**: GUI for fullSimLight configuration
- **gmclash**: a tool that runs clash detection on your input geometry, producing a json file report
- **gmgeantino**: a tool to generate geantino maps from your input geometry
- **gmmasscalc**: a tool to compute the mass of a given piece of detector geometry
- **gm2gdml**: a tool to convert geometries and dump them in gdml format.

For instructions on how to install FullSimLight from development kits please refer to the [Install library and tools section](https://geomodel.web.cern.ch/home/start/install/). If you are a developer and want to build FullSimLight from source code, please refer to the [Build FullSimLight ](https://geomodel.web.cern.ch/home/dev/build_fullsimlight/) page.
For information on the specific tool please refer to the relevant section.

## FullSimLight: an experiment agnostic simulation tool

`fullSimLight` is an experiment agnostic simulation tool that allows to easily run `Geant4` simulations by supporting multiple detector description formats (SQLite `.db`, `.gdml`, plugins `.dylib/.so`). It provides extended functionality by allowing the user to customise their simulation adding customized `Geant4` User Actions, Sensitive Detectors, specialised Physics Lists, custom particle generators, etc. These flexibility is provided through the mechanism of plugins which come in the form of shared libraries (`.dylib` or `.so`) containing user custom code. 

Two plugins that implement User Actions are available as examples for the users: the [Hits](https://geomodel.web.cern.ch/home/fullsimlight/plugins/#hits-plugin) plugin and the [Tracks](https://geomodel.web.cern.ch/home/fullsimlight/plugins/#tracks-plugin) plugin.

A built-in visualization for `fullSimLight` output is available in [gmex](https://geomodel.web.cern.ch/home/components/geomodelvisualization/gmex/) (the interactive 3D geometry visualization tool which is part of GeoModel). Hits steps and tracks generated with `fullSimLight` can be fed into `gmex`and consequently visualised. As an example the following figures can be seen. Figure 1 shows a Hits steps display of events transported with `fullSimLight` in the full ATLAS detector.

{{ imgutils_image_caption('StepDisplay.png', 
   alt='StepDisplay', 
   cap='Figure 1: Steps display of events transported with fullSimLight in the full ATLAS detector.',
   urlFix=False) 
}}

Figure 2 shows a HITS tracks display of events transported with `fullSimLight` in the full ATLAS detector.

{{ imgutils_image_caption('TrackDisplay.png', 
   alt='TrackDisplay', 
   cap='Figure 2: Track display of primaries transported with fullSimLight in the full ATLAS detector.',
   urlFix=False) 
}}


## FullSimLight: run and main components


`fullSimLight` can be built and used both with sequential and multithreaded `Geant4` builds. In case of multithreaded installation of `Geant4`, the application will run in proper multithreaded mode. You can find the executables under the `build/bin` directory and/or under the `<path-to-install>/bin` dir. 

!!! note

    Before running fullSimLight make sure to source the *geant4.sh* file to set correctly all the Geant4 environment variables. 
   
    ```
    source <path_to_geant4_install_dir>/bin/geant4.sh
    ```


`fullSimLight` can be configured in multiple ways (using `fsl` generated json configuration files or the `Geant4` macro files) and run either from the command line or within `fsl`.


## Configure and run fullSimLigth with its GUI fsl

`fullSimLight` can be very easily configured and run via `fsl`, both from within the GUI or from the command-line, passing a json configuration file with the `-c` flag. Watch the following video for a quick demo on how to configure and launch `fullSimLight` with its GUI `fsl`! 

<video src="../fsl/fslDemo.mov" width="720" height="540" controls></video>

For details on how to configure your simulation with `fsl` GUI, please refer to the [fsl page](https://geomodel.web.cern.ch/home/fullsimlight/fsl).

## Run fullSimLigth from the command line

Alternatively, `fullSimLight` can be executed via command line using the basic available flags. The only mandatory parameter necessary for starting a simulation is the geometry file. It can be specified inside the config file or with the `-g` flag.  

Run the executable with the --help option to see the available options:

``` bash
-c :   [OPTIONAL] : json configuration file generated with fsl
-g :   [REQUIRED] : the Geometry file name - if not specified in the configuration file
-i :   [OPTIONAL] : flag  ==> run the application in interactive visual mode (default: FALSE)
-m :   [OPTIONAL] : the standard Geant4 macro file name  
-o :   [OPTIONAL] : flag  ==> run the geometry overlap check (default: FALSE)
-f :   [OPTIONAL] : physics list name (default: FTFP_BERT) 
-P :   [OPTIONAL] : use Pythia primary generator [config. available: ttbar/higgs/minbias or use a Pythia command input file]
``` 
Please refer to the [Command line examples](#command-line-examples) section for some examples that illustrate the different possibilities.

### Parameters configuration via Geant4 macro (disfauvored)

`fullSimLight` and in general `Geant4` based simulations, need a `Geant4` macro to inject some input parameters. The default macro used by `fullSimLight` is called `macro.g4` and it should  be found under the `<install-path>/share/FullSimLight` directory. The macro can be edited to change some parameters, i.e the `Geant4` verbosity, the number of threads, or to tune the simulation. The most relevant macro commands are explained in the sections [Particle gun](#particle-gun) and [Magnetic field](#magnetic-field).

Please note that this option is deprecated, as it doesn't allow full flexibility, and we suggest to configure your simulation via `fsl`.


## Detector Construction

The supported geometry formats are SQLite (.db), GDML (.gdml) and plugins compiled as shared libraries (.dylib/.so) that contains the C++ code that builds from scratch a GeoModel geometry. An example of geometry plugin can be found in the GeoModelExamples folder and it is the so-called [KitchenSinkPlugin](https://gitlab.cern.ch/GeoModelDev/GeoModel/-/tree/main/GeoModelExamples/KitchenSinkPlugin).

Use the `-g` flag to specify the name of the input geometry file.

### ATLAS Geometry Files

If you are interested in running with the ATLAS geometry files, please refer to the [ATLAS Extensions](https://geomodel.web.cern.ch/home/fullsimlight/atlas-extensions/) page. 

## Event generation

`fullSimLight` uses by default the `Geant4` particle gun as primary generator. In addition, it supports also
input events from the [Pythia](http://home.thep.lu.se/Pythia/) generator, [HepMC3](https://gitlab.cern.ch/hepmc/HepMC3) formats or custom event generators plugins.


### Particle gun
 
The particle gun can be tuned either via `fsl` or by editing the `Geant4` macro file. The default configuration of the `macro.g4` file will generate 10 GeV primary particles (randomly chosen between e-,e+ and gammas) at the (0,0,0) position with isotropic momentum direction. 

Following are the UI commands that can be used to tune the `fullSimLight` particle gun.

#### Number of primaries per event:
The number of primary particles per event can be set through the macro
command:

``` bash
/FSLgun/primaryPerEvt <number>
```

By default, i.e. if it is not specified by the above command, the number of primary particles will be randomly selected for each individual event from the [1, 10] interval uniformly.

#### Primary particle energy:
The primary particle energy can be set through the macro command:

``` bash
/FSLgun/energy  <energy-value> <unit>
```
By default, i.e. if it is not specified by the above command, the kinetic energy will be randomly selected for each individual primary particle from the [1 GeV, 100 GeV] uniformly.

#### Primary particle direction:
The primary particle momentum direction can be set through the macro command: 

``` bash
/FSLgun/direction  <xdir-value> <ydir-value> <zdir-value>
```
By default, i.e. if it is not specified by the above command, the momentum direction will be randomly selected for each individual primary particle from isotropic distribution.

#### Primary particle type:
The primary particle type can be set through the macro command:

``` bash
/FSLgun/particle <particle-name>
``` 
By default, i.e. if it is not specified by the above command, the type will be randomly selected from a pre-defined list for each individual primary particle uniformly. The current list of particles includes e-, e+ and gamma particles. It can be extended by adding more particles to the list in the `FSLPrimaryGeneratorAction` class.


### Pythia generator
 
 `fullSimLight` supports `Pythia` as primary particles generator. In order to use `Pythia`, the user should have it installed in their system and if `Pythia` is found `fullSimLight` will be compiled with its support activated. 
 
 In order to specify that `fullSimLight` has o run with `Pythia` event, the `-P` or `--pythia` flag has to be used.
 Three different default options are available to run with `Pythia` events: `ttbar`, `higgs` and `minbias`:
 
``` bash
 -P :   generate events with Pythia [config. available: ttbar/higgs/minbias or use ascii input file]
``` 

The default configurations are currently defined as follows:
``` bash

static const char *pythia_minbias[] = {
  "HardQCD:all = on",
  "Beams:eCM = 13000.0",
  "PhaseSpace:pTHatMin = 20.0",
};

static const char *pythia_higgs[] = {
  "HiggsSM:all = on",
  "Beams:eCM = 13000.0",
  "PhaseSpace:pTHatMin = 20.0",
};

static const char *pythia_ttbar[] = {
  "Top:gg2ttbar = on",
  "Top:qqbar2ttbar = on",
  "Beams:eCM = 13000.0",
  "PhaseSpace:pTHatMin = 20.0",
};

```
For example, in order to simulate the default *ttbar* events, the command to be run is the following:
 
```bash

./fullSimLight -m ../share/FullSimLight/pythia.g4 -P ttbar -g mygeometry.db 

```  
 
Alternatively, the user can plug their own Pythia configuration file to simulate the desired events. 
 
 The number of events that the user wants to simulate must be specified in a `Geant4` macro file. A specific `pythia.g4` macro file can be found in the `<path-to-install>/share/FullSimLight` directory. It should be used when simulating `Pythia` events and can be edited according to the user needs. 
 
### HepMC3 events input

`fullSimLight` supports reading events in `HepMC3` format both the standard Asciiv3 format (introduced in `HepMC3`) as well as the old `Ascii` format (used in `HepMC` and `HepMC2`). This option is only available when configuring `fullSimLight` with `fsl` and the json configuration file. Please refer to the [FSL](https://geomodel.web.cern.ch/home/fullsimlight/fsl/) section for more info. 

## Magnetic field
 
 A constant magnetic field can be set through the macro command:
 
``` bash
/FSLdet/setField <field-value> <unit>
```

The default value is a uniform magnetic field of 4 Tesla.

### ATLAS Magnetic Field

If you are interested in running with the ATLAS magnetic field map, please refer to the [ATLAS Extensions](https://geomodel.web.cern.ch/home/fullsimlight/atlas-extensions/) page. 

<!--- full_bfieldmap_7730_20400_14m_version5.root-->
<!--- solenoid_bfieldmap_7730_0_14m_version5.root-->
<!--- toroid_bfieldmap_0_20400_14m_version5.root-->
<!--- bmagatlas_09_fullAsym20400.data-->
<!---->
<!--If the magnetic field maps are not available to the user, a constant magnetic field can be set through the macro file. The test application "testMagneticField" can be used to test the different magnetic field maps and produce Maps in 2D histograms that are saved in a ROOT file (i.e 'ATLAS_BField_default.root') .-->
<!--Please notice that no dependency on ROOT is introduced, because the histograms are created, filled and saved using the `G4AnalysisManager`.-->
<!---->
<!--### testMagneticField examples:-->
<!--Run the application with the --help flag to see the options:-->
<!---->
<!--``` bash-->
<!--./testMagneticField --help-->
<!---->
<!---f :  OPTIONAL: magnetic field filename [.data/.root]   (default : use ATLAS magnetic field maps)-->
<!---r :  FLAG:     use root field map (default : false, use .data file)-->
<!---s :  FLAG:     set Solenoid Off -->
<!---t :  FLAG:     set Toroids Off -->
<!--```-->
<!---->
<!--By default the file tested is the *bmagatlas_09_fullAsym20400.data*. Use the -r option to test the ROOT file maps, add the -s flag to set the solenoid off and use the *toroid_bfieldmap_0_20400_14m_version5.root*:-->
<!---->
<!--``` bash-->
<!--./testMagneticField -r -s -->
<!--```-->
<!---->
<!--Use the -t to set the Toroids off, and test the *solenoid_bfieldmap_7730_0_14m_version5.root* file. -->
<!---->
<!--``` bash-->
<!--./testMagneticField -r -t -->
<!--```-->

## Physics List

 The Physics List can be specified as an input argument with the `-f` flag
 (e.g. `-f FTFP_BERT_ATL`). Notice that the name of the `Geant4` built in Physics List
 must be in upper case, exactly as the corresponding header file. By default,
 i.e. if the Physics List name is not provided as an input argument, the `FTFP_BERT`
 Physics List will be used.  

## Scoring

A minimal set of "observables" is collected during the simulation per-primary
particle type: mean energy deposit, mean charged and neutral step lengths,
mean number of steps made by charged and neutral particles, mean number of
secondary e-, e+ and gamma particles. The result is reported at the end of
each event for each primary particle that was transported in the given event.

At the end of the simulation a final report is printed showing the run time,
the primary generator and magnetic field settings used during the run, the
total number of events and primary particles transported and the per-primary
type simulation statistics of the above-mentioned quantities.

Please see the following output as an example:
 
``` bash
=======================================================================================  
   Run terminated:                                                                         
     Number of events transported    = 10
     Time:  User=0.020000s Real=0.021719s Sys=0.000000s [Cpu=92.1%]
  =======================================================================================  
 
 ==================================   Run summary   ===================================== 

    Number of events  = 10
 
 ---------------------------------------------------------------------------------------- 

  Mean energy deposit per event = 0.01572 +- 0.01243 [GeV]

  Mean track length (charged) per event = 67.77 +- 43.66 [cm]
  Mean track length (neutral) per event = 154.9 +- 113.1 [cm]

  Number of steps (charged) per event = 303.2 +- 215.3
  Number of steps (neutral) per event = 348.3 +- 255.9

  Number of secondaries per event : 
     Gammas    =  5.3 +- 4.649
     Electrons =  11.4 +- 9.78
     Positrons =  0.3 +- 0.4583
```


<!--The simulation can be executed in "performance" mode by providing the -p-->
<!--input flag. No any user actions are created in this case beyond the-->
<!--only one RunAction (only for the Master-thread in case of MT) that will-->
<!--start and stop a timer at the beginning and the end of the simulation-->
<!--(initialization time won't be included). Therefore, there is no scoring-->
<!--in this case.-->



## Command line examples

If you have created your custom configuration file `myconfig.json` with `fsl`, you can simply run fullSimLight as it follows:

``` bash
./fullSimLight -c myconfig.json  
``` 
This is the best and more complete way of configuring `fullSimLight` because it allows you to customize also the use of plugins and to use all the different primary particles generation methods.

As an alternative to using the json configuration file, one can use the available command line options and a classical geant4 macro to configure `fullSimLight`. 

During the installation of FullSimLight a default macro file `macro.g4` will be installed in your `<install-path >/share/FullSimLight` directory. The macro file doesn't need to be specified if you intend to use the default one.

To execute the application using the default `macro.g4`  macro file, with the default FTFP_BERT
Physics List, building the detector from `mygeometry.db` file:

``` bash
./fullSimLight -g mygeometry.db
```

To execute the application using the default `macro.g4`  macro file and building the detector with a geometry described in one of the [GeoModelPlugins repo](https://gitlab.cern.ch/atlas/geomodelatlas/GeoModelATLAS/-/tree/master/GeoModelPlugins?ref_type=heads), i.e.  `PixelPlugin` :

``` bash
./fullSimLight  -g libPixePlugin.1.0.0.dylib/.so
```

To execute the application using a custom `mymacro.g4` macro file, with the ATLAS `FTFP_BERT_ATL` Physics List, with `Pythia` ttbar as primary events and building the detector from the `mygeometry.db`  file :

``` bash
./fullSimLight -m mymacro.g4 -f FTFP_BERT_ATL -P ttbar -g mygeometry.db 
```
### Run FullSimLight in interactive mode

It is possible to run `fullSimLight` in interactive mode, activating the Geant4 visualization tool. In order to do that you would need to compile Geant4 with the visualization on by using the following options:

``` bash
-DGEANT4_USE_QT=ON -DGEANT4_USE_QT_QT6=ON 
```

For versions of Geant4 greater than Geant 11.3 it will be needed to specify also the following option:

``` bash
-DGEANT4_INSTALL_PACKAGE_CACHE=ON
```
In order to run `fullSimLight` in interactive mode you should use the `-i` option, i.e.

``` bash
./fullSimLight  -g libPixePlugin.1.0.0.dylib/.so -i
```

In this way you will be prompted with a GUI that will display the chosen geometry. The default macro file used with the interacting mode is `initVis.g4`. It is installed under <install-geomodel>/share/FullSimLight and by default it activates some of the Geant4 visualization commands. 

For example, by default only 2 layers of the geometry will be visualized. This is necessary especially for complex geometries like HEP detectors (ATLAS, CMS, LHCb and ALICE) that have a number of layers that would be prohibitive to visualize. The user can change the default number of layers by editing the macro file. 

In order to run the simulation, you have to give the run/beamOn command the GUI. This can be done by editing the "Session" tab:
```
/run/beamOn
```

!!! Tip "Watch!"
    Watch the following video for a quick demo on how to run `fullSimLight` in interactive mode! 
    <video src="fullSimLightVisualDemo.mov" width="720" height="540" controls></video>


You can visualize tracks/showers as they are developing in real-time by using the following command: 
```
/vis/open OGLS
```

If you are interested in more documentation about how to tune the Geant4 visualization you can have a look at the following resources:

[Geant4 Visualization Lecture 1](https://videos.cern.ch/record/2300857)
[Geant4 Visualization Lecture 2](https://videos.cern.ch/record/2300872)
[Geant4 Users Application Manual](https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/Visualization/visualization.html)
