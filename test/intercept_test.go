package test

import (
	"encoding/json"
	"io/ioutil"
	"os"
	"os/exec"
	"path"
	"path/filepath"
	"reflect"
	"testing"

	"github.com/bazelbuild/rules_go/go/tools/bazel"
	"gitlab.com/code-intelligence/core/build_system/types"
)

func TestInterceptor(t *testing.T) {
	interceptPath, ok := bazel.FindBinary("../intercept", "intercept")
	if !ok {
		t.Fatal("intercept not found")
	}
	interceptPath, _ = filepath.Abs(interceptPath)
	t.Log("intercept: ", interceptPath)

	base, err := bazel.RunfilesPath()
	if err != nil {
		t.Fatal(err)
	}

	cwd := path.Join(base, "code_intelligence/build_system/test/data/make")
	t.Log("cwd: ", cwd)

	dbFilePath := path.Join(cwd, "compile_commands.json")

	env := os.Environ()
	env = append(env, "CC=echo")
	cmd := exec.Command(interceptPath, "-create_compiler_db", "make")
	cmd.Env = env
	cmd.Dir = cwd
	out, err := cmd.CombinedOutput()
	t.Log("out: ", string(out))
	if err != nil {
		t.Fatal("process returned non-zero exit status: ", err)
	}

	// expect that dbFilePath exists
	db, err := ioutil.ReadFile(dbFilePath)
	if err != nil {
		t.Fatal(err)
	}
	var commands []types.CompilationCommand
	err = json.Unmarshal(db, &commands)
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
