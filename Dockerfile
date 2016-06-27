#
## Load docker image
#
FROM tatsy/ubuntu-cxx:default

#
## Environment variables
#
ENV TERM xterm
ENV BRANCH_NAME @BRANCH_NAME@
ENV PULL_REQUEST @PULL_REQUEST@
ENV CC @C_COMPILER@
ENV CXX @CXX_COMPILER@

#
## Install Gcovr
#
RUN apt-get -qq install python-pip
RUN pip install gcovr

#
## Install cppcheck, cccc, and doxygen
#
RUN apt-get -qq install cppcheck cccc doxygen

#
## Install Boost
#
RUN wget https://sourceforge.net/projects/boost/files/boost/1.61.0/boost_1_61_0.tar.gz/download
RUN tar zxf download
RUN \
  cd boost_1_61_0 && \
  ./bootstrap.sh --with-libraries=system,filesystem && \
  ./b2 --libdir=/usr/lib/x86_64-linux-gnu/ --includedir=/usr/include/ -d0 -j2 install

#
## Install freeglue and glew
#
RUN apt-get -qq install freeglut3-dev libglew-dev
RUN apt-get -qq install libxmu-dev libxi-dev

#
## Build spica
#
RUN git clone --depth 10 -b $BRANCH_NAME https://github.com/tatsy/spica.git
RUN \
  if [ $PULL_REQUEST != "false" ]; then \
    cd spica && \
    git fetch origin +refs/pull/$PULL_REQUEST/merge && \
    git checkout --quiet --force FETCH_HEAD; \
  fi

RUN \
  cd spica && \
  git submodule update --init --recursive && \
  cmake -D SPICA_BUILD_TEST=ON . && \
  cmake --build .

#
## # of threads used by OpenMP
#
ENV OMP_NUM_THREADS 4

#
## Define working direcotry
#
WORKDIR /root/spica

#
## Run unit tests
#
RUN lcov --directory . --zerocounters
