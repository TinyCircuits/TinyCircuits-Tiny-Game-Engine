#ifndef ENGINE_TRACE_PORTABLE_INCLUDED
#define ENGINE_TRACE_PORTABLE_INCLUDED

#define SOFTWARE_DEBUG_TRACE
#define SOFTWARE_DEBUG_TRACE_COUNT (2)
#define SOFTWARE_DEBUG_TRACE_COLORS (0)
#define SOFTWARE_DEBUG_TRACE_DEPTH (32)

#define SUPPRESS_WARNINGS

#if defined(SUPPRESS_WARNINGS) && (defined(__GNUC__) || defined(__clang__))
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wreturn-type"
#endif

extern const unsigned int __traced_function;

void trace_standard_signals();
void trace_report(const int tracer_number);
void __trace_break_point();
void debug_trace_panic(int sig);

#define TRACE_CALL(X, Y) do TRACE_POINT(PALETTE_6 "TRACE_CALL" PALETTE_RESET) {\
  TRACE_ENTRY(#X) {X Y;} \
} while(0)

#define TRACE_BREAK_POINT() do TRACE_POINT(PALETTE_6 "TRACE_BREAK_POINT" PALETTE_RESET) {\
  TRACE_ENTRY("TRACE_BREAK_POINT") {__trace_break_point();} \
} while(0)

#define TRACE_DECL(X, Y, Z) \
X Y { \
  TRACE_POINT(PALETTE_5 PALETTE_0 #X PALETTE_RESET) { \
    static const int __traced_function = 1; \
    (void)__traced_function; \
    TRACE_ENTRY(#X) { \
      Z \
    } \
  } \
} \

#define DEBUG_TRACER_NUMBER (0)

int __push_trace_point(const char* const file_line, const char* const calling_function, const int tracer_number);
int __pop_trace_point(const int tracer_number);
int __set_trace_entry(const char* const entry, const int tracer_number);

#ifdef SOFTWARE_DEBUG_TRACE

  #define STRINGIZE( L )     #L
  #define STRINGMACRO( M, L ) M(L)
  #define LINE_STR STRINGMACRO( STRINGIZE, __LINE__ )

  #define __FOR_CONTROL for(register unsigned int __control = 0; __control != 2; __control++)
  #define __DEFER_TRACE_POP __FOR_CONTROL if(__control) {__pop_trace_point(DEBUG_TRACER_NUMBER);} else

  #define TRACE_ENTRY(X) if(__set_trace_entry(X, DEBUG_TRACER_NUMBER)) {} else
  #define TRACE_POINT(X) if(__push_trace_point(__FILE__ "(" LINE_STR ") | " X, __func__, DEBUG_TRACER_NUMBER)) {} else __DEFER_TRACE_POP

  #define return __FOR_CONTROL if(!__control && __traced_function) {__pop_trace_point(DEBUG_TRACER_NUMBER);} else return

#else

  #define TRACE_ENTRY(X) if(0) {} else
  #define TRACE_POINT(X) if(0) {} else


#endif /* SOFTWARE_DEBUG_TRACE */

#if(SOFTWARE_DEBUG_TRACE_COLORS == 0)

#define PALETTE_0     ""
#define PALETTE_1     ""
#define PALETTE_2     ""
#define PALETTE_3     ""
#define PALETTE_4     ""
#define PALETTE_5     ""
#define PALETTE_6     ""
#define PALETTE_RESET ""

#else

#define PALETTE_0     "\x1b[0;93m" // regular bright yellow
#define PALETTE_1     "\x1b[0;96m" // regular bright cyan
#define PALETTE_2     "\x1b[0;36m" // regular cyan
#define PALETTE_3     "\x1b[0;97m" "\x1b[41m" // regular bright white; red background
#define PALETTE_4     "\x1b[1;92m" // bold bright green
#define PALETTE_5     "\x1b[0;45m" // regular magenta background
#define PALETTE_6     "\x1b[0;95m" // regular bright magenta
#define PALETTE_RESET "\x1b[0m"  // reset

#endif

#endif /* ENGINE_TRACE_PORTABLE_INCLUDED */
