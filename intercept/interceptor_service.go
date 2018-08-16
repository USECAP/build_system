package main

import (
	"context"
	pb "gitlab.com/code-intelligence/core/build_system/proto"
	"github.com/spf13/viper"
	"fmt"
)

const DefaultMatchCommand = `^([^-]*-)*[mg]cc(-\d+(\.\d+){0,2})?$|` +
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
	fmt.Println(viper.GetString("replace_command"))
	return &pb.InterceptSettings{
		MatchingRules: []*pb.MatchingRule{{
			MatchCommand:    viper.GetString("match_command"),
			ReplaceCommand:  viper.GetString("replace_command"),
			AddArguments:    viper.GetStringSlice("add_arguments"),
			RemoveArguments: viper.GetStringSlice("remove_arguments"),
		}},
	}, nil
}

func (s *interceptorService) ReportInterceptedCommand(ctx context.Context, req *pb.InterceptedCommand) (*pb.Status, error) {
	s.interceptedCommands <- req
	return &pb.Status{}, nil
}
