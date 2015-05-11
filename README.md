spica
===

[![Build Status](https://travis-ci.org/tatsy/spica.svg?branch=master)](https://travis-ci.org/tatsy/spica)
[![Coverage Status](https://coveralls.io/repos/tatsy/spica/badge.svg?branch=master)](https://coveralls.io/r/tatsy/spica?branch=master)

> **spica** is a cross-platform physically-based renderer that is written in C++. 

## Renderer

* Monte-Carlo path tracing
* Bidirectional path tracing
* Metropolis light transport (Kelemen style)

#### Future support

* Photon mapping
* Precomputed radiance transfer

## Geometry

* Plane
* Disk
* Sphere
* Triangle
* Quad
* Trimesh

## Data structure for rendering acceleration

* K-D tree
* QBVH (with SIMD)

#### Future support

## Results

#### Path tracing

<img src="./results/simplept.jpg" width="480" />

#### Bidirectional path tracing

<img src="./results/simplebpt.jpg" width="480" />

#### Metropolis light transport

<img src="./results/simplemlt.jpg" width="480" />

## License

* MIT license 2015, Tatsuya Yatagawa (tatsy).
* The part of this code is largely inspired by [edupt](https://github.com/githole/edupt.git).
