use crate::ast::{Command, Redirection, Script};
use std::fs::{File, OpenOptions};
use std::process::{Child, Stdio};

pub fn execute_script(script: Script) {
    for cmd in script.0 {
        execute(cmd, None, None);
    }
}

pub fn execute(
    command: Command,
    mut stdin: Option<Stdio>,
    mut stdout: Option<Stdio>,
) -> Option<Child> {
    match command {
        Command::Simple(cmd) => {
            for redir in &cmd.redirs {
                match redir {
                    Redirection::Input(_) => stdin = Some(get_stdio(redir.clone())),
                    _ => stdout = Some(get_stdio(redir.clone())),
                }
            }
            let mut child = cmd.spawn(stdin, stdout);
            child.wait().expect("Failed to wait on child");
            Some(child)
        }
        Command::Pipe(cmd1, cmd2) => {
            let child = execute(*cmd1, stdin, Some(Stdio::piped()));
            let stdin = {
                if let Some(mut child) = child {
                    Some(Stdio::from(
                        child.stdout.take().expect("Failed to capture output"),
                    ))
                } else {
                    None
                }
            };
            execute(*cmd2, stdin, stdout)
        }
        _ => None,
    }
}

fn get_stdio(redir: Redirection) -> Stdio {
    let file = match redir {
        Redirection::Input(arg) => File::open(arg.resolve()).expect("Failed to open input file"),
        Redirection::Output(arg) => {
            File::create(arg.resolve()).expect("Failed to create output file")
        }
        Redirection::Append(arg) => OpenOptions::new()
            .append(true)
            .open(arg.resolve())
            .expect("Failed to open file in append mode"),
    };
    Stdio::from(file)
}
