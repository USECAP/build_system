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
	"path"

	"github.com/bazelbuild/rules_go/go/tools/bazel"
	"github.com/spf13/pflag"
	"github.com/spf13/viper"
	pb "gitlab.com/code-intelligence/core/build_system/proto"
	"gitlab.com/code-intelligence/core/build_system/types"
	"google.golang.org/grpc"
)

const serverAddr = "localhost:6774"
const compilerDbPath = "compile_commands.json"
const preloadLibPath = "code_intelligence/build_system/preload_interceptor/preload_interceptor.so"
const CompilationDbFlag = "create_compiler_db"

func init() {
	setDefaultValues()
}

func setDefaultValues() {

	base, err := bazel.RunfilesPath()
	if err != nil {
		log.Fatal("Runfiles base not found: ", err)
	}

	viper.SetConfigFile(path.Join(base, "code_intelligence", "build_system", "config", "config.yaml"))
	err = viper.ReadInConfig()
	if err != nil {
		panic(fmt.Errorf("Fatal error config file: %s \n", err))
	}
	pflag.Bool(CompilationDbFlag, false, "Whether to create compilation database")
	pflag.String("match_command", DefaultMatchCommand, "Override default match command")
	pflag.String("replace_command", "cc", "Whether to create compilation database")
	pflag.String("fuzzer", "", "Whether a specific fuzzer config should be used")

	viper.BindEnv("replace_command", "CC")
}

func main() {
	pflag.CommandLine.AddGoFlagSet(flag.CommandLine)
	pflag.Parse()
	viper.BindPFlags(pflag.CommandLine)

	buildCmd := pflag.Args()

	// if the fuzzer argument is set, set new defaults from config file with that fuzzer name; if not found ignore
	if desiredFuzzer := viper.GetString("fuzzer"); desiredFuzzer != "" && viper.IsSet("fuzzers."+desiredFuzzer) {
		fuzzerConfigPrefix := "fuzzers." + desiredFuzzer + "."
		viper.SetDefault("replace_command", viper.GetString(fuzzerConfigPrefix+"replace_command"))
		viper.SetDefault("add_arguments", viper.GetStringSlice(fuzzerConfigPrefix+"add_arguments"))
		viper.SetDefault("remove_arguments", viper.GetStringSlice(fuzzerConfigPrefix+"remove_arguments"))
	}

	service := newInterceptorService()

	if err := serve(service); err != nil {
		log.Fatal(err)
	}

	var interceptedCommands []*pb.InterceptedCommand
	go func() {
		for c := range service.interceptedCommands {
			interceptedCommands = append(interceptedCommands, c)
		}
	}()

	base, err := bazel.RunfilesPath()
	if err != nil {
		log.Fatal("Runfiles base not found: ", err)
	}

	env := os.Environ()
	env = append(env, "LD_PRELOAD="+path.Join(base, preloadLibPath))
	env = append(env, "REPORT_URL="+serverAddr)
	cmd := exec.Command(buildCmd[0], buildCmd[1:]...)
	cmd.Env = env
	out, err := cmd.CombinedOutput()
	if err != nil {
		log.Fatal("command crashed: ", err)
	}

	log.Print("out:\n", string(out))

	if viper.GetBool("create_compiler_db") {
		commands := createCompilationDb(interceptedCommands)
		out, err := json.Marshal(commands)
		if err != nil {
			panic(err)
		}
		err = ioutil.WriteFile(compilerDbPath, out, 0755)
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
	listener, err := net.Listen("tcp", serverAddr)
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
