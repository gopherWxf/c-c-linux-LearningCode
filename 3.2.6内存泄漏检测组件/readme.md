# first
```bash
root@wxf:/tmp/tmp.d4vz2dOyJP# mkdir mem
root@wxf:/tmp/tmp.d4vz2dOyJP# gcc -o first first.c -ldl -g
root@wxf:/tmp/tmp.d4vz2dOyJP# ./first
root@wxf:/tmp/tmp.d4vz2dOyJP# cd mem/
root@wxf:/tmp/tmp.d4vz2dOyJP/mem# ls
0x55f613d27690.mem
root@wxf:/tmp/tmp.d4vz2dOyJP/mem# cat 0x55f613d27690.mem
```

# second
```bash
root@wxf:/tmp/tmp.d4vz2dOyJP# gcc -o second second.c 
root@wxf:/tmp/tmp.d4vz2dOyJP# ./second 
double free: 0x559730139500
root@wxf:/tmp/tmp.d4vz2dOyJP# cd mem/
root@wxf:/tmp/tmp.d4vz2dOyJP/mem# ls
0x5597301394d0.mem
root@wxf:/tmp/tmp.d4vz2dOyJP/mem# cat 0x5597301394d0.mem 
```

# three
```bash
root@wxf:/tmp/tmp.d4vz2dOyJP# gcc -o three three.c -ldl -g
root@wxf:/tmp/tmp.d4vz2dOyJP# ./three
[+0xc34]--->ptr:0x55b41c3ad260 size:10
[+0xc42]--->ptr:0x55b41c3ad690 size:20
[+0xc50]--->ptr:0x55b41c3ad6b0 size:30
[+0xc5e]--->ptr:0x55b41c3ad6e0 size:40
[-0xc6e]--->ptr:0x55b41c3ad260
[-0xc7a]--->ptr:0x55b41c3ad690
[-0xc86]--->ptr:0x55b41c3ad6e0
root@wxf:/tmp/tmp.d4vz2dOyJP# cd mem/
root@wxf:/tmp/tmp.d4vz2dOyJP/mem# cat 0x55b41c3ad6b0.mem
0x0000000000000c50
main
/tmp/tmp.d4vz2dOyJP/three.c:93
```

# mtrace
```bash
root@wxf:/tmp/tmp.d4vz2dOyJP# export MALLOC_TRACE=./mtrace.log
root@wxf:/tmp/tmp.d4vz2dOyJP# gcc -o mtrace mtrace.c 
root@wxf:/tmp/tmp.d4vz2dOyJP# ./mtrace 
root@wxf:/tmp/tmp.d4vz2dOyJP# ls
first	  mem	     memleak_self.c  mtrace.c	 readme.md  second.c  three
cmake-build-debug  CMakeLists.txt	 first.c  memleak.c  mtrace	     mtrace.log  second     test.c    three.c
root@wxf:/tmp/tmp.d4vz2dOyJP# cat mtrace.log 
= Start
@ ./mtrace:[0x558e233f3731] + 0x558e240bf6a0 0xa
@ ./mtrace:[0x558e233f373f] + 0x558e240bf6c0 0x14
@ ./mtrace:[0x558e233f374d] + 0x558e240bf6e0 0x1e
@ ./mtrace:[0x558e233f375b] + 0x558e240bf710 0x28
@ ./mtrace:[0x558e233f376b] - 0x558e240bf6a0
@ ./mtrace:[0x558e233f3777] - 0x558e240bf6c0
@ ./mtrace:[0x558e233f3783] - 0x558e240bf710
= End
```