mkdir data
mkdir data\lib

xcopy ..\..\lib\beaglebone data\lib\beaglebone /s /e /h
xcopy ..\..\lib\jetson data\lib\jetson /s /e /h
xcopy ..\..\lib\linux data\lib\linux /s /e /h
xcopy ..\..\lib\mac data\lib\mac /s /e /h
xcopy ..\..\lib\raspberry-pi data\lib\raspberry-pi /s /e /h
xcopy ..\..\lib\windows data\lib\windows /s /e /h
