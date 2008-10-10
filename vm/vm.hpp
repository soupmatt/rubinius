#ifndef RBX_VM_H
#define RBX_VM_H

#include "prelude.hpp"
#include "globals.hpp"
#include "symboltable.hpp"

#include <pthread.h>

namespace llvm {
  class Module;
}

namespace rubinius {

  class Exception;

  namespace event {
    class Loop;
  }

  class GlobalCache;
  class TaskProbe;
  class Primitives;
  class ObjectMemory;
  class TypeInfo;
  class Task;
  class MethodContext;
  class ContextCache;
  class String;
  class ConfigParser;

  struct Configuration {
    bool compile_up_front;
  };

  struct Interrupts {
    bool check;
    bool switch_task;
    bool perform_gc;
    bool check_events;
    bool reschedule;
    bool use_preempt;
    bool enable_preempt;

    Interrupts() :
      check(false),
      switch_task(false),
      perform_gc(false),
      check_events(false),
      reschedule(false),
      use_preempt(false),
      enable_preempt(false)
    { }
  };

  class VM {
    public:
    /* Data members */
    Globals globals;
    ObjectMemory* om;
    event::Loop* events;
    event::Loop* signal_events;
    GlobalCache* global_cache;
    TypedRoot<TaskProbe*> probe;
    Primitives* primitives;
    Configuration config;
    Interrupts interrupts;
    SymbolTable symbols;
    ConfigParser *user_config;

    /* Used to implement a simple context cache */
    ContextCache* context_cache;

    bool reuse_llvm;

    // The thread used to trigger preemptive thread switching
    pthread_t preemption_thread;

    static const size_t default_bytes = 1048576;

    /* Inline methods */
    /* Prototypes */
    VM(size_t bytes = default_bytes);
    ~VM();

    // Returns the current VM state object.
    static VM* current_state();

    // Registers a VM* object as the current state.
    static void register_state(VM*);

    void bootstrap_class();
    void bootstrap_ontology();
    void bootstrap_symbol();
    void bootstrap_exceptions();
    void initialize_builtin_classes();
    void initialize_platform_data();
    void boot_threads();

    OBJECT new_object(Class* cls);

    // Create an uninitialized Class object
    Class* new_basic_class(Class* sup, size_t fields);

    // Create a Class of name +name+ as an Object subclass
    Class* new_class(const char* name);

    // Create a Class of name +name+ as a subclass of +super_class+
    Class* new_class(const char* name, Class* super_class);

    // Create a Class of name +name+ as a subclass of +sup+, having
    // +fields+ instance fields
    Class* new_class(const char* name, Class* sup, size_t fields);

    // Create a Class of name +name+ as a subclass of +sup+, having
    // +fields+ instance fields, under Module +under+
    Class* new_class(const char* name, Class* sup, size_t fields, Module* under);

    // Create a Class of name +name+ under +under+
    Class* new_class_under(const char* name, Module* under);

    Module* new_module(const char* name, Module* under = NULL);
    OBJECT new_struct(Class* cls, size_t bytes);
    Task* new_task();

    SYMBOL symbol(const char* str);
    SYMBOL symbol(String* str);
    SYMBOL symbol(std::string str);

    void add_type_info(TypeInfo* ti);
    TypeInfo* find_type(int type);

    void init_ffi();
    void init_native_libraries();

    Thread* current_thread();
    void collect();

    void return_value(OBJECT val);

    void check_events();
    bool run_best_thread();
    void queue_thread(Thread* thread);
    void activate_thread(Thread* thread);
    void activate_task(Task* task);
    bool find_and_activate_thread();

    void raise_from_errno(const char* reason);
    void raise_exception(Exception* exc);
    Exception* new_exception(Class* cls, const char* msg);
    OBJECT current_block();

    void set_const(const char* name, OBJECT val);
    void set_const(Module* mod, const char* name, OBJECT val);

    llvm::Module* llvm_module();
    void llvm_cleanup();

    void print_backtrace();

    // In an infinite loop, run the current task.
    void run_and_monitor();

    void setup_preemption();

    // Run in a seperate thread to provide preemptive thread
    // scheduling.
    void scheduler_loop();
  };
};

#endif
