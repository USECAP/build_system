package main

import (
	"encoding/json"
	"flag"
	"fmt"
	"io/ioutil"
	"log"
	"net"
	"os"
	"os/exec"
	"strings"

	"github.com/spf13/pflag"
	"github.com/spf13/viper"
	"gitlab.com/code-intelligence/core/build_system/intercept/internal/config"
	pb "gitlab.com/code-intelligence/core/build_system/proto"
	"gitlab.com/code-intelligence/core/build_system/types"
	pathUtil "gitlab.com/code-intelligence/core/utils/pathutils"
	"google.golang.org/grpc"
)

func usage() {
	fmt.Printf("Usage: %s [OPTIONS] BUILD_COMMAND ...\n", os.Args[0])
	pflag.PrintDefaults()
}

func main() {
	pflag.Usage = usage
	pflag.CommandLine.AddGoFlagSet(flag.CommandLine)
	pflag.Parse()
	viper.BindPFlags(pflag.CommandLine)

	buildCmd := pflag.Args()

	if len(buildCmd) == 0 {
		pflag.Usage()
		os.Exit(1)
	}

	service := newInterceptorService()

	if err := serve(service); err != nil {
		log.Fatal(err)
	}

	var interceptedCommands []*pb.InterceptedCommand
	go func() {
		for c := range service.interceptedCommands {
			originalCmd := strings.Join(c.OriginalArguments, " ")
			replacedCmd := strings.Join(c.ReplacedArguments, " ")
			fmt.Printf("Original Command:\n%s\n", originalCmd)
			fmt.Printf("Replaced Command:\n%s\n", replacedCmd)
			fmt.Println("---------------------------------")
			interceptedCommands = append(interceptedCommands, c)
		}
	}()

	env := os.Environ()

	preloadLibPath, err := pathUtil.Find("code_intelligence/build_system/preload_interceptor/preload_interceptor.so")
	if err != nil {
		log.Fatalf("Failed to find preload_interceptor.so: %q", err)
	}

	env = append(env, fmt.Sprintf("LD_PRELOAD=%s", preloadLibPath))
	env = append(env, "REPORT_URL="+config.ServerAddr)
	env = append(env, "INTERCEPT_SETTINGS="+config.InterceptSettings().String())
	cmd := exec.Command(buildCmd[0], buildCmd[1:]...)
	cmd.Env = env
	out, err := cmd.CombinedOutput()
	log.Print("out:\n", string(out))
	if err != nil {
		log.Fatal("command crashed: ", err)
	}

	if viper.GetBool("create_compiler_db") {
		commands := createCompilationDb(interceptedCommands)
		out, err := json.MarshalIndent(commands, "", "    ")
		if err != nil {
			panic(err)
		}
		err = ioutil.WriteFile(config.CompilerDbPath, out, 0755)
		if err != nil {
			panic(err)
		}
	}
}

func createCompilationDb(cmds []*pb.InterceptedCommand) (res []types.CompilationCommand) {
	for _, cmd := range cmds {
		log.Printf("cmd: %+v", cmd)
		cl := commandLineFromInterceptedCommand(cmd)
		for _, inputFile := range cl.inputFiles {
			res = append(res, types.CompilationCommand{
				Arguments: cmd.ReplacedArguments,
				Directory: cmd.Directory,
				Output:    cl.outputFile,
				File:      inputFile,
			})
		}

	}
	return res
}

func serve(service *interceptorService) error {
	listener, err := net.Listen("tcp", config.ServerAddr)
	if err != nil {
		return err
	}
	rpcServer := grpc.NewServer()
	pb.RegisterInterceptorServer(rpcServer, service)

	go func() {
		err = rpcServer.Serve(listener)
		if err != nil {
			log.Fatal(err)
		}
	}()

	return nil
}
