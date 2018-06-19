// Copyright (c) 2018 Code Intelligence. All rights reserved.

#pragma once

#include <grpc++/channel.h>
#include <grpc++/create_channel.h>
#include <grpc++/grpc++.h>

#include "build_system/proto/intercept.grpc.pb.h"

/**
 * FetchSettings fetches the intercept settings from the GRPC server.
 */
class InterceptorClient {
 public:
  InterceptorClient(std::shared_ptr<grpc::Channel> channel);

  void GetSettings();
  void ReportInterceptedCommand(const char *path, char *const argv[], const char *new_path, char *const new_argv[]);
  void Done();

 private:
  std::unique_ptr<Interceptor::Stub> stub_;
  InterceptSettings settings_;
  std::unique_ptr<grpc::ClientContext> intercepted_command_context_;
  std::unique_ptr<grpc::ClientWriter<InterceptedCommand> > writer_;
};

InterceptorClient FetchSettings();

