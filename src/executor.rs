use crate::ast::{Argument, Command, Redirection};
use std::process::Command as ProcessCommand;

pub fn execute_command(command: Command) {
    match command {
        Command::Simple(cmd, args) => {
            let mut child = ProcessCommand::new(cmd)
                .args(
                    args.iter()
                        .map(|arg| match arg {
                            Argument::Literal(s) => s.clone(),
                            Argument::Quoted(s) => s.clone(),
                            _ => String::new(),
                        })
                        .collect::<Vec<String>>(),
                )
                .spawn()
                .expect("Failed to execute command");
            child.wait().expect("Failed to wait on child");
        }
        Command::Pipe(cmd1, cmd2) => {}
        Command::Redirect(cmd, redir) => {
            execute_command(*cmd);
            let dir = match redir {
                Redirection::Input(s) => s,
                Redirection::Output(s) => s,
                Redirection::Append(s) => s,
            };
            let val = match dir {
                Argument::Quoted(s) => s,
                Argument::Literal(s) => s,
                Argument::Variable(s) => s,
            };
            println!("Redir: {val}");
        }
    }
}
