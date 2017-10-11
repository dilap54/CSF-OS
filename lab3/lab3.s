.data
    # массив для вызова cat log.txt
    cmd_cat: .string "/usr/bin/awk"
    arg1_cat: .string "{  sum[substr($4,2,11)] += $10;  total += $10; } END { for(i in sum) { printf(\" %d. %s - %d - %.2f%%\\n \" , NR, i, sum[i], sum[i]/total*100); } }"
    arg2_cat: .string "log.txt"
    args_cat: .long cmd_cat, arg1_cat, arg2_cat, 0

    test_str:
		.string "test\n"

    # массив для вызова wc -l
    cmd_wc: .string "/usr/bin/sort"
    arg_wc: .string "-k 4r"
    args_wc: .long cmd_wc, arg_wc, 0

    # массив файловых дескрипторов для pipe
    fds: .int 0, 0

.text
.globl main
main:
    # вызов pipe(fds)
    pushl $fds
    call pipe

    # вызов fork()
    call fork

    # переход к коду дочернего процесса для cat,
    # если fork вернул 0
    cmpl $0, %eax
    je child_cat

    # вызов fork() в родительском процессе
    call fork

    # переход к коду дочернего процесса для wc,
    # если fork вернул 0
    cmpl $0, %eax
    je child_wc

    # close(fd[0]) в родительском процессе
    movl $fds, %eax
    pushl 0(%eax)
    call close

    # close(fd[1]) в родительском процессе
    movl $fds, %eax
    pushl 4(%eax)
    call close

    # вызов wait(NULL) - для cat
    pushl $0
    call wait
    int     $0x80

    # еще один вызов wait(NULL) - для wc
    pushl $0
    call wait
    int     $0x80

    #wait

finish:

    # вызов exit(0)
    int $0x80
    movl $1, %eax
    movl $0, %ebx
    int $0x80

# код дочернего процесса для cat
child_cat:
    # вызов dup2(fds[1],1)
    pushl $1
    movl $fds, %eax
    pushl 4(%eax)
    call dup2

    # вызов close(fds[0]), close(fds[1])
    movl $fds, %eax
    pushl 0(%eax)
    call close
    movl $fds, %eax
    pushl 4(%eax)
    call close

    # вызов execve(cmd_cat, args_cat)
    pushl $args_cat
    pushl $cmd_cat
    call execvp

    call finish

# код дочернего процесса для wc
child_wc:

    # вызов dup2(fds[0],0)
    pushl $0
    movl $fds, %eax
    pushl (%eax)
    call dup2

    # вызов close(fds[0]), close(fds[1])
    movl $fds, %eax
    pushl 0(%eax)
    call close
    movl $fds, %eax
    pushl 4(%eax)
    call close

    # вызов execve(cmd_wc, args_wc)
    pushl $args_wc
    pushl $cmd_wc
    call execvp

    call finish
    