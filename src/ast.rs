use std::process::{Child, Command as Process, Stdio};

pub struct Script(pub Vec<Command>);

#[derive(Debug, Clone, PartialEq, Eq)]
pub enum Command {
    Simple(SimpleCommand),
    Pipe(Box<Command>, Box<Command>),
    Sequence(Vec<Box<Command>>),
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct SimpleCommand {
    pub executable: String,
    pub args: Vec<String>,
    pub redirs: Vec<Redirection>,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub enum Argument {
    Literal(String),
    Variable(String),
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub enum Redirection {
    Input(Argument),
    Output(Argument),
    Append(Argument),
}

impl SimpleCommand {
    pub fn new(executable: String, args: Vec<Argument>, redirs: Vec<Redirection>) -> Self {
        SimpleCommand {
            executable,
            args: args.into_iter().map(|s| s.resolve()).collect(),
            redirs,
        }
    }

    pub fn spawn(self, stdin: Option<Stdio>, stdout: Option<Stdio>) -> Child {
        let mut cmd = Process::new(self.executable);
        let process = cmd.args(self.args);
        if let Some(stdin) = stdin {
            process.stdin(stdin);
        }
        if let Some(stdout) = stdout {
            process.stdout(stdout);
        }
        process.spawn().expect("Failed to execute command")
    }
}

impl Argument {
    pub fn resolve(self) -> String {
        match self {
            Argument::Literal(s) => s,
            Argument::Variable(s) => s,
        }
    }
}
