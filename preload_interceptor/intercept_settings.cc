// Copyright (c) 2018 Code Intelligence. All rights reserved.

#include "intercept_settings.h"

#ifdef CXX17
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

InterceptorClient::InterceptorClient(std::shared_ptr<grpc::Channel> channel)
    : stub_(Interceptor::NewStub(channel)), settings_() {
  Status status;
  intercepted_command_context_ = std::make_unique<grpc::ClientContext>();
  writer_ = stub_->ReportInterceptedCommand(intercepted_command_context_.get(),
                                            &status);
}

void InterceptorClient::GetSettings() {
  grpc::ClientContext context;
  InterceptSettingsRequest request;

  grpc::Status status =
      stub_->GetInterceptSettings(&context, request, &settings_);
  if (!status.ok()) {
    std::cout << "Error receiving settings " << status.error_message()
              << status.error_details() << std::endl;
  }
}

void InterceptorClient::ReportInterceptedCommand(const char *path,
                                                 char *const argv[],
                                                 const char *new_path,
                                                 char *const new_argv[]) {
  auto cwd = fs::current_path();
  InterceptedCommand cmd;
  cmd.set_original_command(path);
  cmd.set_replaced_command(new_path);
  cmd.set_directory(cwd);
  cmd.set_file("");
  cmd.set_output("");

  // This assumes the correct handling of argv in the caller
  for (std::size_t arg_num = 1; argv[arg_num] != nullptr; arg_num++) {
    cmd.add_original_arguments(argv[arg_num]);
  }
  // This assumes the correct handling of new_argv in the caller
  for (std::size_t arg_num = 1; new_argv[arg_num] != nullptr; arg_num++) {
    cmd.add_replaced_arguments(new_argv[arg_num]);
  }
  writer_.get()->Write(cmd);
}

void InterceptorClient::Done() { writer_.get()->WritesDone(); }

InterceptorClient FetchSettings() {
  auto client = InterceptorClient(grpc::CreateChannel(
      std::getenv("REPORT_URL"), grpc::InsecureChannelCredentials()));
  client.GetSettings();
  return client;
}
