kdb_irq
===

(GPL) Keyboard IRQ is a FREE kernel module.


This module replace the current keyboard handler and displays the keycode and  the key status either pressed or released (see dmesg).
This module is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY.


This module is by no means an application and has no other purpose than to demonstrate the use of IRQ handler of GNU Linux.



Instructions
============


Make targets:

     all: Build the module.
     clean: Clean the generated files.


Download the software :

     mkdir devel
     cd devel
     git clone git://github.com/Keidan/lkm_kdb_irq.git
     cd lkm_kdb_irq
     make


Insert/Remove the module:

     insmod ./kdb_irq.ko
     rmmod kdb_irq
	

Usage:

     After the module insertion, the current keyboard IRQ handler is removed and all the logs messages are printed into the current TTY.
     It's important to not forget that you need to reboot your computer to restore the normal behaviour.


License (like GPL)
==================

	You can:
		- Redistribute the sources code and binaries.
		- Modify the Sources code.
		- Use a part of the sources (less than 50%) in an other software, just write somewhere "nhm is great" visible by the user (on your product or on your website with a link to my page).
		- Redistribute the modification only if you want.
		- Send me the bug-fix (it could be great).
		- Pay me a beer or some other things.
		- Print the source code on WC paper ...
	You can NOT:
		- Earn money with this Software (But I can).
		- Add malware in the Sources.
		- Do something bad with the sources.
		- Use it to travel in the space with a toaster.
	
	I reserve the right to change this licence. If it change the version of the copy you have keep its own license


