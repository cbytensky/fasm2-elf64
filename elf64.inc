namespace elf64

	use64

	entry = text
	if defined _start
		entry = _start
	end if

	db 7Fh, "ELF", 0, 0, 0, 0, 0, 7 dup 0 ; e_ident: ELFCLASS64, ELFDATA2LSB, EV_CURRENT, ELFOSABI_LINUX
	dw 2, 3Eh                             ; e_type = ET_EXEC, e_machine = EM_X86_64
	dd 0                                  ; e_version
	dq entry, phdrs, 0                    ; e_entry, e_phoff, e_shoff
	dd 0                                  ; e_flags
	dw 0, 38h                             ; e_ehsize, e_phentsize
	dw phdr.n                             ; e_phnum

	irp name, rel, phdr, shdr, needed, dyn
		name.i = 0
		define inames name
	end irp

	irp name, neededs, syms, rels, sonames, symnames, strsz, strs, shstrtab, phdrs
		if defined name
			virtual at name
				name.virtual::
			end virtual
		end if
	end irp

	macro vload name
		name:
		db name.bytes
		virtual name.virtual
			load name.bytes: $ - $$ from $$
		end virtual
		name.size = $ - name
	end macro

	macro phdr type, flags, offs, addr, size_file, size_mem
		dd type, flags                            ; p_type, p_flags
		dq offs, addr, 0, size_file, size_mem, 0  ; p_offset, p_vaddr, p_paddr, p_filesz, p_memsz, p_align
		incr phdr.i
	end macro

	virtual phdrs.virtual
		phdr    1, 111b, 0         , 0      , filesize        , memsize  ; PT_LOAD
		if dyn.n
			phdr 3, 000b, seg_interp, 0      , seg_interp.size , 0        ; PT_INTERP
			phdr 2, 000b, 0         , seg_dyn, 0               , 0        ; PT_DYNAMIC
		end if
	end virtual

	text:

	macro vdb name, data&
		virtual name.virtual
			@. = $
			db data
			@$. = $ - @
		end virtual
	end macro

	if dyn.n
		vdb sonames, 0
		vdb syms, sym.size dup 0
	end if

	macro incr name*, val: 1
		name = name + val
	end macro

	macro dstr data&
		if asciiz
			vdb strsz, data, 0
		else
			vdb strs, data
		end if
	end macro

	macro setargs regs, args&
		local i, val, isdata
		irp arg, args
			i = %
			if i > 6
				indx 7 + %% - %
				if % = 7 & (%% - 6) and 1
					push 0
				end if
			end if
			isdata = 0
			match _=,_, arg
				isdata = 1
			else if arg eqtype ""
				isdata = 1
			end if
			if isdata
				dstr arg
				val = @
			else
				val = arg
			end if
			if i <= 6
				irp r, regs
					indx i
					if val metadata 1 metadata 1 relativeto x86.reg
						if val metadata 1 <> r metadata 1
							mov r, val
						end if
					else
						movopt r, val
					end if
					break
				end irp
			else
				push val
			end if
		end irp
	end macro

	macro restorestack args&
		irp _, args
			num = %% - 6
			if num > 0
				add rsp, (num + num and 1) * 8
			end if
			break
		end irp
	end macro

	asciiz = 0

	namespace sys

		macro setname names&
			irp name, names
				?name := count + % - 1
			end irp
		end macro

		count = 0
		setname read, write, open, close, stat, fstat, lstat, poll, lseek, mmap, mprotect, munmap, brk, rt_sigaction, rt_sigprocmask, rt_sigreturn, ioctl, pread64, pwrite64, readv, writev, access, pipe, select, sched_yield, mremap, msync, mincore, madvise, shmget, shmat, shmctl, dup, dup2, pause, nanosleep, getitimer, alarm, setitimer, getpid, sendfile, socket, connect, accept, sendto, recvfrom, sendmsg, recvmsg, shutdown, bind, listen, getsockname, getpeername, socketpair, setsockopt, getsockopt, clone, fork, vfork, execve, exit, wait4, kill, uname, semget, semop, semctl, shmdt, msgget, msgsnd, msgrcv, msgctl, fcntl, flock, fsync, fdatasync, truncate, ftruncate, getdents, getcwd, chdir, fchdir, rename, mkdir, rmdir, creat, link, unlink, symlink, readlink, chmod, fchmod, chown, fchown, lchown, umask, gettimeofday, getrlimit, getrusage, sysinfo, times, ptrace, getuid, syslog, getgid, setuid, setgid, geteuid, getegid, setpgid, getppid, getpgrp, setsid, setreuid, setregid, getgroups, setgroups, setresuid, getresuid, setresgid, getresgid, getpgid, setfsuid, setfsgid, getsid, capget, capset, rt_sigpending, rt_sigtimedwait, rt_sigqueueinfo, rt_sigsuspend, sigaltstack, utime, mknod, uselib, personality, ustat, statfs, fstatfs, sysfs, getpriority, setpriority, sched_setparam, sched_getparam, sched_setscheduler, sched_getscheduler, sched_get_priority_max, sched_get_priority_min, sched_rr_get_interval, mlock, munlock, mlockall, munlockall, vhangup, modify_ldt, pivot_root, _sysctl, prctl, arch_prctl, adjtimex, setrlimit, chroot, sync, acct, settimeofday, mount, umount2, swapon, swapoff, reboot, sethostname, setdomainname, iopl, ioperm, create_module, init_module, delete_module, get_kernel_syms, query_module, quotactl, nfsservctl, getpmsg, putpmsg, afs_syscall, tuxcall, security, gettid, readahead, setxattr, lsetxattr, fsetxattr, getxattr, lgetxattr, fgetxattr, listxattr, llistxattr, flistxattr, removexattr, lremovexattr, fremovexattr, tkill, time, futex, sched_setaffinity, sched_getaffinity, set_thread_area, io_setup, io_destroy, io_getevents, io_submit, io_cancel, get_thread_area, lookup_dcookie, epoll_create, epoll_ctl_old, epoll_wait_old, remap_file_pages, getdents64, set_tid_address, restart_syscall, semtimedop, fadvise64, timer_create, timer_settime, timer_gettime, timer_getoverrun, timer_delete, clock_settime, clock_gettime, clock_getres, clock_nanosleep, exit_group, epoll_wait, epoll_ctl, tgkill, utimes, vserver, mbind, set_mempolicy, get_mempolicy, mq_open, mq_unlink, mq_timedsend, mq_timedreceive, mq_notify, mq_getsetattr, kexec_load, waitid, add_key, request_key, keyctl, ioprio_set, ioprio_get, inotify_init, inotify_add_watch, inotify_rm_watch, migrate_pages, openat, mkdirat, mknodat, fchownat, futimesat, newfstatat, unlinkat, renameat, linkat, symlinkat, readlinkat, fchmodat, faccessat, pselect6, ppoll, unshare, set_robust_list, get_robust_list, splice, tee, sync_file_range, vmsplice, move_pages, utimensat, epoll_pwait, signalfd, timerfd_create, eventfd, fallocate, timerfd_settime, timerfd_gettime, accept4, signalfd4, eventfd2, epoll_create1, dup3, pipe2, inotify_init1, preadv, pwritev, rt_tgsigqueueinfo, perf_event_open, recvmmsg, fanotify_init, fanotify_mark, prlimit64, name_to_handle_at, open_by_handle_at, clock_adjtime, syncfs, sendmmsg, setns, getcpu, process_vm_readv, process_vm_writev, kcmp, finit_module, sched_setattr, sched_getattr, renameat2, seccomp, getrandom, memfd_create, kexec_file_load, bpf, execveat, userfaultfd, membarrier, mlock2, copy_file_range, preadv2, pwritev2, pkey_mprotect, pkey_alloc, pkey_free, statx, io_pgetevents, rseq

		count = 424
		setname pidfd_send_signal, io_uring_setup, io_uring_enter, io_uring_register, open_tree, move_mount, fsopen, fsconfig, fsmount, fspick, pidfd_open, clone3, close_range, openat2, pidfd_getfd, faccessat2, process_madvise, epoll_pwait2, mount_setattr, quotactl_fd, landlock_create_ruleset, landlock_add_rule, landlock_restrict_self, memfd_secret, process_mrelease, futex_waitv, set_mempolicy_home_node, cachestat, fchmodat2, map_shadow_stack

	end namespace

