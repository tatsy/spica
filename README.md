spica
===

[![Build Status](https://travis-ci.org/tatsy/spica.svg?branch=master)](https://travis-ci.org/tatsy/spica)
[![Coverage Status](https://coveralls.io/repos/tatsy/spica/badge.svg?branch=master)](https://coveralls.io/r/tatsy/spica?branch=master)

> **spica** is a cross-platform physically-based renderer written with C++.

## Overview

#### Renderer

* Monte-Carlo path tracing
* Bidirectional path tracing
* Metropolis light transport (Kelemen style)
* Photon mapping
* Stochastic progressive photon mapping

#### Materials

* Lambertian BRDF
* Phong BRDF
* Specular BRDF
* Refractive object
* Subsurface scattering (dipole diffusion)

#### Sampler

* Pseudo random number generator (Mersenne Twister)
* Quasi Monte Carlo (permuted Halton sequence)

#### Data structures

* K-D tree
* QBVH (accelerated with SIMD)

## Build

This project uses C++11. The build is tested under following environments.

* GNU C compiler (v4.9.0 or higher)
* LLVM Clang (v3.7.0 or higher)
* Microsoft Visual C++ compiler (MSVC 2015)

#### Command

```shell
$ git clone https://github.com/tatsy.spica.git
$ cmake -DENABLE_AVX=OFF -DSPICA_BUILD_TEST=OFF .
$ cmake --build .
```

## Results

#### Path tracing

<img src="./results/pathtrace.jpg" width="480" />

#### Bidirectional path tracing

<img src="./results/bdpt.jpg" width="480" />

#### Metropolis light transport

<img src="./results/mlt.jpg" width="480" />

#### Photon mapping

<img src="./results/photonmap.jpg" width="480" />

#### Progressive photon mapping

<img src="./results/sppm.jpg" width="480" />

#### Subsurface scattering (Stochastic progressive photon mapping)

<img src="./results/subsurface_sppm.jpg" width="480" />

## Acknowledgment

The author sincerely thanks for HDR images provided by sIBL Archive [http://www.hdrlabs.com/sibl/archive.html](http://www.hdrlabs.com/sibl/archive.html). These images are licensed under the Creative Commons Attribution-Noncommercial-Share Alike 3.0 License.

## License

* MIT license 2015, Tatsuya Yatagawa (tatsy).
