**spica** is the cross-platform rendering engine written by modern C++

### Features

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

### Build

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


### Git Repo

<a href="https://github.com/tatsy/spica.git" title="spica - GitHub" target="_blank">spica - GitHub</a>

### Copyright

MIT Lisence 2015, (c) Tatsuya Yatagawa (tatsy)
