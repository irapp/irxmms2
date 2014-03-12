README
=======================================
1.Introduction

apt-get install lirc liblircclient-dev xmms2 xmms2-dev libxmmsclient-dev  
cd /usr/include && ln -s xmms2/* ./  
wget https://github.com/irapp/irxmms2/archive/master.zip  
unzip master.zip  
gcc -o irxmms2 -llirc_client -lxmmsclient irxmms2.c  
xmms2d  
./irxmms2 /etc/lirc/lircrc_irxmms2  

2.Update

2014030501 add this repo and test program  

