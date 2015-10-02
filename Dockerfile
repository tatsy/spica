#
## Load docker image
#
FROM tatsy/ubuntu-cxx

#
## Environment variables
ENV CC gcc
ENV CXX g++

#
## Install freeglut and glew
#
RUN apt-get -qq install freeglut3 freeglut3-dev libglew-dev libglew1.5
RUN apt-get -qq install libxmu-dev libxi-dev

#
## Install Google Test
#
RUN git clone --depth=1 -b release-1.7.0 https://github.com/google/googletest.git /usr/src/gtest
RUN \
  cd /usr/src/gtest && \
  cmake . && \
  cmake --build . && \
  mkdir -p /usr/local/lib && \
  mkdir -p /usr/include && \
  mv libg* /usr/local/lib && \
  mv include/* /usr/include && \
  cd /
ENV GTEST_LIBRARY /usr/local/lib/libgtest.a
ENV GTEST_MAIN_LIBRARY /usr/local/lib/libgtest_main.a
ENV GTEST_INCLUDE_DIRS /usr/include

#
## Build spica
#
RUN git clone --depth 10 -b develop https://github.com/tatsy/spica.git
RUN \
  cd spica && \
  cmake . && \
  cmake --build .

#
## # of threads used by OpenMP
#
ENV OMP_NUM_THREADS 4

#
## Define working direcotry
#
WORKDIR /root/spica
