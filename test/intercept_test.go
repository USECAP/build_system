package test

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"os/exec"
	"path"
	"path/filepath"
	"reflect"
	"strings"
	"testing"

	"github.com/bazelbuild/rules_go/go/tools/bazel"
	"github.com/spf13/viper"
	"gitlab.com/code-intelligence/core/build_system/types"
)

func readCompilationDb() (commands []types.CompilationCommand) {
	dbFilePath := path.Join(getWorkDir(), "compile_commands.json")
	db, err := ioutil.ReadFile(dbFilePath)
	if err != nil {
		log.Fatalf("Failed to read compilation db %q: %q", dbFilePath, err)
	}
	defer os.Remove(dbFilePath)
	err = json.Unmarshal(db, &commands)
	return
}

func runIntercept(env []string, args ...string) {
	cmd := exec.Command(getInterceptPath(), args...)
	cmd.Dir = getWorkDir()
	cmd.Env = env
	out, err := cmd.CombinedOutput()
	if err != nil {
		log.Fatalf("exec failed, error: %s, stderr: %s, stdout: %s", err, cmd.Stderr, string(out))
	}
}

func getInterceptPath() string {
	var ok bool
	interceptPath, ok := bazel.FindBinary("../intercept", "intercept")
	if !ok {
		log.Fatal("intercept not found")
		os.Exit(-1)
	}
	interceptPath, _ = filepath.Abs(interceptPath)
	return interceptPath
}

func getWorkDir() string {
	base, err := bazel.RunfilesPath()
	if err != nil {
		log.Fatal(err)
	}

	return path.Join(base, "code_intelligence/build_system/test/data/make")
}

func TestInterceptor(t *testing.T) {
	buildSh, err := bazel.Runfile("code_intelligence/build_system/test/data/build_sh/build.sh")
	if err != nil {
		t.Fatal(err)
	}

	env := append(os.Environ(), "CC=echo")
	testCommands := []string{"make", buildSh}
	for _, cmd := range testCommands {
		runIntercept(env, "--create_compiler_db", cmd)

		commands := readCompilationDb()

		expected := []types.CompilationCommand{{
			Arguments: []string{"echo", "-O2", "hello.c", "-o", "hello.o"},
			Directory: getWorkDir(),
			Output:    "hello.o",
			File:      "hello.c",
		}}

		assertCommandsAreEqual(t, commands, expected)
	}
}

func TestInterceptorBashScript(t *testing.T) {
	env := append(os.Environ(), "CC=echo")
	runIntercept(env, "--create_compiler_db", "./build.sh")

	commands := readCompilationDb()

	expected := []types.CompilationCommand{{
		Arguments: []string{"echo", "-O2", "hello.c", "-o", "hello.o"},
		Directory: getWorkDir(),
		Output:    "hello.o",
		File:      "hello.c",
	}}

	assertCommandsAreEqual(t, commands, expected)
}

func TestMatchCommand(t *testing.T) {
	runIntercept(nil, "--create_compiler_db", "--match_cc", "clang", "--replace_cc", "afl-clang", "make")
	commands := readCompilationDb()

	expected := []types.CompilationCommand{{
		Arguments: []string{"afl-clang", "-O2", "hello.c", "-o", "hello.o"},
		Directory: getWorkDir(),
		Output:    "hello.o",
		File:      "hello.c",
	}}

	assertCommandsAreEqual(t, commands, expected)
}

func TestConfigs(t *testing.T) {
	fuzzers := viper.Sub("fuzzers")
	var fuzzerConfigs map[string]interface{}
	err := fuzzers.Unmarshal(&fuzzerConfigs)
	if err != nil {
		t.Fatal(err)
	}

	for fuzzer := range fuzzerConfigs {
		for _, cc := range []string{"cc", "cxx"} {
			runIntercept(nil, "--create_compiler_db", "--fuzzer", fuzzer, "make", cc)
			commands := readCompilationDb()

			fuzzerCompiler := viper.GetString("fuzzers." + fuzzer + ".replace_" + cc)
			fuzzerDefaultArgs := viper.GetStringSlice("fuzzers." + fuzzer + ".add_arguments")
			expected := []types.CompilationCommand{{
				Arguments: append(
					[]string{fuzzerCompiler, "hello.c", "-o", "hello.o"},
					fuzzerDefaultArgs...),
				Directory: getWorkDir(),
				Output:    "hello.o",
				File:      "hello.c",
			}}

			assertCommandsAreEqual(t, commands, expected)
		}
	}
}

func assertCommandsAreEqual(t *testing.T, actual, expected []types.CompilationCommand) {
	t.Helper()
	if !reflect.DeepEqual(actual, expected) {
		t.Errorf("Commands did not match.\nexp: %+v\ngot: %+v", expected, actual)
	}
}

func TestMain(m *testing.M) {
	base, err := bazel.RunfilesPath()
	if err != nil {
		log.Fatal(err)
	}

	if err := useShippedCompiler(); err != nil {
		log.Fatal(err)
	}

	// read the default fuzzer configuration
	viper.SetConfigFile(path.Join(base, "code_intelligence", "build_system", "config", "config.yaml"))
	err = viper.ReadInConfig()
	if err != nil {
		panic(fmt.Errorf("Fatal error config file: %s \n", err))
	}

	retCode := m.Run()
	os.Exit(retCode)
}

func useShippedCompiler() (err error) {
	clang, err := bazel.Runfile(filepath.Join("llvm", "bin", "clang"))
	if err != nil {
		return fmt.Errorf("clang not found: %v", err)
	}
	clangDir := path.Dir(clang)

	aflBinPath, err := bazel.Runfile("afl/bin/afl-clang")
	if err != nil {
		return fmt.Errorf("afl-clang not found: %v", err)
	}
	aflBinDir := path.Dir(aflBinPath)

	paths := []string{clangDir, aflBinDir, os.Getenv("PATH")}
	os.Setenv("PATH", strings.Join(paths, string(os.PathListSeparator)))

	aflLibPath, err := bazel.Runfile("afl/lib/afl/as")
	if err != nil {
		return fmt.Errorf("afl/as not found: %v", err)
	}
	aflLibDir := path.Dir(aflLibPath)
	os.Setenv("AFL_PATH", aflLibDir)
	return
}
