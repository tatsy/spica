#!/bin/sh

########################################
# Deploy documentation to GitHub pages #
########################################

#
## Git user setting
#
git config --global user.name "Travis CI"
git config --global user.email "travis@travis-ci.org"

#
## Run Doxygen
#
doxygen doxyfile
cd html

#
## Deploy GitHub pages
#
git add --all .
git checkout -b gh-pages
git commit -m "Travis build No.$TRAVIS_BUILD_NUMBER commit to GitHub Pages"
git push --force --quiet "https://$GH_TOKEN@github.com/tatsy/spica.git" gh-pages:gh-pages
