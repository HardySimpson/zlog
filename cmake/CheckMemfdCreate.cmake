include(CheckCSourceCompiles)

# Note: uClibc-ng does not provide a native C library wrapper function for
# memfd_create() in standard versions.
# However, since memfd_create is a Linux kernel-level system call (introduced in Linux 3.17),
# so we define a macro that directly invokes the system call.

check_c_source_compiles(
"
#define _GNU_SOURCE
#include <sys/mman.h>
int main(void){ int fd = memfd_create(\"x\",0); return 0; }
"
  HAVE_MEMFD_CREATE
)

if(NOT HAVE_MEMFD_CREATE)
  check_c_source_compiles(
"
#define _GNU_SOURCE
#include <sys/mman.h>
#include <unistd.h>
#include <sys/syscall.h>
#ifndef SYS_memfd_create
#define SYS_memfd_create 319
#endif
int main(void){ int fd = syscall(SYS_memfd_create, \"x\",0); return 0; }
"
    HAVE_MEMFD_CREATE_SYSCALL
  )
  if(NOT HAVE_MEMFD_CREATE_SYSCALL)
    message(FATAL_ERROR "memfd_create is not available")
  else()
      add_compile_definitions(MEMFD_CREATE_SYSCALL)
  endif()
endif()
