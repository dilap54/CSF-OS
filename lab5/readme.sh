mkdir /mnt/fuse #создать папку для монтирования
./lab5 -o allow_other -o uid=1000 /mnt/fuse/ #запустить прогу
fusermount -u /mnt/fuse #размонтировать папку