#line 2 "engine_trace_portable.c"
#include <signal.h>
#include <unistd.h>

#include "engine_trace_portable.h"

#undef return

#include "lib/oofatfs/ff.h"

#include "extmod/vfs.h"
#include "py/objstr.h"
#include "py/stream.h"
#include "py/objstr.h"
#include "py/runtime.h"
#include "py/builtin.h"

MP_REGISTER_ROOT_POINTER(mp_obj_t hard_fault_dump);
const mp_stream_p_t *hard_fault_dump_stream;

#if defined(SUPPRESS_WARNINGS) && (defined(__GNUC__) || defined(__clang__))
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wpedantic"
#endif

const unsigned int __traced_function = 0;
static int last_activated_tracer = -1;

typedef struct trace_point_info_t {
  const char* info[3];
} trace_point_info_t;

/* async-safe error output */
static void errwrite(const char* s) {
  // Compiler almost certainly knows this is strlen
  int len = 0; while(s[len] != 0) len++;

  // len -= write(STDERR_FILENO, s, (unsigned int)len);
  int err;
  len -= mp_stream_rw(MP_STATE_VM(hard_fault_dump), (void*)s, len, &err, MP_STREAM_RW_WRITE);
  if(len != 0) {
    /* Failed to write all error data... */
  }
}

trace_point_info_t default_trace_stack[SOFTWARE_DEBUG_TRACE_COUNT][SOFTWARE_DEBUG_TRACE_DEPTH] = {0};
int default_trace_sp[SOFTWARE_DEBUG_TRACE_COUNT] = {0};

int __push_trace_point(const char* const file_line, const char* const calling_function, const int tracer_number) {
  last_activated_tracer = tracer_number;
  trace_point_info_t* s = default_trace_stack[tracer_number];
  int* sp = &(default_trace_sp[tracer_number]);
  if(*sp < SOFTWARE_DEBUG_TRACE_DEPTH) {
    s[*sp].info[0] = file_line;
    s[*sp].info[1] = NULL;
    s[*sp].info[2] = calling_function;
    (*sp)++;
  }
  return 0;
}

int __pop_trace_point(const int tracer_number) {
  int* sp = &(default_trace_sp[tracer_number]);
  if(*sp > 0) {
    (*sp)--;
  }
  return 0;
}

int __set_trace_entry(const char* const entry, const int tracer_number) {
  last_activated_tracer = tracer_number;
  trace_point_info_t* s = default_trace_stack[tracer_number];
   int* sp = &(default_trace_sp[tracer_number]);
  if(*sp > 0) {
    s[*sp-1].info[1] = entry;
  }
  // errwrite("Writing trace entry!\n\r");
  // char num[3];
  // num[0] = "0123456789ABCDEF"[(*sp) >> 4];
  // num[1] = "0123456789ABCDEF"[(*sp) & 0xF];
  // num[2] = 0;
  // errwrite(num);
  // errwrite("\n\r");
  return 0;
}

void trace_report(const int tracer_number) {
  trace_point_info_t* const s = default_trace_stack[tracer_number];
  int* const sp = &(default_trace_sp[tracer_number]);
  int indent = 0;
  if(*sp > 0) {
    while(*sp > 0) {
      if(s[*sp-1].info[2]) {
        for(int i = 0; i < indent; i++) errwrite("  ");
        errwrite(PALETTE_4 "FROM: " PALETTE_0);
        errwrite(s[*sp-1].info[1]);
        errwrite("\n\r");
        indent++;
      }

      for(int i = 0; i < indent; i++) errwrite("  ");
      errwrite(PALETTE_1 "AT:   ");
      errwrite(s[*sp-1].info[0]);
      errwrite("\n\r");

      for(int i = 0; i < indent; i++) errwrite("  ");
      errwrite(PALETTE_0 "IN:   ");
      errwrite(s[*sp-1].info[2]);
      errwrite("\n\r");

      if(*sp > 1) errwrite("\n\r");
      (*sp)--;
    }
    errwrite(PALETTE_RESET);
  }
}

