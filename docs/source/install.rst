Installation Guide
===================

Download sources
-----------------

Please download the sources from |github_link|.

.. code-block:: shell

  git clone https://github.com/tatsy/spica.git

.. |github_link| raw:: html

  <a href="https://github.com/tatsy/lime.git" target="_blank">GitHub</a>

Build library
--------------

This project uses C++11/14. The build is tested under following environments.

* GNU C compiler (v4.9.0 or higher)
* LLVM Clang (v3.7.0 or higher)
* Xcode 8.0
* Microsoft Visual C++ compiler (MSVC 2015)

Requirements
**************

This project uses **Boost C++ Libraries** for XML parsing. You can download required package
by ``git submodule update`` or you can specify your own boost library which is already
installed in your computer.

* Boost 1.60.0 (property_tree)

The optional GUI for the renderer is programmed with **Qt 5.5**. Therefore,
you should download and install Qt 5.5 or higher to use the GUI.

Command
*********

.. code-block:: shell

  $ git clone https://github.com/tatsy.spica.git
  $ cd spica && mkdir build && cd build
  $ cmake  ..
  $ make
