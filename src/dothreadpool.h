/*
 * -------------
 *  Dark Oberon
 * -------------
 * 
 * An advanced strategy game.
 *
 * Copyright (C) 2002 - 2005 Valeria Sventova, Jiri Krejsa, Peter Knut,
 *                           Martin Kosalko, Marian Cerny, Michal Kral
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (see docs/gpl.txt) as
 * published by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 */

/**
 *  @file dowalk.cpp
 *
 *  Template for working with pool of threads.
 *
 *  @author Jiri Krejsa
 *
 *  @date 2004
 */


#ifndef __dothreadpool_h__
#define __dothreadpool_h__


//========================================================================
// Included files
//========================================================================

#ifdef NEW_GLFW3
#include <glfw3.h>
#include <tinycthread.h>
#else
#include <glfw.h>
#endif

#include "dopool.h"


//========================================================================
// Definitions
//========================================================================

#define THP_QUEUE_SIZE      100


//========================================================================
// Forward declarations
//========================================================================

template <class I, class O, class A> class TTHREAD_POOL;


//========================================================================
// Template TTHREAD_POOL
//========================================================================

/**
 *  The template enables working with pool of threads.
 *  The each thread from the pool is able to compute a response by a method
 *  which is a member of the third template argument @p A. This method has
 *  exactly one parameter which type is pointer to the class of the type from
 *  the first template argument @p I. The return value of this method has to be
 *  pointer instance of the second template argument @p O. The auxiliary data
 *  are exclusive for the each thread and is necessary existance of 
 *  of the constructor without parameters. The preparation for using of this 
 *  datas is in the responsibility of the method.
 *  The method which computes response is determine as the second parameter 
 *  of the request (@sa AddRequest).
 *  Its are possible two way of the template using. The first kind of the using
 *  is with the queue of the responses. The second kind is without it.
 *  The decision about using or not of the queue is maken during creating 
 *  of the pool (@sa CreateNewThreadPool). The response is added into 
 *  the responses queue only if the method prepares and returns pointer to 
 *  response object. If returns NULL nothing will be added into the queue. 
 *  The method is available for taking out the responses from the queue 
 *  (@sa TakeOutResponse).
 *
 *  @param I  It is class which encapsulate all request parameters.
 *  @param O  It is class which encapsulate all attributes of a response.
 *  @param A  It is class which contains all auxiliary datas which each thread
 *  needs for computing of the response to request.
 */
template <class I, class O, class A>
class TTHREAD_POOL {

  typedef O* (A::*TPMETHOD)(I*);

public:

