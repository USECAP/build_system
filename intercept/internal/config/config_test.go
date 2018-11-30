package config

import (
	"os"
	"testing"

	"reflect"

	"github.com/spf13/viper"
	"path/filepath"
)

var (
	settingsTestCases = []struct {
		engine     string
		sanitizer  string
		replaceCmd string
		addArgs    []string
	}{{
		engine:     "afl",
		sanitizer:  "some-unsupported-sanitizer",
		replaceCmd: "afl-clang",
		addArgs: []string{
			"-O0", "-g", "-gline-tables-only",
			"-DFUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION"},
	}, {
		engine:     "libfuzzer",
		sanitizer:  "", // defaults to address
		replaceCmd: "clang",
		addArgs: []string{
			"-fno-omit-frame-pointer", "-fsanitize=fuzzer-no-link", "-Og", "-g",
			"-gline-tables-only", "-DFUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION",
			"-fsanitize=address,undefined", "-fsanitize-address-use-after-scope"},
	}, {
		engine:     "libfuzzer",
		sanitizer:  "memory",
		replaceCmd: "clang",
		addArgs: []string{
			"-fno-omit-frame-pointer", "-fsanitize=fuzzer-no-link", "-Og", "-g",
			"-gline-tables-only", "-DFUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION",
			"-fsanitize=memory,undefined"},
	}, {
		engine:     "libfuzzer",
		sanitizer:  "thread",
		replaceCmd: "clang",
		addArgs: []string{
			"-fno-omit-frame-pointer", "-fsanitize=fuzzer-no-link", "-Og", "-g",
			"-gline-tables-only", "-DFUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION",
			"-fsanitize=thread,undefined"},
	}}

	aflEnvVarsTestCases = []struct {
		sanitizer string
		setEnvs   []string
		unsetEnvs []string
	}{{
		sanitizer: "", // default to address
		setEnvs:   []string{"AFL_USE_ASAN"},
		unsetEnvs: []string{"AFL_USE_MSAN"},
	}, {
		sanitizer: "memory",
		setEnvs:   []string{"AFL_USE_MSAN"},
		unsetEnvs: []string{"AFL_USE_ASAN"},
	}}
)

func TestGetSettings(t *testing.T) {
	for _, tc := range settingsTestCases {
		t.Run(tc.engine+"/"+tc.sanitizer, func(t *testing.T) {
			os.Setenv("CI_FUZZER", tc.engine)
			os.Setenv("CI_SANITIZER", tc.sanitizer)

			settings := InterceptSettings()

			// Check replace command for C compiler
			replaceCmd := settings.MatchingRules[0].ReplaceCommand
			if filepath.Base(replaceCmd) != tc.replaceCmd {
				t.Errorf("got %q, want %q", replaceCmd, tc.replaceCmd)
			}

			// Check added arguments for all matching rules
			for _, rule := range settings.MatchingRules {
				if !reflect.DeepEqual(rule.AddArguments, tc.addArgs) {
					t.Errorf("\ngot  %+v,\nwant %+v", rule.AddArguments, tc.addArgs)
				}
			}
		})
	}
}

func TestGetSettingsSetsAFLEnvVarsForSanitizers(t *testing.T) {
	os.Setenv("CI_FUZZER", "afl")
	for _, tc := range aflEnvVarsTestCases {
		t.Run(tc.sanitizer, func(t *testing.T) {
			os.Setenv("CI_SANITIZER", tc.sanitizer)

			// This should set the env vars!
			InterceptSettings()

			for _, e := range tc.setEnvs {
				if _, ok := os.LookupEnv(e); !ok {
					t.Errorf("%q should have been set", e)
				}
			}
			for _, e := range tc.unsetEnvs {
				if _, ok := os.LookupEnv(e); ok {
					t.Errorf("%q should not have been set", e)
				}
			}
		})
	}
}

func TestGetReplaceCC(t *testing.T) {
	os.Setenv("CC", "my-shiny-compiler")
	if viper.GetString("replace_cc") != "my-shiny-compiler" {
		t.Errorf("got replace_cc: %q", viper.GetString("replace_cc"))
	}
}
