package build_system

type CompilationCommand struct {
	Arguments []string `json:"arguments"`
	Directory string   `json:"directory"`
	Output    string   `json:"output"`
	File      string   `json:"file"`
}