  /** 
   *  The method creates new pool of the threads.
   *
   *  @param thread_count The count of threads in the asked thread pool.
   *  @param req_queue_size The expected queue length with requests. 
   *  The default value is @see THP_QUEUE_SIZE
   *  @param use_res_queue  The flag which says whether will be used queue
   *  with responses. The default value is false.
   *  @param res_queue_size The expected queue length with responses. 
   *  The default value is @see THP_QUEUE_SIZE
   *  @return The method returns pointer to the new thread pool. If count
   *  of the asked threads was lower than zero or creating of the pool failed
   *  then returns NULL.
   */
  static TTHREAD_POOL<I, O, A>* CreateNewThreadPool(int thread_count, unsigned int req_queue_size = THP_QUEUE_SIZE, 
                                                    bool use_res_queue = false, unsigned int res_queue_size = THP_QUEUE_SIZE)
  {
    //check whether is asked nonzero count of the threads
    if (thread_count <= 0)
      return NULL;

    TTHREAD_POOL<I, O, A> *new_threadpool = NEW TTHREAD_POOL<I, O, A>;

    //prepare requests queue
    new_threadpool->requests = new_threadpool->requests->CreateNewQueue(2*thread_count + req_queue_size, thread_count);
    if (new_threadpool->requests == NULL)
    {
      delete new_threadpool;
      return NULL;
    }

    new_threadpool->use_response_queue = use_res_queue;
    //if template user wants to use response queue then prepare it
    if (new_threadpool->use_response_queue)
    {
      new_threadpool->responses = new_threadpool->responses->CreateNewQueue(2*thread_count + res_queue_size, thread_count);
      if (new_threadpool->responses == NULL)
      {
        delete new_threadpool;  
        return NULL;
      }
    }
	
    //create condition variable
#ifdef NEW_GLFW3
	if(cnd_init(&new_threadpool->condition) == thrd_error) 
	{
      delete new_threadpool;
      return NULL;
    }
#else
	new_threadpool->condition = glfwCreateCond();
    if (new_threadpool->condition == NULL)
    {
      delete new_threadpool;
      return NULL;
    }
#endif
    

    //create condition mutex
#ifdef NEW_GLFW3
	if(mtx_init(&new_threadpool->condition_mutex, mtx_plain) == thrd_error) 
	{
      delete new_threadpool;
      return NULL;
    }
#else
	new_threadpool->condition_mutex = glfwCreateMutex();
    if (new_threadpool->condition_mutex == NULL) 
    {
      delete new_threadpool;
      return NULL;
    }
#endif
    
     //lock mutex to stop threads start too fast
#ifdef NEW_GLFW3
	mtx_lock(&new_threadpool->condition_mutex);
#else
	glfwLockMutex(new_threadpool->condition_mutex);
#endif
   
    //as last create instances of the class with thread info in the pool
    new_threadpool->threads = NEW TTHREAD<A>[thread_count];
    //create new threads
    int i = 0;
    for (bool last_ok = true; (i < thread_count) && last_ok; ++i)
      last_ok = new_threadpool->threads[i].CreateThreadNew(new_threadpool);
    //check success of creating all of the threads
    if (i < thread_count)
    {
      for (--i; i >= 0; --i)
      {
        new_threadpool->threads[i].KillThread();
      }
      new_threadpool->threads = NULL;

#ifdef NEW_GLFW3
	mtx_unlock(&new_threadpool->condition_mutex);
#else
	glfwUnlockMutex(new_threadpool->condition_mutex);
#endif
      
      delete new_threadpool;
      return NULL;
    }

    //unlock mutex, threads can start now
#ifdef NEW_GLFW3
	mtx_unlock(&new_threadpool->condition_mutex);
#else
	glfwUnlockMutex(new_threadpool->condition_mutex);
#endif
    

    return new_threadpool;
  }

  /** The destructor kills threads, and destroyes queues and condition variable.*/
  ~TTHREAD_POOL<I, O, A>()
  {
    //if exist array with threads kill them and destroy it
    if (threads != NULL)
      delete []threads;

	//if condition variable exists destroy it
#ifdef NEW_GLFW3
	cnd_destroy(&condition);
#else
	
    if (condition != NULL)
      glfwDestroyCond(condition);
#endif
    
	//if condition mutex exists destroy it
#ifdef NEW_GLFW3
	mtx_destroy(&condition_mutex);
#else
    if (condition_mutex != NULL)
      glfwDestroyMutex(condition_mutex);
#endif
    

    //if requests queue exists destroy it
    if (requests != NULL)
      delete requests;

    //if responses queue exists destroy it
    if (use_response_queue && (responses != NULL))
      delete responses;
  }

  /**
   *  The method adds the request to the thread pool.
   *  The parameter is handed over to the function which the threads compute.
   *
   *  @param new_request  The pointer to the object with request.
   *  @return The method returns count of the requests in the queue after 
   *  the addition of the request.
   */
  unsigned int AddRequest(I *request, O* (A::*processor)(I*))
  {
    //add request into the queue
    unsigned int count = requests->Push(request, processor);
	
	//wake up waiting threads
#ifdef NEW_GLFW3
	cnd_signal(&condition);
#else
	glfwSignalCond(condition);
#endif
	
    //return count of the waiting requests
    return count;
  }

  /**
   *  The method takes out the oldest response of the thread pool.
   *
   *  @return The method returns pointer to the oldest response of the thread
   *  pool. If was specified during creating of the thread pool that 
   *  the responses queue hasn't been used or the queue is empty then returns 
   *  NULL.
   */
  O* TakeOutResponse()
  {
    O* result = NULL;

    //if responses queue is used then return oldest response
    if (use_response_queue)
      result = responses->Pop();

    return result;
  }

