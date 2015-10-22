# quaddamage

Another C++ raytracer for the v4 FMI raytracing course.
The course site is [http://raytracing-bg.net/](http://raytracing-bg.net/)

How to set it up on your machine
--------------------------------

1. Copy the appropriate for your OS qdamage-{win32,linux}.cbp to qdamage.cbp
   This step is important to avoid source-control thinking that you modified the project file

2. If you're using Windows:

   1. Download the SDL-devel package for mingw32 from [libsdl.org](http://libsdl.org) (Download > SDL 1.2)
   2. Unpack it anywhere (mine is in F:\develop\SDK\SDL-1.2.15) and create a substitution
      using subst:
                     
                     subst L: F:\develop\SDK
                     
      This will create a virtual drive L:, pointing to your F:\develop\SDK.
      Of course, on your machine the latter path can be different.
      You can put this command in a .bat file for easier execution on system start up.
   3. Copy SDL-1.2.15\bin\SDL.dll in the root directory of the raytracer (i.e., where the *.cbp
      files are).
   4. (If not yet done): copy qdamage-win32.cbp to qdamage.cbp and open it. You're all set.

3. If you're using Linux:
   
   1. Ensure you installed the gcc compiler, the SDL-development package and the OpenEXR
      development package ("SDL-devel" and "OpenEXR-devel" on Fedora-based distros,
      "libsdl-dev" and "libopenexr-dev" on Ubuntu et al).
   2. Copy qdamage-linux.cbp to qdamage.cbp and open it. You're all set.

