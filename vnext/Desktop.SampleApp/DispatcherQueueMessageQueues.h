// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "cxxreact/MessageQueueThread.h"
#include <winrt/Windows.System.h>

namespace Microsoft::React {

class DispatcherQueueMessageQueue : public facebook::react::MessageQueueThread
{
public:
  virtual void runOnQueue(std::function<void()>&&) override;
  virtual void runOnQueueSync(std::function<void()>&&) override;
  
protected:
  virtual winrt::Windows::System::DispatcherQueue GetDispatcherQueue() = 0;
};

class UIThreadDispatcherQueueMessageQueue final : public DispatcherQueueMessageQueue
{
public:
  UIThreadDispatcherQueueMessageQueue();

  virtual void quitSynchronous() override;

protected:
  virtual winrt::Windows::System::DispatcherQueue GetDispatcherQueue() override;

private:
  winrt::Windows::System::DispatcherQueue m_dispatcherQueue;
};

class BGThreadDispatcherQueueMessageQueue final : public DispatcherQueueMessageQueue
{
public:
  BGThreadDispatcherQueueMessageQueue();

  virtual void quitSynchronous() override;

protected:
  virtual winrt::Windows::System::DispatcherQueue GetDispatcherQueue() override;

private:
  winrt::Windows::System::DispatcherQueueController m_dispatcherQueueController;
};

}