  /**
  *  The function provides waiting for request of the threads in the pool. When
  *  the request arrives the thread take it from the requests queue. 
  *  The function which was specified with the request is started to compute 
  *  response to the request. The response is added into the responses queue
  *  if this queue is used and the response isn't NULL.
  *
  *  @param p_thread The pointer to the array of the threads in the pool.
  */
  static void GLFWCALL FunctionStarter(void *p_thread)
  {
    //get instance of the class with information about the thread
    TTHREAD<A> *thread = reinterpret_cast<TTHREAD<A>*>(p_thread);
    O* (A::*p_process_method)(I*);

    //infinite cycle
    while (true)
    {
      //get pool mutex necessary for condition checking
#ifdef NEW_GLFW3
		mtx_lock(&thread->threadpool->condition_mutex);
#else
		glfwLockMutex(thread->threadpool->condition_mutex);
#endif

	//while request queue is empty wait for signal
#ifdef NEW_GLFW3
		while (thread->threadpool->requests->GetQueueLength() == 0)
			cnd_wait(&thread->threadpool->condition, &thread->threadpool->condition_mutex);
#else
		
      while (thread->threadpool->requests->GetQueueLength() == 0)
        glfwWaitCond(thread->threadpool->condition, thread->threadpool->condition_mutex, GLFW_INFINITY);
#endif     
      

      #if DEBUG_THREADS
        Info(LogMsg("Request adopted by thread %d", (thread - thread->threadpool->threads)));
      #endif

      //get pointer to the process method
      p_process_method = thread->threadpool->requests->GetProcessMethodOfFirstNode();

      //take out request from the queue
      I* request = thread->threadpool->requests->Pop();
	
	//unlock pool mutex necessary for condition checking
#ifdef NEW_GLFW3
		mtx_unlock(&thread->threadpool->condition_mutex);
#else
		glfwUnlockMutex(thread->threadpool->condition_mutex);
#endif
      
      //compute response to request
      O* response = (thread->auxiliary_data.*p_process_method)(request);

      //if response queue is used and some response was created add into the response queue
      if (thread->threadpool->use_response_queue && (response != NULL))
        thread->threadpool->responses->Push(response, NULL);
    }
  }

private:

  /** The constructor zeroizes attributes.*/
  TTHREAD_POOL<I, O, A>()
  { 
    requests = NULL; 
    responses = NULL; 
    use_response_queue = false; 
    threads = NULL; 
    condition = NULL;
    condition_mutex = NULL;
  }

  /** The instances of the of the class encapsulate the queue of the swimmers. */
  template <class S>
  class TQUEUE {
    public:
    
    /** The instances of the class are members of the request and response 
     *  queues. */ 
    class TSWIMMER : protected TPOOL_ELEMENT {
      public:
        
        /** @return The method returns follower in the list.*/
        TSWIMMER* GetNext() const
          { return reinterpret_cast<TSWIMMER*>(pool_next);};
        
        /** The method sets follower in the request or response queue. 
         *  @param new_follower The new follower in the list.*/
        void SetNext(TSWIMMER *new_follower)
          { pool_next = new_follower;};
        
        /** @return The method returns pointer to the node cargo.*/
        S* GetCargo() const
          { return cargo;};

        /** The method sets cargo to value from the parameter. It doesn't destroy old cargo.
         *  @param new_cargo  The pointert to new cargo of the node. */
        void SetCargo(S *new_cargo)
          { cargo = new_cargo;};

        /** @return The method returns pointer to the method which process request. */
        TPMETHOD GetProcessMethod()
          { return p_method;}

        /** The method sets pointer to the method which process request. 
         *  @param new_method The pointet to the method which process request. */
        void SetProcessMethod(TPMETHOD new_method)
          { p_method = new_method;}

        /** The method clears not inherited data which aren't inherited 
         *  from the ancestor. 
         *  @param all  If is true clears all attributes. */
        virtual void Clear(bool all)
          { TPOOL_ELEMENT::Clear(all); cargo = NULL; p_method = NULL;}

      private:
        S *cargo;             //!< Cargo of the node.
        TPMETHOD p_method;    //!< The pointer to method which process request.
        //friend class TTHREAD_POOL<I, O, A>::TQUEUE<I>;
    };

    public:
      
