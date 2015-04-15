#!/bin/sh

# copy rendered image to result directory
convert ./simplept.ppm ./results/simplept.jpg

if [ "$TRAVIS_PULL_REQUEST" = "false" ]; then
  echo -e "Starting to update gh-pages\n"

  git config --global user.email "travis@travis-ci.org"
  git config --global user.name "Travis CI"

  git checkout gh-pages

  git add -u
  git commit -m "Travis build $TRAVIS_BUILD_NUMBER pushed to gh-pages"
  git remote set-url origin https://github.com/tatsy/rainy.git
  git push -fq origin gh-pages > /dev/null
fi
