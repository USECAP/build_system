// Copyright (c) 2018 University of Bonn.

#include "intercept_settings.h"
#include "build_system/replacer/path.h"
#include <unistd.h>

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

void InterceptorClient::ReportInterceptedCommand(
    const CompilationCommand& orig_cc, const CompilationCommand& new_cc) {
  grpc::ClientContext context;
  Status response;

  auto cwd = current_directory();
  InterceptedCommand cmd;
  cmd.set_original_command(orig_cc.command);
  cmd.set_replaced_command(new_cc.command);
  cmd.set_directory(cwd);

  *cmd.mutable_original_arguments() = {orig_cc.arguments.begin(),
                                       orig_cc.arguments.end()};
  *cmd.mutable_replaced_arguments() = {new_cc.arguments.begin(),
                                       new_cc.arguments.end()};

  auto status = stub_->ReportInterceptedCommand(&context, cmd, &response);
  if (!status.ok()) {
    std::cout << "Error reporting command " << status.error_message()
              << status.error_details() << std::endl;
  }
}

void InterceptorClient::SetDefaultDeadline(grpc::ClientContext* context) {
  auto deadline =
      std::chrono::system_clock::now() + std::chrono::milliseconds(100);
  context->set_deadline(deadline);
}
