// Copyright (c) 2018 University of Bonn.

#pragma once

#include <grpc++/channel.h>
#include <grpc++/create_channel.h>
#include <grpc++/grpc++.h>

#include "absl/types/optional.h"
#include "build_system/intercept_support/compilation_command.h"
#include "build_system/proto/intercept.grpc.pb.h"

/**
 * FetchSettings fetches the intercept settings from the GRPC server.
 */
class InterceptorClient {
 public:
  InterceptorClient(std::shared_ptr<grpc::Channel> channel);
  ~InterceptorClient() = default;

  absl::optional<InterceptSettings> GetSettings();
  void ReportInterceptedCommand(const CompilationCommand& orig_cc,
                                const CompilationCommand& new_cc);

      private : void SetDefaultDeadline(grpc::ClientContext* context);

  std::unique_ptr<Interceptor::Stub> stub_;
};
