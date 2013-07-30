Lucene++
==========

Welcome to lucene++ version **3.0.4**.

Lucene++ is an up to date C++ port of the popular Java `Lucene <http://lucene.apache.org/>`_ library, a high-performance, full-featured text search engine.


Components
----------------

- liblucene++ library
- liblucene_contrib library
- lucene_tester (unit tester)
- deletefiles (demo)
- indexfiles (demo)
- searchfiles (demo)


Useful Resources
----------------

Official `Java Lucene <http://lucene.apache.org/java/docs/index.html>`_ - useful links and documentation relevant to Lucene and lucene++.
`Lucene in Action <http://www.amazon.com/Lucene-Action-Otis-Gospodnetic/dp/1932394281/ref=sr_1_1?ie=UTF8&s=books&qid=1261343174&sr=8-1>`_ by Otis Gospodnetic and Erik Hatcher.


Build Instructions using CMake
------------------------------

You'll need the `Boost <http://www.boost.org>`_ libraries installed somewhere.

On Debian systems, the following packages are required:

- libboost-date-time-dev
- libboost-filesystem-dev
- libboost-regex-dev
- libboost-thread-dev
- libboost-iostreams-dev
- libboost-test-dev

.. note::

   At present, you must use Boost 1.49 or older. There is an incompatibility
   to Boost 1.50 and newer that causes ``make`` to fail. See `issue #30`__.

__ https://github.com/luceneplusplus/LucenePlusPlus/issues/30

To build the library the following commands should be issued::

    $ cmake .
    $ make
    $ make install

To build the demo programs, execute the following after having first built
the library::

    $ make indexfiles searchfiles deletefiles


Build Instructions using Waf
------------------------------

**After running CMake** you can use `Waf <http://code.google.com/p/waf/>`_ to drive the build. Waf requires that you have a recent version of `Python <http://python.org>`_ installed on your system.

To build the library the following commands should be issued::

    $ ./waf configure
    $ ./waf --static build


Additionally static builds of the following libraries are required for a successful build:

- boost::date_time
- boost::filesystem
- boost::regex
- boost::thread
- boost::system
- boost::iostreams
- boost::unit_test_framework

The libraries and headers should be made available at a standard prefix (``/usr/local`` for example).


Build Instructions for Windows systems
--------------------------------------

Open solution lucene++.sln located in the *msvc* folder into Visual Studio 2008 and build.

**Note: "BOOST_ROOT" environment variable must be defined to point to the Boost library directory (eg. c:\\boost_1_44_0)**

You'll need Boost installed.

`BoostPro <http://www.boostpro.com>`_ has some precompiled Windows packages. You'll need the following extras installed::

- boost::system
- boost::thread
- boost::filesystem
- boost::regex
- boost::date_time
- boost::iostreams
- boost::unit_test_framework


Building Performance
--------------------

Use of ccache will speed up build times a lot. I found it easiest to add the ``/usr/lib/ccache`` directory to the beginning of your paths. This works for most common compilers::

    PATH=/usr/lib/ccache:$PATH


To run unit test suite
----------------------

lucene_tester is built using the `Boost Unit Test Framework <http://www.boost.org/doc/libs/1_44_0/libs/test/doc/html/index.html>`_ and is launched by the following command::

    $ bin/lucene_tester --show_progress=yes

Other `command options <http://www.boost.org/doc/libs/1_44_0/libs/test/doc/html/utf/user-guide/runtime-config/reference.html>`_ can be supplied.


Acknowledgements
----------------

- Ben van Klinken and contributors to the CLucene project for inspiring this project.
- Jamie Kirkpatrick for cross-platform and waf build support.

- `nedmalloc <http://sourceforge.net/projects/nedmalloc/>`_ Copyright 2005-2006 Niall Douglas
- md5 Copyright (C) 1999, 2000, 2002 Aladdin Enterprises
- `Unicode character properties (guniprop) <http://library.gnome.org/devel/glib/>`_ Copyright (C) 1999 Tom Tromey, Copyright (C) 2000 Red Hat, Inc.
