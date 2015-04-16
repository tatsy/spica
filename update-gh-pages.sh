#!/bin/sh


if [ "$TRAVIS_PULL_REQUEST" = "false" ]; then
  echo -e "Starting to update gh-pages\n"

  ls -l
  convert ./simplept.ppm $HOME/simplept.jpg

  cd $HOME
  git config --global user.email "travis@travis-ci.org"
  git config --global user.name "Travis CI"

  git clone --quiet --branch=gh-pages https://${GH_TOKEN}@github.com/tatsy/spica.git gh-pages > /dev/null

  cd gh-pages
  cp $HOME/simplept.jpg results

  git add -u
  git commit -m "Travis build $TRAVIS_BUILD_NUMBER pushed to gh-pages"
  git push -fq origin gh-pages > /dev/null
fi
