#!/usr/bin/env bash
sudo apt-get install libreadline-dev
sudo apt-get install python-lxml
sudo apt-get install python-qt4
sudo apt-get install scons
sudo apt-get install screen
sudo apt-get install uml-utilities
sudo apt-get install openssh-server
mkdir -p ~/downloads
cd ~/downloads
wget http://libslack.org/download/libslack-0.6.tar.gz
tar xzf libslack-0.6.tar.gz
cd libslack-0.6
make
sudo make install
cd ~/downloads
git clone git clone git://github.com/anrl/gini.git
export GINI_HOME=/usr/local
cd gini/source
scons
sudo scons install
