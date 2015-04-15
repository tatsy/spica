#!/bin/sh

# copy rendered image to result directory
convert ./image.ppm ./result/simplept.jpg

if [ "$TRAVIS_PULL_REQUEST" == "false" ]; then
  echo -e "Starting to update gh-pages\n"

  cd $HOME
  git config --global user.emal "travis@travis-ci.org"
  git config --global user.name "Travis CI"

  git checkout gh-pages

  git add -f .
  git commit -m "Travis build $TRAVIS_BUILD_NUMBER pushed to gh-pages"
  git push -fq origin gh-pages > /dev/null
fi
