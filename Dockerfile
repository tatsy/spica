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
## Install freeglut and glew
#
RUN apt-get -qq install freeglut3 freeglut3-dev libglew-dev libglew1.5
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
