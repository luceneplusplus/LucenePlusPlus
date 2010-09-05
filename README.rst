Lucene++
==========

Welcome to lucene++ version **3.0.2**.

Lucene++ is an up to date C++ port of the popular Java Lucene library, a high-performance, full-featured text search engine.


Components
----------------

- liblucene++ library
- liblucene_contrib library
- lucene_tester
- deletefiles
- indexfiles
- searchfiles


Useful Resources
----------------

Official `Java Lucene <http://lucene.apache.org/java/docs/index.html>`_ - useful links and documentation relevant to Lucene and lucene++.
`Lucene in Action <http://www.amazon.com/Lucene-Action-Otis-Gospodnetic/dp/1932394281/ref=sr_1_1?ie=UTF8&s=books&qid=1261343174&sr=8-1>`_ by Otis Gospodnetic and Erik Hatcher.


Build Instructions for POSIX systems
------------------------------------

We use `Waf <http://code.google.com/p/waf/>`_ to drive the build. Waf requires that you have a recent version of `Python <http://python.org>`_ installed on your system.  

To build the library the following commands should be issued::

    $ ./waf configure
    $ ./waf build


Additionally static builds of the following libraries are required for a successful build:

- boost::system
- boost::thread
- boost::filesystem
- boost::regex
- boost::date_time
- boost::unit_test_framework

The libraries and headers should be made available at a standard prefix (/usr/local for example).


Build Instructions for Windows systems
--------------------------------------

Open solution lucene++.sln located in the *msvc* folder into Visual Studio 2008 and build.

**Note: "BOOST_ROOT" environment variable must be defined to point to the boost library directory (eg. C:\boost_1_41_0)**


To run unit test suite
----------------------

lucene_tester is built using the `Boost Unit Test Framework <http://www.boost.org/doc/libs/1_44_0/libs/test/doc/html/index.html>`_ and is launched by the following command::

    $ bin/default/lucene_tester --test_dir=./test/testfiles --show_progress=yes

Other `command options <http://www.boost.org/doc/libs/1_44_0/libs/test/doc/html/utf/user-guide/runtime-config/reference.html>`_ can be supplied.


Acknowledgements
----------------

- Ben van Klinken and contributors to the CLucene project for inspiring this project.
- Jamie Kirkpatrick for cross-platform and waf build support.

- `Zlib <http://www.zlib.net>`_ Copyright (C) 1995-2010 Jean-loup Gailly and Mark Adler
- `UTF8-CPP <http://utfcpp.sourceforge.net/>`_ Copyright 2006 Nemanja Trifunovic
- `nedmalloc <http://sourceforge.net/projects/nedmalloc/>`_ Copyright 2005-2006 Niall Douglas
- md5 Copyright (C) 1999, 2000, 2002 Aladdin Enterprises
- `Unicode character properties (guniprop) <http://library.gnome.org/devel/glib/>`_ Copyright (C) 1999 Tom Tromey, Copyright (C) 2000 Red Hat, Inc.
