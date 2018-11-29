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
	pb "gitlab.com/code-intelligence/core/build_system/proto"
	"gitlab.com/code-intelligence/core/build_system/types"
	pathUtil "gitlab.com/code-intelligence/core/utils/pathutils"
	"google.golang.org/grpc"
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
	pflag.String("replace_cc", "clang", "The command to replace the C compiler with")
	pflag.String("replace_cxx", "clang++", "The command to replace the C++ compiler with")
	pflag.String("fuzzer", "", "Whether a specific fuzzer config should be used")
	pflag.String("sanitizer_flags", "", "The sanitizer flags")
	viper.BindEnv("replace_cc", "CC")
	viper.BindEnv("replace_cxx", "CXX")
	viper.BindEnv("sanitizer_flags", "SANITIZER_FLAGS")
}

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

	// if the fuzzer argument is set, set new defaults from config file with that fuzzer name; if not found ignore
	if desiredFuzzer := viper.GetString("fuzzer"); desiredFuzzer != "" && viper.IsSet("fuzzers."+desiredFuzzer) {
		fuzzerConfigPrefix := "fuzzers." + desiredFuzzer + "."
		viper.SetDefault("replace_cc", viper.GetString(fuzzerConfigPrefix+"replace_cc"))
		viper.SetDefault("replace_cxx", viper.GetString(fuzzerConfigPrefix+"replace_cxx"))

		removeArgs, err := arguments(fuzzerConfigPrefix, "remove_arguments")
		if err != nil {
			panic("failed to fetch the remove_arguments configuration")
		}
		viper.SetDefault("remove_arguments", removeArgs)

		addArgs, err := arguments(fuzzerConfigPrefix, "add_arguments")
		if err != nil {
			panic("failed to fetch the add_arguments configuration")
		}

		if sanFlags := viper.GetString("sanitizer_flags"); sanFlags != "" {
			addArgs = append(addArgs, strings.Fields(sanFlags)...)
		}
		viper.SetDefault("add_arguments", addArgs)
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

func arguments(prefix, tag string) (args []string, err error) {
	configKey := prefix + tag
	args = viper.GetStringSlice(configKey)
	args = append(args, viper.GetStringSlice(configKey+"+")...)
	return
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
