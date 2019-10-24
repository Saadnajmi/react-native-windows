// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "stdafx.h"

#include "DispatcherQueueMessageQueues.h"
#include <future>

namespace WS = winrt::Windows::System;

namespace Microsoft::React {

void DispatcherQueueMessageQueue::runOnQueue(std::function<void()>&& fn)
{
  auto dispatcherQueue = GetDispatcherQueue();

  if (!dispatcherQueue)
    return;

  dispatcherQueue.TryEnqueue([fn { std::move(fn) }]()
  {
    fn();
  });
}

void DispatcherQueueMessageQueue::runOnQueueSync(std::function<void()>&& fn)
{
  auto dispatcherQueue = GetDispatcherQueue();

  if (!dispatcherQueue)
    return;

  if (dispatcherQueue.HasThreadAccess())
  {
    fn();
  }
  else
  {
    std::promise<void> voidPromise;
    auto future = voidPromise.get_future();
    WS::DispatcherQueueHandler syncTask {
      [fn { std::move(fn) }, vp { std::move(voidPromise) }] () mutable
      {
        fn();
        vp.set_value();
      }
    };

    if (dispatcherQueue.TryEnqueue(syncTask))
    {
      future.wait();
    }
  }
}

UIThreadDispatcherQueueMessageQueue::UIThreadDispatcherQueueMessageQueue()
  : m_dispatcherQueue(WS::DispatcherQueue::GetForCurrentThread())
{
}

void UIThreadDispatcherQueueMessageQueue::quitSynchronous()
{
  if (!m_dispatcherQueue)
    return;

  WS::DispatcherQueue dispatcherQueue { nullptr };
  std::swap(m_dispatcherQueue, dispatcherQueue); // not thread-safe

  if (!dispatcherQueue.HasThreadAccess())
  {
    std::promise<void> voidPromise;
    auto future = voidPromise.get_future();
    WS::DispatcherQueueHandler syncTask {
      [vp { std::move(voidPromise) }] () mutable
      {
        vp.set_value();
      }
    };

    if (dispatcherQueue.TryEnqueue(syncTask))
    {
      future.wait();
    }
  }
}

WS::DispatcherQueue UIThreadDispatcherQueueMessageQueue::GetDispatcherQueue()
{
  return m_dispatcherQueue;
}

BGThreadDispatcherQueueMessageQueue::BGThreadDispatcherQueueMessageQueue()
  : m_dispatcherQueueController(WS::DispatcherQueueController::CreateOnDedicatedThread())
{
}

void BGThreadDispatcherQueueMessageQueue::quitSynchronous()
{
  WS::DispatcherQueueController dispatcherQueueController { nullptr };
  std::swap(m_dispatcherQueueController, dispatcherQueueController);
  (void) dispatcherQueueController.ShutdownQueueAsync();
  
}

WS::DispatcherQueue BGThreadDispatcherQueueMessageQueue::GetDispatcherQueue()
{
  if (m_dispatcherQueueController)
    return m_dispatcherQueueController.DispatcherQueue();
  else
    return nullptr;
}


}