end namespace

@  = 0
@$ = 0

macro @optspeed
	speed = 1
end macro

macro @optsize
	speed = 0
end macro

macro align n
	local m
	m := $ and (n - 1)
	if m & n
		rb n - m
	end if
end macro

macro proc name*
	if used name
		name:
end macro

macro end.proc!
	end if
end macro

macro movopt reg*, val*
	local i, meta, r, reg64, reg8
	meta := reg metadata 1
	i := meta scale 0
	if val >= 0 & val < 1 shl 32
		element r: x86.r32 + i
	else
		element r: meta
	end if
	if val = 0
		xor r, r
	else if val = -1
		or r, val
	else if ~speed & val >= -80h & val < 80h
		element reg64: x86.r64 + i
		push val
		pop reg64
	else if ~speed & val >= 80h & val < 100h & i <= 4
		element reg8: x86.r8 + i
		xor r, r
		mov reg8, val
	else
		mov r, val
	end if
end macro

macro needed names*&
	namespace elf64
		if dyn.n
			irp name, names
				vdb sonames, name, 0
				virtual neededs.virtual
					dq 1, @ - strtab  ; DT_NEEDED
				end virtual
			end irp
		else
			display "needed: no “dyncall”, ignored", 10
		end if
		needed.i = 1
	end namespace
