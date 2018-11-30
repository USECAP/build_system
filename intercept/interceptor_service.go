package main

import (
	"context"

	"github.com/spf13/viper"

	pb "gitlab.com/code-intelligence/core/build_system/proto"
)

type interceptorService struct {
	interceptedCommands chan *pb.InterceptedCommand
}

func newInterceptorService() *interceptorService {
	s := new(interceptorService)
	s.interceptedCommands = make(chan *pb.InterceptedCommand)
	return s
}

func (*interceptorService) GetInterceptSettings(ctx context.Context,
	req *pb.InterceptSettingsRequest) (*pb.InterceptSettings, error) {

	return &pb.InterceptSettings{
		MatchingRules: []*pb.MatchingRule{{
			MatchCommand:    viper.GetString("match_cc"),
			ReplaceCommand:  viper.GetString("replace_cc"),
			AddArguments:    viper.GetStringSlice("add_arguments"),
			RemoveArguments: viper.GetStringSlice("remove_arguments"),
		}, {
			MatchCommand:    viper.GetString("match_cxx"),
			ReplaceCommand:  viper.GetString("replace_cxx"),
			AddArguments:    viper.GetStringSlice("add_arguments"),
			RemoveArguments: viper.GetStringSlice("remove_arguments"),
		}},
	}, nil
}

func (s *interceptorService) ReportInterceptedCommand(ctx context.Context,
	req *pb.InterceptedCommand) (*pb.Status, error) {

	s.interceptedCommands <- req
	return &pb.Status{}, nil
}
