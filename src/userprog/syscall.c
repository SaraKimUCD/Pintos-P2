#include "userprog/syscall.h"
#include "devices/shutdown.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/malloc.h"
#include "lib/string.h"
#include "lib/kernel/list.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "userprog/process.h"
#include "lib/stdbool.h"


static void syscall_handler(struct intr_frame *f);
static bool user_memory_ok(const void *, int);
static int file_sys_ok();
static uint32_t get_user_int32(const void *);
static bool get_syscall_args(const void *, struct user_syscall *);
struct process_file *search_files(struct list *files, int fid);


void
syscall_init(void) {
    intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
}

struct process_file *search_files(struct list *files, int fid) {
    struct list_elem *e;
    for (e = list_begin(files); e != list_end(files); e = list_next(e));
    {
        struct process_file *f = list_entry(e, struct process_file, elem);

        if (f->fid == fid) {
            return f;
        }
    }
    return NULL;
}

static bool user_memory_ok(const void *stack_pointer, int byte_size) {
	for (int i = 0; i < byte_size; i++) {
    if (!is_user_vaddr(stack_pointer) || stack_pointer == NULL) {
      return false;
    }
  }
  return true;
}

static bool
get_syscall_args(const void *stack_pointer, struct user_syscall *new_syscall) {
    if (!user_memory_ok(stack_pointer, 1)) {
      return false;
    }

    new_syscall->arg_count = 0;

    for (int i = 0; i < 3; i++) {
      if (!user_memory_ok(stack_pointer, 4)) {
        printf("Error retrieving System Call Arguments");
        thread_exit;
      }

      new_syscall->args[i] = get_user_int32(stack_pointer);
      new_syscall->arg_count++;
      stack_pointer += 4;
    }
};

static uint32_t get_user_int32(const void *stack_pointer){

    uint8_t byte_array[4];
   
    for (int i = 0; i < 4; i++) {
        byte_array[i] = get_user(stack_pointer);
        stack_pointer++;
    }

  return *(uint32_t *) byte_array;
}

void
exit_process_by_code(int code) {
    struct list_elem *e;

    for (e = list_begin(&thread_current()->parent->child_processes);
         e != list_end(&thread_current()->parent->child_processes); e = list_next(e)) {
        struct child_parent *c = list_entry(e,
        struct child_parent, elem);
        if (c->tid == thread_current()->tid) {
            c->has_exited = true;
            c->exit_code = code;
        }
    }

    thread_current()->exit_code = code;
    if (thread_current()->parent->waiting_on_thread == thread_current()->tid)
        sema_up(&thread_current()->parent->child_sema);
    thread_exit();
}


static void
syscall_handler(struct intr_frame *f) {
    printf("System call!\n");

  if (!user_memory_ok(f->esp, 1)) {
    printf("Invalid memory location");
    thread_exit();
  }

  void *stack_pointer = (void *) f->esp;
  struct user_syscall new_syscall;

    new_syscall.syscall_index = get_user_int32(stack_pointer);
    stack_pointer += 4;

    get_syscall_args(stack_pointer, &new_syscall);
    int sys_call_num = (int) new_syscall.syscall_index;

  switch (sys_call_num) {
    case SYS_HALT :
      printf("calling SYS_HALT");
      halt();
      break;                   /* Halt the operating system. */

    case SYS_EXIT :
      printf("calling SYS_EXIT");
//      exit((int) new_syscall.args[0]);
      break;                   /* Terminate this process. */

    case SYS_EXEC :
      printf("calling SYS_EXEC");
//      exec((const char *) new_syscall.args[0]);
      break;                   /* Start another process. */

    case SYS_WAIT :
      printf("calling SYS_WAIT");
//      wait((pid_t) new_syscall.args[0]))
      break;                   /* Wait for a child process to die. */

    case SYS_CREATE :
      printf("calling SYS_CREATE");
      create((const char *) new_syscall.args[0], (unsigned) new_syscall.args[1]);
      break;                 /* Create a file. */

    case SYS_REMOVE :
      printf("calling SYS_REMOVE");
      create((const char *) new_syscall.args[0], (unsigned) new_syscall.args[1]);
      break;                 /* Delete a file. */

    case SYS_OPEN :
      open((const char *) new_syscall.args[0]);
      printf("calling SYS_OPEN");
      break;                   /* Open a file. */

    case SYS_FILESIZE :
      filesize((int) new_syscall.args[0]);
      printf("calling SYS_FILESIZE");
      break;               /* Obtain a file's size. */

    case SYS_READ :
      printf("calling SYS_READ");
//      read((int) new_syscall.args[0], (const void *) new_syscall.args[1], (unsigned) new_syscall.args[2]);
      break;                   /* Read from a file. */

    case SYS_WRITE :
      printf("calling SYS_WRITE");
//      write((int) new_syscall.args[0], (const void *) new_syscall.args[1], (unsigned) new_syscall.args[2]);

      break;                  /* Write to a file. */

    case SYS_SEEK :
      printf("calling SYS_SEEK");
//      seek((int) new_syscall.args[0], (unsigned) new_syscall.args[1]);
      break;                   /* Change position in a file. */

    case SYS_TELL :
      printf("calling SYS_TELL");
//      tell((int) new_syscall.args[0]);

      break;                   /* Report current position in a file. */

    case SYS_CLOSE :
      printf("calling SYS_CLOSE");
      close((int) new_syscall.args[0]);
      break;                  /* Close a file. */

            /* Project 3 and optionally project 4. */
//    case SYS_MMAP :
//      printf("calling SYS_MMAP");
//      break;                   /* Map a file into memory. */
//    case SYS_MUNMAP :
//      printf("calling SYS_MUNMAP");
//      break;                 /* Remove a memory mapping. */
//
//      /* Project 4 only. */
//    case SYS_CHDIR :
//      printf("calling SYS_CHDIR");
//      break;                  /* Change the current directory. */
//    case SYS_MKDIR :
//      printf("calling SYS_MKDIR");
//      break;                  /* Create a directory. */
//    case SYS_READDIR :
//      printf("calling SYS_READDIR");
//      break;                /* Reads a directory entry. */
//    case SYS_ISDIR :
//      printf("calling SYS_ISDIR");
//      break;                  /* Tests if a fd represents a directory. */
//    case SYS_INUMBER:
//      printf("calling SYS_INUMBER");
//      break;

        default:
            printf("Unrecognized System Call");
            break;

  }
}

    // MODIFIED BY SHAWN JOHNSON
    void halt(void) {
        shutdown_power_off();
    }

    void
    exit(int status) {
        exit_process_by_code(status);
    }

    pid_t
    exec(const char *file) {
        return process_execute(file);
    }

    int
    wait(pid_t pid) {

        return;
    }

    // MODIFIED BY SHAWN JOHNSON
    bool
    create(const char *file, unsigned initial_size) {
        file_lock_acquire();
        bool error = filesys_create(file, initial_size);
        file_lock_release();
        return error;
    }

    // MODIFIED BY SHAWN JOHNSON
    bool
    remove(const char *file) {
        file_lock_acquire();
        bool error = filesys_remove(file);
        file_lock_release();
        return error;
    }

    // MODIFIED BY SHAWN JOHNSON
    int
    open(const char *file) {
        file_lock_acquire();
        struct file *oFile = filesys_open(file);
        if (oFile == NULL) {
            file_lock_release();
            return -1;
        } else {
            int fd = process_affix_file(oFile);
            file_lock_release();
            return fd;
        }
    }

    int
    filesize(int fid) {
        file_lock_acquire();
        struct file *fs = process_get_file(fid);
        int f_size = -1;
        if (fs != NULL) {
            f_size = file_length(fs);
        }
        file_lock_release();
        return f_size;
    }