      friend class TTHREAD_POOL<I, O, A>;
      
      /** 
       *  The method adds parameter at the end of the queue.
       *
       *  @param new_node Pointer to the added node.
       *  @param to_execute The pointer to method which process request.
       *  @return The method returns count of the nodes in the queue.
       */
      unsigned int Push(S *new_node, TPMETHOD to_execute)
      {
        TSWIMMER *swimmer = swimmers->GetFromPool();
        swimmer->SetNext(NULL);
        swimmer->SetCargo(new_node);
        swimmer->SetProcessMethod(to_execute);

        //get exclusive access to the queue
#ifdef NEW_GLFW3
		mtx_lock(&mutex);
#else
		glfwLockMutex(mutex);
#endif
        //if it is first node in the queue set new swimmer as head and tail
        if (tail == NULL)
          head = swimmer;
        else
          tail->SetNext(swimmer);
        
        //shift queue tail
        tail = swimmer;

        //increase count of the nodes in the queue
        ++length;

        //unlock mutex
#ifdef NEW_GLFW3
		mtx_unlock(&mutex);
#else
		glfwUnlockMutex(mutex);
#endif
        
        return length;
      }

      /** 
       *  The method takes out the beginning of the queue.
       *
       *  @return The method returns pointer to the content of the first node 
       *  in the queue. If the queue is empty then returns NULL. 
       */
      S* Pop()
      {
        S* result = NULL;
        TSWIMMER *swimmer = NULL;

        //get exclusive access to the queue
#ifdef NEW_GLFW3
		mtx_lock(&mutex);
#else
		glfwLockMutex(mutex);
#endif
        

        if (length > 0)
        {
          result = head->GetCargo();
          head->SetCargo(NULL);
          swimmer = head;
          head = head->GetNext();

          //if it is last node in the queue set NULL to head and tail
          if (length == 1)
          {
            tail = NULL;
          }

          //decrease count of the nodes in the queue
          --length;
        }

        //unlock mutex
#ifdef NEW_GLFW3
		mtx_unlock(&mutex);
#else
		glfwUnlockMutex(mutex);
#endif
 
        if (swimmer != NULL)
        {
          //return swimmer back to the pool
          swimmer->SetCargo(NULL);
          swimmer->SetProcessMethod(NULL);
          swimmers->PutToPool(swimmer);
        }

        return result;
      }

      /**
       *  @return The method returns the pointer to the process method of the first
       *  node in the queue. The pointer is returned as void* but is returned 
       *  the member pointer to the attribute p_method which type is 
       *  O* (A::*new_method)(I*).
       */
      TPMETHOD GetProcessMethodOfFirstNode()
      { return ((head != NULL)?(head->GetProcessMethod()):(NULL));};

      /** 
       *  The method creates new queue with pool.
       *  @param swimmers_count The count of the swimmers in the pool.
       *  @param count_critical The minimal count of the swimmers in the pool.
       *  @return The method returns pointer to the new queue if success otherwise returns NULL. 
       */
      static TQUEUE<S>* CreateNewQueue(int swimmers_count, int count_critical)
      {
        //get new empty queue without pool
        TQUEUE<S>* new_queue = NEW TQUEUE<S>;

        new_queue->swimmers = new_queue->swimmers->CreateNewPool(swimmers_count, count_critical, count_critical);
        //test success of creating new pool
        if (new_queue->swimmers == NULL)
        {
          delete new_queue;
          return NULL;
        }

        //create queue mutex necessary for synchronize access to queue
#ifdef NEW_GLFW3
		if(mtx_init(&new_queue->mutex, mtx_plain) == thrd_error) 
		{
          delete new_queue;
          return NULL;
        }
#else
		new_queue->mutex = glfwCreateMutex();
        if (new_queue->mutex == NULL) 
        {
          delete new_queue;
          return NULL;
        }
#endif
       
        return new_queue;
      }

      /** @return The method returns length of the queue. */
      unsigned int GetQueueLength() const
        { return length;};

