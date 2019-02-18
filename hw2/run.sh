cd /mnt/c/Users/Mark/Documents/courses/os/code_project/hw2
export MYPATH=/usr/local/bin#/usr/bin#/bin#.
gcc -g -D DEBUG main.c functions.c
./a.out


cd /mnt/c/Users/Mark/Documents/courses/os/code_project/hw2
export MYPATH=/usr/local/bin#/usr/bin#/bin#.
gcc -g -D DEBUG test.c
./a.out

gdb ./a.out