void* sig_hdlr_reset[] = {
  SIG_DFL,
  SIG_DFL,
  SIG_DFL,
  SIG_DFL,
  SIG_DFL,
  SIG_DFL,
};

void debug_trace_panic(int sig) {
  const char* sig_name = "unknown";
  void (*reset)(int) = SIG_DFL;
  switch(sig) {
    case SIGTERM: sig_name = "SIGTERM"; reset = sig_hdlr_reset[0]; break;
    case SIGSEGV: sig_name = "SIGSEGV"; reset = sig_hdlr_reset[1];  break;
    case SIGINT: sig_name = "SIGINT"; reset = sig_hdlr_reset[2];  break;
    case SIGILL: sig_name = "SIGILL"; reset = sig_hdlr_reset[3];  break;
    case SIGABRT: sig_name = "SIGABRT"; reset = sig_hdlr_reset[4];  break;
    case SIGFPE: sig_name = "SIGFPE"; reset = sig_hdlr_reset[5];  break;
    default: break;
  }
  char sig_num[3];
  sig_num[0] = "0123456789ABCDEF"[sig >> 4];
  sig_num[1] = "0123456789ABCDEF"[sig & 0xF];
  sig_num[2] = 0;

  mp_obj_t file_open_args[2] = {
      mp_obj_new_str("/hard_fault_log.txt", 19),
      MP_ROM_QSTR(MP_QSTR_wb) // See extmod/vfs_posix_file.c and extmod/vfs_lfsx_file.c
  };

  MP_STATE_VM(hard_fault_dump) = mp_vfs_open(2, &file_open_args[0], (mp_map_t*)&mp_const_empty_map);

  hard_fault_dump_stream = mp_get_stream(MP_STATE_VM(hard_fault_dump));

  errwrite(PALETTE_3); errwrite(sig_name); errwrite(" (0x");
  errwrite(sig_num); errwrite(")"); errwrite(PALETTE_RESET "\n\r");

  #ifndef SOFTWARE_DEBUG_TRACE
    errwrite("define SOFTWARE_DEBUG_TRACE in build for debug stack info!\n\r");
  #else
    for(int i = 0; i < SOFTWARE_DEBUG_TRACE_COUNT; i++) {

      char t_num[3];
      t_num[0] = "0123456789ABCDEF"[i >> 4];
      t_num[1] = "0123456789ABCDEF"[i & 0xF];
      t_num[2] = 0;

      errwrite(PALETTE_2 "== Debug trace ");
      errwrite(t_num);
      errwrite(" ==");
      // if(i == last_activated_tracer) errwrite(PALETTE_4 " (LAST ACTIVE)" PALETTE_RESET);
      errwrite("\n\r");
      trace_report(i);
      errwrite(PALETTE_2 "== == == == == == ==");
      errwrite(PALETTE_RESET "\n\r");
    }
  #endif

  mp_stream_close(MP_STATE_VM(hard_fault_dump));

  /* Reset to saved handler and re-raise */
  signal(sig, reset);
  raise(sig);
}

void trace_standard_signals() {
  sig_hdlr_reset[0] = signal(SIGTERM, debug_trace_panic);
  sig_hdlr_reset[1] = signal(SIGSEGV, debug_trace_panic);
  sig_hdlr_reset[2] = signal(SIGINT, debug_trace_panic);
  sig_hdlr_reset[3] = signal(SIGILL, debug_trace_panic);
  sig_hdlr_reset[4] = signal(SIGABRT, debug_trace_panic);
  sig_hdlr_reset[5] = signal(SIGFPE, debug_trace_panic);
}

void __trace_break_point() {
  debug_trace_panic(SIGABRT);
}

#if defined(SUPPRESS_WARNINGS) && (defined(__GNUC__) || defined(__clang__))
  #pragma GCC diagnostic pop
#endif
