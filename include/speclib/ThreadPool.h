/*
 SpecLib: Library for speculative execution of loops
 Copyright (C) 2023 Millan A. Martinez, Basilio B. Fraguela, Jose C. Cabaleiro, Francisco F. Rivera. Universidade da Coruna
 
 Distributed under the MIT License. (See accompanying file LICENSE)
*/

///
/// \file     ThreadPool.h
/// \brief    Provides a resizeable reusable pool of threads
/// \author   Millan A. Martinez  <millan.alvarez@udc.es>
/// \author   Basilio B. Fraguela <basilio.fraguela@udc.es>
/// \author   Jose C. Cabaleiro   <jc.cabaleiro@usc.es>
/// \author   Francisco F. Rivera <ff.rivera@usc.es>
///


#ifndef __THREADPOOL_H_
#define __THREADPOOL_H_

#include <vector>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>

/// \brief Resizeable and reusable pool of threads
/// \internal It must be manipulated by an external main thread, not one in the pool
class ThreadPool {
  
  std::vector<std::thread> threads_;
  std::mutex mutex_;
  std::condition_variable cond_var_;
  std::function<void()> func_;
  volatile size_t nthreads_in_use_; //< Number of threads that participate in parallel executions
  volatile size_t count_;
  volatile bool ready_, finish_;

  void main()
  {
    while (!finish_) {
      std::unique_lock<std::mutex> my_lock(mutex_);
      while (!ready_ && !finish_) {
        cond_var_.wait(my_lock);
      }
#if __cplusplus < 202002L
      const size_t my_id = count_++;
#else
      // '++' expression of 'volatile'-qualified type is deprecated in C++20
      const size_t my_id = count_;
      count_ = count_ + 1;
#endif
      if(count_ == threads_.size()) {
        ready_ = false;
      }
      my_lock.unlock();
      
      if (!finish_ && (my_id < nthreads_in_use_)) {
        func_();
      }
      
      while(ready_ && !finish_) {} // ensure all threads restarted
      my_lock.lock();
#if __cplusplus < 202002L
      count_--;
#else
      // '--' expression of 'volatile'-qualified type is deprecated in C++20
      count_ = count_ - 1;
#endif
      my_lock.unlock();
      
      //wait();
    }

  }

public:
  
  ThreadPool(const size_t n) :
  func_{[]{}}, count_{0}, ready_{false}, finish_{false}
  {
    resize(n);
  }
  
  size_t nthreads() const noexcept { return nthreads_in_use_; }

  /// \brief Change the number of threads in the pool
  /// \internal When the new number is larger than the actual number of threads currently
  ///           in the pool, new threads are created. But when the new number is smaller
  ///           the class simply changes the number of threads in use so that only the
  ///           required ones do work
  void resize(const size_t new_nthreads)
  {
    wait();

    // can only grow
    while (threads_.size() < new_nthreads) {
      threads_.emplace_back(&ThreadPool::main, this);
    }
    nthreads_in_use_ = new_nthreads;
  }

  void launchTheads()
  {
    if (nthreads_in_use_) {
      wait();
      {
        std::lock_guard<std::mutex> my_guard_lock(mutex_);
        ready_ = true;
      }
      cond_var_.notify_all();
    }
  }
  
  template<class F, class... Args>
  void setFunction(F&& f, Args&&... args)
  {
    func_ = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
  }
  
  /// ensure all threads finished. Spin wait
  void wait() const noexcept
  {
    while(ready_ || count_) {};
  }
  
  ~ThreadPool()
  {
    wait();
    
    finish_ = true;
    launchTheads();
    for (auto& thread : threads_) {
      thread.join();
    }
  }
  
};

#endif
