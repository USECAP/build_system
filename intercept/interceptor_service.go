package main

import (
	"context"
	pb "gitlab.com/code-intelligence/core/build_system/proto"
	"os"
)

var (
	replaceCommand string
)

func init() {
	var ok bool
	replaceCommand, ok = os.LookupEnv("CC")
	if !ok {
		replaceCommand = "cc"
	}
}

const matchCommand = `^([^-]*-)*[mg]cc(-\d+(\.\d+){0,2})?$|` +
	`^([^-]*-)*clang(-\d+(\.\d+){0,2})?$|` +
	`^(|i)cc$|^(g|)xlc$`

type interceptorService struct {
	interceptedCommands chan *pb.InterceptedCommand
}

func newInterceptorService() *interceptorService {
	s := new(interceptorService)
	s.interceptedCommands = make(chan *pb.InterceptedCommand)
	return s
}

func (*interceptorService) GetInterceptSettings(ctx context.Context, req *pb.InterceptSettingsRequest) (*pb.InterceptSettings, error) {
	return &pb.InterceptSettings{
		MatchingRules: []*pb.MatchingRule{{
			MatchCommand:    matchCommand,
			ReplaceCommand:  replaceCommand,
			AddArguments:    []string{},
			RemoveArguments: []string{},
		}},
	}, nil
}

func (s *interceptorService) ReportInterceptedCommand(ctx context.Context, req *pb.InterceptedCommand) (*pb.Status, error) {
	s.interceptedCommands <- req
	return &pb.Status{}, nil
}