end macro

macro dyncall name*, args&
	namespace elf64
		dyn.i = 1
		if ~needed.n
			err "dyncall: “needed” required"
		end if
		if ~definite defined.name
			local n, offs
			defined.name := 1
			n := 7
			offs := rel.i * 8
			addr.name = phdrs + (rel.i - n) * 8
			if rel.i <= n
				addr.name = offs
			end if
			vdb symnames, `name, 0
			virtual syms.virtual
				dd @ - strtab      ; st_name
				db 1 shl 4 + 2, 0  ; st_info = STB_GLOBAL + STT_FUNC, st_other
				dw 0               ; st_shndx
				dq 0, 0            ; st_value, st_size
			end virtual
			virtual rels.virtual
				dq addr.name, (rel.i + 1) shl 32 + 1, 0  ; r_offset, r_info = symind + R_X86_64_64, r_addend
			end virtual
			incr rel.i
		end if
		setargs <rdi, rsi, rdx, rcx, r8, r9>, args
		call qword [addr.name]
		restorestack args
	end namespace
end macro

macro sys name*, args&
	namespace elf64
		movopt eax, sys.name
		setargs <rdi, rsi, rdx, r10, r8, r9>, args
		syscall
		restorestack args
	end namespace
end macro

macro dstr data*&
	namespace elf64
		dstr data
	end namespace
end macro

macro leastr reg*, data*&
	dstr data
	movopt reg, @
end macro

irp name, dyncall, sys, dstr, leastr
	macro name#z args*&
		elf64.asciiz =: 1
		name args
		restore elf64.asciiz
	end macro
end irp

@optsize

postpone

	namespace elf64

		text.size = $ - text

		data:

		vload strs
		vload strsz

		data.size = $ - data

		vload phdrs

		if dyn.n
			sym.size := 18h
			rel.size := 18h
			align 8 * speed
			seg_dyn:
			dq 6, syms              ; DT_SYMTAB
			dq 7, rels              ; DT_RELA
			dq 5, strtab            ; DT_STRTAB
			dq 8, rel.n * rel.size  ; DT_RELASZ
			dq 9, rel.size          ; DT_RELAENT
			vload neededs
			seg_dyn.size := $ - seg_dyn
			vload syms
			vload rels

			seg_interp:
			db "/lib64/ld-linux-x86-64.so.2"
			seg_interp.size := $ - seg_interp + 1

			strtab:
			vload sonames
			vload symnames
			strtab.size := $ - strtab
		end if  ; dyn.n

		macro npages name, val
			name := ((val - 1) shr 12 + 1) shl 12
		end macro
		npages filesize, $%%
		npages memsize, $%

		irpv name, inames
			name.n := name.i
		end irpv

	end namespace
end postpone