      /** 
       *  The destructor of the queue first destroy all nodes in the queue and then 
       *  destroy the mutex.
       */
      ~TQUEUE<S>()
      {
#ifdef NEW_GLFW3
		mtx_lock(&mutex);
#else
		glfwLockMutex(mutex);
#endif
        
        for (S* poped = this->Pop(); poped; poped = this->Pop())
          delete poped;
          
        delete swimmers;
#ifdef NEW_GLFW3
		mtx_destroy(&mutex);
#else
		glfwDestroyMutex(mutex);
#endif
      }

    private:
      /** The constructor creates empty queue. */
      TQUEUE<S>()
      {
        head = NULL; 
        tail = NULL; 
        length = 0;
        swimmers = NULL;
#ifdef NEW_GLFW3
		mutex = 0;
#else
		mutex = NULL;
#endif
        
      }

    private:
      TPOOL<TSWIMMER> *swimmers;   //!< The pool with preprepared nodes of the queue.
      TSWIMMER *head;    //!< The pointer to the first node in the queue.
      TSWIMMER *tail;    //!< The pointer to the last node in the queue.
      unsigned int length;        //!< The length of the queue.
#ifdef NEW_GLFW3
	mtx_t mutex;
#else
	GLFWmutex mutex;            //!< The mutex which supports exclusive access to the queue.
#endif
      
  };

  /** The template encapsulate thread and his auxiliary class.*/
  template <class T>
  class TTHREAD {
    public:

      /** Constructor creates new thread.*/
      TTHREAD<T>()
      { 
        thread = -1;
        threadpool = NULL;
      }

      /**
       *  The method creates new thread.
       *
       *  @param tp The pointer to the pool of the threads in which is 
       *  new thread created.
       *  @return The method returns true if creating was successful otherwise
       *  returns false.
       */
      bool CreateThreadNew(TTHREAD_POOL<I, O, A> *tp)
      {
        threadpool = tp;
        
#ifdef NEW_GLFW3
		if(thrd_create(&thread, FunctionStarter, this) == thrd_success)
			return true;
		else
			return false;
#else
		thread = glfwCreateThread(FunctionStarter, this);
		if (thread >= 0)
          return true;
        else
          return false;
#endif
        

        
      }
#ifdef NEW_GLFW3
	/** @return The method returns pointer to the thread.*/
    thrd_t GetThread(){ return thread;};
#else
	/** @return The method returns pointer to the thread.*/
    GLFWthread GetThread(){ return thread;};
#endif
      

#ifdef NEW_GLFW3
	/** The method kills the thread.*/
      void KillThread()
      { 
      	int res;
        thrd_join(thread, &res);
 
        thread = -1;
      }
#else
	/** The method kills the thread.*/
      void KillThread()
      { 
        if (thread >= 0)
          glfwDestroyThread(thread); 
        thread = -1;
      }
#endif
      

      /** Destructor kills thread if still is alive. */
      ~TTHREAD<T>()
      { 
        KillThread();
        threadpool = NULL;
      }

      /** @return The method returns reference to the threads auxiliary data.*/
      A& GetAuxiliaryData()
        { return auxiliary_data;};

    private:
    	
#ifdef NEW_GLFW3
	thrd_t thread;        //!< The thread from the pool.
#else
	GLFWthread thread;        //!< The thread from the pool.
#endif
      
      A auxiliary_data;         //!< Auxiliary data of the thread.
      TTHREAD_POOL<I, O, A> *threadpool;    //!< The pointer to the pool which member the thread is.
      friend class TTHREAD_POOL<I, O, A>;
  };

  friend bool TTHREAD_POOL<I, O, A>::TTHREAD<A>::CreateThreadNew(TTHREAD_POOL<I, O, A> *threadpool);

private:

  TQUEUE<I> *requests;     //!< The queue of the requests.
  
#ifdef NEW_GLFW3
	mtx_t condition_mutex;
	cnd_t condition;
#else
	GLFWmutex condition_mutex;                      //!< The mutex used in pair with condtion variable.
	GLFWcond condition;           //!< The condition variable which synchronizes threads in the pool.
#endif  
  
  TQUEUE<O> *responses;    //!< The queue of the responses.
  TTHREAD<A> *threads;     //!< The array of the threads with its exclusive auxiliary data in the pool.
  bool use_response_queue;      //!< The flag which informs whether is used the queue of the responses.
  
};


#endif //__dothreadpool_h__


//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

