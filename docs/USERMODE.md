# myOS user-mode foundation

The kernel now has the first process/scheduler and syscall abstraction layer.

## What is real

- Limine memory-map discovery and a kernel heap backed by usable physical memory.
- Process table with PID allocation and lifecycle states.
- Cooperative scheduler dispatching registered kernel tasks.
- Stable syscall numbers for yield, exit, getpid and write.
- The syscall dispatcher never dereferences an untrusted user pointer.

## What is deliberately not claimed yet

This is not yet CPU privilege-level 3 execution. The next layer must add page tables, an IDT/syscall entry mechanism, per-process address spaces, ELF loading, user stacks and a context switch into ring 3.

The current scheduler is therefore a safe intermediate kernel scheduler: it lets the OS architecture evolve without pretending that kernel callbacks are isolated user programs.
