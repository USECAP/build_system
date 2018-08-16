package test

import (
	"encoding/json"
	"io/ioutil"
	"os"
	"os/exec"
	"path"
	"reflect"
	"testing"

	"github.com/bazelbuild/rules_go/go/tools/bazel"
	"gitlab.com/code-intelligence/core/build_system/types"
	"log"
	"path/filepath"
	"fmt"
)

var (
	interceptPath string
	cwd string
	dbFilePath string
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
		Arguments: []string{"echo", "hello.c", "-o", "hello.o"},
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
		Arguments: []string{"clang-6.0", "hello.c", "-o", "hello.o"},
		Directory: cwd,
		Output:    "hello.o",
		File:      "hello.c",
	}}

	if !reflect.DeepEqual(commands, expected) {
		t.Errorf("expected %+v to equal %+v", commands, expected)
	}

}
func TestConfig(t *testing.T) {
	commands, err := readCompilationDbForExec(nil,"--create_compiler_db", "--fuzzer", "libfuzzer", "make")
	if err != nil {
		t.Fatal(err)
	}

	expected := []types.CompilationCommand{{
		Arguments: []string{"clang-6.0", "hello.c", "-o", "hello.o", "-fsanitize=fuzzer-no-link", "-O1"},
		Directory: cwd,
		Output:    "hello.o",
		File:      "hello.c",
	}}

	if !reflect.DeepEqual(commands, expected) {
		t.Errorf("expected %+v to equal %+v", commands, expected)
	}
}

func TestMain(m *testing.M) {
	var ok bool
	log.Println("cwd: ", cwd)
	interceptPath, ok = bazel.FindBinary("../intercept", "intercept")
	if !ok {
		log.Fatal("intercept not found")
		os.Exit(-1)
	}
	interceptPath, _ = filepath.Abs(interceptPath)

	base, err := bazel.RunfilesPath()
	if err != nil {
		log.Fatal(err)
		os.Exit(-1)
	}
	cwd = path.Join(base, "code_intelligence/build_system/test/data/make")
	dbFilePath = path.Join(cwd, "compile_commands.json")

	retCode := m.Run()
	os.Exit(retCode)
}
