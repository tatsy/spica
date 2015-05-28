spica
===

[![Build Status](https://travis-ci.org/tatsy/spica.svg?branch=master)](https://travis-ci.org/tatsy/spica)
[![Coverage Status](https://coveralls.io/repos/tatsy/spica/badge.svg?branch=master)](https://coveralls.io/r/tatsy/spica?branch=master)

> **spica** is a cross-platform physically-based renderer that is written in C++.

## Renderer

* Monte-Carlo path tracing
* Bidirectional path tracing
* Metropolis light transport (Kelemen style)
* Photon mapping
* Stochastic progressive photon mapping

## Materials

* Lambertian
* Specular
* Complete refract
* Subsurface scattering (dipole diffusion)

## Sampler

* Mersenne twister
* Quasi Monte Carlo (permuted Halton sequence)

#### Future support

* Precomputed radiance transfer

## Data structure for rendering acceleration

* K-D tree
* QBVH (with SIMD)

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

## License

* MIT license 2015, Tatsuya Yatagawa (tatsy).
