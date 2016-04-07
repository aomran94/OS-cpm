bcc -ansi -c -o kernel.o kernel.c
as86 kernel.asm -o kernel_asm.o
ld86 -o kernel -d kernel.o kernel_asm.o
bcc -ansi -c -o shell.o shell.c
as86 lib.asm -o lib.o
ld86 -o shell -d shell.o lib.o
dd if=/dev/zero of=floppya.img bs=512 count=2880
nasm bootload.asm
dd if=bootload of=floppya.img bs=512 count=1 conv=notrunc
dd if=map.img of=floppya.img bs=512 count=1 seek=1 conv=notrunc
dd if=dir.img of=floppya.img bs=512 count=1 seek=2 conv=notrunc
dd if=kernel of=floppya.img bs=512 conv=notrunc seek=3
./loadFile tstprg
./loadFile shell
./loadFile messg1.txt
./loadFile messg2.txt
./loadFile messg6.txt
./loadFile prgtr
./loadFile messg3.txt
./loadFile messg4.txt
./loadFile messg5.txt
./loadFile tstpr

bochs -f conf.bxrc