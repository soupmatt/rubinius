#include "signal.hpp"
#include "vm.hpp"

#include "native_thread.hpp"

#include "builtin/module.hpp"
#include <iostream>
#include <sys/select.h>
#include <fcntl.h>

namespace rubinius {
  struct CallFrame;

  static SignalHandler* handler_ = 0;
  pthread_t main_thread;

  SignalHandler::SignalHandler(VM* vm)
    : thread::Thread()
    , vm_(vm)
    , queued_signals_(0)
    , executing_signal_(false)
  {
    handler_ = this;
    main_thread = pthread_self();

    for(int i = 0; i < NSIG; i++) {
      pending_signals_[i] = 0;
      running_signals_[i] = 0;
    }

    int f[2];
    pipe(f);
    read_fd_ = f[0];
    write_fd_ = f[1];
  }

  void SignalHandler::on_fork() {
    if(handler_) handler_->run();
  }

  void SignalHandler::perform() {
    sigset_t set;
    sigfillset(&set);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    for(;;) {
      fd_set fds;
      FD_ZERO(&fds);
      FD_SET(read_fd_, &fds);

      int n = select(read_fd_ + 1, &fds, NULL, NULL, NULL);
      if(n == 1) {
        // drain a bunch
        char buf[512];
        read(read_fd_, buf, sizeof(buf));

        vm_->global_lock().lock();

        vm_->check_local_interrupts = true;
        vm_->wakeup();

        vm_->global_lock().unlock();

        // Spin until we see that vm_ has actually picked things up
        struct timespec ts = { 0, 1000000 };
        nanosleep(&ts, NULL);

        while(vm_->check_local_interrupts) {
          vm_->global_lock().lock();

          vm_->wakeup();

          vm_->global_lock().unlock();

          nanosleep(&ts, NULL);
        }
      }
    }
  }

  void SignalHandler::signal_tramp(int sig) {
    handler_->handle_signal(sig);
  }

  void SignalHandler::handle_signal(int sig) {
    queued_signals_ = 1;
    pending_signals_[sig] = 1;

    // If the main thread is running, just tell it that
    // there are local interrupts waiting.
    if(!vm_->waiting_p()) {
      vm_->check_local_interrupts = true;
      return;
    }

    if(vm_->should_interrupt_with_signal()) {
      vm_->check_local_interrupts = true;

      if(pthread_self() != main_thread) {
        pthread_kill(main_thread, SIGVTALRM);
      }
      return;
    }

    write(write_fd_, "!", 1);
  }

  void SignalHandler::add_signal(int sig, bool def) {
    sigset_t sigs;
    sigemptyset(&sigs);
    sigaddset(&sigs, sig);
    sigprocmask(SIG_UNBLOCK, &sigs, NULL);

    struct sigaction action;

    if(def) {
      action.sa_handler = SIG_DFL;
    } else {
      action.sa_handler = signal_tramp;
    }

    action.sa_flags = 0;
    sigfillset(&action.sa_mask);

    sigaction(sig, &action, NULL);
  }

  void SignalHandler::deliver_signals(CallFrame* call_frame) {
    if(executing_signal_) return;
    executing_signal_ = true;
    queued_signals_ = 0;

    // We run all handlers, even if one handler raises an exception. The
    // rest of the signals handles will simply see the exception.
    for(int i = 0; i < NSIG; i++) {
      if(pending_signals_[i] > 0) {
        pending_signals_[i] = 0;
        running_signals_[i] = 1;

        Array* args = Array::create(vm_, 1);
        args->set(vm_, 0, Fixnum::from(i));

        vm_->globals().rubinius->send(vm_, call_frame,
            vm_->symbol("received_signal"), args, Qnil);
        running_signals_[i] = 0;
      }
    }

    executing_signal_ = false;
  }
}
