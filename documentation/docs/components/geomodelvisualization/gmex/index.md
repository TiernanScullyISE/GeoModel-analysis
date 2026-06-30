# GeoModelVisualization

GeoModelVisualization (gmex) is a 3D interactive visualization tool that allows you to display, navigate and debug complex detector geometries as the HEP experiments ones. 

## Examples

In order to launch gmex you could simply type:

``` bash
./gmex 
``` 

This will open a dialogue window where you will be able to select a SQLite (.db) file that you want to display (see Figure 1).

{{ imgutils_image_caption('Fig 1.png', 
   alt='GmexGui', 
   cap='Figure 1: Gmex GUI with dialogue window to select the geometry to be visualized.',
   urlFix=False) 
}}

Alternatively you can feed the geometry to `gmex` directly via the command line. For example:

``` bash
./gmex myGeometry.db
``` 

Once the geometry is selected, by clicking on the `Geo`tab on the left, gmex will diplay a list of all the subdetectors present in the loaded geometry, as shown in Figure 2
{{ imgutils_image_caption('Fig 2.png', 
   alt='Subdetectors', 
   cap='Figure 2: List of all the subdetectors present in the loaded geometry.',
   urlFix=False) 
}}

In order to visualize all or some of them you have to click on the respective checkbox and click on the `eye icon` on the right (3rd icon from the top, in the right panel).

If you want to display every top-level volume at once, use the `Enable All Volumes` button at the top of the `Subsystems` panel in the `Geo` window. This button checks all available volume entries in the list below it, enabling their visualization without having to select each subsystem checkbox individually.

{{ imgutils_image_caption('Fig 3.png', 
   alt='SelectedGeometry', 
   cap='Figure 3: Visualization of the selected geometry.',
   urlFix=False) 
}}

Now that you have loaded your geometry you can start investigating it. A good way to improve the visualization experience is to apply Phi sector cutouts. To do that you have to click on the `Display` tab under `Geo` (Figure 4), unselect a few of the `phi sector cutaways` (Fig 4a) and then click `Close`. 

{{ imgutils_image_caption('Fig 4.png', 
   alt='DisplayTab', 
   cap='Figure 4: `Display` tab under `Geo`.',
   urlFix=False) 
}}

{{ imgutils_image_caption('Fig 4a.png', 
   alt='PhiSector', 
   cap='Figure 4a: Select/unselect the `phi sector cutaways`.',
   urlFix=False) 
}}

At this point you have a cutaway view of your detector (Fig 5) and you can start investigating it by "opening" it. 

{{ imgutils_image_caption('Fig 5.png', 
   alt='Cutaway', 
   cap='Figure 5: Cutaway view of the ATLAS detector.',
   urlFix=False) 
}}

In order to do so you can follow the instructions written on the bottom part of the left panel. For instance, if you want to expand to child volumes you will have to use the following combination 'Ctrl/Cmd'+click. In this way you will remove the first layer of your geometry and be able to inspect its content (see Fig. 6). 

{{ imgutils_image_caption('Fig 6.png', 
   alt='TrackDisplay', 
   cap='Figure 6: Cutaway view of the ATLAS detector without the first mother volume layer.',
   urlFix=False) 
}}
 
