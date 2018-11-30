package config

import (
	"fmt"

	"log"

	"path/filepath"

	"github.com/spf13/pflag"
	"github.com/spf13/viper"
	"gitlab.com/code-intelligence/core/utils/pathutils"
)

const (
	// ServerAddr is the address for the server to listen on.
	ServerAddr        = "localhost:6774"
	CompilerDbPath    = "compile_commands.json"
	CompilationDbFlag = "create_compiler_db"
	ccMatchCommand    = `^([^-]*-)*[mg]cc(-\d+(\.\d+){0,2})?$|` +
		`^([^-]*-)*clang(-\d+(\.\d+){0,2})?$|` +
		`^(|i)cc$|^(g|)xlc$`
	cxxMatchCommand = `^([^-]*-)*[cmg]\+\+(-\d+(\.\d+){0,2})?$|` +
		`^([^-]*-)*clang\+\+(-\d+(\.\d+){0,2})?$|` +
		`^(|i)cc$|^(g|)xlc$`
)

type fuzzerCfg struct {
	ReplaceCC  string   `mapstructure:"replace_cc"`
	ReplaceCXX string   `mapstructure:"replace_cxx"`
	RemoveArgs []string `mapstructure:"remove_arguments"`
	AddArgs    []string `mapstructure:"add_arguments"`
	Sanitizers sanitizerCfgs
}

type sanitizerCfgs map[string]sanitizerCfg

type sanitizerCfg struct {
	Flags    []string
	SetEnv   []string
	UnsetEnv []string
}

type stringMap map[string]interface{}

func init() {
	setDefaultValues()
	viper.BindEnv("replace_cc", "CC")
	viper.BindEnv("replace_cxx", "CXX")
	viper.SetEnvPrefix("CI")
	viper.AutomaticEnv()
}

func shippedToolPath(runfilePath string) string {
	fallback := filepath.Base(runfilePath)
	pth, err := pathutils.FindBinary(runfilePath)
	if err != nil {
		log.Printf("Warning: could not find shipped %s, using fallback %q: %v",
			runfilePath, fallback, err)
		return fallback
	}
	return pth
}

func setDefaultValues() {
	ccPath := shippedToolPath("llvm/bin/clang")
	cxxPath := shippedToolPath("llvm/bin/clang++")
	aflCCPath := shippedToolPath("afl/bin/afl-clang")
	aflCXXPath := shippedToolPath("afl/bin/afl-clang++")

	var (
		fuzzerDefaults = map[string][]string{
			"remove_arguments": {"-O1", "-O2", "-O3", "-O4", "-Ofast", "-Os", "-Oz", "-Og"},
			"add_arguments":    {"-g", "-gline-tables-only", "-DFUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION"},
		}
		libfuzzerDefaults = map[string]interface{}{
			"replace_cc":       ccPath,
			"replace_cxx":      cxxPath,
			"remove_arguments": []string{"-fomit-frame-pointer"},
			"add_arguments":    []string{"-fno-omit-frame-pointer", "-fsanitize=fuzzer-no-link", "-Og"},
			"sanitizers": sanitizerCfgs{
				"address": {Flags: []string{"-fsanitize=address,undefined", "-fsanitize-address-use-after-scope"}},
				"memory":  {Flags: []string{"-fsanitize=memory,undefined"}},
				"thread":  {Flags: []string{"-fsanitize=thread,undefined"}},
			},
		}
		aflDefaults = map[string]interface{}{
			"replace_cc":       aflCCPath,
			"replace_cxx":      aflCXXPath,
			"add_arguments":    []string{"-O0"},
			"remove_arguments": []string{},
			"sanitizers": sanitizerCfgs{
				"address": {SetEnv: []string{"AFL_USE_ASAN"}, UnsetEnv: []string{"AFL_USE_MSAN"}},
				"memory":  {SetEnv: []string{"AFL_USE_MSAN"}, UnsetEnv: []string{"AFL_USE_ASAN"}},
				"thread":  {SetEnv: []string{}, UnsetEnv: []string{"AFL_USE_MSAN", "AFL_USE_ASAN"}},
			},
		}
		llvmCovDefaults = map[string]interface{}{
			"replace_cc":       ccPath,
			"replace_cxx":      cxxPath,
			"remove_arguments": []string{"-fomit-frame-pointer"},
			"add_arguments":    []string{"-fno-omit-frame-pointer", "-fprofile-instr-generate", "-fcoverage-mapping", "-O0"},
			"sanitizers":       sanitizerCfgs{},
		}
	)

	viper.SetDefault("fuzzers", map[string]stringMap{
		"libfuzzer": merge(fuzzerDefaults, libfuzzerDefaults),
		"afl":       merge(fuzzerDefaults, aflDefaults),
		"llvm-cov":  merge(fuzzerDefaults, llvmCovDefaults),
	})
	viper.SetDefault("match_cc", ccMatchCommand)
	viper.SetDefault("match_cxx", cxxMatchCommand)
	viper.SetDefault("replace_cc", ccPath)
	viper.SetDefault("replace_cxx", cxxPath)
	viper.SetDefault("sanitizer", "address")

	pflag.Bool(CompilationDbFlag, false, "Whether to create compilation database")
	pflag.String("match_cc", "", "Override default cc match command")
	pflag.String("match_cxx", "", "Override default cxx match command")
	pflag.String("replace_cc", "", "The command to replace the C compiler with")
	pflag.String("replace_cxx", "", "The command to replace the C++ compiler with")
	pflag.String("fuzzer", "", "Whether a specific fuzzer config should be used")
	pflag.String("sanitizer", "", "Whether a specific sanitizer config should be used")
}

// Merge defaults into the config.
func merge(defaults map[string][]string, config stringMap) (merged stringMap) {
	merged = make(stringMap)
	for k, v := range config {
		// copy from config first
		merged[k] = v
		// merge in defaults if default exists
		if defaultList, ok := defaults[k]; ok {
			if list, ok := v.([]string); ok {
				merged[k] = append(list, defaultList...)
			}
		}
	}
	return
}

// Get the config for the fuzzer.
func fuzzerConfig(fuzzer string) (*fuzzerCfg, error) {
	fuzzers := make(map[string]fuzzerCfg)
	if err := viper.UnmarshalKey("fuzzers", &fuzzers); err != nil {
		return nil, err
	}
	cfg, ok := fuzzers[fuzzer]
	if !ok {
		return nil, fmt.Errorf("config for fuzzer %q not found", fuzzer)
	}
	return &cfg, nil
}
