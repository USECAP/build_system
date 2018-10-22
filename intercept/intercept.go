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
	"github.com/spf13/pflag"
	"github.com/spf13/viper"
	pb "gitlab.com/code-intelligence/core/build_system/proto"
	"gitlab.com/code-intelligence/core/build_system/types"
	"google.golang.org/grpc"
	pathUtil "gitlab.com/code-intelligence/core/utils/pathutils"
)

const serverAddr = "localhost:6774"
const compilerDbPath = "compile_commands.json"
const CompilationDbFlag = "create_compiler_db"

func init() {
	setDefaultValues()
}

func setDefaultValues() {

	configFilePath, err := pathUtil.Find("code_intelligence/build_system/config/config.yaml")
	if err != nil {
		log.Fatalf("Failed to find config.yaml: %q", err)
	}

	viper.SetConfigFile(configFilePath)
	err = viper.ReadInConfig()
	if err != nil {
		panic(fmt.Errorf("Fatal error config file: %s \n", err))
	}
	pflag.Bool(CompilationDbFlag, false, "Whether to create compilation database")
	pflag.String("match_cc", ccMatchCommand, "Override default cc match command")
	pflag.String("match_cxx", cxxMatchCommand, "Override default cxx match command")
	pflag.String("replace_cc", "cc", "Whether to replace cc")
	pflag.String("replace_cxx", "c++", "Whether to replace cxx")
	pflag.String("fuzzer", "", "Whether a specific fuzzer config should be used")

	viper.BindEnv("replace_cc", "CC")
	viper.BindEnv("replace_cxx", "CXX")
}

func main() {
	pflag.CommandLine.AddGoFlagSet(flag.CommandLine)
	pflag.Parse()
	viper.BindPFlags(pflag.CommandLine)

	buildCmd := pflag.Args()

	// if the fuzzer argument is set, set new defaults from config file with that fuzzer name; if not found ignore
	if desiredFuzzer := viper.GetString("fuzzer"); desiredFuzzer != "" && viper.IsSet("fuzzers."+desiredFuzzer) {
		fuzzerConfigPrefix := "fuzzers." + desiredFuzzer + "."
		viper.SetDefault("replace_cc", viper.GetString(fuzzerConfigPrefix+"replace_cc"))
		viper.SetDefault("replace_cxx", viper.GetString(fuzzerConfigPrefix+"replace_cxx"))
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
			fmt.Printf("%+v\n", c)
			interceptedCommands = append(interceptedCommands, c)
		}
	}()

	env := os.Environ()

	preloadLibPath, err := pathUtil.Find("code_intelligence/build_system/preload_interceptor/preload_interceptor.so")
	if err != nil {
		log.Fatalf("Failed to find preload_interceptor.so: %q", err)
	}

	env = append(env, fmt.Sprintf("LD_PRELOAD=%s", preloadLibPath))
	env = append(env, "REPORT_URL="+serverAddr)
	env = append(env, "INTERCEPT_SETTINGS="+settings())
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

func settings() string {
	settings := &pb.InterceptSettings{
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
	}
	return settings.String()
}
