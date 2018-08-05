package main

import (
	"encoding/json"
	"flag"
	"github.com/bazelbuild/rules_go/go/tools/bazel"
	"gitlab.com/code-intelligence/core/build_system/types"
	pb "gitlab.com/code-intelligence/core/build_system/proto"
	"google.golang.org/grpc"
	"io/ioutil"
	"log"
	"net"
	"os"
	"os/exec"
	"path"
)

const serverAddr = "localhost:6774"
const compilerDbPath = "compile_commands.json"
const preloadLibPath = "code_intelligence/build_system/preload_interceptor/preload_interceptor.so"

var createCompilerDb = flag.Bool(
	"create_compiler_db",
	false,
	"Whether to create the compiler db")

func main() {
	flag.Parse()
	buildCmd := flag.Args()
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

	if *createCompilerDb {
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
