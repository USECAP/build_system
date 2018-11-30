package config

import (
	"fmt"

	"os"

	"log"

	"github.com/spf13/viper"
	"gitlab.com/code-intelligence/core/build_system/proto"
)

// InterceptSettings returns the settings for the interception config.
func InterceptSettings() *proto.InterceptSettings {
	var (
		replaceCC  = viper.GetString("replace_cc")
		replaceCXX = viper.GetString("replace_cxx")
		addArgs    []string
		removeArgs []string
	)

	// if the fuzzer argument is set, set new defaults from config file with
	// that fuzzer name; if not found ignore.
	fuzzerName := viper.GetString("fuzzer")
	if cfg, err := fuzzerConfig(fuzzerName); err != nil {
		if fuzzerName != "" {
			log.Printf("Warning: Ignoring unknown fuzzer %q", fuzzerName)
		}
	} else {
		replaceCC = cfg.ReplaceCC
		replaceCXX = cfg.ReplaceCXX
		addArgs = cfg.AddArgs
		removeArgs = cfg.RemoveArgs
		if sanName := viper.GetString("sanitizer"); sanName != "" {
			if sanCfg, err := sanitizer(cfg, sanName); err != nil {
				log.Printf("Warning: Ignoring unknown sanitizer %q", sanName)
			} else {
				addArgs = append(addArgs, sanCfg.Flags...)
				for _, e := range sanCfg.SetEnv {
					os.Setenv(e, "1")
				}
				for _, e := range sanCfg.UnsetEnv {
					os.Unsetenv(e)
				}
			}
		}
	}

	return &proto.InterceptSettings{
		MatchingRules: []*proto.MatchingRule{{
			MatchCommand:    viper.GetString("match_cc"),
			ReplaceCommand:  replaceCC,
			AddArguments:    addArgs,
			RemoveArguments: removeArgs,
		}, {
			MatchCommand:    viper.GetString("match_cxx"),
			ReplaceCommand:  replaceCXX,
			AddArguments:    addArgs,
			RemoveArguments: removeArgs,
		}},
	}
}

func sanitizer(cfg *fuzzerCfg, sanitizer string) (sanitizerCfg, error) {
	sanCfg, found := cfg.Sanitizers[sanitizer]
	if !found {
		return sanitizerCfg{}, fmt.Errorf("sanitizer %q not found", sanitizer)
	}
	return sanCfg, nil
}
