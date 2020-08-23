Lucene++
==========

Welcome to lucene++ version **3.0.7**.

Lucene++ is an up to date C++ port of the popular Java [Lucene](http://lucene.apache.org/)
library, a high-performance, full-featured text search engine.


Components
----------------

- liblucene++ library
- liblucene++-contrib library
- lucene++-tester (unit tester)
- deletefiles (demo)
- indexfiles (demo)
- searchfiles (demo)


Useful Resources
----------------

Official [Java Lucene](http://lucene.apache.org/java/docs/index.html) - useful links and 
documentation relevant to Lucene and lucene++. [Lucene in Action](https://www.amazon.com/dp/1932394281/) 
by Otis Gospodnetic and Erik Hatcher.


Build Instructions
------------------

You'll need the [Boost](http://www.boost.org) libraries installed somewhere.

On Debian systems, the following packages are required:
- zlib1g-dev
- libboost-date-time-dev
- libboost-filesystem-dev
- libboost-regex-dev
- libboost-thread-dev
- libboost-iostreams-dev

To build the library the following commands should be issued::

    $ mkdir build; cd build
    $ cmake ..
    $ make
    $ make install


Build Instructions for Windows systems
--------------------------------------

Once installing the dependencies, open cmake-gui and configure the build.
When building on windows, ensure that the `ENABLE_CYCLIC_CHECK` option
is set to `true`.

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


To run unit test suite
----------------------

lucene_tester is built using the [Google Testing Framework](https://code.google.com/p/googletest/).
you can run the test suite on unix with the following command run from the
repository root::
```
    $ build/src/test/lucene++-tester
```

the test suite can also be run from the repository root on NT systems, but the required DLL
files must manually be copied into the test binary path before executing, otherwise you will
recieve errors telling you that required libraries cannot be found.
```
    $ build/src/test/lucene++-tester
```

Command options can be discovered by supplying `--help`.




To run the demos
----------------

Start by indexing a directory of files - open a command prompt and run
```
    ./indexfiles <directory to index> <directory to store index>
```	
Once the indexer has finished, you can query the index using searchfiles
```
    ./searchfiles -index <directory you stored the index in>
```
This uses an interactive command for you to enter queries, type a query to search the index press enter and you'll see the results.
	

Acknowledgements
----------------

- Ben van Klinken and contributors to the CLucene project for inspiring this project.
- md5 Copyright (C) 1999, 2000, 2002 Aladdin Enterprises
- `Unicode character properties (guniprop)[http://library.gnome.org/devel/glib/] Copyright (C) 1999 Tom Tromey, Copyright (C) 2000 Red Hat, Inc.
- `Cotire (compile time reducer)[https://github.com/sakra/cotire] by Sascha Kratky.
