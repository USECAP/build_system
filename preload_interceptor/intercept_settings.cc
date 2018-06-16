// Copyright (c) 2018 Code Intelligence. All rights reserved.

#include <grpc++/channel.h>
#include <grpc++/create_channel.h>
#include <grpc++/grpc++.h>
#include "build_system/proto/intercept.grpc.pb.h"

class InterceptorClient {
 public:
  InterceptorClient(std::shared_ptr<grpc::Channel> channel)
      : stub_(Interceptor::NewStub(channel)) {}

  void Settings() {
    grpc::ClientContext context;
    InterceptSettingsRequest request;

    grpc::Status status =
        stub_->GetInterceptSettings(&context, request, &settings);
    if (!status.ok()) {
      std::cout << "Error receiving settings " << status.error_message()
                << status.error_details() << std::endl;
    }
  }

 private:
  std::unique_ptr<Interceptor::Stub> stub_;
  InterceptSettings settings;
};

void FetchSettings() {
  InterceptorClient client(grpc::CreateChannel(
      std::getenv("REPORT_URL"), grpc::InsecureChannelCredentials()));
  client.Settings();
}
