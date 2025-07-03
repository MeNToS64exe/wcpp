#!/bin/bash
set -e

make
sudo make install

echo "Installed to /usr/bin/wcpp"

