# open-microscopy
Microscope control software for our Open Microscopes

This is the code for a stand alone dummy open microscope.
It can be used as the basis for a new microscope.
The code for many components are included but not all are compiled into this base version, they can be included or changed as required.

Compiling.

To create a visual studio project file you must first download and install.
cmake 2.6
http://www.cmake.org/files/v2.6/cmake-2.6.1-win32-x86.exe

Run the cmake executable specifing the location of the CMakeLists.txt file
ie
ATD_Microscopy\Microscope Systems\OpenMicroscope\trunk

Fill out the locations of the dependencies and then generate the project files.


Note.

The debug build from visual studio requires the debug python library
These can be found on in ATD_Dlls and should be in your PATH, and used by CMAKE.
