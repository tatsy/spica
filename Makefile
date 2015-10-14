CC	       = clang
CXX        = clang++
GIT_BRANCH = development

all: info build run testing coverage cppcheck cccc clean

info:
	docker info

build:
	docker pull tatsy/ubuntu-cxx; \
	sed -e "s/@C_COMPILER@/$(CC)/;s/@CXX_COMPILER@/$(CXX)/;s/@BRANCH_NAME@/$(GIT_BRANCH)/" Dockerfile.in > Dockerfile; \
	docker build --tag=spica-env .

rebuild:
	docker pull tatsy/ubuntu-cxx; \
	sed -e "s/@C_COMPILER@/$(CC)/;s/@CXX_COMPILER@/$(CXX)/;s/@BRANCH_NAME@/$(GIT_BRANCH)/" Dockerfile.in > Dockerfile; \
	docker build --no-cache --tag=spica-env .

run:
	docker run --name=spica-env --env="CTEST_OUTPUT_ON_FAILURE=TRUE" -itd spica-env

testing:
	docker exec spica-env make check

coverage:
	docker exec spica-env gcovr --xml --output="gcovr.xml" --gcov-executable=llvm-cov -r "./src/"; \
	docker exec spica-env cat gcovr.xml > gcovr.xml

cppcheck:
	docker exec spica-env cppcheck --enable=all --xml src 2> cppcheck.xml

cccc:
	docker exec spica-env cccc src/**/*.cc src/**/*.h; \
	docker exec spica-env cat .cccc/cccc.xml > cccc.xml

clean:
	docker stop spica-env; \
	docker rm spica-env; \
	docker rmi spica-env
