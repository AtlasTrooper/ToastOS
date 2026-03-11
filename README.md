# ToastOS
In this repo I will slowly build an operating system. Starting from a basic kernel capable of only displaying text in a terminal, this will hopefully grow into something more capable
in the longrun.

The project will be divided into 5 core stages:
1.Shell stage:
Upon reaching Shell stage, we will have implemented a basic vga driver, printf, a gdt, idt, as well as a PS2 Keyboard driver. This should be enough to create a simple shell capable of dealing with simple commands, printing their output to the screen, and scrolling when necessary, and clearing when needed.

2. Memory allocation and file system
2.5. Moving to 64bit
3. Networking 
4. Running software like bash or doom
5. GUI


Credits and Sources:

OSdev wiki: https://wiki.osdev.org/Expanded_Main_Page

OSdever paper&tutorial archive: http://www.osdever.net/tutorials/

The little OS book: https://littleosbook.github.io

printf implementation by Scott Cosentino: https://gitlab.com/olivestem/Jazz2-0/-/blob/main/src/stdlib/stdio.c?ref_type=heads


