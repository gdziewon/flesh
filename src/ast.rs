#[derive(Debug, Clone, PartialEq, Eq)]
pub enum Command {
    Simple(String, Vec<Argument>),
    Pipe(Box<Command>, Box<Command>),
    Redirect(Box<Command>, Redirection),
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub enum Argument {
    Literal(String),
    Quoted(String),
    Variable(String),
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub enum Redirection {
    Input(Argument),
    Output(Argument),
    Append(Argument),
}
