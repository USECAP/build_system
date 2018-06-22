// Copyright (c) 2018 University of Bonn.

#pragma once

#include <grpc++/channel.h>
#include <grpc++/create_channel.h>
#include <grpc++/grpc++.h>

#include "build_system/proto/intercept.grpc.pb.h"
#include "absl/types/optional.h"

/**
 * FetchSettings fetches the intercept settings from the GRPC server.
 */
class InterceptorClient {
 public:
  InterceptorClient(std::shared_ptr<grpc::Channel> channel);
  ~InterceptorClient() = default;

  absl::optional<InterceptSettings> GetSettings();
  void ReportInterceptedCommand(const char *path, char *const argv[],
                                const char *new_path, char *const new_argv[]);

 private:
  void SetDefaultDeadline(grpc::ClientContext *context);

  std::unique_ptr<Interceptor::Stub> stub_;
};

