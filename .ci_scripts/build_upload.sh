#!/bin/bash

set -e

mkdir deploy
build_dir="$(pwd)/build"

VERSION=$(cat res/version.txt)
if [[ "$GITHUB_REF" =~ ^refs/tags/v ]]
then
  REPO=release
elif [[ "$GITHUB_REF" == "refs/heads/master" ]]
then
  REPO=development
elif [[ "$GITHUB_REF" =~ ^refs/pull/ ]]
then
  PR_ID=${GITHUB_REF##refs/pull/}
  PR_ID=${PR_ID%%/merge}
  VERSION=pr-$PR_ID-$VERSION
else
  #echo "Unknown branch type $GITHUB_REF - skipping upload"
  REPO=release
  VERSION=6.6.6
fi

DEPLOY_FILE=
case "$DEPLOY" in
"linux")
  PACKAGE=linux
  DEPLOY_FILE=augustus-$VERSION-linux-x86_64.zip
  cp "${build_dir}/augustus.zip" "deploy/$DEPLOY_FILE"
  ;;
"flatpak")
  PACKAGE=linux-flatpak
  DEPLOY_FILE=augustus-$VERSION-linux.flatpak
  flatpak build-export export repo
  flatpak build-bundle export augustus.flatpak com.github.keriew.augustus --runtime-repo=https://flathub.org/repo/flathub.flatpakrepo
  cp augustus.flatpak "deploy/$DEPLOY_FILE"
  ;;
"vita")
  PACKAGE=vita
  DEPLOY_FILE=augustus-$VERSION-vita.vpk
  cp "${build_dir}/augustus.vpk" "deploy/$DEPLOY_FILE"
  ;;
"switch")
  PACKAGE=switch
  DEPLOY_FILE=augustus-$VERSION-switch.nro
  cp "${build_dir}/augustus.nro" "deploy/$DEPLOY_FILE"
  ;;
"appimage")
  PACKAGE=linux-appimage
  DEPLOY_FILE=augustus-$VERSION-linux.AppImage
  cp "${build_dir}/augustus.AppImage" "deploy/$DEPLOY_FILE"
  ;;
"mac")
  PACKAGE=mac
  DEPLOY_FILE=augustus-$VERSION-mac.dmg
  cp "${build_dir}/augustus.dmg" "deploy/$DEPLOY_FILE"
  ;;
"android")
  PACKAGE=android
  if [ -f "${build_dir}/augustus.apk" ]
  then
    DEPLOY_FILE=augustus-$VERSION-android.apk
    cp "${build_dir}/augustus.apk" "deploy/$DEPLOY_FILE"
  fi
  ;;
"emscripten")
  PACKAGE=emscripten
  if [ -f "${build_dir}/augustus.html" ]
  then
    DEPLOY_FILE=augustus-$VERSION-emscripten.html
    cp "${build_dir}/augustus.html" "deploy/$DEPLOY_FILE"
  fi
  ;;
*)
  echo "Unknown deploy type $DEPLOY - skipping upload"
  exit
  ;;
esac

if [ ! -z "$SKIP_UPLOAD" ]
then
  echo "Build is configured to skip deploy - skipping upload"
  exit
fi

if [ -z "$REPO" ] || [ -z "$DEPLOY_FILE" ]
then
  echo "No repo or deploy file found - skipping upload"
  exit
fi

if [ -z "$UPLOAD_TOKEN" ]
then
  echo "No upload token found - skipping upload"
  exit
fi

curl -u "$UPLOAD_TOKEN" -T deploy/$DEPLOY_FILE https://augustus.josecadete.net/upload/$REPO/$PACKAGE/$VERSION/$DEPLOY_FILE
echo "Uploaded. URL: https://augustus.josecadete.net/$REPO.html" 
