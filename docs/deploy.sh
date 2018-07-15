#!/bin/sh

########################################
# Deploy documentation to GitHub pages #
########################################

set -x

#
## Git user setting
#
git config --global user.name "Travis CI"
git config --global user.email "travis@travis-ci.org"

#
## Run Doxygen and Sphinx docs
#
rm -rf html/*

doxygen doxyfile
make html
mv build/html html
mv classref html/reference

#
## Deploy GitHub pages
#
cd html
git checkout -b gh-pages
git add --all .
git commit -m "Travis build No.$TRAVIS_BUILD_NUMBER commit to GitHub Pages"
git push --force --quiet "https://$GH_TOKEN@github.com/tatsy/spica.git" gh-pages:gh-pages 2> /dev/null

set +x
