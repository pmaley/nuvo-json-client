nuvo-json-client
================

Cross platform controller app for json API


Steps to build:

1) Clone repo: https://github.com/turnerba/nuvo-json-client.git

2) Download QT 5: qt-project.org/downloads

3) Install QT 5
  - OS X: Double click installer
	- Ubuntu:	'chmod a+x' .run file
				run from command line

4) Install Bonjour library (Linux only)
  - Download Bonjour: http://www.macosforge.org
  	- http://www.opensource.apple.com/tarballs/mDNSResponder/mDNSResponder-320.10.80.tar.gz
  - Build:
  	- untar /mDNSResponder-320.10.80.tar.gz	
  	- cd to 'mDNSResponder-320.10.80/mDNSPosix/'
  	- Run "make os=linux install"


5) Open basiclayouts.pro in QT Creator

6) In QT Creator, click "configure project" button

7) From build menu, select "Run"



Hotkeys:
ctrl+D	Toggle debug mode (hide/show console and connection window)
ctrl+K	Clear console window
