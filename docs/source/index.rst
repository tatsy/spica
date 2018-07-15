.. spica documentation master file, created by
   sphinx-quickstart on Thu Nov 24 15:29:46 2016.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to spica's documentation!
=================================

The project **spica** is for a cross-platform physically based rendering engine.
This program employs a plug-in system and is easy-to-build for the user convenience!

Installation
************

Requirements
------------

The project is implemented and tested with the following compilers that supports C++17.

* GNU C++ compiler (v7.3.0 or higher)
* Microsoft Visual C++ compiler (MSVC 2017)

I confirmed the programs can be build on the following environments. The build will be successful also on other environments if you compile them with the compilers above.

* Windows 10 64bit
* MacOS X High Sierra
* Ubuntu 16.04 LTS

Dependencies
------------

All the dependencies are included by Git's submodule system (and they can be immediately installed!).

Build command
-------------

You can build the programs with **CMake** (v3.6.0 or higher). For UNIX users, the following command will build them.

.. code-block:: shell

  $ git clone --depth=10 https://github.com/tatsy.spica.git
  $ cd spica && mkdir build && cd build
  $ cmake -D CMAKE_BUILD_TYPE=Release -D SPICA_BUILD_MAIN=ON [-D WITH_SSE=ON] ..
  $ make -j4
  $ make install  # No sudo required!

Usage
*****

Executable binary **spica** is made by the build typically in ``build/bin``. In this directory, run **spica** as the following command.

.. code-block:: shell

  $ ./spica --input INPUT XML [--threads NUM_THREADS]

You can find sample scene files (written with XML) in `GitHub <https://github.com/tatsy/spica/blob/master/scenes/README.md>`_.
The scene files can be written as that of `Mitsuba renderer <https://www.mitsuba-renderer.org/>`_ (but only selected functions are supported).

Other resources
***************

* `Class reference <./reference/index.html>`_


Indices and tables
******************

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
