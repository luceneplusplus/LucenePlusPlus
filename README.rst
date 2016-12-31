Lucene++
==========

Welcome to lucene++ version **3.0.7**.

Lucene++ is an up to date C++ port of the popular Java `Lucene <http://lucene.apache.org/>`_ library, a high-performance, full-featured text search engine.


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

Official `Java Lucene <http://lucene.apache.org/java/docs/index.html>`_ - useful links and documentation relevant to Lucene and lucene++.
`Lucene in Action <http://www.amazon.com/Lucene-Action-Otis-Gospodnetic/dp/1932394281/ref=sr_1_1?ie=UTF8&s=books&qid=1261343174&sr=8-1>`_ by Otis Gospodnetic and Erik Hatcher.


Build Instructions
------------------

You'll need the `Boost <http://www.boost.org>`_ libraries installed somewhere.

On Debian systems, the following packages are required:

- libboost-date-time-dev
- libboost-filesystem-dev
- libboost-regex-dev
- libboost-thread-dev
- libboost-iostreams-dev
- libboost-test-dev

To build the library the following commands should be issued::

    $ mkdir build; cd build
    $ cmake ..
    $ make
    $ make install


Build Instructions for Windows systems
--------------------------------------

Open solution lucene++.sln located in the *msvc* folder into Visual Studio 2010+ and build.

**Note: you will need to edit the include\Config.h.cmake file. Make a copy, rename it to Config.h, and replace**

  #@DEFINE_USE_CYCLIC_CHECK@ LPP_USE_CYCLIC_CHECK

with

  #define LPP_USE_CYCLIC_CHECK

**Note: "BOOST_ROOT" environment variable must be defined to point to the Boost library directory (eg. c:\\boost_1_57_0)**

Boost libs must be located in a subdirectory lib32-msvc-10.0 
This is the default name of the install directory from the sf.net  `Boost-binaries <http://sourceforge.net/projects/boost/files/boost-binaries/>`_ project.

You'll need Boost installed from http://boost.org.



To run unit test suite
----------------------

lucene_tester is built using the `Google Testing Framework <https://code.google.com/p/googletest/>`_ and is launched by the following command::

    $ build/src/test/lucene++-tester

Command options can be discovered by supplying `--help`.




To run the demos
----------------

Start by indexing a directory of files - open a command prompt and run

    indexfiles.exe <directory to index> <directory to store index>
	
Once the indexer has finished, you can query the index using searchfiles

    searchfiles.exe -index <directory you stored the index in>

This uses an interactive command for you to enter queries, type a query to search the index press enter and you'll see the results.
	

Acknowledgements
----------------

- Ben van Klinken and contributors to the CLucene project for inspiring this project.
- md5 Copyright (C) 1999, 2000, 2002 Aladdin Enterprises
- `Unicode character properties (guniprop) <http://library.gnome.org/devel/glib/>`_ Copyright (C) 1999 Tom Tromey, Copyright (C) 2000 Red Hat, Inc.
- `Cotire (compile time reducer) <https://github.com/sakra/cotire>`_ by Sascha Kratky.
