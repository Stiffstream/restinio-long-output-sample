This repository contains a simple RESTinio-based HTTP-server that responds with arbitrary long responses to incoming request at requested speed.

# How To Obtain And Try?

## Prerequisites

A C++ complier with support of C++14.

## How To Obtain?

This repository contains only source codes of the examples. RESTinio's source code as well as source code of RESTinio's dependencies is not included into the repository. There are two ways to get the examples and all necessary dependencies.

### Download The Full Archive

There is a [Downloads section](https://bitbucket.org/sobjectizerteam/restinio-long-output-sample/downloads/). It contains archives with all source codes (it means that an archive contains sources of the examples and sources
of all necessary subprojects). The simpliest way is to download a corresponding archive, unpack it, go into
`restinio-long-output-sample/dev`, then compile and run.

### Use MxxRu::externals

It this case you need to have Ruby + MxxRu + various utilities which every Linux/FreeBSD/macOS-developer usually have (like git, tar, unzip and stuff like that). Then:

1. Install Ruby, RubyGems and Rake (usually RubyGems is installed with Ruby but sometimes you have to install it separatelly).
2. Install MxxRu: `gem install Mxx_ru`
3. Do hg clone: `hg clone https://bitbucket.org/sobjectizerteam/restinio-long-output-sample`
4. Go into appropriate folder: `cd restinio-long-output-sample`
5. Run command `mxxruexternals`
6. Wait while add dependencies will be downloaded.

Then go to `dev` subfolder, compile and run.

## How To Try?

### Building With CMake

A well known chain of actions:

~~~~~
cd restinio-long-output-sample/dev
mkdir cmake_build
cd cmake_build
cmake -DCMAKE_INSTALL_PREFIX=target -DCMAKE_BUILD_TYPE=release ..
cmake --build . --config Release
cmake --build . --config Release --target install
~~~~~

### Building With MxxRu

The following chain of actions is necessary for building with MxxRu:

~~~~~
cd restinio-long-output-sample/dev
ruby build.rb
~~~~~