/* NEW CHANGE */
    int
    read(int fd, void *buffer, unsigned size) {
	struct file *f;
	file_lock_acquire();

	
	
//	if(fd == 0)
//	{		
//	}

	f = process_get_file(fd);
	if (f == NULL) // If file could not be read, return -1
	{	
		file_lock_release();
		return -1;
	}

	size = file_read(f, buffer, size);
	file_lock_release();
        return size;
    }

/* NEW CHANGE */
    int
    write(int fd, const void *buffer, unsigned size) {
	struct file *f;
	file_lock_acquire();
	
	if (fd == 1)
	{
		putbuf(buffer, size);		// Buffer writen using putbuf
		file_lock_release();
		return size;
	}

	f = process_get_file(fd);
	if (f == NULL)			// If nothing written to file, return 0
	{
		file_lock_release();
		return 0;
	}

	size = file_write(f, buffer, size);
	file_lock_release();
        return size;
    }

/* NEW CHANGE */
    void
    seek(int fd, unsigned position) {
	struct file *f;
	f = process_get_file(fd);
	//f = search_files(f, fd);
	if (f == NULL)
	{	
		printf("Seek failed. Exiting");
		exit(-1);	// Should exit
	}
	//f->pos = position;	
	file_seek(f, position);		// Set file position to new position from file.c
        return;
    }

/* NEW CHANGE */
    unsigned
    tell(int fd) {
	struct file *f;
	f= process_get_file(fd);
	if (f == NULL)
	{
		printf("Tell failed. Exiting");
		exit(-1);	// Should exit
	}
        return file_tell(f);	// Return file position from file.c
    }

    void
    close(int fid) {
        struct list_elem *e;
        struct process_file *ff;
        file_lock_acquire();

        struct thread *curr_thread = thread_current();

        for (e = list_begin(&curr_thread->files); e != list_end(&curr_thread->files); e = list_next(e)) {
            ff = list_entry(e, struct process_file, elem);
            if (ff->fid == fid) {
                file_close(ff->file_ptr);
                list_remove(&ff->elem);
		        break;
            }
        }
        free(ff);
        file_lock_release();
    }



//  mapid_t
//  mmap(int fd, void *addr) {
//    return;
//  }
//
//  void
//  munmap(mapid_t mapid) {
//    return;
//  }
//
//  bool
//  chdir(const char *dir) {
//    return;
//  }
//
//  bool
//  mkdir(const char *dir) {
//    return;
//  }
//
//  bool
//  readdir(int fd, char name[READDIR_MAX_LEN + 1]) {
//    return;
//  }
//
//  bool
//  isdir(int fd) {
//    return;
//  }
//
//  int
//  inumber(int fd) {
//    return;
//  }



//TODO memory cleanups for parent / child / grancchild deaths
//TODO use semaphors for parent child grandchild synchronization
