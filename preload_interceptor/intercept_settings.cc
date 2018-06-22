// Copyright (c) 2018 University of Bonn.

#include "intercept_settings.h"
#ifdef CXX17
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

InterceptorClient::InterceptorClient(std::shared_ptr<grpc::Channel> channel)
    : stub_(Interceptor::NewStub(channel)) {}

absl::optional<InterceptSettings> InterceptorClient::GetSettings() {
  grpc::ClientContext context;
  InterceptSettingsRequest request;
  InterceptSettings settings;

  SetDefaultDeadline(&context);

  grpc::Status status =
      stub_->GetInterceptSettings(&context, request, &settings);
  if (!status.ok()) {
    std::cout << "Error receiving settings " << status.error_message()
              << status.error_details() << std::endl;
    return {};
  }
  return settings;
}

void InterceptorClient::ReportInterceptedCommand(const char *path,
                                                 char *const argv[],
                                                 const char *new_path,
                                                 char *const new_argv[]) {
  grpc::ClientContext context;
  Status response;
  auto cwd = fs::current_path();
  InterceptedCommand cmd;
  cmd.set_original_command(path);
  cmd.set_replaced_command(new_path);
  cmd.set_directory(cwd);
  cmd.set_file("");
  cmd.set_output("");

  SetDefaultDeadline(&context);

  // This assumes the correct handling of argv in the caller
  for (std::size_t arg_num = 1; argv[arg_num] != nullptr; arg_num++) {
    cmd.add_original_arguments(argv[arg_num]);
  }
  // This assumes the correct handling of new_argv in the caller
  for (std::size_t arg_num = 1; new_argv[arg_num] != nullptr; arg_num++) {
    cmd.add_replaced_arguments(new_argv[arg_num]);
  }
  auto status = stub_->ReportInterceptedCommand(&context, cmd, &response);
  if (!status.ok()) {
    std::cout << "Error reporting command " << status.error_message()
              << status.error_details() << std::endl;
  }
}

void InterceptorClient::SetDefaultDeadline(grpc::ClientContext *context) {
  auto deadline =
      std::chrono::system_clock::now() + std::chrono::milliseconds(100);
  context->set_deadline(deadline);
}
