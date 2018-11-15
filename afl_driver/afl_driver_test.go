package afl_driver

import (
	"log"
	"os"
	"os/exec"
	"path"
	"path/filepath"
	"testing"

	"github.com/bazelbuild/rules_go/go/tools/bazel"
	"gitlab.com/code-intelligence/core/utils/pathutils"
)

var (
	testdataPath string
)

// performs make clean and make in the testdata directory, building the files
// for the tests
func buildTestdata() (out []byte, err error) {
	aflPath, err := pathutils.Find("code_intelligence/external/afl/lib/afl")
	if err != nil {
		log.Fatalf("Could not find AFL path: %v", err)
	}

	cmd := exec.Command("make", "clean")
	cmd.Dir = testdataPath
	err = cmd.Run()

	cmd = exec.Command("make")
	cmd.Env = append(os.Environ(), "AFL_PATH=" + aflPath)
	cmd.Dir = testdataPath
	out, err = cmd.CombinedOutput()
	return
}

// simply runs the target to see if the binary itself works
func TestAflDriver(t *testing.T) {
	testData, err := os.Open(path.Join(testdataPath, "test.data"))
	if err != nil {
		log.Fatal("test.data could not be opened: ", err)
	}
	cmd := exec.Command(path.Join(testdataPath, "fuzzer"))
	cmd.Stdin = testData

	err = cmd.Run()
	if err != nil {
		log.Fatal("Running the fuzzer failed: ", err)
	}
}

func TestMain(m *testing.M) {
	_, err := bazel.RunfilesPath()
	if err != nil {
		log.Fatal(err)
		os.Exit(-1)
	}

	runfilePath, err := bazel.Runfile("testdata")
	testdataPath, err = filepath.Abs(runfilePath)
	log.Print(testdataPath)
	if err != nil {
		log.Fatal("testdata not found")
		os.Exit(-1)
	}

	out, err := buildTestdata()
	if err != nil {
		log.Fatalf("exec failed: error: %s, stdout: %s", err, string(out))
		os.Exit(-1)
	}

	retCode := m.Run()
	os.Exit(retCode)
}
