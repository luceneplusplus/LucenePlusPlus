Build Instructions
==========

You'll need the following dependencies installed on your system.

- [ZLib](https://zlib.net/)
- [Boost](http://www.boost.org) libraries::
    - date-time
    - filesystem
    - regex
    - thread
    - iostreams 

e.g. on Debian systems, the following packages are required:
- zlib1g-dev
- libboost-date-time-dev
- libboost-filesystem-dev
- libboost-regex-dev
- libboost-thread-dev
- libboost-iostreams-dev


Build Instructions for linux systems
--------------------------------------

To build the library the following commands should be issued::

    $ mkdir build; cd build
    $ cmake ..
    $ make
    $ make install

Build Instructions for Windows systems
--------------------------------------

Once you have installed the dependencies and added the installation 
location to your `CMAKE_PREFIX_PATH`, open cmake-gui and configure the 
build. When building on windows, ensure that the `ENABLE_CYCLIC_CHECK`
option is set to `true`.

Next, open the visual studio project with the 'open project' button. the
project is built using the `ALL_BUILD` solution in the projects column.
If you would like to install the project, build the `INSTALL` solution 
after the fact.

**
Note: if you wish to install the Lucene++ library to a protected area, you
must re-open the visual studio project as an administrator
**

**
Note: "BOOST_ROOT" environment variable must be defined to point to the 
Boost library directory (eg. c:\\local\\Boost). cmake should automatically
find the installed libraries if they are installed within that path; 
e.g. C:\\local\\Boost\\lib64-msvc-14.2 
**
