package test

import (
	"encoding/json"
	"io/ioutil"
	"os"
	"os/exec"
	"path"
	"reflect"
	"testing"

	"fmt"
	"log"
	"path/filepath"

	"github.com/bazelbuild/rules_go/go/tools/bazel"
	"github.com/spf13/viper"
	"gitlab.com/code-intelligence/core/build_system/types"
)

var (
	interceptPath string
	cwd           string
	dbFilePath    string
)

func readCompilationDb(dbFilePath string) (commands []types.CompilationCommand, err error) {
	// expect that dbFilePath exists
	db, err := ioutil.ReadFile(dbFilePath)
	if err != nil {
		return
	}
	defer os.Remove(dbFilePath)
	err = json.Unmarshal(db, &commands)

	return
}

func readCompilationDbForExec(env []string, args ...string) (commands []types.CompilationCommand, err error) {
	cmd := exec.Command(interceptPath, args...)
	if env != nil {
		cmd.Env = env
	}
	cmd.Dir = cwd
	out, err := cmd.CombinedOutput()
	fmt.Println("out", string(out))
	if err != nil {
		err = fmt.Errorf("exec failed, error: %s, output: %s", err, string(out))
		return
	}
	commands, err = readCompilationDb(dbFilePath)
	if err != nil {
		return
	}

	return
}

func TestInterceptor(t *testing.T) {

	env := os.Environ()
	env = append(env, "CC=echo")
	commands, err := readCompilationDbForExec(env, "--create_compiler_db", "make")
	if err != nil {
		t.Fatal(err)
	}

	expected := []types.CompilationCommand{{
		Arguments: []string{"echo", "-O2", "hello.c", "-o", "hello.o"},
		Directory: cwd,
		Output:    "hello.o",
		File:      "hello.c",
	}}

	if !reflect.DeepEqual(commands, expected) {
		t.Errorf("expected %+v to equal %+v", commands, expected)
	}
}

func TestMatchCommand(t *testing.T) {
	commands, err := readCompilationDbForExec(nil, "--create_compiler_db", "--match_command", "gcc", "--replace_command", "clang-6.0", "make")
	if err != nil {
		t.Fatal(err)
	}

	expected := []types.CompilationCommand{{
		Arguments: []string{"clang-6.0", "-O2", "hello.c", "-o", "hello.o"},
		Directory: cwd,
		Output:    "hello.o",
		File:      "hello.c",
	}}

	if !reflect.DeepEqual(commands, expected) {
		t.Errorf("expected %+v to equal %+v", commands, expected)
	}

}
func TestConfigs(t *testing.T) {
	fuzzers := viper.Sub("fuzzers")
	var fuzzerConfigs map[string]interface{}
	err := fuzzers.Unmarshal(&fuzzerConfigs)
	if err != nil {
		t.Fatal(err)
	}

	for fuzzer := range fuzzerConfigs {
		fuzzerCompiler := viper.GetString("fuzzers." + fuzzer + ".replace_command")
		fuzzerDefaultArgs := viper.GetStringSlice("fuzzers." + fuzzer + ".add_arguments")
		commands, err := readCompilationDbForExec(nil, "--create_compiler_db", "--fuzzer", fuzzer, "make")
		if err != nil {
			t.Fatal(err)
		}

		expected := []types.CompilationCommand{{
			Arguments: append([]string{fuzzerCompiler, "hello.c", "-o", "hello.o"}, fuzzerDefaultArgs...),
			Directory: cwd,
			Output:    "hello.o",
			File:      "hello.c",
		}}

		if !reflect.DeepEqual(commands, expected) {
			t.Errorf("expected %+v to equal %+v", commands, expected)
		}
	}
}

func TestMain(m *testing.M) {
	base, err := bazel.RunfilesPath()
	if err != nil {
		log.Fatal(err)
		os.Exit(-1)
	}

	// read the default fuzzer configuration
	viper.SetConfigFile(path.Join(base, "code_intelligence", "build_system", "config", "config.yaml"))
	err = viper.ReadInConfig()
	if err != nil {
		panic(fmt.Errorf("Fatal error config file: %s \n", err))
	}

	var ok bool
	interceptPath, ok = bazel.FindBinary("../intercept", "intercept")
	if !ok {
		log.Fatal("intercept not found")
		os.Exit(-1)
	}
	interceptPath, _ = filepath.Abs(interceptPath)

	cwd = path.Join(base, "code_intelligence/build_system/test/data/make")
	dbFilePath = path.Join(cwd, "compile_commands.json")

	retCode := m.Run()
	os.Exit(retCode)
}
