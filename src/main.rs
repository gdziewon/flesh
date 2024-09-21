mod parser {
    include!(concat!(env!("OUT_DIR"), "/parser/flesh.rs"));
}

mod ast;
mod executor;
use executor::execute_script;
use std::io::{self, Write};

fn main() {
    println!("Welcome to flesh shell!");

    loop {
        print!("flesh> ");
        io::stdout().flush().unwrap();

        let mut input = String::new();
        if io::stdin().read_line(&mut input).is_err() {
            break;
        }

        let input = input.trim();
        if input.is_empty() {
            continue;
        }

        match parser::CommandsParser::new().parse(input) {
            Ok(command) => execute_script(command),
            Err(lalrpop_util::ParseError::UnrecognizedToken { token, expected }) => {
                eprintln!("Syntax error at {:?}, expected {:?}", token, expected);
            }
            Err(e) => {
                eprintln!("Error parsing input: {}", e);
            }
        };
    }
}
