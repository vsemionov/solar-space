===========
Solar Space
===========
------------------------------------------------------------------------------------------
A three-dimensional, animated screen saver for Windows, showing a view of the Solar System
------------------------------------------------------------------------------------------


Overview
========
Solar Space is a three-dimensional, animated screen saver for Windows, showing a view of the Solar System. It is written in C++ and uses the OpenGL and FreeType libraries. Its features are:

* Visually pleasing - high-resolution realistic graphic textures and models, effects like lens flares, reflections from oceans and ice, semi-transparent cloud layers, etc. The appearance and proportions of objects were carefully designed for a balance between realism and aesthetics.
* Complex camera movement - the camera tracks either the whole Solar system or individual objects for a predefined period of time. Afterwards, it randomly picks a new view, smoothly moves to its destination position, and adjusts its orientation and field of view (zoom).
* Complete - about 20 objects of different types: the Sun, all planets and most of their well-known moons, rings, irregularly-shaped bodies like Phobos and Deimos.
* A background scattered with stars - each star on the background corresponds to a real star. The star data is acquired from Yale Bright Star Catalogue, containing information about more than 9000 stars.
* Highly customizable - all objects and their properties can be modified by editing the planetary system definition files.
* Extensible - besides the Solar system, new planetary systems can easily be added by creating a package containing the object hierarchy definition, the textures and models, and the star map file. The active system is selected from the program's configuration interface, integrated with the screen saver settings in the Windows control panel.
* Easy to install - the whole package is distributed as a single-file installer, which presents the user with a well-known installer wizard interface.


System Requirements
===================
For platform support and acceptable performance, Solar Space requires the following:

* Windows XP Service Pack 2 or later operating system with Internet Explorer 4 or later (both required as a minimum)
* AMD Athlon XP 1800+ or equivalent processor
* 512 MB RAM
* 12 MB free disk space (minimum)
* NVIDIA GeForce4 MX 440 or equivalent video card with 32 MB of memory

Note that most of the hardware listed above is not a strict minimum.


Known Issues
============
On some systems, artifacts may appear when rendering distant objects that are very close to each other. If one such object is obscured by another one, polygons from the former may bleed through the latter. For example, the far side of a planet's rings may appear visible, when it should be hidden behind the planet. This effect, known az z-fighting, is a result of insufficient z-buffer precision.


Other Information
=================
Solar Space is written and maintained by Victor Semionov <vsemionov@gmail.com>.
Solar Space is licensed under the BSD license.

For the latest information and downloads, please visit:
http://www.vsemionov.org/solar-space/

The development website is at:
https://bitbucket.org/vsemionov/solar-space
