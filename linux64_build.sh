rm monothread_demo
rm multithread_demo
gcc -D_64_ -DPTHREAD_OFF -O3 -pthread -fno-stack-protector -omonothread_demo demo.c
gcc -D_64_ -DPTHREAD -O3 -pthread -fno-stack-protector -omultithread_demo demo.c
