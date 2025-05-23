########################################################################
##  SET ALL VERBOSE LEVELS TO 0
## ---------------------------------------------------------------------
/control/verbose  0
/run/verbose      0
/event/verbose    0
/tracking/verbose 0
#
########################################################################
##  SET NUMBER OF THREADS (will be ignored in case of sequential Geant4)
## ---------------------------------------------------------------------
/run/numberOfThreads 1
#
########################################################################
##  MAGNETIC FIELD
## ---------------------------------------------------------------------
## To Switch off the field set it to 0 tesla
/FSLdet/setField 4.0 tesla
#
########################################################################
##  INIT
## ---------------------------------------------------------------------
/run/initialize
#
########################################################################
##  PRIMARY GENERATOR
## ---------------------------------------------------------------------
/FSLgun/primaryPerEvt 2
/FSLgun/particle e-
/FSLgun/energy 10 GeV
#
########################################################################
##  OPEN DEFAULT GRAPHICS WINDOWS
## ---------------------------------------------------------------------
/vis/open
#
########################################################################
##  DRAW DETECTOR
## ---------------------------------------------------------------------
# Disable auto refresh and quieten vis messages whilst scene and
# trajectories are established:
#/vis/viewer/set/autoRefresh false
#/vis/verbose errors
#
# Draw geometry:
/vis/drawVolume ! ! 2  # draw top two levels
#
# Specify view angle:
/vis/viewer/set/viewpointThetaPhi 120 150
#
# Specify style (surface or wireframe):
/vis/viewer/set/style wireframe
/vis/viewer/set/auxiliaryEdge true
#
# Decorations
/vis/set/textColour green
/vis/scene/add/text2D -0.9 -.9 24 ! ! fullSimLight
/vis/set/textColour    # Revert to default text colour (blue)
#
# Axes, scale, etc.
/vis/scene/add/axes    # Simple axes: x=red, y=green, z=blue.
/vis/scene/add/eventID # Drawn at end of event
/vis/scene/add/date    # Date stamp
#
########################################################################
##  VISUALIZATION OF EVENTS
## ---------------------------------------------------------------------
# Draw smooth trajectories at end of event, showing trajectory points
# as markers 2 pixels wide:
/vis/scene/add/trajectories smooth
/vis/modeling/trajectories/create/drawByCharge
/vis/modeling/trajectories/drawByCharge-0/default/setDrawStepPts true
/vis/modeling/trajectories/drawByCharge-0/default/setStepPtsSize 2
# (if too many tracks cause core dump => /tracking/storeTrajectory 0)
#
# Draw hits at end of event:
/vis/scene/add/hits
#
# To superimpose all of the events from a given run:
/vis/scene/endOfEventAction accumulate
#
# Re-establish auto refreshing and verbosity:
/vis/viewer/set/autoRefresh true
/vis/verbose warnings
