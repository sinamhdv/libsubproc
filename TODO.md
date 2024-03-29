- [ ] valgrind memtest
- [X] fd leak test
- [X] handle redirection of stderr to stdout (SPIO_STDOUT for fd_err only)
- [ ] (maybe?) add a SIGCHLD handler to automatically detect child terminations and set sp->is_alive to false there. (requires keeping a hash-table/list of all created subproc structs to be found by their pid so that we can set their is_alive to false).
- [ ] add docs for functions
- [X] add SPIO_DEVNULL to fd modes
- [ ] add select/poll support to fds (maybe in IO module?)
- [X] NOTE: PIDs are not reused until waited on => subproc->is_alive is not necessary => it is necessary in sp_free() because sp_free() might be called after sp_wait() when the PID is reused. But, it can be renamed and shouldn't be part of the public interface. (maybe cange it to _waited_on or something).
- [ ] add a module for error handling containing the latest errno faced in libsubproc functions and a global `char *` containing the name of the function we faced an error in. Then, we need an `sp_strerror` function to print this name and the `strerror` of the library errno.
- [X] test SPIO_STDOUT and SPIO_DEVNULL
- [ ] add `wstatus` argument to `sp_wait`
- [ ] add the option of providing user-allocated struct to `sp_open`
- [ ] (maybe?) set `fd_assignment` for a custom fd to -1 to prevent closing it in `sp_free`
- [ ] capturing return-code of programs terminating with signals (e.g. SIGSEGV) in `sp_wait`?
