package main

import (
	"gitlab.com/code-intelligence/core/build_system/proto"
	"path"
)

var inputSuffixes = map[string]struct{}{
	".c":   {},
	".i":   {},
	".ii":  {},
	".m":   {},
	".mi":  {},
	".mm":  {},
	".mii": {},
	".C":   {},
	".cc":  {},
	".CC":  {},
	".cp":  {},
	".cpp": {},
	".cxx": {},
	".c++": {},
	".C++": {},
	".txx": {},
	".s":   {},
	".S":   {},
	".sx":  {},
	".asm": {},
}

func commandLineFromInterceptedCommand(cmd *proto.InterceptedCommand) *commandLine {
	result := new(commandLine)
	outputFollows := false
	for _, arg := range cmd.ReplacedArguments {
		if arg == "-o" {
			outputFollows = true
			continue
		}
		if outputFollows {
			outputFollows = false
			result.outputFile = arg
			continue
		}
		if _, found := inputSuffixes[path.Ext(arg)]; found {
			result.inputFiles = append(result.inputFiles, arg)
		}
	}

	return result
}

type commandLine struct {
	inputFiles []string
	outputFile string
}