########################################################################
##  SET ALL VERBOSE LEVELS TO 0
## ---------------------------------------------------------------------
/control/verbose 0
/run/verbose 0
/event/verbose 0
/tracking/verbose 0
########################################################################
##  SET NUMBER OF THREADS (will be ignored in case of sequential Geant4)
## ---------------------------------------------------------------------
/run/numberOfThreads 1
/control/cout/prefixString G4Worker_
########################################################################
##  MAGNETIC FIELD
## ---------------------------------------------------------------------
/FSLdet/setField 0 tesla
########################################################################
##  Geantino Maps Configuration : ATLAS Tile
## ---------------------------------------------------------------------
/gmaps/rmin 0 mm
/gmaps/rmax 4300 mm
/gmaps/zmin -7000 mm
/gmaps/zmax 7000 mm
/gmaps/xmin -4300 mm
/gmaps/xmax 4300 mm
/gmaps/ymin -4300 mm
/gmaps/ymax 4300 mm
/gmaps/etamin -6
/gmaps/etamax 6
/gmaps/volumeslist Tile
########################################################################
##  INIT
## ---------------------------------------------------------------------
/run/initialize
########################################################################
##  PRIMARY GENERATOR
## ---------------------------------------------------------------------
/gps/verbose 0
/gps/particle geantino
#/gps/pos/type Plane
#/gps/pos/shape Square
#/gps/pos/centre 1 2 1 cm
#/gps/pos/halfx 2 cm
#/gps/pos/halfy 2 cm
/gps/ang/type iso
/gps/energy 10 GeV
########################################################################
##  EXECUTE with 100000 events
## ---------------------------------------------------------------------
/run/beamOn 100000
